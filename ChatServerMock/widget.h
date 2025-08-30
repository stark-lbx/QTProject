#ifndef WIDGET_H
#define WIDGET_H

#include "server.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_sendTextMessage_clicked();
    void on_sendImageMessage_clicked();
    void on_sendFileMessage_clicked();
    void on_sendSpeechMessage_clicked();

    void on_notifyFriendRemove_clicked();
    void on_notifyFriendApply_clicked();
    void on_notifyFriendAgree_clicked();
    void on_notifyFriendReject_clicked();

    void on_notifyNewChatSession_clicked();
private:
    Ui::Widget *ui;

    WebsocketServer* websocketServer;
};
#endif // WIDGET_H
