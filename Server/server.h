#ifndef SERVER_H
#define SERVER_H

#include <MyTcpServer.h>
#include <QWidget>

class Server : public QWidget
{
    Q_OBJECT

public:
    static Server& getInstance();
    ~Server();

    QString m_strIP;//str类型的ip
    quint16 m_usPort;//ushort类型的port
    //QTcpServer m_tcpServer;//tcp协议服务器

    QString m_strRootPath;//程序中的文件的默认存储路径

    void loadConfig();
private:
    Server(QWidget *parent = nullptr);

    Server(const Server&)=delete;
    Server& operator=(const Server&)=delete;

    Server(Server&&)=delete;
    Server& operator=(Server&&)=delete;
};
#endif // SERVER_H
