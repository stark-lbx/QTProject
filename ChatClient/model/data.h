#pragma once

#include <QString>
#include <QIcon>
#include <QUuid>
#include <QFileInfo>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

#include "base.qpb.h"
#include "gateway.qpb.h"
#include "user.qpb.h"
#include "friend.qpb.h"
#include "file.qpb.h"
#include "notify.qpb.h"
#include "speech_recognition.qpb.h"
#include "message_storage.qpb.h"
#include "message_transmit.qpb.h"


// 创建命名空间
namespace model{

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
#define TAG QString("[%1: %2]").arg(model::getFileName(__FILE__),QString::number(__LINE__))

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


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 用户信息
///
/// 用户编号：字符串的方式作为Id，可以有更灵活的方式来生成（后续可以通过uuId、雪花算法来生成分布式系统的唯一Id）
////////////////////////////////////////////////////////////////////////////////////////////////////////

class UserInfo{
public:
    QString userId = "";        //用户编号
    QString nickname = "";      //用户昵称
    QString description = "";   //用户签名
    QString phone = "";         //手机号码
    QIcon   avatar;             //用户头像

    // 从 protobuffer 的 UserInfo 对象, 转成当前代码的 UserInfo 对象
    void load(const stark_im::UserInfo& userInfo){
        this->userId = userInfo.userId();
        this->nickname = userInfo.nickname();
        this->description = userInfo.description();
        this->phone = userInfo.phone();

        // if(userInfo.avatar().isEmpty()){
        //     this->avatar = QIcon(":/resource/image/defaultAvatar.png");
        // } else {
        //     this->avatar = makeIcon(userInfo.avatar());
        // }
        this->avatar = userInfo.avatar().isEmpty()?
                           QIcon(":/resource/image/defaultAvatar.png"):
                           makeIcon(userInfo.avatar());
    }

    bool operator==(const UserInfo& oth) const {return userId == oth.userId;}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 消息信息
/// 会话-消息：一对多的关系
///
/// 消息时间：通过格式化方式表示
/// 消息内容：如果是图片、文件、语音，正文就是一个“二进制序列”
/// 文件标识：当消息类型为 文件、图片、语音这些消息时，体积可能比较大。
///         一旦一个聊天会话中，包含多个这种消息时，就会使从服务器获取消息列表时的操作变得非常低效
///         一般的做法是，获取消息列表时，只拿 fileId、等到客户端得到消息列表后，
///             再根据拿到的fileId，给服务器发送额外的请求，获取文件内容
/// 文件名称：当消息类型为 文件 时，才需要。图片和语音不需要显示
////////////////////////////////////////////////////////////////////////////////////////////////////////

// 消息类型
enum MessageType{
    TEXT_TYPE,      //文本类型
    IMAGE_TYPE,     //图片类型
    FILE_TYPE,      //文件类型
    SPEECH_TYPE,    //语音类型
    INVALId_TYPE    //无效类型
};

class Message{
public:
    QString     messageId = "";         //消息编号
    QString     chatSessionId = "";     //所属会话
    QString     time = "";              //发送时间
    MessageType messageType;            //消息类型
    UserInfo    sender;                 //发送者信息
    QByteArray  content;                //消息内容
    QString     fileId = "";            //文件标识
    QString     fileName = "";          //文件名称

    // 从 protobuffer 的 UserInfo 对象, 转成当前代码的 UserInfo 对象
    void load(const stark_im::MessageInfo& messageInfo){
        this->messageId = messageInfo.messageId();
        this->chatSessionId = messageInfo.chatSessionId();
        this->time = formatTime(messageInfo.timestamp());
        this->sender.load(messageInfo.sender());

        //设置消息类型
        auto type = messageInfo.message().messageType();
        if (type == stark_im::MessageTypeGadget::MessageType::STRING) {
            this->messageType = TEXT_TYPE;
            this->content = messageInfo.message().stringMessage().content().toUtf8();
        } else if (type == stark_im::MessageTypeGadget::MessageType::IMAGE) {
            this->messageType = IMAGE_TYPE;
            if (messageInfo.message().imageMessage().hasImageContent()) {
                this->content = messageInfo.message().imageMessage().imageContent();
            }
            if (messageInfo.message().imageMessage().hasFileId()) {
                this->fileId = messageInfo.message().imageMessage().fileId();
            }
        } else if (type == stark_im::MessageTypeGadget::MessageType::FILE) {
            this->messageType = FILE_TYPE;
            if (messageInfo.message().fileMessage().hasFileContents()) {
                this->content = messageInfo.message().fileMessage().fileContents();
            }
            if (messageInfo.message().fileMessage().hasFileId()) {
                this->fileId = messageInfo.message().fileMessage().fileId();
            }
            this->fileName = messageInfo.message().fileMessage().fileName();
        } else if (type == stark_im::MessageTypeGadget::MessageType::SPEECH) {
            this->messageType = SPEECH_TYPE;
            if (messageInfo.message().speechMessage().hasFileContents()) {
                this->content = messageInfo.message().speechMessage().fileContents();
            }
            if (messageInfo.message().speechMessage().hasFileId()) {
                this->fileId = messageInfo.message().speechMessage().fileId();
            }
        } else {
            this->messageType = INVALId_TYPE;

            // 错误的类型, 啥都不做了, 只是打印一个日志
            LOG() << "无效的消息类型! type=" << type;
        }
    }

    // 工厂模式: 解决C++/Java等语言中，构造函数，不太够用的问题
    // 此处 extraInfo 目前只是在消息类型为文件消息时, 作为 "文件名" 补充.
    static Message makeMessage(MessageType messageType,const QString& chatSessionId,const UserInfo& sender,
                               const QByteArray& content, const QString& extraInfo){
        if(messageType == TEXT_TYPE){
            return makeTextMessage(chatSessionId,sender,content);
        }else if(messageType == IMAGE_TYPE){
            return makeImageMessage(chatSessionId,sender,content);
        }else if(messageType == FILE_TYPE){
            return makeFileMessage(chatSessionId,sender,content,extraInfo);
        }else if(messageType == SPEECH_TYPE){
            return makeSpeechMessage(chatSessionId,sender,content);
        }

        //无效消息类型
        return Message();
    }

private:
    // 通过这个方法生成唯一的messageId
    static QString makeId(){
        //messageId 是一个"唯一"这样的内容
        //UUID这个东西，背后是一套算法，通过这个算法，能够生成全球唯一的身份标识
        //QT中对UUID是有封装的
        return "Msg_" + QUuid::createUuid().toString().sliced(25,12);
    }

    static Message makeTextMessage(const QString& chatSessionId,const UserInfo& sender, const QByteArray& content){
        Message message;
        // 确保设置的ID是唯一的
        message.messageId = makeId();
        message.chatSessionId = chatSessionId;
        message.time = formatTime(getTime()); // 生成一个格式化时间
        message.messageType = TEXT_TYPE;
        message.sender = sender;
        message.content = content;

        // 对于文本消息来说，这俩属性不使用
        message.fileId = "";
        message.fileName = "";
        return message;
    }
    static Message makeImageMessage(const QString& chatSessionId,const UserInfo& sender, const QByteArray& content){
        Message message;
        message.messageId = makeId();
        message.chatSessionId = chatSessionId;
        message.time = formatTime(getTime());
        message.messageType = IMAGE_TYPE;
        message.sender = sender;
        message.content = content;

        message.fileId = "";
        message.fileName = "";
        return message;
    }
    static Message makeFileMessage(const QString& chatSessionId,const UserInfo& sender, const QByteArray& content,const QString& fileName){
        Message message;
        message.messageId = makeId();
        message.chatSessionId = chatSessionId;
        message.time = formatTime(getTime());
        message.messageType = FILE_TYPE;
        message.sender = sender;
        message.content = content;

        message.fileId = "";
        message.fileName = fileName;
        return message;
    }
    static Message makeSpeechMessage(const QString& chatSessionId,const UserInfo& sender, const QByteArray& content){
        Message message;
        message.messageId = makeId();
        message.chatSessionId = chatSessionId;
        message.time = formatTime(getTime());
        message.messageType = SPEECH_TYPE;
        message.sender = sender;
        message.content = content;

        message.fileId = "";
        message.fileName = "";
        return message;
    }
};





////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 聊天会话信息
///
/// 会话名称：单聊（对方昵称）| 群聊（设置的名称）
/// 上层消息：最新的消息：这个内容主要是为了在会话列表中，起到显示-提示这样的作用
/// 会话头像：单聊（对方头像）| 群聊（设置的头像）
/// 会话对象：单聊（对方编号）| 群聊（设为""，后续通过其它方式拿到Id列表）
////////////////////////////////////////////////////////////////////////////////////////////////////////

class ChatSessionInfo{
public:
    QString chatSessionId = "";     //会话编号
    QString chatSessionName = "";   //会话名字
    Message lastMessage;            //上层消息
    QIcon   avatar;                 //会话头像
    QString userId = "";            //会话对象

    // 从 protobuffer 的 ChatSessionInfo 对象, 转成当前代码的 ChatSessionInfo 对象
    void load(const stark_im::ChatSessionInfo& chatSessionInfo){
        this->chatSessionId = chatSessionInfo.chatSessionId();
        this->chatSessionName = chatSessionInfo.chatSessionName();

        if(chatSessionInfo.hasSingleChatFriendId()){
            this->userId = chatSessionInfo.singleChatFriendId();
        }
        if(chatSessionInfo.hasPrevMessage()){
            lastMessage.load(chatSessionInfo.prevMessage());
        }

        if(chatSessionInfo.hasAvatar() && !chatSessionInfo.avatar().isEmpty()){
            // 已经有头像了, 则直接设置这个头像
            this->avatar = makeIcon(chatSessionInfo.avatar());
        }else{
            // 如果没有头像，则根据当前会话是单聊还是群聊，使用不同的默认头像
            if(userId != ""){
                // 单聊
                this->avatar = QIcon(":/resource/image/defaultAvatar.png");
            }else {
                // 群聊
                this->avatar = QIcon(":/resource/image/groupAvatar.png");
            }
        }
    }
};


} // end model
