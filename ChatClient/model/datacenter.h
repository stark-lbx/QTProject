#ifndef DATACENTER_H
#define DATACENTER_H

#include <QObject>

#include "data.h"
#include "../network/netclient.h"

namespace model{


////////////////////////////////////////////////////////////
///
/// 管理所有客户端需要的数据, 这是一个单例类
///
////////////////////////////////////////////////////////////

class DataCenter : public QObject
{
    Q_OBJECT
private:
    // 当前客户端 登录到服务器 的 登录会话id - Cookie
    // 这个信息是登录成功后，服务器返回的
    // 返回之后这个数据需要被客户端永久保存在文件中
    // 后续每次启动, 都从这个文件拿到这个内容
    // 当用户显式点击 "退出登陆", 则删除这个信息
    QString loginSessionId = ""; // 持久化存储

    // 当前登录用户的用户信息
    model::UserInfo* myself = nullptr;

    // 当前用户的好友列表
    QList<model::UserInfo>* friendList = nullptr;

    // 当前用户的会话列表
    QList<model::ChatSessionInfo>* chatSessionList = nullptr;
    // 记录当前的会话是哪个?
    /* new */QString currentChatSessionId = "";
    // 每个会话中的用户列表 - 主要针对群聊
    QHash<QString, QList<model::UserInfo>>* memberList = nullptr;

    // 待处理的好友申请列表
    /* new */QList<model::UserInfo>* applyList = nullptr;

    // 与当前用户的最近聊天消息内容, key 为 ChatSessionInfo 的 chatSessionId
    QHash<QString, QList<Message>>* recentMessages = nullptr;

    // 未读消息数目的统计, key 为 ChatSessionInfo 的 chatSessionId
    QHash<QString, int>* unreadMessageCount = nullptr; // 持久化存储

    // 用户搜索结果
    QList<model::UserInfo>* searchUserResult = nullptr;

    // 历史消息搜索结果
    QHash<QString, QList<model::Message>>* searchMessageResult = nullptr;

    // 当前短信验证码的验证 id
    QString currentVerifyCodeId = "";

    // 持有网络通信客户端
    network::NetClient netClient;

    // 本次登录存储的未读消息会话集合
    QSet<QString>* currentUnreadChatSession = nullptr;

    // 构造单例模式
    static DataCenter* instance;
    DataCenter();
    DataCenter(const DataCenter&)=delete;
    DataCenter& operator=(const DataCenter&)=delete;
public:
    static DataCenter* getInstance();
    ~DataCenter();

    // 初始化数据文件
    void initDataFile();
    // 保存必要的数据到文件
    void saveDataFile();
    // 从文件加载必要的数据
    void loadDataFile();

    // 清空未读消息数目
    void clearUnread(const QString& chatSessionId);
    // 增加未读消息数目
    void addUnread(const QString& chatSessionId);
    // 获取未读消息数目
    uint getUnread(const QString& chatSessionId);
    // 获取本次登录 未读消息集合
    QSet<QString>* getCurrentUnreadChatSession();

    // 获取到当前的登录会话ID
    const QString& getLoginSessionId() { return loginSessionId; }

    // 验证网络连通性
    void ping() { netClient.ping(); }
    // 初始化 websocket
    void initWebsocket(){ netClient.initWebsocket(); }


    ////////////////////////////////////////////////
    /// 核心函数
    ////////////////////////////////////////////////

    // 通过网络获取用户的个人信息(async: 异步)
    void getMyselfAsync();
    model::UserInfo* getMyself();
    void resetMyself(std::shared_ptr<stark_im::GetUserInfoRsp> resp);

    // 加载好友列表
    void getFriendListAsync();
    QList<model::UserInfo>* getFriendList();
    void resetFriendList(std::shared_ptr<stark_im::GetFriendListRsp> resp);

    // 加载聊天会话列表
    void getChatSessionListAsync();
    QList<ChatSessionInfo>* getChatSessionList();
    void resetChatSessionList(std::shared_ptr<stark_im::GetChatSessionListRsp> resp);

    // 加载好友申请列表
    void getApplyListAsync();
    QList<UserInfo>* getApplyList();
    void resetApplyList(std::shared_ptr<stark_im::GetPendingFriendEventListRsp> resp);

    // 获取近期消息列表
    void getRecentMessageListAsync(const QString& chatSessionId, bool updateUI = true);
    QList<Message>* getRecentMessageList(const QString& chatSessionId);
    void resetRecentMessageList(const QString& chatSessionId, std::shared_ptr<stark_im::GetRecentMsgRsp> resp);

    // 发送消息给服务器
    void sendTextMessageAsync(const QString& chatSessionId, const QString& content);
    void sendImageMessageAsync(const QString& chatSessionId, const QByteArray& content);
    void sendFileMessageAsync(const QString& chatSessionId, const QString& filename, const QByteArray& content);
    void sendSpeechMessageAsync(const QString& chatSessionId, const QByteArray& content);

    // 修改用户昵称
    void changeNicknameAsync(const QString& nickname);
    void resetNickname(const QString& nickname);

    // 修改用户签名
    void changeDescriptionAsync(const QString& desc);
    void resetDescription(const QString& desc);

    // 获取短信验证码
    void getVerifyCodeAsync(const QString& phone);
    void resetVerifyCodeId(const QString& verifyCodeId);
    const QString& getVerifyCodeId();// 获取VerifyCodeId

    // 修改用户手机号
    void changePhoneAsync(const QString& phone, const QString& verifyCodeId, const QString& verifyCode);
    void resetPhone(const QString& phone);

    // 修改头像
    void changeAvatarAsync(const QByteArray& avatar);
    void resetAvatar(const QByteArray& avatar);

    // 删除好友
    void deleteFriendAsync(const QString& userId);
    void removeFriend(const QString& userId);

    // 发送好友申请
    void addFriendApplyAsync(const QString& userId);
    void accpetFriendApplyAsync(const QString& userId);
    model::UserInfo removeFromApplyList(const QString& userId);
    void rejectFriendApplyAsync(const QString& userId);

    // 创建群聊会话
    void createGroupChatSessionAsync(const QList<QString>& userIdList);
    // 加载群聊成员列表
    void getMemberListAsync(const QString& chatSessionId);
    void resetMemberList(const QString& chatSessionId, const stark_im::UserInfoRepeated& memberInfoList);
    QList<model::UserInfo>* getMemberList(const QString& chatSessionId);

    // 搜索用户
    void searchUserAsync(const QString& keyword);
    void resetSearchUserResult(const stark_im::UserInfoRepeated& userList);
    QList<model::UserInfo>* getSearchUserResult();

    // 聊天记录搜索
    void searchMessageAsync(const QString& searchKey);
    void resetSearchMessageResult(const QString& chatSessionId, const stark_im::MessageInfoRepeated& msgList);
    QList<Message>* getSearchMessageResult(const QString& chatSessionId);
    void searchMessageAsync(const QDateTime& begTime, const QDateTime& endTime);

    // 登录注册
    void userLoginAsync(const QString& username, const QString& password);
    void resetLoginSessionId(const QString& loginSessionId);
    void userRegisterAsync(const QString& username, const QString& password);

    void phoneLoginAsync(const QString& phone, const QString& verifyCode);
    void phoneRegisterAsync(const QString& phone, const QString& verifyCode);

    // 获取单个文件
    void getSingleFileAsync(const QString& fileId);
    // 语音转文字
    void speechConvertTextAsync(const QString& fileId, const QByteArray& content);

    ////////////////////////////////////////////////
    /// 辅助函数
    ////////////////////////////////////////////////

    // 根据会话ID查询会话信息
    model::ChatSessionInfo* findChatSessionById(const QString& chatSessionId);
    // 根据用户ID查询会话信息
    model::ChatSessionInfo* findChatSessionByUserId(const QString& userId);
    // 根据好友ID查询好友信息
    model::UserInfo* findFriendInfoByUserId(const QString& userId);
    // 将指定的会话信息置顶
    void topChatSessionInfo(const model::ChatSessionInfo& info);

    // 根据好友ID查询好友项
    UserInfo* findFriendByUserId(const QString& userId);

    // 设置/获取当前选中的会话ID
    void setCurrentChatSessionId(const QString& chatSessionId);
    const QString& getCurrentChatSessionId();

    // 添加一个消息到DataCenter中
    void addMessage(const model::Message& message);


signals:
    void getMyselfDone();
    void getFriendListDone();
    void getChatSessionListDone();
    void getApplyListDone();
    void getRecentMessageListDone(const QString& chatSessionId);
    void getRecentMessageListDoneNoUI(const QString& chatSessionId);
    void sendMessageDone(model::MessageType messageType, const QByteArray& content, const QString& extraInfo);

    void updateLastMessage(const QString& chatSessionId);
    void receiveMessageDone(const model::Message& lastMessage);

    void changeNicknameDone();
    void changeDescriptionDone();
    void getVerifyCodeDone();
    void changePhoneDone();
    void changeAvatarDone();

    void deleteFriendDone();
    void clearCurrentSession();
    void addFriendApplyDone();
    void receiveFriendApplyDone();
    void acceptFriendApplyDone();
    void receiveFriendProcessDone(const model::UserInfo& userInfo,bool agree);
    void rejectFriendApplyDone();

    void createGroupChatSessionDone();
    void receiveSessionCreateDone();
    void getMemberListDone(const QString &chatSessionId);
    void searchUserDone();
    void searchMessageDone(const QString &chatSessionId);

    void userLoginDone(bool ok, const QString& reason);
    void userRegisterDone(bool ok, const QString& reason);
    void phoneLoginDone(bool ok, const QString& reason);
    void phoneRegisterDone(bool ok, const QString& reason);
    void getSingleFileDone(const QString& fileId, const QByteArray& fileContent);
    void speechConvertTextDone(const QString& fileId, const QString& content);

    // 自增拓展信号
    void trayIconBlink();
    void trayIconBlinkDone();
};

}   // end namspace model
#endif // DATACENTER_H
