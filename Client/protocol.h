#ifndef PROTOCOL_H
#define PROTOCOL_H


#include <QObject>

using uint = unsigned int;


enum ENUM_MSG_TYPE{
    ENUM_MSG_TYPE_MIN = 0x00000000,

    ENUM_MSG_TYPE_REGISTER_REQUEST,//请求注册
    ENUM_MSG_TYPE_REGISTER_RESPOND,//响应注册

    ENUM_MSG_TYPE_LOGIN_REQUEST,//请求登录
    ENUM_MSG_TYPE_LOGIN_RESPOND,//响应登录

    ENUM_MSG_TYPE_FINDUSER_REQUEST,//请求查找
    ENUM_MSG_TYPE_FINDUSER_RESPOND,//响应查找

    ENUM_MSG_TYPE_ADDFRIEND_REQUEST,//请求添加好友
    ENUM_MSG_TYPE_ADDFRIEND_RESPOND,//响应添加好友

    ENUM_MSG_TYPE_DELFRIEND_REQUEST,//请求删除好友
    ENUM_MSG_TYPE_DELFRIEND_RESPOND,//响应删除好友

    ENUM_MSG_TYPE_ADD_RESEND,//添加好友的转发请求

    ENUM_MSG_TYPE_AGREEADD_REQUEST,//请求同意添加
    ENUM_MSG_TYPE_AGREEADD_RESPOND,//响应同意添加

    ENUM_MSG_TYPE_GETFRIENDLIST_REQUEST,//请求获取好友列表
    ENUM_MSG_TYPE_GETFRIENDLIST_RESPOND,//获取好友列表响应

    ENUM_MSG_TYPE_CHAT_REQUEST,//请求聊天
    ENUM_MSG_TYPE_CHAT_RESPOND,//响应聊天请求

    ENUM_MSG_TYPE_MKDIR_REQUEST,//请求创建文件夹
    ENUM_MSG_TYPE_MKDIR_RESPOND,//响应创建文件夹

    ENUM_MSG_TYPE_FLUSHFILES_REQUEST,//请求创建文件夹
    ENUM_MSG_TYPE_FLUSHFILES_RESPOND,//响应创建文件夹

    ENUM_MSG_TYPE_MOVEFILE_REQUEST,//请求移动文件
    ENUM_MSG_TYPE_MOVEFILE_RESPOND,//响应移动文件

    ENUM_MSG_TYPE_RENAME_REQUEST,//请求重命名文件
    ENUM_MSG_TYPE_RENAME_RESPOND,//响应重命名文件

    ENUM_MSG_TYPE_UPLOAD_FILE_INIT_REQUEST,//请求上传文件init
    ENUM_MSG_TYPE_UPLOAD_FILE_INIT_RESPOND,

    ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST,//请求上传文件data
    ENUM_MSG_TYPE_UPLOAD_FILE_DATA_RESPOND,

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,//请求分享文件
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,

    ENUM_MSG_TYPE_SHARE_FILE_AGREE_REQUEST,//请求同意分享文件
    ENUM_MSG_TYPE_SHARE_FILE_AGREE_RESPOND,

    ENUM_MSG_TYPE_MAX = 0xffffffff
};

enum ENUM_FILE_TYPE{
    //dir,reg,lnk,dev
    //other(linux:pipe,socket...; Windows:sys,dll...)
    ENUM_FILE_TYPE_MIN = 0x00000000,

    ENUM_FILE_TYPE_DIR,//目录类型
    ENUM_FILE_TYPE_REG,//普通类型
    ENUM_FILE_TYPE_LNK,//链接文件
    ENUM_FILE_TYPE_DEV,//设备文件
    ENUM_FILE_TYPE_OTHER,//特殊文件

    // ENUM_FILE_TYPE_PIPE,//管道文件
    // ENUM_FILE_TYPE_SOCK,//套接字文件

    ENUM_FILE_TYPE_MAX = 0xffffffff
};

struct FileInfo{
    char caName[32];
    int iType;
};

struct PDU{
    uint uiPDULen;//协议数据总长度
    uint uiMsgLen;//消息内容长度
    uint uiMsgType;//消息类型
    char caData[64];//默认大小的内容容器
    char caMsg[];//柔性大小的内容容器
};

std::unique_ptr<PDU> mkPDU(uint msg_len = 0);

std::unique_ptr<PDU> clone(PDU* self);
#endif // PROTOCOL_H
