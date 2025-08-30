#include "server.h"

#include "base.qpb.h"
#include "notify.qpb.h"
#include "user.qpb.h"
#include "friend.qpb.h"
#include "message_storage.qpb.h"
#include "message_transmit.qpb.h"
#include "speech_recognition.qpb.h"
#include "file.qpb.h"
#include "gateway.qpb.h"

#include <QUuid>
#include <QTimer>
#include <QEventLoop>
//在客户端与网关服务器的通信中，使用HTTP协议进行通信
//  通信时采用POST请求作为请求方法
//  通信时，正文采用protobuf作为正文协议格式，具体内容字段以前边各个文件中定义的字段格式为准
/*  以下是HTTP请求的功能与接口路径对应关系：
    SERVICE HTTP PATH:
    {
        获取随机验证码                  /service/user/get_random_verify_code
        获取短信验证码                  /service/user/get_phone_verify_code
        用户名密码注册                  /service/user/username_register
        用户名密码登录                  /service/user/username_login
        手机号码注册                    /service/user/phone_register
        手机号码登录                    /service/user/phone_login
        获取个人信息                    /service/user/get_user_info
        修改头像                        /service/user/set_avatar
        修改昵称                        /service/user/set_nickname
        修改签名                        /service/user/set_description
        修改绑定手机                    /service/user/set_phone

        获取好友列表                    /service/friend/get_friend_list
        获取好友信息                    /service/friend/get_friend_info
        发送好友申请                    /service/friend/add_friend_apply
        好友申请处理                    /service/friend/add_friend_process
        删除好友                        /service/friend/remove_friend
        搜索用户                        /service/friend/search_friend
        获取指定用户的消息会话列表       /service/friend/get_chat_session_list
        创建消息会话                    /service/friend/create_chat_session
        获取消息会话成员列表             /service/friend/get_chat_session_member
        获取待处理好友申请事件列表       /service/friend/get_pending_friend_events

        获取历史消息/离线消息列表        /service/message_storage/get_history
        获取最近N条消息列表             /service/message_storage/get_recent
        搜索历史消息                    /service/message_storage/search_history

        发送消息                        /service/message_transmit/new_message

        获取单个文件数据                /service/file/get_single_file
        获取多个文件数据                /service/file/get_multi_file
        发送单个文件                    /service/file/put_single_file
        发送多个文件                    /service/file/put_multi_file

        语音转文字                     /service/speech/recognition
    }
*/


////////////////////////////////////////////////////
/// 一些辅助函数
////////////////////////////////////////////////////

stark_im::UserInfo makeUserInfo(int i, const QByteArray& avatar){
    stark_im::UserInfo userInfo;

    QString index = QString::number(i);
    userInfo.setUserId(QString::number(1000+i)/*QUuid::createUuid().toString().sliced(5,18)*/);
    userInfo.setNickname("张三"+index/*QUuid::createUuid().toString().sliced(5,14)*/);
    userInfo.setDescription("签名"+index/*QUuid::createUuid().toString().sliced(5,14)*/);
    userInfo.setPhone("178" + QString::number(10000000 + i));
    userInfo.setAvatar(avatar);

    return userInfo;
}

// 生成默认的 MessageInfo 对象. 文本消息
stark_im::MessageInfo makeTextMessageInfo(int index, const QString& chatSessionId, const QByteArray& avatar) {
    stark_im::MessageInfo messageInfo;
    messageInfo.setMessageId("Msg_" + QString::number(3000 + index));
    messageInfo.setChatSessionId(chatSessionId);
    messageInfo.setTimestamp(getTime());
    messageInfo.setSender(makeUserInfo(index, avatar));

    stark_im::StringMessageInfo stringMessageInfo;
    stringMessageInfo.setContent("这是一条消息内容" + QString::number(index));

    stark_im::MessageContent messageContent;
    messageContent.setMessageType(stark_im::MessageTypeGadget::MessageType::STRING);
    messageContent.setStringMessage(stringMessageInfo);
    messageInfo.setMessage(messageContent);
    return messageInfo;
}

stark_im::MessageInfo makeImageMessageInfo(int index, const QString& chatSessionId, const QByteArray& avatar) {
    stark_im::MessageInfo messageInfo;
    messageInfo.setMessageId("Msg_" + QString::number(4000 + index));
    messageInfo.setChatSessionId(chatSessionId);
    messageInfo.setTimestamp(getTime());
    messageInfo.setSender(makeUserInfo(index, avatar));

    stark_im::ImageMessageInfo imageMessageInfo;
    imageMessageInfo.setFileId("testImage");
    // 真实服务器推送的消息数据里, 本身也就不带图片的正文. 只是 fileId.
    // 需要通过 fileId, 二次发起请求, 通过 getSingleFile 接口来获取到内容.
    // imageMessageInfo.setImageContent();

    stark_im::MessageContent messageContent;
    messageContent.setMessageType(stark_im::MessageTypeGadget::MessageType::IMAGE);
    messageContent.setImageMessage(imageMessageInfo);
    messageInfo.setMessage(messageContent);
    return messageInfo;
}

stark_im::MessageInfo makeFileMessageInfo(int index, const QString& chatSessionId, const QByteArray& avatar) {
    stark_im::MessageInfo messageInfo;
    messageInfo.setMessageId("Msg_" + QString::number(5000 + index));
    messageInfo.setChatSessionId(chatSessionId);
    messageInfo.setTimestamp(getTime());
    messageInfo.setSender(makeUserInfo(index, avatar));

    stark_im::FileMessageInfo fileMessageInfo;
    fileMessageInfo.setFileId("testFile");
    // 真实服务器推送的消息数据里, 本身也就不带图片的正文. 只是 fileId.
    // 需要通过 fileId, 二次发起请求, 通过 getSingleFile 接口来获取到内容.
    fileMessageInfo.setFileName("test.txt");
    // 此处文件大小, 无法设置. 由于 fileSize 属性, 不是 optional , 此处先设置一个 0 进来
    fileMessageInfo.setFileSize(0);

    stark_im::MessageContent messageContent;
    messageContent.setMessageType(stark_im::MessageTypeGadget::MessageType::FILE);
    messageContent.setFileMessage(fileMessageInfo);
    messageInfo.setMessage(messageContent);
    return messageInfo;
}

stark_im::MessageInfo makeSpeechMessageInfo(int index, const QString& chatSessionId, const QByteArray& avatar) {
    stark_im::MessageInfo messageInfo;
    messageInfo.setMessageId("Msg_" + QString::number(6000 + index));
    messageInfo.setChatSessionId(chatSessionId);
    messageInfo.setTimestamp(getTime());
    messageInfo.setSender(makeUserInfo(index, avatar));

    stark_im::SpeechMessageInfo speechMessageInfo;
    // 真实服务器推送的消息数据里, 本身也就不带图片的正文. 只是 fileId.
    // 需要通过 fileId, 二次发起请求, 通过 getSingleFile 接口来获取到内容.
    speechMessageInfo.setFileId("testSpeech");

    stark_im::MessageContent messageContent;
    messageContent.setMessageType(stark_im::MessageTypeGadget::MessageType::SPEECH);
    messageContent.setSpeechMessage(speechMessageInfo);
    messageInfo.setMessage(messageContent);
    return messageInfo;
}

////////////////////////////////////////////////////
/// HTTP服务器
////////////////////////////////////////////////////

HttpServer* HttpServer::instance = nullptr;
HttpServer *HttpServer::getInstance()
{
    if(instance==nullptr){
        instance = new HttpServer();
    }
    return instance;
}

bool HttpServer::init()
{
    // 返回的是成功绑定的端口号的数值
    int ret = httpServer.listen(QHostAddress::Any, 8000);

    // 配置路由- Web路由是路径, 不是IP中的路由
    httpServer.route("/ping", [](const QHttpServerRequest& req){
        (void) req;
        qDebug() << "[http] 收到ping请求";
        return "pong";
    });

    httpServer.route("/service/user/get_user_info", [=](const QHttpServerRequest& req){
        return this->getUserInfo(req);
    });

    httpServer.route("/service/friend/get_friend_list", [=](const QHttpServerRequest& req){
        return this->getFriendList(req);
    });

    httpServer.route("/service/friend/get_chat_session_list", [=](const QHttpServerRequest& req){
        return this->getChatSessionList(req);
    });

    httpServer.route("/service/friend/get_pending_friend_events", [=](const QHttpServerRequest& req){
        return this->getApplyList(req);
    });

    httpServer.route("/service/message_storage/get_recent", [=](const QHttpServerRequest& req){
        return this->getRecentMessageList(req);
    });

    httpServer.route("/service/message_transmit/new_message", [=](const QHttpServerRequest& req){
        return this->newMessage(req);
    });

    httpServer.route("/service/user/set_nickname", [=](const QHttpServerRequest& req){
        return this->setNickname(req);
    });

    httpServer.route("/service/user/set_description", [=](const QHttpServerRequest& req){
        return this->setDescription(req);
    });

    httpServer.route("/service/user/get_phone_verify_code", [=](const QHttpServerRequest& req){
        return this->getPhoneVerifyCode(req);
    });

    httpServer.route("/service/user/set_phone", [=](const QHttpServerRequest& req){
        return this->setPhone(req);
    });

    httpServer.route("/service/user/set_avatar", [=](const QHttpServerRequest& req){
        return this->setAvatar(req);
    });

    httpServer.route("/service/friend/remove_friend", [=](const QHttpServerRequest& req){
        return this->removeFriend(req);
    });

    httpServer.route("/service/friend/add_friend_apply", [=](const QHttpServerRequest& req){
        return this->addFriendApply(req);
    });

    httpServer.route("/service/friend/add_friend_process", [=](const QHttpServerRequest& req){
        return this->addFriendProcess(req);
    });

    httpServer.route("/service/friend/create_chat_session", [=](const QHttpServerRequest& req){
        return this->createChatSession(req);
    });

    httpServer.route("/service/friend/get_chat_session_member", [=](const QHttpServerRequest& req){
        return this->getChatSessionMember(req);
    });

    httpServer.route("/service/friend/search_friend", [=](const QHttpServerRequest& req){
        return this->searchFriend(req);
    });

    httpServer.route("/service/message_storage/search_history", [=](const QHttpServerRequest& req){
        return this->searchHistory(req);
    });

    httpServer.route("/service/message_storage/get_history", [=](const QHttpServerRequest& req){
        return this->getHistory(req);
    });

    httpServer.route("/service/user/username_login", [=](const QHttpServerRequest& req){
        return this->userLogin(req);
    });

    httpServer.route("/service/user/username_register", [=](const QHttpServerRequest& req){
        return this->userRegister(req);
    });

    httpServer.route("/service/user/phone_login", [=](const QHttpServerRequest& req){
        return this->phoneLogin(req);
    });

    httpServer.route("/service/user/phone_register", [=](const QHttpServerRequest& req){
        return this->phoneRegister(req);
    });

    httpServer.route("/service/file/get_single_file", [=](const QHttpServerRequest& req){
        return this->getSingleFile(req);
    });

    httpServer.route("/service/speech/recognition", [=](const QHttpServerRequest& req){
        return this->recognition(req);
    });

    return ret == 8000;
}

QHttpServerResponse HttpServer::getUserInfo(const QHttpServerRequest &req)
{
    // 解析请求
    stark_im::GetUserInfoReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() <<"[REQ 获取用户信息] requestId="<<pbReq.requestId() << ", loginSessionId=" <<pbReq.sessionId();

    // 构造响应数据
    stark_im::GetUserInfoRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    stark_im::UserInfo userInfo;
    userInfo.setUserId("123456");
    userInfo.setNickname("张三");
    userInfo.setDescription("签名");
    userInfo.setPhone("17812345678");
    userInfo.setAvatar(loadFileToByteArray(":/image/groupAvatar.png"));
    pbResp.setUserInfo(userInfo);

    QByteArray body = pbResp.serialize(&serializer);

    //构造HTTP响应信息
    QHttpServerResponse httpResp(body, QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/x-protobuf");

    return httpResp;
}

QHttpServerResponse HttpServer::getFriendList(const QHttpServerRequest &req)
{
    // 解析请求
    stark_im::GetFriendListReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() <<"[REQ 获取好友列表] requestId="<<pbReq.requestId() << ", loginSessionId=" <<pbReq.sessionId();

    // 构造响应
    stark_im::GetFriendListRsp pbRsp;
    pbRsp.setRequestId(pbReq.requestId());
    pbRsp.setSuccess(true);
    pbRsp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
    for(int i=1; i<=20; i++){
        pbRsp.friendList().push_back( makeUserInfo(i, avatar) );
    }

    QByteArray body = pbRsp.serialize(&serializer);

    //构造HTTP响应信息
    QHttpServerResponse httpResp(body, QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/x-protobuf");

    return httpResp;
}

QHttpServerResponse HttpServer::getChatSessionList(const QHttpServerRequest &req)
{
    // 解析请求
    stark_im::GetChatSessionListReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() <<"[REQ 获取会话列表] requestId="<<pbReq.requestId() << ", loginSessionId=" <<pbReq.sessionId();

    // 构造响应
    stark_im::GetChatSessionListRsp pbRsp;
    pbRsp.setRequestId(pbReq.requestId());
    pbRsp.setSuccess(true);
    pbRsp.setErrmsg("");

    // 创建群聊会话测试
    QByteArray group_avatar = loadFileToByteArray(":/image/groupAvatar.png");
    stark_im::ChatSessionInfo gchatSessionInfo;
    gchatSessionInfo.setChatSessionId(QString::number(1000));   //群聊
    gchatSessionInfo.setChatSessionName("唯一的群聊");
    gchatSessionInfo.setAvatar(group_avatar);
    stark_im::MessageInfo messageInfo = makeTextMessageInfo(0, gchatSessionInfo.chatSessionId(), group_avatar);
    gchatSessionInfo.setPrevMessage(messageInfo);
    pbRsp.chatSessionInfoList().push_back(gchatSessionInfo);

    QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
    for(int i=1; i<30; i++){
        stark_im::ChatSessionInfo chatSessionInfo;
        chatSessionInfo.setChatSessionId(QString::number(2000 + i));
        chatSessionInfo.setChatSessionName("张三" + QString::number(i));
        chatSessionInfo.setSingleChatFriendId(QString::number(1000 + i));
        chatSessionInfo.setAvatar(avatar);
        stark_im::MessageInfo messageInfo = makeTextMessageInfo(i, chatSessionInfo.chatSessionId(), avatar);

        chatSessionInfo.setPrevMessage(messageInfo);

        pbRsp.chatSessionInfoList().push_back(chatSessionInfo);
    }

    QByteArray body = pbRsp.serialize(&serializer);

    //构造HTTP响应信息
    QHttpServerResponse httpResp(body, QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/x-protobuf");

    return httpResp;
}

QHttpServerResponse HttpServer::getApplyList(const QHttpServerRequest &req)
{
    // 解析请求
    stark_im::GetPendingFriendEventListReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() <<"[REQ 获取申请列表] requestId="<<pbReq.requestId() << ", loginSessionId=" <<pbReq.sessionId();

    // 构造响应
    stark_im::GetPendingFriendEventListRsp pbRsp;
    pbRsp.setRequestId(pbReq.requestId());
    pbRsp.setSuccess(true);
    pbRsp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
    for(int i=100;i<105;i++){
        stark_im::FriendEvent friendEvent;
        friendEvent.setEventId("");
        friendEvent.setSender(makeUserInfo(i,avatar));

        pbRsp.event().push_back(friendEvent);
    }

    QByteArray body = pbRsp.serialize(&serializer);

    //构造HTTP响应信息
    QHttpServerResponse httpResp(body, QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/x-protobuf");

    return httpResp;
}

QHttpServerResponse HttpServer::getRecentMessageList(const QHttpServerRequest &req)
{
    // 解析请求
    stark_im::GetRecentMsgReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取指定用户的最近消息列表] requestId="<<pbReq.requestId()
          << ", loginSessionId=" <<pbReq.sessionId()
          << ", chatSessionId=" <<pbReq.chatSessionId();

    // 构造响应
    stark_im::GetRecentMsgRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");


    QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
    for(int i=0; i<30; ++i){// 对方发送的消息
        stark_im::MessageInfo messageInfo = makeTextMessageInfo(i, pbReq.chatSessionId(), avatar);
        pbResp.msgList().push_back(messageInfo);
    }

    QByteArray my_avatar = loadFileToByteArray(":/image/groupAvatar.png");
    stark_im::MessageInfo messageInfo = makeTextMessageInfo(123456-1000, pbReq.chatSessionId(), my_avatar);
    pbResp.msgList().push_back(messageInfo);// 我发送的消息


    // 序列化
    QByteArray body = pbResp.serialize(&serializer);

    // 构造HTTP响应对象
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::newMessage(const QHttpServerRequest &req)
{
    // 1.解析请求
    stark_im::NewMessageReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 发送信息] requestId="<< pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", chatSessionId=" << pbReq.chatSessionId()
          << ", messageType=" << pbReq.message().messageType();

    if(pbReq.message().messageType() == stark_im::MessageTypeGadget::STRING){
        LOG() << "发送的消息内容为: size="<< pbReq.message().stringMessage().content().size();
    } else if(pbReq.message().messageType() == stark_im::MessageTypeGadget::IMAGE){
        LOG() << "发送的图片内容为: size="<< pbReq.message().imageMessage().imageContent().size();
    }

    // 2.构造响应
    stark_im::NewMessageRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray body = pbResp.serialize(&serializer);

    // 3.构造Http响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::setNickname(const QHttpServerRequest &req)
{
    // 1.解析请求
    stark_im::SetUserNicknameReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 修改用户昵称] requestId="<< pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId();

    // 2.构造响应
    stark_im::SetUserNicknameRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray body = pbResp.serialize(&serializer);

    // 3.构造Http响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::setDescription(const QHttpServerRequest &req)
{
    // 1.解析请求
    stark_im::SetUserNicknameReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 修改用户签名] requestId="<< pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId();

    // 2.构造响应
    stark_im::SetUserNicknameRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray body = pbResp.serialize(&serializer);

    // 3.构造Http响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::getPhoneVerifyCode(const QHttpServerRequest &req)
{
    // 1.解析请求
    stark_im::PhoneVerifyCodeReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取短信验证码] requestId="<< pbReq.requestId()
          << ", phone=" << pbReq.phoneNumber();

    // 2.构造响应
    stark_im::PhoneVerifyCodeRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setVerifyCodeId("testVerifyCodeId");

    QByteArray body = pbResp.serialize(&serializer);

    // 3.构造Http响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::setPhone(const QHttpServerRequest &req)
{
    stark_im::SetUserPhoneNumberReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 修改用户手机号] requestId="<< pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId();

    stark_im::SetUserPhoneNumberRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray body = pbResp.serialize(&serializer);

    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::setAvatar(const QHttpServerRequest &req)
{
    stark_im::SetUserAvatarReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 修改用户头像] requestId="<< pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId();

    stark_im::SetUserAvatarRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray body = pbResp.serialize(&serializer);

    // 创建局部事件循环
    QEventLoop loop;
    QTimer timer;
    // 10秒后退出事件循环
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.setSingleShot(true);
    timer.start(1000);
    // 阻塞当前方法
    loop.exec();

    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::removeFriend(const QHttpServerRequest &req)
{
    stark_im::FriendRemoveReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 删除好友] requestId=" << pbReq.requestId()
          << ", loginSessionId=" <<pbReq.sessionId()
          << ", peerId=" << pbReq.peerId();

    // 构造响应body
    stark_im::FriendRemoveRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);


    // 响应HTTP
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::addFriendApply(const QHttpServerRequest &req)
{
    stark_im::FriendAddReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 发送好友申请] requestId=" << pbReq.requestId()
          << ", loginSessionId=" <<pbReq.sessionId()
          << ", respondentId=" << pbReq.respondentId();

    stark_im::FriendAddRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 响应HTTP
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::addFriendProcess(const QHttpServerRequest &req)
{
    stark_im::FriendAddProcessReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 处理好友申请] requestId=" << pbReq.requestId()
          << ", loginSessionId=" <<pbReq.sessionId()
          << ", applyUserId=" << pbReq.applyUserId();

    stark_im::FriendAddProcessRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setNewSessionId( QString::number(pbReq.applyUserId().toInt() + 1000) );
    QByteArray body = pbResp.serialize(&serializer);

    // 响应HTTP
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::createChatSession(const QHttpServerRequest &req)
{
    // 解析请求
    stark_im::ChatSessionCreateReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 创建会话] requestId="<<pbReq.requestId()
          <<", loginSessionId="<<pbReq.sessionId()
          <<", userIdList="<<pbReq.memberIdList();

    // 构造响应body
    stark_im::ChatSessionCreateRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 响应HTTP
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::getChatSessionMember(const QHttpServerRequest &req)
{
    // 解析请求
    stark_im::GetChatSessionMemberReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 创建会话] requestId="<<pbReq.requestId()
          <<", loginSessionId="<<pbReq.sessionId()
          <<", chatSessionId="<<pbReq.chatSessionId();

    // 构造响应body
    stark_im::UserInfoRepeated member_info_list;
    QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
    for(int i=0;i<30;i++){
        stark_im::UserInfo userInfo = makeUserInfo(i,avatar);
        member_info_list.push_back(userInfo);
    }

    stark_im::GetChatSessionMemberRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setMemberInfoList(member_info_list);
    QByteArray body = pbResp.serialize(&serializer);

    // 响应HTTP
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::searchFriend(const QHttpServerRequest &req)
{
    stark_im::FriendSearchReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 搜索用户] requestId="<<pbReq.requestId()
          <<", sessionId="<<pbReq.sessionId()
          <<", searchKey="<<pbReq.searchKey();

    // 构造响应体
    stark_im::FriendSearchRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
    for(int i=0; i<50; ++i){
        stark_im::UserInfo userInfo = makeUserInfo(i, avatar);
        pbResp.userInfo().push_back(userInfo);
    }
    QByteArray body = pbResp.serialize(&serializer);

    // 构建HTTP响应
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::searchHistory(const QHttpServerRequest &req)
{
    stark_im::MsgSearchReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 搜索历史消息] requestId="<<pbReq.requestId()
          << ", sessionId="<<pbReq.sessionId()
          << ", chatSessionId="<<pbReq.chatSessionId()
          << ", searchKey="<<pbReq.searchKey();

    // 构造响应体
    stark_im::MsgSearchRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
    stark_im::MessageInfo message;
    message = makeImageMessageInfo(11,pbReq.chatSessionId(),avatar);
    pbResp.msgList().push_back(message);
    message = makeFileMessageInfo(12,pbReq.chatSessionId(),avatar);
    pbResp.msgList().push_back(message);
    message = makeSpeechMessageInfo(13,pbReq.chatSessionId(),avatar);
    pbResp.msgList().push_back(message);
    for(int i=0;i<10;i++){
        message = makeTextMessageInfo(i,pbReq.chatSessionId(),avatar);
        pbResp.msgList().push_back(message);
    }
    QByteArray body = pbResp.serialize(&serializer);

    // 构建HTTP响应
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::getHistory(const QHttpServerRequest &req)
{
    stark_im::GetHistoryMsgReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取历史范围消息] requestId="<<pbReq.requestId()
          << ", sessionId="<<pbReq.sessionId()
          << ", chatSessionId="<<pbReq.chatSessionId()
          << ", timeRange="<<pbReq.startTime()<<" - "<<pbReq.overTime();

    // 构造响应体
    stark_im::GetHistoryMsgRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
    stark_im::MessageInfo message;
    message = makeImageMessageInfo(11,pbReq.chatSessionId(),avatar);
    pbResp.msgList().push_back(message);
    message = makeFileMessageInfo(12,pbReq.chatSessionId(),avatar);
    pbResp.msgList().push_back(message);
    message = makeSpeechMessageInfo(13,pbReq.chatSessionId(),avatar);
    pbResp.msgList().push_back(message);
    for(int i=0;i<10;i++){
        message = makeTextMessageInfo(i,pbReq.chatSessionId(),avatar);
        pbResp.msgList().push_back(message);
    }
    QByteArray body = pbResp.serialize(&serializer);

    // 构建HTTP响应
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::userLogin(const QHttpServerRequest &req)
{
    stark_im::UserLoginReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 用户名登录] requestId=" <<pbReq.requestId()
          << ", nickname="<<pbReq.nickname() << ", password="<<pbReq.password();

    stark_im::UserLoginRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    // pbResp.setSuccess(false);
    // pbResp.setErrmsg("test_login_false");
    pbResp.setLoginSessionId("Session_" + QUuid::createUuid().toString().sliced(25,12));
    QByteArray body = pbResp.serialize(&serializer);

    // 构建HTTP响应
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    // LOG() <<"登录成功";
    return resp;
}

QHttpServerResponse HttpServer::userRegister(const QHttpServerRequest &req)
{
    stark_im::UserRegisterReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 用户名注册] requestId=" <<pbReq.requestId()
          << ", nickname="<<pbReq.nickname() << ", password="<<pbReq.password();

    stark_im::UserRegisterRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    // pbResp.setSuccess(false);
    // pbResp.setErrmsg("test_register_false");
    QByteArray body = pbResp.serialize(&serializer);

    // 构建HTTP响应
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    // LOG() <<"注册成功";
    return resp;
}

QHttpServerResponse HttpServer::phoneLogin(const QHttpServerRequest &req)
{
    stark_im::PhoneLoginReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 手机号登录] requestId=" <<pbReq.requestId()
          << ", phoneNumber="<<pbReq.phoneNumber()
          << ", verifyCodeId="<<pbReq.verifyCodeId()
          << ", verifyCode="<<pbReq.verifyCode();

    stark_im::PhoneLoginRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setLoginSessionId("Session_" + QUuid::createUuid().toString().sliced(25,12));
    QByteArray body = pbResp.serialize(&serializer);

    // 构建HTTP响应
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::phoneRegister(const QHttpServerRequest &req)
{
    stark_im::PhoneRegisterReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 手机号注册] requestId=" <<pbReq.requestId()
          << ", phoneNumber="<<pbReq.phoneNumber()
          << ", verifyCodeId="<<pbReq.verifyCodeId()
          << ", verifyCode="<<pbReq.verifyCode();

    stark_im::PhoneRegisterRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 构建HTTP响应
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::getSingleFile(const QHttpServerRequest &req)
{
    stark_im::GetSingleFileReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取单个文件] requestId=" <<pbReq.requestId()
          << ", fileId=" <<pbReq.fileId();

    stark_im::GetSingleFileRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    stark_im::FileDownloadData fileData;
    fileData.setFileId(pbReq.fileId());
    if(pbReq.fileId() == "testImage") {
        fileData.setFileContent(loadFileToByteArray(":/image/logo.png"));
    } else if(pbReq.fileId() == "testFile") {
        fileData.setFileContent(loadFileToByteArray(":/file/test.txt"));
    } else if(pbReq.fileId() == "testSpeech") {
        fileData.setFileContent(loadFileToByteArray(":/speech/testSpeech.pcm"));
    } else {
        pbResp.setSuccess(false);
        pbResp.setErrmsg("fileId 不是预期的 fileId");
    }
    pbResp.setFileData(fileData);
    QByteArray body = pbResp.serialize(&serializer);

    // 构建HTTP响应
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::recognition(const QHttpServerRequest &req)
{
    stark_im::SpeechRecognitionReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 语音转文字] requestId=" <<pbReq.requestId()
          << ", loginSessionId="<<pbReq.sessionId()
          << ", content.size="<<pbReq.speechContent().size();

    stark_im::SpeechRecognitionRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setRecognitionResult("这是一条语音转换文字后的消息结果");
    QByteArray body = pbResp.serialize(&serializer);

    // 构建HTTP响应
    QHttpServerResponse resp(body);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}





////////////////////////////////////////////////////
/// Websocket服务器
////////////////////////////////////////////////////


WebsocketServer* WebsocketServer::instance = nullptr;

WebsocketServer::WebsocketServer()
    :websocketServer("websocket server", QWebSocketServer::NonSecureMode)
{}

WebsocketServer *WebsocketServer::getInstance()
{
    if(instance == nullptr){
        instance = new WebsocketServer();
    }
    return instance;
}

bool WebsocketServer::init()
{
    // 1.信号槽的关联
    connect(&websocketServer, &QWebSocketServer::newConnection, this, [=](){
        // 连接建立成功之后
        LOG() <<"[websocket] 连接建立成功!";

        // 获取通信 socket 对象，nextPendingConnection类似于原生socket的accept
        QWebSocket* socket = websocketServer.nextPendingConnection();

        // 针对这个 socket 对象，进行剩余信号的处理
        connect(socket, &QWebSocket::disconnected, this, [=](){
            LOG() << "[websocket] 连接断开";
            disconnect(this, &WebsocketServer::sendTextResp, this, nullptr);
            disconnect(this, &WebsocketServer::sendImageResp, this, nullptr);
            disconnect(this, &WebsocketServer::sendFileResp, this, nullptr);
            disconnect(this, &WebsocketServer::sendSpeechResp, this, nullptr);

            disconnect(this, &WebsocketServer::sendFriendRemove, this, nullptr);
            disconnect(this, &WebsocketServer::sendAddFriendApply, this, nullptr);
            disconnect(this, &WebsocketServer::sendAddFriendProcess, this, nullptr);
            disconnect(this, &WebsocketServer::sendCreateChatSession, this,nullptr);
        });
        connect(socket, &QWebSocket::errorOccurred, this, [=](QAbstractSocket::SocketError error){
            LOG() << "[websocket] 连接出错" << error;
        });
        connect(socket, &QWebSocket::textMessageReceived, this, [=](const QString& message){
            LOG() << "[websocket] 收到文本消息" <<message;
        });
        connect(socket, &QWebSocket::binaryMessageReceived, this, [=](const QByteArray& byteArray){
            LOG() << "[websocket] 收到二进制消息" << byteArray.length();
        });

        ////////////////////////////////////////////
        /// 通过信号发送指定推送消息
        ////////////////////////////////////////////
        connect(this, &WebsocketServer::sendTextResp, this, [=](){
            // 此处就可以捕获到socket对象, 从而通过socket对象给客户端返回数据
            if(socket == nullptr || !socket->isValid()){
                LOG() << "socket 对象无效";
                return;
            }
            // 构造响应数据
            QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
            stark_im::MessageInfo messageInfo = makeTextMessageInfo(this->messageIndex++, "2001", avatar);  //模拟 张三_1发送的消息
            // messageInfo

            stark_im::NotifyNewMessage notifyNewMessage;
            notifyNewMessage.setMessageInfo(messageInfo);

            stark_im::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(stark_im::NotifyTypeGadget::CHAT_MESSAGE_NOTIFY);
            notifyMessage.setNewMessageInfo(notifyNewMessage);

            // 序列化
            QByteArray body = notifyMessage.serialize(&this->serializer);

            // 发送消息给客户端
            socket->sendBinaryMessage(body);

            LOG() << "发送文本消息响应" << messageInfo.message().stringMessage().content();
        });
        connect(this, &WebsocketServer::sendImageResp, this, [=](){
            if(socket == nullptr || !socket->isValid()){
                LOG() << "socket 对象无效";
                return;
            }
            // 构造响应数据
            QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
            stark_im::MessageInfo messageInfo = makeImageMessageInfo(this->messageIndex++, "2001", avatar);  //模拟 张三_1发送的消息

            stark_im::NotifyNewMessage notifyNewMessage;
            notifyNewMessage.setMessageInfo(messageInfo);

            stark_im::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(stark_im::NotifyTypeGadget::CHAT_MESSAGE_NOTIFY);
            notifyMessage.setNewMessageInfo(notifyNewMessage);

            // 序列化
            QByteArray body = notifyMessage.serialize(&this->serializer);

            // 发送消息给客户端
            socket->sendBinaryMessage(body);

            LOG() << "发送图片消息响应";
        });
        connect(this, &WebsocketServer::sendFileResp, this, [=](){
            if(socket == nullptr || !socket->isValid()){
                LOG() << "socket 对象无效";
                return;
            }
            // 构造响应数据
            QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
            stark_im::MessageInfo messageInfo = makeFileMessageInfo(this->messageIndex++, "2001", avatar);  //模拟 张三_1发送的消息

            stark_im::NotifyNewMessage notifyNewMessage;
            notifyNewMessage.setMessageInfo(messageInfo);

            stark_im::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(stark_im::NotifyTypeGadget::CHAT_MESSAGE_NOTIFY);
            notifyMessage.setNewMessageInfo(notifyNewMessage);

            // 序列化
            QByteArray body = notifyMessage.serialize(&this->serializer);

            // 发送消息给客户端
            socket->sendBinaryMessage(body);

            LOG() << "发送文件消息响应";
        });
        connect(this, &WebsocketServer::sendSpeechResp, this, [=](){
            if(socket == nullptr || !socket->isValid()){
                LOG() << "socket 对象无效";
                return;
            }

            // 构造响应数据
            QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
            stark_im::MessageInfo messageInfo = makeSpeechMessageInfo(this->messageIndex++, "2001", avatar);  //模拟 张三_1发送的消息

            stark_im::NotifyNewMessage notifyNewMessage;
            notifyNewMessage.setMessageInfo(messageInfo);

            stark_im::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(stark_im::NotifyTypeGadget::CHAT_MESSAGE_NOTIFY);
            notifyMessage.setNewMessageInfo(notifyNewMessage);

            // 序列化
            QByteArray body = notifyMessage.serialize(&this->serializer);

            // 发送消息给客户端
            socket->sendBinaryMessage(body);

            LOG() << "发送语音消息响应";
        });


        connect(this, &WebsocketServer::sendFriendRemove, this, [=](){
            if(socket == nullptr || !socket->isValid()){
                LOG() <<"socket 对象无效";
                return;
            }

            stark_im::NotifyFriendRemove notifyFriendRemove;
            notifyFriendRemove.setUserId("1000");

            stark_im::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(stark_im::NotifyTypeGadget::FRIEND_REMOVE_NOTIFY);
            notifyMessage.setFriendRemove(notifyFriendRemove);

            QByteArray body = notifyMessage.serialize(&serializer);
            socket->sendBinaryMessage(body);
            LOG() << "通知对方好友被删除 userId=1000";
        });

        connect(this, &WebsocketServer::sendAddFriendApply, this, [=](){
            if(socket == nullptr || !socket->isValid()){
                LOG() <<"socket 对象无效";
                return;
            }

            QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
            stark_im::UserInfo userInfo = makeUserInfo(100, avatar);

            stark_im::NotifyFriendAddApply friendAddApply;
            friendAddApply.setUserInfo(userInfo);

            stark_im::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(stark_im::NotifyTypeGadget::FRIEND_ADD_APPLY_NOTIFY);
            notifyMessage.setFriendAddApply(friendAddApply);

            QByteArray body = notifyMessage.serialize(&serializer);
            socket->sendBinaryMessage(body);
            LOG() <<"通知对方好友申请";
        });

        connect(this, &WebsocketServer::sendAddFriendProcess, this, [=](bool agree){
            if(socket == nullptr || !socket->isValid()){
                LOG() <<"socket 对象无效";
                return;
            }

            QByteArray avatar = loadFileToByteArray(":/image/defaultAvatar.png");
            stark_im::UserInfo userInfo = makeUserInfo(100, avatar);

            stark_im::NotifyFriendAddProcess friendAddProcess;
            friendAddProcess.setAgree(agree);
            friendAddProcess.setUserInfo(userInfo);

            stark_im::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(stark_im::NotifyTypeGadget::FRIEND_ADD_PROCESS_NOTIFY);
            notifyMessage.setFriendProcessResult(friendAddProcess);

            QByteArray body = notifyMessage.serialize(&serializer);
            socket->sendBinaryMessage(body);
            LOG() <<"通知好友申请的处理结果, userId="<<userInfo.userId()<<", agree="<<agree;
        });

        connect(this, &WebsocketServer::sendCreateChatSession, this, [=](){
            if(socket == nullptr || !socket->isValid()){
                LOG() <<"socket 对象无效";
                return;
            }

            QByteArray groupAvatar = loadFileToByteArray(":/image/groupAvatar.png");
            stark_im::ChatSessionInfo chatSessionInfo;
            chatSessionInfo.setChatSessionId("2222");
            chatSessionInfo.setSingleChatFriendId("");
            chatSessionInfo.setAvatar(groupAvatar);
            chatSessionInfo.setChatSessionName("新的群聊会话");
            chatSessionInfo.setPrevMessage(makeTextMessageInfo(2222, "2222", groupAvatar));

            stark_im::NotifyNewChatSession newChatSession;
            newChatSession.setChatSessionInfo(chatSessionInfo);

            stark_im::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(stark_im::NotifyTypeGadget::CHAT_SESSION_CREATE_NOTIFY);
            notifyMessage.setNewChatSessionInfo(newChatSession);

            // 序列化操作
            QByteArray body = notifyMessage.serialize(&serializer);
            // 通过websocket推送数据
            socket->sendBinaryMessage(body);

            LOG() <<"通知创建了一个新的会话";
        });
    });


    // 2.绑定端口，启动服务
    bool ok = websocketServer.listen(QHostAddress::Any, 8001);

    return ok;
}




















