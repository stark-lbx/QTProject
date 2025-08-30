#include "datacenter.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace model{

DataCenter* DataCenter::instance = nullptr;

DataCenter::DataCenter()
    :netClient(this)
{
    // 这几个 哈希表 提前把对象 new 好，其它QList类型的属性，都暂时不实例化
    // 主要是为了使用 nullptr 表示"非法状态"
    // 对于hash来说, 不关心整个 QHash 是否是 nullptr, 而是关心某个 key 对应的 value 是否存在
    // 通过key是否存在，也能表示该值是否有效
    recentMessages = new QHash<QString, QList<model::Message>>;
    memberList = new QHash<QString, QList<model::UserInfo>>;
    unreadMessageCount = new QHash<QString, int>;
    searchMessageResult = new QHash<QString, QList<model::Message>>;

    // 加载数据 从文件中
    loadDataFile();
}

DataCenter* DataCenter::getInstance(){
    if(instance == nullptr) {
        instance = new DataCenter();
    }
    return instance;
}

DataCenter::~DataCenter()
{
    // C++标准明确规定，对nullptr进行delete 是合法行为，不会有任何副作用
    delete myself;
    delete friendList;
    delete chatSessionList;
    delete memberList;
    delete recentMessages;
    delete searchMessageResult;
    delete unreadMessageCount;
    delete searchUserResult;
}

//////////////////////////////////////////////////
/// 使用文件存储 sessionId 和 未读消息信息
//////////////////////////////////////////////////

void DataCenter::initDataFile()
{
    // 拼装一个文件目录, 作为 appData 作为目录
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = basePath + "/ChatClient.json";

    QDir dir;
    if(!dir.exists(basePath)){
        dir.mkpath(basePath);
    }

    // 打开文件
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qCritical() << TAG << "Cannot open file:" << file.errorString() << filePath;
        return;
    }

    // 写入初始内容
    QString data = "{\n\n}";
    file.write(data.toUtf8());
    file.close();
}

void DataCenter::saveDataFile()
{
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/ChatClient.json";

    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qCritical() << TAG << "Cannot open file:" << file.errorString() << filePath;
        return;
    }

    // 按照 json 格式来写入数据
    // 这个对象就可以当作map一样来使用
    QJsonObject jsonObj;
    jsonObj["loginSessionId"] = loginSessionId;

    QJsonObject jsonUnread;
    for(auto it = unreadMessageCount->begin(); it != unreadMessageCount->end(); ++it){
        // Qt 的迭代器使用细节和 STL 略有差别，此处不是使用 first / second 的方式
        jsonUnread[it.key()] = it.value();
    }
    jsonObj["unread"] = jsonUnread;

    // 把 json 写入文件了
    QJsonDocument jsonDoc(jsonObj);
    QString s = jsonDoc.toJson();
    file.write(s.toUtf8());

    // 关闭文件
    file.close();
}

// 加载文件, 是在 DataCenter 被实例化的时候, 调用执行的
void DataCenter::loadDataFile()
{
    // 确保在加载之前, 先针对文件进行初始化操作.
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/ChatClient.json";

    // 判定文件是否存在, 不存在则初始化, 并创建出新的空白的 json 文件
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        initDataFile();
    }

    // 读方式打开文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG() << "打开文件失败! " << file.errorString();
        return;
    }

    // 读取到文件内容, 解析为 JSON 对象
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    if (jsonDoc.isNull()) {
        LOG() << "解析 JSON 文件失败! JSON 文件格式有错误!";
        file.close();
        return;
    }

    // 获取Json中的属性
    QJsonObject jsonObj = jsonDoc.object();
    this->loginSessionId = jsonObj["loginSessionId"].toString();

    this->unreadMessageCount->clear();
    QJsonObject jsonUnread = jsonObj["unread"].toObject();
    for (auto it = jsonUnread.begin(); it != jsonUnread.end(); ++it) {
        this->unreadMessageCount->insert(it.key(), it.value().toInt());
    }

    // 检查读到的内容是否正确
    if(loginSessionId == ""){
        LOG() << "读到的 sessionId 为空! loginSessionId=" << this->loginSessionId;
    }

    // 关闭文件
    file.close();
}

void DataCenter::clearUnread(const QString &chatSessionId)
{
    (*unreadMessageCount)[chatSessionId] = 0;
    // 手动保存一下结果到文件中
    saveDataFile();

    if(currentUnreadChatSession==nullptr) return;
    if(currentUnreadChatSession->contains(chatSessionId)){
        currentUnreadChatSession->remove(chatSessionId);
        if(currentUnreadChatSession->count() == 0){
            emit trayIconBlinkDone();
        }
    }
}

void DataCenter::addUnread(const QString &chatSessionId)
{
    ++(*unreadMessageCount)[chatSessionId];
    // 手动保存一下结果到文件中
    saveDataFile();

    if(currentUnreadChatSession==nullptr){
        currentUnreadChatSession = new QSet<QString>;
    }
    currentUnreadChatSession->insert(chatSessionId);
    emit trayIconBlink();
}

uint DataCenter::getUnread(const QString &chatSessionId)
{
    return (*unreadMessageCount)[chatSessionId];
}

QSet<QString> *DataCenter::getCurrentUnreadChatSession()
{
    return currentUnreadChatSession;
}

const QString &DataCenter::getVerifyCodeId()
{
    return currentVerifyCodeId;
}

void DataCenter::changePhoneAsync(const QString &phone, const QString &verifyCodeId, const QString &verifyCode)
{
    netClient.changePhone(loginSessionId, phone, verifyCodeId, verifyCode);
}

void DataCenter::resetPhone(const QString &phone)
{
    if(myself == nullptr){
        return;
    }
    myself->phone = phone;
}

void DataCenter::changeAvatarAsync(const QByteArray &avatar)
{
    netClient.changeAvatar(loginSessionId, avatar);
}

void DataCenter::resetAvatar(const QByteArray &avatar)
{
    if(myself == nullptr){
        return;
    }
    myself->avatar = model::makeIcon(avatar);
}

void DataCenter::deleteFriendAsync(const QString &userId)
{
    netClient.deleteFriend(loginSessionId, userId);
}

void DataCenter::removeFriend(const QString &userId)
{
    // 遍历FriendList, 删除其中匹配的元素
    if(friendList == nullptr){
        return;
    }
    friendList->removeIf([=](const UserInfo& u) {
        return u.userId == userId;
    });

    // 还要考虑会话列表, 删除对应的会话列表
    // 客户端删除、服务器也要删除，在删除好友的请求中，服务器就处理了
    if(chatSessionList == nullptr){
        return;
    }

    QString removeChatSessionId = "";
    chatSessionList->removeIf([=, &removeChatSessionId](const ChatSessionInfo& c){
        if(c.userId == "") return false;// 群聊, 忽略
        if(c.userId == userId){ //指定userId与c.userId相同，就删除这个c
            removeChatSessionId = c.chatSessionId;
            LOG() <<"removeChatSessionId => "<<removeChatSessionId;
            // 如果此处删除的会话就是用户当前打开的会话, 特殊处理
            if(this->currentChatSessionId == c.chatSessionId){
                emit this->clearCurrentSession();
                this->currentChatSessionId = "";
            }
            return true;
        }
        return false;
    });

    // 在数据上, 删除与该用户的最近聊天内容、删除
    if(removeChatSessionId != ""){
        this->unreadMessageCount->remove(removeChatSessionId);  /*[stark_add]*/
        this->recentMessages->remove(removeChatSessionId);      /*[stark_add]*/
    }
}

void DataCenter::addFriendApplyAsync(const QString &userId)
{
    netClient.addFriendApply(loginSessionId, userId);
}

void DataCenter::accpetFriendApplyAsync(const QString &userId)
{
    netClient.accpetFriendApply(loginSessionId, userId);
}

UserInfo DataCenter::removeFromApplyList(const QString &userId)
{
    UserInfo toDelete = UserInfo();
    if(applyList == nullptr){
        return toDelete;
    }

    for(auto it = applyList->begin(); it != applyList->end(); ++it){
        if(it->userId == userId){
            // 拷贝一下要删除的对象
            toDelete = *it;
            applyList->erase(it);
            break;
        }
    }
    return toDelete;
}

void DataCenter::rejectFriendApplyAsync(const QString &userId)
{
    netClient.rejectFriendApply(loginSessionId, userId);
}

void DataCenter::createGroupChatSessionAsync(const QList<QString> &userIdList)
{
    netClient.createGroupChatSession(loginSessionId, userIdList);
}

void DataCenter::getMemberListAsync(const QString &chatSessionId)
{
    netClient.getMemberList(loginSessionId, chatSessionId);
}

void DataCenter::resetMemberList(const QString &chatSessionId, const stark_im::UserInfoRepeated &memberInfoList)
{
    QList<UserInfo>& ulist = (*memberList)[chatSessionId];
    for(int i=0; i<memberInfoList.count(); ++i){
        UserInfo userInfo;
        userInfo.load(memberInfoList[i]);
        ulist.push_back(userInfo);
    }
}

QList<UserInfo> *DataCenter::getMemberList(const QString &chatSessionId)
{
    if(!memberList->contains(chatSessionId)){
        return nullptr;
    }
    return &(*memberList)[chatSessionId];
}

void DataCenter::searchUserAsync(const QString &keyword)
{
    netClient.searchUser(loginSessionId, keyword);
}

void DataCenter::resetSearchUserResult(const stark_im::UserInfoRepeated &userList)
{
    if(searchUserResult == nullptr){
        searchUserResult = new QList<model::UserInfo>;
    }
    searchUserResult->clear();

    // 填充结果
    for(const auto& u : userList){
        model::UserInfo userInfo;
        userInfo.load(u);
        searchUserResult->push_back(userInfo);
    }
}

QList<UserInfo> *DataCenter::getSearchUserResult()
{
    return this->searchUserResult;
}

void DataCenter::searchMessageAsync(const QString &searchKey)
{
    netClient.searchMessage(loginSessionId, this->currentChatSessionId, searchKey);
}

void DataCenter::resetSearchMessageResult(const QString &chatSessionId, const stark_im::MessageInfoRepeated &msgList)
{
    QList<model::Message>* messageList = &(*searchMessageResult)[chatSessionId];
    messageList->clear();

    for(const auto& m: msgList){
        Message message;
        message.load(m);
        messageList->push_back(message);
    }
}

QList<Message> *DataCenter::getSearchMessageResult(const QString &chatSessionId)
{
    return &(*searchMessageResult)[chatSessionId];
}

void DataCenter::searchMessageAsync(const QDateTime &begTime, const QDateTime &endTime)
{
    netClient.searchMessage(loginSessionId, this->currentChatSessionId, begTime, endTime);
}

void DataCenter::userLoginAsync(const QString &username, const QString &password)
{
    netClient.userLogin(username, password);
}

void DataCenter::resetLoginSessionId(const QString &loginSessionId_)
{
    this->loginSessionId = loginSessionId_;
    // 一旦会话id改变, 那么就要保存到硬盘上
    saveDataFile();
}

void DataCenter::userRegisterAsync(const QString &username, const QString &password)
{
    netClient.userRegister(username, password);
}

void DataCenter::phoneLoginAsync(const QString &phone, const QString &verifyCode)
{
    netClient.phoneLogin(phone, this->currentVerifyCodeId, verifyCode);
}

void DataCenter::phoneRegisterAsync(const QString &phone, const QString &verifyCode)
{
    netClient.phoneRegister(phone, this->currentVerifyCodeId, verifyCode);
}

void DataCenter::getSingleFileAsync(const QString &fileId)
{
    netClient.getSingleFile(loginSessionId, fileId);
}

void DataCenter::speechConvertTextAsync(const QString &fileId, const QByteArray &content)
{
    netClient.speechConvertText(loginSessionId, fileId, content);
}


//////////////////////////////////////////////////
/// 加载个人信息数据
//////////////////////////////////////////////////

void DataCenter::getMyselfAsync()
{
    // DataCenter 只负责处理数据(中转站)
    // 网络通信使用 NetClient
    netClient.getMyself(loginSessionId);
}

UserInfo *DataCenter::getMyself()
{
    return myself;
}

void DataCenter::resetMyself(std::shared_ptr<stark_im::GetUserInfoRsp> resp)
{
    if(myself == nullptr){
        myself = new UserInfo();
    }

    // 取出resp信息
    const stark_im::UserInfo& userInfo = resp->userInfo();

    // 将protobuf中的UserInfo转化为UserInfo
    myself->load(userInfo);
}


//////////////////////////////////////////////////
/// 加载好友列表数据
//////////////////////////////////////////////////

void DataCenter::getFriendListAsync()
{
    netClient.getFriendList(loginSessionId);
}

QList<UserInfo> *DataCenter::getFriendList()
{
    return friendList;
}

void DataCenter::resetFriendList(std::shared_ptr<stark_im::GetFriendListRsp> resp)
{
    if (friendList == nullptr) {
        friendList = new QList<UserInfo>();
    }
    friendList->clear();

    QList<stark_im::UserInfo>& friendListPB = resp->friendList();
    for (auto& f : friendListPB) {
        UserInfo userInfo;
        userInfo.load(f);
        friendList->push_back(userInfo);
    }
}


//////////////////////////////////////////////////
/// 加载会话列表数据
//////////////////////////////////////////////////

void DataCenter::getChatSessionListAsync()
{
    netClient.getChatSessionList(loginSessionId);
}

QList<ChatSessionInfo> *DataCenter::getChatSessionList()
{
    return chatSessionList;
}

void DataCenter::resetChatSessionList(std::shared_ptr<stark_im::GetChatSessionListRsp> resp)
{
    if(chatSessionList == nullptr){
        chatSessionList = new QList<ChatSessionInfo>();
    }
    chatSessionList->clear();

    auto& chatSessionListPB = resp->chatSessionInfoList();
    for(auto& c: chatSessionListPB){
        ChatSessionInfo chatSessionInfo;
        chatSessionInfo.load(c);
        chatSessionList->push_back(chatSessionInfo);
    }
}

//////////////////////////////////////////////////
/// 加载好友申请列表的数据
//////////////////////////////////////////////////

void DataCenter::getApplyListAsync()
{
    netClient.getApplyList(loginSessionId);
}

QList<UserInfo> *DataCenter::getApplyList()
{
    return applyList;
}

void DataCenter::resetApplyList(std::shared_ptr<stark_im::GetPendingFriendEventListRsp> resp)
{
    if(applyList == nullptr){
        applyList = new QList<UserInfo>();
    }
    applyList->clear();

    auto& applyListPB = resp->event();
    for(auto& a: applyListPB){
        UserInfo u;
        u.load(a.sender());
        applyList->push_back(u);
    }
}

//////////////////////////////////////////////////
/// 加载最近消息列表的数据
//////////////////////////////////////////////////

void DataCenter::getRecentMessageListAsync(const QString &chatSessionId, bool updateUI)
{
    netClient.getRecentMessageList(loginSessionId, chatSessionId, updateUI);
}

QList<Message> *DataCenter::getRecentMessageList(const QString &chatSessionId)
{
    if(!recentMessages->contains(chatSessionId)){
        return nullptr;
    }
    return &(*recentMessages)[chatSessionId];
}

void DataCenter::resetRecentMessageList(const QString &chatSessionId, std::shared_ptr<stark_im::GetRecentMsgRsp> resp)
{
    // 拿到chatSessionId对应的消息列表，并清空
    QList<Message>& messageList = (*recentMessages)[chatSessionId];
    messageList.clear();

    // 遍历响应结果的列表
    for(auto& m: resp->msgList()){
        Message message;
        message.load(m);

        messageList.push_back(message);
    }
}

void DataCenter::sendTextMessageAsync(const QString &chatSessionId, const QString &content)
{
    // sendMessage 一个方法支持四种不同的消息
    netClient.sendMessage(loginSessionId, chatSessionId, MessageType::TEXT_TYPE, content.toUtf8());
}

void DataCenter::sendImageMessageAsync(const QString &chatSessionId, const QByteArray &content)
{
    netClient.sendMessage(loginSessionId, chatSessionId, MessageType::IMAGE_TYPE, content);
}

void DataCenter::sendFileMessageAsync(const QString &chatSessionId, const QString &filename, const QByteArray &content)
{
    netClient.sendMessage(loginSessionId, chatSessionId, MessageType::FILE_TYPE, content, filename);
}

void DataCenter::sendSpeechMessageAsync(const QString &chatSessionId, const QByteArray &content)
{
    netClient.sendMessage(loginSessionId, chatSessionId, MessageType::SPEECH_TYPE, content);
}

void DataCenter::changeNicknameAsync(const QString &nickname)
{
    netClient.changeNickname(loginSessionId, nickname);
}

void DataCenter::resetNickname(const QString &nickname)
{
    if(myself == nullptr) return;
    myself->nickname = nickname;
}

void DataCenter::changeDescriptionAsync(const QString &desc)
{
    netClient.changeDescription(loginSessionId, desc);
}

void DataCenter::resetDescription(const QString &desc)
{
    if(myself == nullptr) return;
    myself->description = desc;
}

void DataCenter::getVerifyCodeAsync(const QString &phone)
{
    // 这个操作不需要传入loginSessionId
    // 后续还需要实现通过手机验证码登录
    // 登陆的时候, 没有loginSessionId
    netClient.getVerifyCode(phone);
}

void DataCenter::resetVerifyCodeId(const QString &verifyCodeId)
{
    this->currentVerifyCodeId = verifyCodeId;
}




ChatSessionInfo *DataCenter::findChatSessionById(const QString &chatSessionId)
{
    if(chatSessionList != nullptr) {
        for(auto& info: *chatSessionList){
            if(info.chatSessionId == chatSessionId){
                return &info;
            }
        }
    }
    return nullptr;
}

ChatSessionInfo *DataCenter::findChatSessionByUserId(const QString &userId)
{
    if(chatSessionList != nullptr) {
        for(auto& info: *chatSessionList){
            if(info.userId == userId){
                return &info;
            }
        }
    }
    return nullptr;
}

UserInfo *DataCenter::findFriendInfoByUserId(const QString &userId)
{
    if(friendList != nullptr){
        for(auto& info: *friendList){
            if(info.userId == userId){
                return &info;
            }
        }
    }
    return nullptr;
}

void DataCenter::setCurrentChatSessionId(const QString &chatSessionId)
{
    this->currentChatSessionId = chatSessionId;
}

const QString& DataCenter::getCurrentChatSessionId()
{
    return this->currentChatSessionId;
}

void DataCenter::addMessage(const Message &message)
{
    (*recentMessages)
        [message.chatSessionId]
        .push_back(message);
}

void DataCenter::topChatSessionInfo(const ChatSessionInfo &info)
{
    if(chatSessionList == nullptr) return;

    // 1.把这个元素中找到
    auto it = chatSessionList->begin();
    for(; it!=chatSessionList->end(); ++it){
        if(it->chatSessionId == info.chatSessionId){
            break;
        }
    }
    if(it == chatSessionList->end()){
        // 上面的循环没有找到匹配的元素, 直接返回
        // 正常来说, 不会找不到
        return;
    }

    // 2.把这个元素备份一下, 然后删除
    ChatSessionInfo backup = info;
    chatSessionList->erase(it);

    // 3.将备份的元素, 插入到头部
    chatSessionList->push_front(backup);
}

UserInfo *DataCenter::findFriendByUserId(const QString &userId)
{
    if(this->friendList != nullptr){
        for(auto& f : *friendList){
            if(f.userId == userId){
                return &f;
            }
        }
    }
    return nullptr;
}
























} // end namespace model
