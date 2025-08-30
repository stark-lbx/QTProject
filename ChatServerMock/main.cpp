#include "widget.h"

#include <QApplication>

#include "server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    HttpServer* httpServer = HttpServer::getInstance();
    if(!httpServer->init()){
        qDebug() <<"httpd init error";
        return 0;
    }
    qDebug() <<"httpd init sucessfully";


    WebsocketServer* websocketServer = WebsocketServer::getInstance();
    if(!websocketServer->init()){
        qDebug() <<"websocketd init error";
        return 0;
    }
    qDebug() << "websocketd init sucessfully";



    // 界面适用于将主动推送的数据发送给客户端的数据
    Widget w;
    w.show();


    return a.exec();
}
