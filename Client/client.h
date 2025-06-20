#ifndef CLIENT_H
#define CLIENT_H

#include "protocol.h"

#include <QTcpSocket>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {class Client;}
QT_END_NAMESPACE

class Client : public QWidget
{
    Q_OBJECT
public:
    //使用单例模式，提供全局访问结点，懒汉模式
    static Client& getInstance();

    ~Client();
    QString m_strIP;//str类型的ip
    quint16 m_usPort;//ushort类型的port
    QTcpSocket m_tcpSocket;
    QString m_strLoginName;
    QString m_strRootPath;
    QByteArray buffer;

    void loadConfig();

    void sendMsg(std::unique_ptr<PDU> pdu);
    std::unique_ptr<PDU> readMsg();
    void handleMsg(std::unique_ptr<PDU> pdu);
    void recvMsg();
    std::unique_ptr<PDU> dealData(uint dealType);
private slots:

    void on_register_PB_clicked();
    void on_login_PB_clicked();

private:
    Ui::Client *ui;
    //隐藏构造函数
    Client(QWidget *parent = nullptr);

    //禁用拷贝构造函数与拷贝赋值运算符
    Client(const Client& o)=delete;
    Client& operator=(const Client& o)=delete;

    //禁用移动拷贝构造函数与移动拷贝运算符
    Client(Client&& o)=delete;
    Client& operator=(Client&& o)=delete;
};
#endif // CLIENT_H
