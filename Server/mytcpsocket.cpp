#include "mytcpsocket.h"

#include "OperateDB.h"
#include "reqhandler.h"


MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    // connect(this,&MyTcpSocket::readyRead,this,&MyTcpSocket::recvMsg);
    connect(this,&MyTcpSocket::readyRead,this,[this](){
        qDebug()<<"myTcpSocket recv a message from client";
        recvMsg();
    });
    connect(this,&MyTcpSocket::disconnected,this,&MyTcpSocket::offonline);
}

std::unique_ptr<PDU> MyTcpSocket::handleMsg(std::unique_ptr<PDU> pdu)
{
    if(pdu.get()==nullptr)return pdu;
    uint type = pdu->uiMsgType;


    //测试：读取数据输出
    qDebug() <<"myTcpSocket(handlerMsg message's details):"
             <<"\n\t pdu->uiMsgType: "<< pdu->uiMsgType
             <<"\n\t pdu->uiMsgLen: "<< pdu->uiMsgLen
             <<"\n\t pdu->caData: "<< pdu->caData
             <<"\n\t pdu->caData+32: "<< pdu->caData+32
             <<"\n\t pdu->caMsg: "<< pdu->caMsg;

    rh.resetPDU(std::move(pdu));
    qDebug()<<"MyTcpSocket::handleMsg: switch begin";
    switch(type){
    //===============消息类型: 注册请求===============
    case ENUM_MSG_TYPE_REGISTER_REQUEST:return rh.regist();
    //===============消息类型: 登录请求===============
    case ENUM_MSG_TYPE_LOGIN_REQUEST:return rh.login(m_strLoginName);
    //===============消息类型: 搜索用户请求===============
    case ENUM_MSG_TYPE_FINDUSER_REQUEST:return rh.findUser();
    //===============消息类型: 添加用户请求===============
    case ENUM_MSG_TYPE_ADDFRIEND_REQUEST:return rh.addFriend();
    //===============消息类型: 创建文件夹===============
    case ENUM_MSG_TYPE_DELFRIEND_REQUEST:return rh.delFriend();
    //===============消息类型: 同意添加的请求===============
    case ENUM_MSG_TYPE_AGREEADD_REQUEST:return rh.agreeAdd();
    //===============消息类型: 获取好友列表的请求===============
    case ENUM_MSG_TYPE_GETFRIENDLIST_REQUEST:return rh.getFriendList();
    //===============消息类型: 聊天请求===============
    case ENUM_MSG_TYPE_CHAT_REQUEST:return rh.chat();
    //===============消息类型: 创建文件夹===============
    case ENUM_MSG_TYPE_MKDIR_REQUEST:return rh.mkdir();
    //===============消息类型: 刷新文件夹===============
    case ENUM_MSG_TYPE_FLUSHFILES_REQUEST:return rh.flushFiles();
    //===============消息类型: 移动文件===============
    case ENUM_MSG_TYPE_MOVEFILE_REQUEST:return rh.moveFile();
    //===============消息类型: 上传头部===============
    case ENUM_MSG_TYPE_UPLOAD_FILE_INIT_REQUEST:return rh.uploadFileInit();
    //===============消息类型: 上传数据===============
    case ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST:return rh.uploadFileData();
    //===============消息类型: 分享文件===============
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:return rh.shareFile();
    //===============消息类型: 分享文件===============
    case ENUM_MSG_TYPE_SHARE_FILE_AGREE_REQUEST:return rh.shareFileAgree();
    //===================================================
    default:break;
    }
    return mkPDU();
}


void MyTcpSocket::recvMsg()
{
    qDebug()<<"recvMsg 接收消息长度"<<this->bytesAvailable();
    //socket中的数据全部取出
    QByteArray data = this->readAll();
    //定义一个成员变量存半包数据
    buffer.append(data);
    qDebug()<<buffer.size();
    qDebug()<<int(sizeof(PDU));
    //判断是否够一个完整的pdu，转成char*再转成PDU*
    while(buffer.size() >= int(sizeof(PDU))){
        std::unique_ptr<PDU> tmppdu= clone((PDU*)(buffer.data()));
        //判断是否够一个完整的包
        if(buffer.size() < int(tmppdu->uiPDULen))break;
        //处理完的数据从buffer中移除
        buffer.remove(0,tmppdu->uiPDULen);

        qDebug()<<"recvMsg: begin to handle";
        std::unique_ptr<PDU> respdu = handleMsg(std::move(tmppdu));
        qDebug()<<"recvMsg: handle over";

        qDebug()<<"recvMsg: begin to respond client";
        sendMsg(std::move(respdu));
        qDebug()<<"recvMsg: already send to client";
    }
}

void MyTcpSocket::offonline()
{
    OperateDB::getInstance().handleOffline(m_strLoginName.toStdString().c_str());
    emit offline(this);
}

// std::unique_ptr<PDU> MyTcpSocket::readMsg()
// {
//     qDebug()<<"myTcpSocket::readMsg 接收消息长度: "<<this->bytesAvailable();

//     //读取协议总长度: PDU结构体长度 + caMsg长度
//     uint uiPDULen = 0;
//     this->read((char*)&uiPDULen, sizeof(uint));

//     //读取PDU中协议总长度以外的内容
//     uint uiMsgLen = uiPDULen - sizeof(PDU);
//     std::unique_ptr<PDU> pdu = mkPDU(uiMsgLen);
//     this->read((char*)pdu.get()+sizeof(uint),uiPDULen-sizeof(uint));

//     //测试：读取数据输出
//     qDebug() <<"myTcpSocket(readMsg-details):"
//              <<"\n\t pdu->uiMsgType: "<< pdu->uiMsgType
//              <<"\n\t pdu->uiMsgLen: "<< pdu->uiMsgLen
//              <<"\n\t pdu->caData: "<< pdu->caData
//              <<"\n\t pdu->caData+32: "<< pdu->caData+32
//              <<"\n\t pdu->caMsg: "<< pdu->caMsg;

//     return pdu;
// }

void MyTcpSocket::sendMsg(std::unique_ptr<PDU> pdu)
{
    if(pdu.get()==nullptr)return;
    this->write((char*)pdu.get(),pdu->uiPDULen);
    //测试：发送数据
    qDebug() <<"myTcpSocket(sendMsg-details):"
             <<"\n\t pdu->uiMsgType: "<< pdu->uiMsgType
             <<"\n\t pdu->uiMsgLen: "<< pdu->uiMsgLen
             <<"\n\t pdu->caData: "<< pdu->caData
             <<"\n\t pdu->caData+32: "<< pdu->caData+32
             <<"\n\t pdu->caMsg: "<< pdu->caMsg;
}
