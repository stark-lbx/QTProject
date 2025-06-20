#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include "protocol.h"
#include "reqhandler.h"

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString m_strLoginName;
    QByteArray buffer;
    ReqHandler rh;


    // std::unique_ptr<PDU> readMsg();
    void sendMsg(std::unique_ptr<PDU> pdu);
    std::unique_ptr<PDU> handleMsg(std::unique_ptr<PDU> pdu);
signals:
    void offline(MyTcpSocket*);
public slots:
    void recvMsg();
    void offonline();
};


#endif // MYTCPSOCKET_H
