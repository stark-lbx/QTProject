#include "server.h"


#include <QFile>

Server &Server::getInstance()
{
    static Server instance;
    return instance;
}

Server::~Server()
{
}
Server::Server(QWidget *parent)
    : QWidget(parent)
{
    loadConfig();

    //服务器监听
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);
}




void Server::loadConfig()
{
    QFile file(":/cfg/server.config");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"Server(loadConfig):\n\t open the config_file error";
        return;
    }

    QStringList strList = QString(file.readAll()).split("\r\n");
    m_strIP =strList.at(0);
    m_usPort = strList.at(1).toUShort();
    m_strRootPath = strList.at(2);
    qDebug() <<"Server(loadConfig):\n\t "
             <<"ip:"<<m_strIP
             <<"port:"<<m_usPort
             <<"rootPath:"<<m_strRootPath;
    file.close();
    /*
        i.新建QFile文件，设置路径参数
        ii.设置为只读权限QIODevice::ReadOnly
        iii.读取文件内容，返回值转成QString
        iv.按照换行切割（.split(\r\n)）,返回值QStringList
        v.取出ip和port（toUShort => quint16 ）
        vi.关闭文件
    */
}
