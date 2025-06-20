#include "mytcpserver.h"


MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr handle)
{
    MyTcpSocket *pSocket = new MyTcpSocket();
    pSocket->setSocketDescriptor(handle);//设置socket文件的句柄(文件描述符)

    connect(pSocket,&MyTcpSocket::offline,this,&MyTcpServer::deleteSocket);
    connect(pSocket,&MyTcpSocket::readyRead, this,[=](){
        while(pSocket->bytesAvailable()>0){
            QByteArray data = pSocket->readAll();
            qDebug()<<data;
        }
    });
    m_tcpSocketList.emplace_back(pSocket);

    qDebug()<<"myTcpServer(incomingConnection):\n"
             <<"\t new client connect successfully";

    // for(const auto& client :m_tcpSocketList){
    //     qDebug()<<client->socketDescriptor();
    // }
}

void MyTcpServer::deleteSocket(MyTcpSocket *tcpSock)
{
    for(auto it = m_tcpSocketList.begin();it!=m_tcpSocketList.end();++it){
        if(it->get() == tcpSock){
            m_tcpSocketList.erase(it);
            return;
        }
    }
}

void MyTcpServer::resend(char *tarName, std::unique_ptr<PDU> pdu)
{
    if(tarName==nullptr || pdu.get()==nullptr)return;


    // foreach(auto& tcpSocket,m_tcpSocketList){
    //     MyTcpSocket* pSocket = tcpSocket.get();
    //     if(pSocket->m_strLoginName == tarName){
    //         pSocket->write((char*)pdu.get(),pdu->uiPDULen);
    //         qDebug()<<"MyTcpServer(resend):"<<pdu->uiPDULen
    //                  <<"\n\t pdu->uiMsgType: "<< pdu->uiMsgType
    //                  <<"\n\t pdu->uiMsgLen: "<< pdu->uiMsgLen
    //                  <<"\n\t pdu->caData: "<< pdu->caData
    //                  <<"\n\t pdu->caData+32: "<< pdu->caData+32
    //                  <<"\n\t pdu->caMsg: "<< pdu->caMsg;
    //     }
    // }

    for(auto& pSocket:m_tcpSocketList){
        if(pSocket->m_strLoginName==tarName){
            qDebug()<<"MyTcpServer(resend):";
            pSocket->sendMsg(std::move(pdu));
            break;
        }
    }
}

MyTcpServer::MyTcpServer(QObject *parent)
    : QTcpServer{parent}
{
}
