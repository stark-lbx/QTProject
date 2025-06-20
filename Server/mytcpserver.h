#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    static MyTcpServer& getInstance();

    std::list<std::unique_ptr<MyTcpSocket>> m_tcpSocketList;
    void incomingConnection(qintptr handle) override;
    void deleteSocket(MyTcpSocket* tcpSock);
    void resend(char* tarName,std::unique_ptr<PDU> pdu);
private:
    //隐藏构造函数
    explicit MyTcpServer(QObject *parent = nullptr);

    //禁止拷贝
    MyTcpServer(const MyTcpServer& o)=delete;
    MyTcpServer& operator=(const MyTcpServer& o)=delete;

    //禁止移动构造
    MyTcpServer(MyTcpServer&& o)=delete;
    MyTcpServer& operator=(MyTcpServer&& o)=delete;
};

#endif // MYTCPSERVER_H
