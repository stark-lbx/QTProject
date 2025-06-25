#include "client.h"
#include "ui_client.h"

#include "QFile"
#include "QDebug"
#include "QTcpSocket"
#include "protocol.h"
#include "reshandler.h"

#include <QMessageBox>

//Client的全局唯一访问接口
Client &Client::getInstance(){
    static Client instance;
    return instance;
}


Client::~Client(){
    delete ui;
}
Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    loadConfig();

    //测试时快速登录测试使用账户
    ui->uid_LE->setText("stark");
    ui->pwd_LE->setText("123456");


    //与通信服务器进行连接-连接成功后使用lambda表达式给出成功回应
    connect(&m_tcpSocket,&QTcpSocket::connected,this,[](){
        qDebug()<<"Client(init):\n\t connected to host sucessfully";
    });
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);
    connect(&m_tcpSocket,&QTcpSocket::readyRead,this,&Client::recvMsg);

}

//加载配置文件
void Client::loadConfig(){
    QFile file(":/cfg/client.config");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"Client(loadConfig):\n\t open the config_file error";
        return;
    }

    QStringList strList = QString(file.readAll()).split("\r\n");
    m_strIP =strList.at(0);
    m_usPort = strList.at(1).toUShort();
    m_strRootPath = strList.at(2);
    qDebug() <<"Client(loadConfig):\n\t "
             <<"ip:"<<m_strIP
             <<"port:"<<m_usPort
             <<"rootPath:"<<m_strRootPath;
    file.close();
}

//处理消息
void Client::handleMsg(std::unique_ptr<PDU> pdu){
    uint type = pdu->uiMsgType;
    ResHandler rh(std::move(pdu));
    //根据消息类型执行合适的功能
    switch(type){
    case ENUM_MSG_TYPE_REGISTER_RESPOND:{//消息类型: 响应注册
        rh.regist();
        break;
    }case ENUM_MSG_TYPE_LOGIN_RESPOND:{//消息类型：响应登录
        rh.login();
        break;
    }case ENUM_MSG_TYPE_FINDUSER_RESPOND:{
        rh.findUser();
        break;
    }case ENUM_MSG_TYPE_ADDFRIEND_RESPOND:{
        rh.addFriend();
        break;
    }case ENUM_MSG_TYPE_DELFRIEND_RESPOND:{
        rh.delFriend();
        break;
    }
    case ENUM_MSG_TYPE_ADD_RESEND:{
        rh.addFriendResend();
        break;
    }case ENUM_MSG_TYPE_AGREEADD_RESPOND:{
        rh.agreeAdd();
        break;
    }case ENUM_MSG_TYPE_GETFRIENDLIST_RESPOND:{
        rh.loadFriend();
        break;
    }case ENUM_MSG_TYPE_CHAT_RESPOND:{
        rh.chat();
        break;
    }case ENUM_MSG_TYPE_MKDIR_RESPOND:{
        rh.mkdir();
        break;
    }case ENUM_MSG_TYPE_FLUSHFILES_RESPOND:{
        rh.flushFiles();
        break;
    }case ENUM_MSG_TYPE_MOVEFILE_RESPOND:{
        rh.moveFile();
        break;
    }case ENUM_MSG_TYPE_UPLOAD_FILE_INIT_RESPOND:{
        rh.uploadFileInit();
        break;
    }case ENUM_MSG_TYPE_UPLOAD_FILE_DATA_RESPOND:{
        rh.flushFiles();
        break;
    }case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:{
        rh.shareFile();
        break;
    }case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:{
        rh.shareFileResend();
        break;
    }case ENUM_MSG_TYPE_SHARE_FILE_AGREE_RESPOND:{
        rh.flushFiles();
        break;
    }
    // case ENUM_MSG_TYPE_DELETEFRIEND_RESPOND:{
    //     rh.loadFriend();
    //     break;
    // }
    default:break;
    }
}


//发送消息
void Client::sendMsg(std::shared_ptr<PDU> pdu){
    if(pdu.get()==nullptr)return;

    m_tcpSocket.write((char*)pdu.get(),pdu->uiPDULen);
    //测试：发送数据
    qDebug() <<"client(sendMsg-details):"
             <<"\n\t pdu->uiMsgType: "<< pdu->uiMsgType
             <<"\n\t pdu->uiPDULen: "<< pdu->uiPDULen
             <<"\n\t pdu->uiMsgLen: "<< pdu->uiMsgLen
             <<"\n\t pdu->caData: "<< pdu->caData
             <<"\n\t pdu->caData+32: "<< pdu->caData+32
             <<"\n\t pdu->caMsg: "<< pdu->caMsg;
}

//接收消息
void Client::recvMsg(){
    //处理粘包/半包问题-循环

    QByteArray data = m_tcpSocket.readAll();
    buffer.append(data);
    while(buffer.size() >= int(sizeof(PDU))){
        std::unique_ptr<PDU> tmppdu = clone((PDU*)buffer.data());
        if(buffer.size()<int(tmppdu->uiPDULen)) break;
        buffer.remove(0,tmppdu->uiPDULen);

        handleMsg(std::move(tmppdu));
    }
}

//读取消息
std::unique_ptr<PDU> Client::readMsg(){
    qDebug()<<"Client(readMsg)"<<m_tcpSocket.bytesAvailable();
    //读取协议总长度: PDU结构体长度 + caMsg长度
    uint uiPDULen = 0;
    m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));

    //读取PDU中协议总长度以外的内容
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    std::unique_ptr<PDU> pdu = mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu.get()+sizeof(uint),uiPDULen-sizeof(uint));

    //测试：读取数据输出
    qDebug() <<"client(readMsg-details):"
             <<"\n\t pdu->uiMsgType: "<< pdu->uiMsgType
             <<"\n\t pdu->uiMsgLen: "<< pdu->uiMsgLen
             <<"\n\t pdu->caData: "<< pdu->caData
             <<"\n\t pdu->caData+32: "<< pdu->caData+32
             <<"\n\t pdu->caMsg: "<< pdu->caMsg;
    return pdu;
}

//构造特定类型的pdu
std::unique_ptr<PDU> Client::dealData(uint dealType){
    //从输入框获取用户名和密码，验证长度是否合法
    QString struid = ui->uid_LE->text();
    QString strpwd = ui->pwd_LE->text();
    if(struid.size()>32|| struid.isEmpty()){
        QMessageBox::information(
            this,
            "提示",
            "账号非法长度(err: empty or >32)");
        return nullptr;
    }
    if(strpwd.size()>32 ||strpwd.isEmpty()){
        QMessageBox::information(
            this,
            "提示",
            "密码非法长度(err: empty or >32)");
        return nullptr;
    }

    //构建pdu 用户名和密码放入caData
    std::unique_ptr<PDU> pdu =  mkPDU();//无参: 扩展消息长度默认为0
    pdu->uiMsgType = dealType;
    memcpy(pdu->caData,struid.toStdString().c_str(),32);
    memcpy(pdu->caData+32,strpwd.toStdString().c_str(),32);

    return pdu;
}



void Client::on_register_PB_clicked(){
    sendMsg(this->dealData(ENUM_MSG_TYPE_REGISTER_REQUEST));
}

void Client::on_login_PB_clicked(){
    sendMsg(this->dealData(ENUM_MSG_TYPE_LOGIN_REQUEST));
}


