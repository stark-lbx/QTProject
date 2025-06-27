#include "server.h"

#include <OperateDB.h>
#include <QApplication>
#include <QDir>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OperateDB::getInstance();
    Server::getInstance();
    return a.exec();
}
