#ifndef SERVER_H
#define SERVER_H

#include <QFileInfo>
#include <QHttpServer>
#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QProtobufSerializer>
#include <QWebSocketServer>


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 工具函数：后续很多模块可能都要用到
////////////////////////////////////////////////////////////////////////////////////////////////////////


// 从一个完整路径中提取文件名
static inline QString getFileName(const QString& path){
    return QFileInfo(path).fileName();
    // QFileInfo fileInfo(path);
    // return fileInfo.fileName();
}

// 封装一个 宏 作为打印日志的方式
#define TAG QString("[%1: %2]").arg(getFileName(__FILE__),QString::number(__LINE__))

// qDebug 打印字符串的时候, 自动加上日志
#define LOG() qDebug().noquote() << TAG

// 要求函数的定义写在 .h 中, 必须加 static 或者 inline (二者都加也可以), 避免链接阶段出现 "函数重定义" 的问题
static inline QString formatTime(int64_t timestamp){
    // 先把时间戳转换成 QDateTime 对象
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
    // 把 QDateTime 对象转换成 "格式化时间"
    return dateTime.toString("MM-dd HH:mm:ss");
}

// 获取秒级时间戳
static inline int64_t getTime(){
    return QDateTime::currentSecsSinceEpoch();
}

// 根据 QByteArray, 转换成 QIcon
static inline QIcon makeIcon(const QByteArray& byteArray){
    QPixmap pixmap;
    pixmap.loadFromData(byteArray);

    return QIcon(pixmap);
}

// 文件读写操作
// 从指定文件中, 读取所有的二进制内容, 得到一个 QByteArray
static inline QByteArray loadFileToByteArray(const QString& path){
    QFile file(path);
    if(!file.open(QFile::ReadOnly)){
        LOG() << "文件打开失败!";
        return QByteArray();
    }

    QByteArray content = file.readAll();
    file.close();
    return content;
}

// 把 QByteArray 中的内容, 写入到某个指定文件中
static inline void writeByteArrayToFile(const QString& path, const QByteArray& content){
    QFile file(path);
    if(!file.open(QFile::WriteOnly)){
        LOG() << "文件打开失败!";
        return;
    }

    file.write(content);
    file.flush();
    file.close();
}


////////////////////////////////////////////////////
/// HTTP服务器
////////////////////////////////////////////////////
class HttpServer : public QObject{
    Q_OBJECT
private:
    static HttpServer* instance;
    HttpServer() = default;

    QHttpServer httpServer;
    QProtobufSerializer serializer;
public:
    static HttpServer* getInstance();

    // 针对Http 服务器进行初始化
    bool init();

    // 1.获取个人信息
    QHttpServerResponse getUserInfo(const QHttpServerRequest &req);

    // 2.获取好友列表
    QHttpServerResponse getFriendList(const QHttpServerRequest &req);

    // 3.获取会话列表
    QHttpServerResponse getChatSessionList(const QHttpServerRequest &req);

    // 4.获取申请列表
    QHttpServerResponse getApplyList(const QHttpServerRequest &req);

    // 5.获取最近消息列表
    QHttpServerResponse getRecentMessageList(const QHttpServerRequest &req);

    // 6.处理发送消息
    QHttpServerResponse newMessage(const QHttpServerRequest &req);

    // 7.修改用户昵称
    QHttpServerResponse setNickname(const QHttpServerRequest &req);

    // 8.修改用户签名
    QHttpServerResponse setDescription(const QHttpServerRequest &req);

    // 9.获取短信验证码
    QHttpServerResponse getPhoneVerifyCode(const QHttpServerRequest &req);

    // 10.修改用户手机号
    QHttpServerResponse setPhone(const QHttpServerRequest &req);

    // 11.修改用户头像
    QHttpServerResponse setAvatar(const QHttpServerRequest &req);

    // 12.删除好友
    QHttpServerResponse removeFriend(const QHttpServerRequest &req);

    // 13.添加好友申请
    QHttpServerResponse addFriendApply(const QHttpServerRequest &req);

    // 14.处理好友申请
    QHttpServerResponse addFriendProcess(const QHttpServerRequest &req);

    // 15.创建会话
    QHttpServerResponse createChatSession(const QHttpServerRequest &req);

    // 16.获取会话成员列表
    QHttpServerResponse getChatSessionMember(const QHttpServerRequest &req);

    // 17.搜索用户
    QHttpServerResponse searchFriend(const QHttpServerRequest &req);

    // 18.搜索历史消息
    QHttpServerResponse searchHistory(const QHttpServerRequest &req);

    // 19.获取指定时间的历史消息
    QHttpServerResponse getHistory(const QHttpServerRequest &req);

    // 20.用户名登录
    QHttpServerResponse userLogin(const QHttpServerRequest &req);

    // 21.用户名注册
    QHttpServerResponse userRegister(const QHttpServerRequest &req);

    // 22.手机号登录
    QHttpServerResponse phoneLogin(const QHttpServerRequest &req);

    // 23.手机号注册
    QHttpServerResponse phoneRegister(const QHttpServerRequest &req);

    // 24.获取单个文件
    QHttpServerResponse getSingleFile(const QHttpServerRequest &req);

    // 25.语音转文字
    QHttpServerResponse recognition(const QHttpServerRequest &req);

};



////////////////////////////////////////////////////
/// Websocket服务器
////////////////////////////////////////////////////

class WebsocketServer : public QObject {
    Q_OBJECT

private:
    static WebsocketServer* instance;
    WebsocketServer();

    QWebSocketServer websocketServer;
    QProtobufSerializer serializer;

    uint messageIndex = 0;
public:
    static WebsocketServer* getInstance();

    // 针对WebSocket 服务器进行初始化
    bool init();

signals:
    void sendTextResp();
    void sendImageResp();
    void sendFileResp();
    void sendSpeechResp();

    void sendFriendRemove();
    void sendAddFriendApply();
    void sendAddFriendProcess(bool agree);
    void sendCreateChatSession();
};

#endif // SERVER_H
