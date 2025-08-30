#include "netclient.h"

#include <QUuid>
#include <QNetworkReply>

#include "../model/data.h"
#include "../model/datacenter.h"
#include "../common/toast.h"

namespace network{


NetClient::NetClient(model::DataCenter *dataCenter)
    : dataCenter(dataCenter)
{
    // initWebsocket(); //在MainWidget中进行连接
}

void NetClient::ping()
{
    QNetworkRequest httpReq;
    httpReq.setUrl(QUrl(HTTP_URL + "/ping"));


    // 使用 get/post 发送请求的时候，只要执行到 get/post
    // 此时函数就已经执行了，立即把网络数据发送出去了
    // 但是，响应回来还得需要时间
    // 因此此时得到的httpRsp不一定是 有效对象
    QNetworkReply* httpRsp = httpClient.get(httpReq);
    connect(httpRsp, &QNetworkReply::finished, this,[=](){
        if(httpRsp->error() != QNetworkReply::NoError){
            LOG() << "http 请求失败!"<<httpRsp->errorString();
            httpRsp->deleteLater();
            return;
        }
        //获取到响应的body
        QByteArray body = httpRsp->readAll();
        LOG() <<"响应内容: " <<body;
        httpRsp->deleteLater();
    });


}

// 网络上传输的数据, 都是通过pb序列化得到的数据 => 二进制数据

////////////////////////////////////////////////
/// 以下的操作都是与 Websocket 相关的
////////////////////////////////////////////////

void NetClient::initWebsocket()
{
    /*
     * 编写websocket代码，一般都是要处理这几个环节的：
     * 1）建立连接后，connected信号
     * 2）断开连接后，disconnected信号
     * 3）连接出错时，errorOccurred信号
     * 4）收到文本消息，textMessageReceived信号
     * 5）二进制消息，binaryMessageReceived信号
    */

    // 1. 准备好所有需要的信号槽
    connect(&websocketClient, &QWebSocket::connected, this, [=](){
        LOG() <<"[websocket连接] 连接成功";

        // 连接成功后，发送身份认证
        sendAuth();
    });
    connect(&websocketClient, &QWebSocket::disconnected, this, [=](){
        LOG()<<"[websocket连接] 连接断开";
    });
    connect(&websocketClient, &QWebSocket::errorOccurred, this, [=](QAbstractSocket::SocketError error){
        LOG()<<"[websocket连接] 连接出错" << error;
    });
    connect(&websocketClient, &QWebSocket::textMessageReceived, this, [=](const QString& message){
        LOG()<<"[websocket连接] 收到文本消息: "<< message;
    });
    connect(&websocketClient, &QWebSocket::binaryMessageReceived, this, [=](const QByteArray& byteArray){
        LOG()<<"[websocket连接] 收到二进制消息, 长度为: "<< byteArray.length();

        stark_im::NotifyMessage notifyMessage;
        notifyMessage.deserialize(&serializer, byteArray);
        handleWsResponse(notifyMessage);
    });

    // 2. 和服务器真正建立连接
    websocketClient.open(WEBSOCKET_URL);

}

void NetClient::handleWsResponse(const stark_im::NotifyMessage &notifyMessage)
{
    using stark_im::NotifyTypeGadget::NotifyType;
    NotifyType t = notifyMessage.notifyType();

    if(t == NotifyType::CHAT_MESSAGE_NOTIFY){
        // 收到好友消息通知
        // 1. 把pb中的MessageInfo转成客户端自己的Message
        model::Message message;
        message.load(
            notifyMessage.newMessageInfo().messageInfo() );

        // 2. 针对自己的message进行进一步的处理
        handleWsMessage(message);
    }else if(t == NotifyType::CHAT_SESSION_CREATE_NOTIFY){
        // 创建群聊会话通知
        model::ChatSessionInfo chatSessionInfo;
        chatSessionInfo.load(notifyMessage.newChatSessionInfo().chatSessionInfo());
        handleWsChatSessionCreate(chatSessionInfo);
    }else if(t == NotifyType::FRIEND_ADD_APPLY_NOTIFY){
        // 添加好友申请通知
        model::UserInfo userInfo;
        userInfo.load(notifyMessage.friendAddApply().userInfo());
        handleWsAddFriendApply(userInfo);
    }else if(t == NotifyType::FRIEND_ADD_PROCESS_NOTIFY){
        // 添加好友处理通知
        model::UserInfo userInfo;
        userInfo.load(notifyMessage.friendProcessResult().userInfo());
        bool agree = notifyMessage.friendProcessResult().agree();
        handleWsAddFriendProcess(userInfo, agree);
    }else if(t == NotifyType::FRIEND_REMOVE_NOTIFY){
        // 删除好友通知
        const QString& userId = notifyMessage.friendRemove().userId();
        handleWsRemoveFriend(userId);
    }

}

void NetClient::handleWsMessage(const model::Message &message)
{
    // 这里要考虑两个情况
    QList<model::Message>* messageList = dataCenter->getRecentMessageList(message.chatSessionId);
    if(messageList == nullptr){
        // 1.如果当前这个消息所属的会话, 里面的消息列表, 没有在本地加载, 此时需要通过网络先加载整个消息列表
        connect(dataCenter, &model::DataCenter::getRecentMessageListDoneNoUI,
                this, &NetClient::receiveMessage,
                Qt::UniqueConnection);
        dataCenter->getRecentMessageListAsync(message.chatSessionId, false);

    } else {
        // 2.如果当前这个消息所属的会话，里面的消息已经在本地加载了，直接把这个消息尾插到消息列表即可
        messageList->push_back(message);
        receiveMessage(message.chatSessionId);
    }

    // 1.如果当前这个消息所属的会话, 里面的消息列表, 没有在本地加载
    /*
     * > QHash<QString, QList<Message>>* recentMessages = nullptr;
     * > 这个 hash 中的数据, 都是按需加载的
     * > 点了哪个会话，哪个会话才会加载消息
     * > 没点的会话自然就没有消息列表（对应的键值对在hash上不存在）
     *
     * > 如果客户端收到的消息来自会话1, 由于会话1, 并没有加载最近消息列表,
     * > 因此此时就不太方便在本地直接保存收到的消息(通过websocket收到的Message对象)
     *
     * > 此时, 就需要客户端能够把这个会话对应的消息列表, 整个的都加载到客户端
     * > 另外, 由于ws收到的数据，也是通过服务器转发的：
     *      从服务器拿到的完整消息列表里, 已经包含了新的消息
     *      (新的消息不需要单独存储了，直接从拿到的消息列表中就可以get到了)
     *
     * > 再进一步把这个消息做显示的处理即可
     * > 但是注意：此时不应该把消息显示到消息展示区
     * > 虽然不显示在消息展示区，但是需要在会话列表的"最后一条消息"预览位置，进行显示
     * > 不仅在这里显示消息预览，还要显示"未读消息"数目
     */



    // 2.如果当前这个消息所属的会话，里面的消息已经在本地加载了，直接把这个消息尾插到消息列表即可
    /*
     * > 需要进一步判定，当前选中的会话，是否就是收到消息的会话
     * > --如果是，就把消息显示在消息展示区，同时更新会话列表的消息预览
     * > --如果不是，就只把消息更新在会话列表的消息预览即可，同时新增"未读消息"计数
     */
}

void NetClient::handleWsRemoveFriend(const QString &userId)
{
    // 1.删除数据，DataCenter 好友列表的数据
    dataCenter->removeFriend(userId);
    // 2.通知界面，更新好友列表/会话列表
    emit dataCenter->deleteFriendDone();
}

void NetClient::handleWsAddFriendApply(const model::UserInfo &userInfo)
{
    // 1.DataCenter中有一个好友申请列表
    auto* applyList = dataCenter->getApplyList();
    if(applyList==nullptr){
        LOG() << "客户端尚未加载好友申请列表!";
        return;
    }
    // for(const auto& u: *applyList){
    //     if(u.userId == userInfo.userId){
    //         LOG() <<"已经接受过了该好友的申请";
    //         // 处理方案1. 不再添加好友申请项，将该好友申请top一下即可
    //         // 处理方案2. 允许重复的好友申请项，处理同意按钮时，判断是否处于好友中
    //     }
    // }
    applyList->push_front(userInfo);
    // 2.通知界面更新
    emit dataCenter->receiveFriendApplyDone();
}

void NetClient::handleWsAddFriendProcess(const model::UserInfo &userInfo, bool agree)
{
    // if(agree){
    //     // 对方同意了你的好友申请
    //     auto* friendList= dataCenter->getFriendList();
    //     if(friendList == nullptr){
    //         LOG() <<"客户端尚未加载好友列表";
    //         return;
    //     }
    //     friendList->push_front(userInfo);

    //     // 同时也更新一下界面
    //     emit dataCenter->receiveFriendProcessDone(agree);
    // } else {
    //     // 不同意
    //     emit dataCenter->receiveFriendProcessDone(agree);
    // }

    if(agree){
        // 对方同意了你的好友申请
        auto* friendList= dataCenter->getFriendList();
        if(friendList == nullptr){
            LOG() <<"客户端尚未加载好友列表";
            return;
        }
        friendList->push_front(userInfo);
    }
    // 同不同意都得通知界面
    emit dataCenter->receiveFriendProcessDone(userInfo, agree);
}

void NetClient::handleWsChatSessionCreate(const model::ChatSessionInfo &chatSessionInfo)
{
    // 把新的chatSessionInfo添加到聊天会话列表中即可
    QList<model::ChatSessionInfo>* chatSessionList = dataCenter->getChatSessionList();
    if(chatSessionList==nullptr){
        LOG() <<"[严重错误]客户端尚未加载会话列表";
        return;
    }
    // 新的元素添加到列表头部
    chatSessionList->push_front(chatSessionInfo);
    // 发送信号, 通知界面更新
    emit dataCenter->receiveSessionCreateDone();
}

void NetClient::sendAuth()
{
    // 请求ID, 每个请求，都会随机生成一个唯一的 id，作为请求的身份标识
    // 针对一个请求进行处理时，服务器有很多操作环节
    stark_im::ClientAuthenticationReq req;
    req.setRequestId(makeRequestId());
    req.setSessionId(dataCenter->getLoginSessionId());
    QByteArray body = req.serialize(&serializer);
    websocketClient.sendBinaryMessage(body);

    LOG() << "[WS身份认证] requestId=" << req.requestId()
          << ", loginSessionId=" << req.sessionId();
}





////////////////////////////////////////////////
/// 以下的操作都是与 HTTP 相关的
////////////////////////////////////////////////

QString NetClient::makeRequestId()
{
    // 基本要求: 确保每个请求的id都是不重复的
    // 通过UUID来实现上述效果
    return "Req_" + QUuid::createUuid().toString().sliced(25,12);
}

QNetworkReply *NetClient::sendHttpRequest(const QString &apiPath, const QByteArray &body)
{
    QNetworkRequest httpReq;
    httpReq.setUrl(QUrl(HTTP_URL + apiPath));
    httpReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-protobuf");

    QNetworkReply* httpRsp = httpClient.post(httpReq, body);

    return httpRsp;
}

// 完成具体的网络通信即可
void NetClient::getMyself(const QString &loginSessionId)
{
    // 1. 构造 HTTP 请求 body...
    stark_im::GetUserInfoReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[获取个人信息] 即将发送请求 requestId="<< pbReq.requestId()
          <<", loginSessionId="<<pbReq.sessionId();

    // 2. 构造出 HTTP 请求, 并发送出去
    QNetworkReply* httpRsp = sendHttpRequest("/service/user/get_user_info", body);
    connect(httpRsp, &QNetworkReply::finished, this, [=](){
        // a) 响应已经返回到客户端了
        bool ok = false;
        QString reason;
        auto resp = handleHttpResponse<stark_im::GetUserInfoRsp>(httpRsp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok){
            LOG() << "[获取个人信息] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        // c) 把响应的数据保存到DataCenter中
        dataCenter->resetMyself(resp);

        // d) 通知调用逻辑，响应回来了
        emit dataCenter->getMyselfDone();

        // e) 打印日志
        LOG() << "[获取个人信息] 响应处理完毕 requestId=" << pbReq.requestId();
    });
}

void NetClient::getFriendList(const QString &loginSessionId)
{
    stark_im::GetFriendListReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[获取好友列表] 即将发送请求 requestId="<< pbReq.requestId()
          <<", loginSessionId="<<pbReq.sessionId();

    QNetworkReply* httpRsp = sendHttpRequest("/service/friend/get_friend_list", body);
    connect(httpRsp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto friendListResp = handleHttpResponse<stark_im::GetFriendListRsp>(httpRsp, &ok, &reason);

        if(!ok){
            LOG() << "[获取好友列表] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        dataCenter->resetFriendList(friendListResp);

        emit dataCenter->getFriendListDone();

        LOG() << "[获取好友列表] 响应处理完毕 requestId=" << pbReq.requestId();
    });
}

void NetClient::getChatSessionList(const QString &loginSessionId)
{
    stark_im::GetChatSessionListReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[获取会话列表] 即将发送请求 requestId="<< pbReq.requestId()
          <<", loginSessionId="<<pbReq.sessionId();

    QNetworkReply* httpRsp = sendHttpRequest("/service/friend/get_chat_session_list", body);

    connect(httpRsp, &QNetworkReply::finished,this,[=](){
        bool ok = false;
        QString reason;
        auto chatSessionListResp = handleHttpResponse<stark_im::GetChatSessionListRsp>(httpRsp,&ok,&reason);

        if(!ok){
            LOG() << "[获取会话列表] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        dataCenter->resetChatSessionList(chatSessionListResp);

        emit dataCenter->getChatSessionListDone();

        LOG() << "[获取会话列表] 响应处理完毕 requestId=" << pbReq.requestId();
    });
}

void NetClient::getApplyList(const QString &loginSessionId)
{
    stark_im::GetPendingFriendEventListReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[获取申请列表] 即将发送请求 requestId="<< pbReq.requestId()
          <<", loginSessionId="<<pbReq.sessionId();

    QNetworkReply* httpRsp = sendHttpRequest("/service/friend/get_pending_friend_events", body);

    connect(httpRsp, &QNetworkReply::finished,this,[=](){
        bool ok = false;
        QString reason;
        auto applyListResp = handleHttpResponse<stark_im::GetPendingFriendEventListRsp>(httpRsp,&ok,&reason);

        if(!ok){
            LOG() << "[获取申请列表] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        dataCenter->resetApplyList(applyListResp);

        emit dataCenter->getApplyListDone();

        LOG() << "[获取申请列表] 响应处理完毕 requestId=" << pbReq.requestId();
    });
}

void NetClient::getRecentMessageList(const QString &loginSessionId, const QString &chatSessionId, bool updateUI)
{
    // path: /service/message_storage/get_recent
    // 通过protobuf构造请求的body
    stark_im::GetRecentMsgReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setChatSessionId(chatSessionId);
    pbReq.setMsgCount(50);    //此处固定获取最近50条
    pbReq.setSessionId(loginSessionId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[获取最近消息列表] 即将发送请求 requestId="<< pbReq.requestId()
          << ", loginSessionId="<<pbReq.sessionId()
          << ", chatSessionId="<<pbReq.chatSessionId();

    // 发送http请求
    QNetworkReply* resp = sendHttpRequest("/service/message_storage/get_recent", body);

    // 处理响应
    connect(resp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = handleHttpResponse<stark_im::GetRecentMsgRsp>(resp, &ok, &reason);

        if(!ok){
            LOG() << "[获取最近消息列表] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        dataCenter->resetRecentMessageList(chatSessionId, pbResp);

        if(updateUI) emit dataCenter->getRecentMessageListDone(chatSessionId);
        else emit dataCenter->getRecentMessageListDoneNoUI(chatSessionId);

        LOG() << "[获取最近消息列表] 响应处理完毕 requestId=" << pbReq.requestId();
    });

}

void NetClient::sendMessage(const QString &loginSessionId, const QString &chatSessionId,
                            model::MessageType messageType, const QByteArray &content, const QString& extraInfo)
{
    // 1.通过protobuf构造body
    stark_im::NewMessageReq pbReq;
    pbReq.setRequestId(makeRequestId());
    // req.setUserId(); //服务器内部使用, 这里用不到
    pbReq.setSessionId(loginSessionId);
    pbReq.setChatSessionId(chatSessionId);

    stark_im::MessageContent messageContent;
    if(messageType == model::TEXT_TYPE){
        messageContent.setMessageType(stark_im::MessageTypeGadget::STRING);

        stark_im::StringMessageInfo textMessage;
        textMessage.setContent(content);

        messageContent.setStringMessage(textMessage);
    } else if(messageType == model::IMAGE_TYPE) {
        messageContent.setMessageType(stark_im::MessageTypeGadget::IMAGE);

        stark_im::ImageMessageInfo imageMessage;
        imageMessage.setFileId(""); //文件ID 在服务器存储时生成, 暂时置空
        imageMessage.setImageContent(content);

        messageContent.setImageMessage(imageMessage);
    } else if(messageType == model::FILE_TYPE) {
        messageContent.setMessageType(stark_im::MessageTypeGadget::FILE);

        stark_im::FileMessageInfo fileMessage;
        fileMessage.setFileId("");
        fileMessage.setFileSize(content.size());
        fileMessage.setFileName(extraInfo);
        fileMessage.setFileContents(content);

        messageContent.setFileMessage(fileMessage);
    } else if(messageType == model::SPEECH_TYPE) {
        messageContent.setMessageType(stark_im::MessageTypeGadget::SPEECH);

        stark_im::SpeechMessageInfo speechMessage;
        speechMessage.setFileId("");
        speechMessage.setFileContents(content);

        messageContent.setSpeechMessage(speechMessage);
    } else {
        LOG() << "错误的消息类型! MessageType=" <<messageType;
    }
    pbReq.setMessage( messageContent );

    // 2.序列化
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[发送消息] 即将发送请求 requestId="<< pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", chatSessionId=" << pbReq.chatSessionId()
          << ", messageType=" << messageType;

    // 3.发送Http请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/message_transmit/new_message", body);

    // 4.处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a)针对响应结果进行解析
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::NewMessageRsp>(httpResp, &ok, &reason);

        // b)判断响应是否正确
        if(!ok){
            LOG() << "[发送消息] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        // c)只是需要记录一个 成功or失败, 不需要把内容写入到DataCenter中

        // d)通知调用者，响应处理完毕
        emit dataCenter->sendMessageDone(messageType, content, extraInfo);

        // e)打印日志
        LOG() << "[发送消息] 响应处理完毕 requestId=" << pbReq.requestId();
    });
}

void NetClient::receiveMessage(const QString &chatSessionId)
{
    // 1. 先确定一下, 当前这个收到的消息对应的会话，是否是正在被用户选中的
    //      当前会话: 把消息显示到消息展示区，同时需要更新会话列表中的消息预览
    //      不是当前会话: 只需要更新会话列表中的消息预览, 并且更新未读消息数
    if(chatSessionId == dataCenter->getCurrentChatSessionId()){
        // 收到的消息会话, 就是选中会话
        // a) 在消息展示区, 新增一个消息
        const model::Message& lastMessage = dataCenter->getRecentMessageList(chatSessionId)->back();
        // 通过信号, 让NetClient模块，能够通知界面
        emit dataCenter->receiveMessageDone(lastMessage);

    } else {
        // 收到的消息会话, 不是选中会话
        // b) 更新未读消息数目
        dataCenter->addUnread(chatSessionId);

        // 确定好友昵称:
        if( !dataCenter->findChatSessionById(chatSessionId)){
            const QString& userId = dataCenter->findChatSessionById(chatSessionId)->userId;
            if( !dataCenter->findFriendByUserId(userId)){
                const QString& nickname = dataCenter->findFriendByUserId(userId)->nickname;
                Toast::showMessage(QString("收到一条来自%1的消息").arg(nickname), true);
            }
        }
    }
    // c) 统一更新会话列表的消息预览
    emit dataCenter->updateLastMessage(chatSessionId);
}

void NetClient::changeNickname(const QString &loginSessionId, const QString &nickname)
{
    // "/service/user/set_nickname"
    stark_im::SetUserNicknameReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setNickname(nickname);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[修改用户昵称] 发送请求 requestId="<<pbReq.requestId()
          << ", sessionId=" <<pbReq.sessionId()
          << ", nickname="<<pbReq.nickname();

    // 2. 发送http请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/user/set_nickname", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = handleHttpResponse<stark_im::SetUserNicknameRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[修改用户昵称] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        // 把数据设置到DataCenter里面
        dataCenter->resetNickname(nickname);

        emit dataCenter->changeNicknameDone();

        LOG() << "[修改用户昵称] 响应处理完毕 requestId=" << pbReq.requestId();
    });
}

void NetClient::changeDescription(const QString &loginSessionId, const QString &description)
{
    // "/service/user/set_description"
    stark_im::SetUserDescriptionReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setDescription(description);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[修改用户签名] 发送请求 requestId="<<pbReq.requestId()
          << ", sessionId=" <<pbReq.sessionId()
          << ", description="<<pbReq.description();

    // 2. 发送http请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/user/set_description", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = handleHttpResponse<stark_im::SetUserDescriptionRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[修改用户签名] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        // 把数据设置到DataCenter里面
        dataCenter->resetDescription(description);

        emit dataCenter->changeDescriptionDone();

        LOG() << "[修改用户签名] 响应处理完毕 requestId=" << pbReq.requestId();
    });
}

void NetClient::changeAvatar(const QString &loginSessionId, const QByteArray &avatar)
{
    stark_im::SetUserAvatarReq pbReq;
    pbReq.setRequestId(makeRequestId());
    // pbReq.setUserId("");
    pbReq.setSessionId(loginSessionId);
    pbReq.setAvatar(avatar);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[修改用户头像] 发送请求 requestId="<<pbReq.requestId()
          << ", sessionId=" <<pbReq.sessionId();

    // 发送http请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/user/set_avatar", body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = handleHttpResponse<stark_im::SetUserAvatarRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[修改用户头像] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        // 把数据设置到DataCenter里面
        dataCenter->resetAvatar(avatar);

        emit dataCenter->changeAvatarDone();

        LOG() << "[修改用户头像] 响应处理完毕 requestId=" << pbReq.requestId();
    });

}

void NetClient::deleteFriend(const QString &loginSessionId, const QString &peerId)
{
    stark_im::FriendRemoveReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setPeerId(peerId);
    // pbReq.setUserId("");
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[删除好友] 发送请求 requestId="<<pbReq.requestId()
          << ", sessionId=" <<pbReq.sessionId()
          << ", peerId=" << pbReq.peerId();

    QNetworkReply* httpResp = this->sendHttpRequest("/service/friend/remove_friend", body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::FriendRemoveRsp>(httpResp,&ok,&reason);

        if(!ok){
            LOG() << "[删除好友] 出错 requestId=" << pbReq.requestId() <<", reason=" << reason;
            return;
        }

        // 把结果更新到DataCenter, 把该删除的用户, 从好友列表删除
        dataCenter->removeFriend(peerId);

        // 发送信号, 通知调用者当前好友删除完毕
        emit dataCenter->deleteFriendDone();

        LOG() << "[删除好友] 响应处理完毕 requestId="<<pbResp->requestId();
    });
}

void NetClient::addFriendApply(const QString &loginSessionId, const QString &respondentId)
{
    stark_im::FriendAddReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setRespondentId(respondentId);
    // pbReq.setUserId("");
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[发送好友申请] 发送请求 requestId="<< pbReq.requestId()
          << ", sessionId=" << pbReq.sessionId()
          << ", respondentId=" << pbReq.respondentId();

    QNetworkReply* httpResp = this->sendHttpRequest("/service/friend/add_friend_apply", body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::FriendAddRsp>(httpResp,&ok,&reason);

        if(!ok){
            LOG() << "[发送好友申请] 出错 requestId=" << pbReq.requestId() <<", reason=" << reason;
            return;
        }

        // 此处无需记录任何数据

        // 发送信号, 通知UI
        emit dataCenter->addFriendApplyDone();

        LOG() << "[发送好友申请] 响应处理完毕 requestId="<<pbResp->requestId();
    });
}

void NetClient::accpetFriendApply(const QString &loginSessionId, const QString &applyUserId)
{
    stark_im::FriendAddProcessReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setApplyUserId(applyUserId);
    pbReq.setAgree(true);
    // pbReq.setUserId("");
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[同意好友申请] 发送请求 requestId="<< pbReq.requestId()
          << ", sessionId=" << pbReq.sessionId()
          << ", applyUserId=" << pbReq.applyUserId();

    QNetworkReply* httpResp = this->sendHttpRequest("/service/friend/add_friend_process", body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::FriendAddProcessRsp>(httpResp,&ok,&reason);

        if(!ok){
            LOG() << "[同意好友申请] 出错 requestId=" << pbReq.requestId() <<", reason=" << reason;
            return;
        }

        // 把好友列表更新、增加会话列表、修改申请按钮(已同意)/删除申请项
        model::UserInfo applyUser = dataCenter->removeFromApplyList(applyUserId);
        QList<model::UserInfo>* friendList = dataCenter->getFriendList();
        friendList->push_front(applyUser);

        // 发送信号, 通知UI更新
        emit dataCenter->acceptFriendApplyDone();

        LOG() << "[同意好友申请] 响应处理完毕 requestId="<<pbResp->requestId();
    });
}

void NetClient::rejectFriendApply(const QString &loginSessionId, const QString &applyUserId)
{
    stark_im::FriendAddProcessReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setApplyUserId(applyUserId);
    pbReq.setAgree(false);
    // pbReq.setUserId("");
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[拒绝好友申请] 发送请求 requestId="<< pbReq.requestId()
          << ", sessionId=" << pbReq.sessionId()
          << ", applyUserId=" << pbReq.applyUserId();

    QNetworkReply* httpResp = this->sendHttpRequest("/service/friend/add_friend_process", body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::FriendAddProcessRsp>(httpResp,&ok,&reason);

        if(!ok){
            LOG() << "[拒绝好友申请] 出错 requestId=" << pbReq.requestId() <<", reason=" << reason;
            return;
        }

        // 将申请项删除
        dataCenter->removeFromApplyList(applyUserId);

        // 发送信号, 通知UI更新
        emit dataCenter->rejectFriendApplyDone();

        LOG() << "[拒绝好友申请] 响应处理完毕 requestId="<<pbResp->requestId();
    });
}

void NetClient::createGroupChatSession(const QString &loginSessionId, const QList<QString> &userIdList)
{
    // "/service/friend/create_chat_session"
    // 创建单聊, 是在添加好友成功之后, 服务器自动添加的
    // 对于客户端，这个接口只用来创建群聊会话
    // 实际上，通过这个接口创建单聊，也是兼容的
    stark_im::ChatSessionCreateReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    // pbReq.setUserId("");
    pbReq.setChatSessionName(QString("群聊会话(%1)").arg(userIdList.count()));
    pbReq.setMemberIdList(userIdList);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[创建群聊会话] 发送请求 requestId="<< pbReq.requestId()
          << ", sessionId=" << pbReq.sessionId()
          << ", groupMemberCount=" << pbReq.memberIdList().count();

    // 发送HTTP请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/friend/create_chat_session", body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::ChatSessionCreateRsp>(httpResp,&ok,&reason);

        if(!ok){
            LOG() << "[创建群聊会话] 出错 requestId=" << pbReq.requestId() <<", reason=" << reason;
            return;
        }

        // 由于此处创建好的会话，是由websocket推送过来的, 此处无需存储数据

        // 发送信号, 通知UI更新
        emit dataCenter->createGroupChatSessionDone();

        LOG() << "[创建群聊会话] 响应处理完毕 requestId="<<pbResp->requestId();
    });
}

void NetClient::getMemberList(const QString &loginSessionId, const QString &chatSessionId)
{
    // 1. 构造请求 body
    stark_im::GetChatSessionMemberReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setChatSessionId(chatSessionId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[获取会话成员列表] 发送请求 requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", chatSessionId=" << pbReq.chatSessionId();

    // 2. 发送 HTTP 请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/friend/get_chat_session_member", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=]() {
        // a) 解析响应
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::GetChatSessionMemberRsp>(httpResp, &ok, &reason);

        // b) 判定响应结果是否正确
        if (!ok) {
            LOG() << "[获取会话成员列表] 响应失败 reason= requestId="<<pbReq.requestId()<<", reason=" << reason;
            return;
        }

        // c) 把结果记录到 DataCenter
        dataCenter->resetMemberList(chatSessionId, pbResp->memberInfoList());

        // d) 发送信号
        emit dataCenter->getMemberListDone(chatSessionId);

        // e) 打印日志
        LOG() << "[获取会话成员列表] 响应完成 requestId=" << pbResp->requestId();
    });
}

void NetClient::searchUser(const QString &loginSessionId, const QString &keyword)
{
    // 1.构造请求
    stark_im::FriendSearchReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    // pbReq.setUserId("");
    pbReq.setSearchKey(keyword);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[搜索用户] requestId="<<pbReq.requestId()
          << ", sessionId="<<pbReq.sessionId()
          << ", keyword="<<pbReq.searchKey();

    // 2.发送请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/friend/search_friend", body);

    // 3.处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 解析响应
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::FriendSearchRsp>(httpResp,&ok,&reason);

        // b) 判断是否ok
        if(!ok){
            LOG() << "[搜索用户] 响应失败 requestId="<<pbReq.requestId() << ", reason: "<<reason;
            return;
        }

        // c)得到的结果，记录到DataCenter中
        dataCenter->resetSearchUserResult(pbResp->userInfo()); //userInfo: repeated

        // d)通知UI更新
        emit dataCenter->searchUserDone();

        // e) 打印日志
        LOG() <<"[搜索用户] 响应成功 requestId="<<pbResp->requestId();
    });
}

void NetClient::searchMessage(const QString &loginSessionId, const QString &chatSessionId, const QString &searchKey)
{
    // 1.构造请求
    stark_im::MsgSearchReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setChatSessionId(chatSessionId);
    // pbReq.setUserId("");
    pbReq.setSearchKey(searchKey);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[关键词搜索历史消息] requestId="<<pbReq.requestId()
          << ", sessionId="<<pbReq.sessionId()
          << ", chatSessionId="<<pbReq.chatSessionId()
          << ", keyword="<<pbReq.searchKey();

    // 2.发送请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/message_storage/search_history", body);

    // 3.处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 解析响应
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::MsgSearchRsp>(httpResp, &ok, &reason);

        // b) 判断是否ok
        if(!ok){
            LOG() << "[关键词搜索历史消息] 响应失败 requestId="<<pbReq.requestId() << ", reason: "<<reason;
            return;
        }

        // c)得到的结果，记录到DataCenter中
        dataCenter->resetSearchMessageResult(pbReq.chatSessionId(), pbResp->msgList());

        // d)通知UI更新
        emit dataCenter->searchMessageDone(pbReq.chatSessionId());

        // e) 打印日志
        LOG() <<"[关键词搜索历史消息] 响应成功 requestId="<<pbResp->requestId();
    });
}

void NetClient::searchMessage(const QString &loginSessionId, const QString &chatSessionId,
                              const QDateTime &begTime, const QDateTime &endTime) {
    // 1.构造请求
    stark_im::GetHistoryMsgReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setChatSessionId(chatSessionId);
    // pbReq.setUserId("");
    pbReq.setStartTime(begTime.toSecsSinceEpoch());
    pbReq.setOverTime(endTime.toSecsSinceEpoch());
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[时间范围搜索历史消息] requestId="<<pbReq.requestId()
          << ", sessionId="<<pbReq.sessionId()
          << ", chatSessionId="<<pbReq.chatSessionId()
          << ", timerange="<<pbReq.startTime()<<" - "<<pbReq.overTime();

    // 2.发送请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/message_storage/get_history", body);

    // 3.处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 解析响应
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::GetHistoryMsgRsp>(httpResp, &ok, &reason);

        // b) 判断是否ok
        if(!ok){
            LOG() << "[时间范围搜索历史消息] 响应失败 requestId="<<pbReq.requestId() << ", reason: "<<reason;
            return;
        }

        // c)得到的结果，记录到DataCenter中
        dataCenter->resetSearchMessageResult(pbReq.chatSessionId(), pbResp->msgList());

        // d)通知UI更新
        emit dataCenter->searchMessageDone(pbReq.chatSessionId());

        // e) 打印日子
        LOG() <<"[时间范围搜索历史消息] 响应成功 requestId="<<pbResp->requestId();
    });
}

void NetClient::userLogin(const QString &username, const QString &password)
{
    stark_im::UserLoginReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setNickname(username);
    pbReq.setPassword(password);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[用户名登录] 发起请求 requestId=" << pbReq.requestId()
          << ", username="<<pbReq.nickname() << ", password="<<pbReq.password();

    QNetworkReply* httpResp=this->sendHttpRequest("/service/user/username_login",body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::UserLoginRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[用户名登录] 响应出错 requestId=" << pbReq.requestId() << ", reason=" << reason;
            emit dataCenter->userLoginDone(ok, reason);
            return;
        }

        // 将响应结果保存到dataCenter中
        dataCenter->resetLoginSessionId(pbResp->loginSessionId());

        // 通知登录界面关闭，进入主界面
        emit dataCenter->userLoginDone(ok, reason);

        LOG() << "[用户名登录] 响应成功 requestId="<< pbResp->requestId();
    });
}

void NetClient::userRegister(const QString &username, const QString &password)
{
    stark_im::UserRegisterReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setNickname(username);
    pbReq.setPassword(password);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[用户名注册] 发起请求 requestId=" << pbReq.requestId()
          << ", username="<<pbReq.nickname() << ", password="<<pbReq.password();

    QNetworkReply* httpResp=this->sendHttpRequest("/service/user/username_register", body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::UserRegisterRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[用户名注册] 响应出错 requestId=" << pbReq.requestId() << ", reason=" << reason;
            emit dataCenter->userRegisterDone(ok, reason);
            return;
        }

        // 响应结果没有保存到数据中心的，pass

        // 通知登录界面关闭，进入主界面
        emit dataCenter->userRegisterDone(ok, reason);

        LOG() << "[用户名注册] 响应成功 requestId="<< pbResp->requestId();
    });
}

void NetClient::phoneLogin(const QString &phone, const QString &verifyCodeId, const QString &verifyCode)
{
    stark_im::PhoneLoginReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setPhoneNumber(phone);
    pbReq.setVerifyCodeId(verifyCodeId);
    pbReq.setVerifyCode(verifyCode);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[手机号登录] 发起请求 requestId=" << pbReq.requestId() << ", phoneNumber="<<pbReq.phoneNumber()
          << ", (verifyCodeId: verifyCode)" << pbReq.verifyCodeId()<<": " << pbReq.verifyCode();

    QNetworkReply* httpResp=this->sendHttpRequest("/service/user/phone_login",body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::PhoneLoginRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[手机号登录] 响应出错 requestId=" << pbReq.requestId() << ", reason=" << reason;
            emit dataCenter->phoneLoginDone(ok, reason);
            return;
        }

        // 将响应结果保存到dataCenter中
        dataCenter->resetLoginSessionId(pbResp->loginSessionId());

        // 通知登录界面关闭，进入主界面
        emit dataCenter->phoneLoginDone(ok, reason);

        LOG() << "[手机号登录] 响应成功 requestId="<< pbResp->requestId();
    });
}

void NetClient::phoneRegister(const QString &phone, const QString &verifyCodeId, const QString &verifyCode)
{
    stark_im::PhoneRegisterReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setPhoneNumber(phone);
    pbReq.setVerifyCodeId(verifyCodeId);
    pbReq.setVerifyCode(verifyCode);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[手机号注册] 发起请求 requestId=" << pbReq.requestId() << ", phoneNumber="<<pbReq.phoneNumber()
          << ", (verifyCodeId: verifyCode)" << pbReq.verifyCodeId()<<": " << pbReq.verifyCode();

    QNetworkReply* httpResp=this->sendHttpRequest("/service/user/phone_login",body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::PhoneRegisterRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[手机号注册] 响应出错 requestId=" << pbReq.requestId() << ", reason=" << reason;
            emit dataCenter->phoneRegisterDone(ok, reason);
            return;
        }

        // 通知登录界面关闭，进入主界面
        emit dataCenter->phoneRegisterDone(ok, reason);

        LOG() << "[手机号注册] 响应成功 requestId="<< pbResp->requestId();
    });
}

void NetClient::getSingleFile(const QString &loginSessionId, const QString &fileId)
{
    stark_im::GetSingleFileReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setFileId(fileId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[获取单个文件] 发起请求 requestId=" << pbReq.requestId()
          << ", sessionId=" << pbReq.sessionId() << ", fileId=" << pbReq.fileId();

    QNetworkReply* httpResp = this->sendHttpRequest("/service/file/get_single_file",body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::GetSingleFileRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[获取单个文件] 响应出错 requestId=" << pbReq.requestId() << ", reason=" << reason;
            return;
        }

        // c) 响应结果保存下来, 之前都是保存到dataCenter中
        //      这里涉及到的文件会很多，不使用DataCenter中
        //      直接通过信号把文件数据，投送到调用者的位置上

        // 通知登录界面关闭，进入主界面
        emit dataCenter->getSingleFileDone(fileId, pbResp->fileData().fileContent());

        LOG() << "[获取单个文件] 响应成功 requestId="<< pbResp->requestId();
    });
}

void NetClient::speechConvertText(const QString &loginSessionId, const QString &fileId, const QByteArray &content)
{
    stark_im::SpeechRecognitionReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    // pbReq.setUserId("");
    pbReq.setSpeechContent(content);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[语音转文字] 发起请求 requestId=" << pbReq.requestId()
          << ", sessionId="<<pbReq.sessionId();

    QNetworkReply* httpResp = this->sendHttpRequest("/service/speech/recognition", body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::SpeechRecognitionRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[语音转文字] 响应出错 requestId=" << pbReq.requestId() << ", reason=" << reason;
            return;
        }

        // 通知登录界面关闭，进入主界面
        emit dataCenter->speechConvertTextDone(fileId, pbResp->recognitionResult());

        LOG() << "[语音转文字] 响应成功 requestId="<< pbResp->requestId();
    });
}

void NetClient::getVerifyCode(const QString &phone)
{
    stark_im::PhoneVerifyCodeReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setPhoneNumber(phone);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[获取手机验证码] 发送请求 requsetId=" << pbReq.requestId();

    // 发送http请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/user/get_phone_verify_code", body);

    // 处理响应
    connect(httpResp, &QNetworkReply::finished, this,[=](){
        bool ok = false;
        QString reason;
        auto pbResp = handleHttpResponse<stark_im::PhoneVerifyCodeRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[获取手机验证码] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        // 把数据设置到DataCenter里面
        dataCenter->resetVerifyCodeId(pbResp->verifyCodeId());

        emit dataCenter->getVerifyCodeDone();

        LOG() << "[获取手机验证码] 响应处理完毕 requestId=" << pbReq.requestId();
    });

}

void NetClient::changePhone(const QString& loginSessionId, const QString &phone, const QString &verifyCodeId, const QString &verifyCode)
{
    stark_im::SetUserPhoneNumberReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    // pbReq.setUserId("");
    pbReq.setPhoneNumber(phone);
    pbReq.setPhoneVerifyCodeId(verifyCodeId);
    pbReq.setPhoneVerifyCode(verifyCode);

    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[修改用户手机号] 发送请求 requsetId=" << pbReq.requestId();

    QNetworkReply* httpResp = this->sendHttpRequest("/service/user/set_phone", body);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        bool ok = false;
        QString reason;
        auto pbResp = this->handleHttpResponse<stark_im::SetUserPhoneNumberRsp>(httpResp, &ok, &reason);

        if(!ok){
            LOG() << "[修改用户手机号] 出错, requestId="<<pbReq.requestId()<<", reason:" << reason;
            return;
        }

        dataCenter->resetPhone(phone);

        emit dataCenter->changePhoneDone();

        LOG() << "[修改用户手机号] 响应处理完毕 requestId=" << pbReq.requestId();
    });

}


























} // end namespace network
