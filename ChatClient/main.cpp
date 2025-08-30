#include "mainwidget.h"

#include "loginwidget.h"
#include "common/debug.h"
#include "common/toast.h"
#include "common/roundeddialog.h"
#include "model/datacenter.h"
#include "network/netclient.h"

#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>

FILE* output = nullptr;




/////////////////////////////////
/// Qt可以给qDebug这样的调试宏，注册一个回调函数
/// 回调函数就可以在每次触发日志打印的时候执行到
/// 回调函数, 日志重定向使用
/// \param type: 日志级别(debug、info、warning...)
/// \param content: 日志打印的上下文(所处文件、代码行号)
/// \param msg: 日志消息
////////////////////////////////
void msgHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // 将msg写到文件中即可
    (void)type;
    (void)context;

    const QByteArray& log = msg.toUtf8();
    fprintf(output, "%s\n", log.constData());
    fflush(output); //确保数据刷到磁盘中
}

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
    QApplication app(argc, argv);

#if DEPOLY
    // 初始化日志文件
    output = fopen("./log.txt", "a");
    // 注册回调函数
    qInstallMessageHandler(msgHandler);
#endif

#if TEST_NETWORK
    // network::NetClient netClient(nullptr);
    // netClient.ping();
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    dataCenter->ping();
#endif

#if TEST_SKIP_LOGIN
    MainWidget* w = MainWidget::getInstance();
    w->show();
#else
    LoginWidget* login = new LoginWidget();
    login->show();
#endif

    return app.exec();
}











































