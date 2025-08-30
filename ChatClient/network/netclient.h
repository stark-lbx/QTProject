#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QProtobufSerializer>
#include <QWebSocket>

#include "../model/data.h"
#include "../common/debug.h"

// 此处为了避免 "循环包含" 问题, 就需要使用 前置声明 代替包含头文件
namespace model{
class DataCenter;
} // end namespace model


namespace network{

class NetClient : public QObject
{
    Q_OBJECT
private:
#if DEPOLY
    const QString HTTP_URL = "http://122.51.82.148:9000";
    const QString WEBSOCKET_URL = "ws://122.51.82.148:9001/ws";
#else
    // 定义重要常量, ip都暂时使用本地环回地址, 端口号约定为 8000 和 8001
    const QString HTTP_URL = "http://127.0.0.1:8000";
    const QString WEBSOCKET_URL = "ws://127.0.0.1:8001/ws";
#endif
public:
    explicit NetClient(model::DataCenter *dataCenter = nullptr);

    // 验证网络联通
    void ping();

    // 初始化websocket
    void initWebsocket();

    // 针对websocket的处理
    void handleWsResponse(const stark_im::NotifyMessage& notifyMessage);
    void handleWsMessage(const model::Message& message);
    void handleWsRemoveFriend(const QString& userId);
    void handleWsAddFriendApply(const model::UserInfo& userInfo);
    void handleWsAddFriendProcess(const model::UserInfo& userInfo, bool agree);
    void handleWsChatSessionCreate(const model::ChatSessionInfo& chatSessionInfo);

    // 发送身份认证请求
    void sendAuth();


    // 生成请求ID
    static QString makeRequestId();

    // 封装发送请求的逻辑
    QNetworkReply* sendHttpRequest(const QString& apiPath, const QByteArray& body);
    // 封装处理响应的逻辑( HTTP正确性 反序列化 业务判定)
    template<typename T>
    std::shared_ptr<T> handleHttpResponse(QNetworkReply *httpRsp, bool *ok, QString *reason){
        // a) 先判定HTTP是否出错
        if(httpRsp->error() != QNetworkReply::NoError){
            *ok = false;
            *reason = httpRsp->errorString();

            httpRsp->deleteLater();
            return std::shared_ptr<T>();
        }

        // b) 拿到响应的body
        QByteArray respBody = httpRsp->readAll();

        // c) 针对body反序列化
        std::shared_ptr<T> respObj = std::make_shared<T>();
        respObj->deserialize(&serializer, respBody);

        // d) 业务上是否出错
        if(!respObj->success()){
            *ok = false;
            *reason = respObj->errmsg();

            httpRsp->deleteLater();
            return std::shared_ptr<T>();
        }

        *ok = true;
        httpRsp->deleteLater();
        return respObj;
    }

    void getMyself(const QString& loginSessionId);
    void getFriendList(const QString& loginSessionId);
    void getChatSessionList(const QString& loginSessionId);
    void getApplyList(const QString& loginSessionId);
    void getRecentMessageList(const QString& loginSessionId, const QString& chatSessionId, bool updateUI);

    // 此处的extraInfo 可以用来传递"扩展信息", 尤其对于文件消息来说, 通过这个字段标识"文件名"
    // 其他类型暂时不涉及, 直接设为"", 如果后续有消息类型需要, 都可以给这个参数, 赋予一定的特殊含义
    void sendMessage(const QString& loginSessionId, const QString& chatSessionId,
                     model::MessageType messageType,const QByteArray& content, const QString& extraInfo = "");
    void receiveMessage(const QString& chatSessionId);

    void getVerifyCode(const QString& phone);
    void changePhone(const QString& loginSessionId, const QString &phone, const QString &verifyCodeId, const QString &verifyCode);
    void changeNickname(const QString& loginSessionId, const QString& nickname);
    void changeDescription(const QString& loginSessionId, const QString& description);
    void changeAvatar(const QString& loginSessionId, const QByteArray& avatar);

    void deleteFriend(const QString& loginSessionId, const QString& peerId);
    void addFriendApply(const QString& loginSessionId, const QString& respondentId);
    void accpetFriendApply(const QString& loginSessionId, const QString& applyUserId);
    void rejectFriendApply(const QString& loginSessionId, const QString& applyUserId);

    void createGroupChatSession(const QString& loginSessionId, const QList<QString>& userIdList);
    void getMemberList(const QString& loginSessionId, const QString& chatSessionId);
    void searchUser(const QString& loginSessionId, const QString& keyword);

    void searchMessage(const QString& loginSessionId, const QString& chatSessionId, const QString& searchKey);
    void searchMessage(const QString& loginSessionId, const QString& chatSessionId, const QDateTime& begTime, const QDateTime& endTime);

    void userLogin(const QString& username, const QString& password);
    void userRegister(const QString& username, const QString& password);

    void phoneLogin(const QString& phone, const QString& verifyCodeId, const QString& verifyCode);
    void phoneRegister(const QString& phone, const QString& verifyCodeId, const QString& verifyCode);

    void getSingleFile(const QString& loginSessionId, const QString& fileId);
    void speechConvertText(const QString& loginSessionId, const QString& fileId, const QByteArray& content);

private:
    model::DataCenter* dataCenter;
    QNetworkAccessManager httpClient;       // http 客户端
    QWebSocket websocketClient;             // websocket 客户端
    QProtobufSerializer serializer;         // protobuf 序列化器


};


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


} // end namespace network
#endif // NETCLIENT_H
