#ifndef FRIEND_H
#define FRIEND_H

#include "chat.h"

#include <QWidget>
#include <QListWidget>

namespace Ui {
class Friend;
}

class Friend : public QWidget
{
    Q_OBJECT

public:
    explicit Friend(QWidget *parent = nullptr);
    ~Friend();

    Chat* m_pChat;

    void updateFriend_LW(QStringList friendList);
    void flushFriends();
    QListWidget* getFriend_LW();
private slots:
    void on_findUser_PB_clicked();

    void on_flush_PB_clicked();

    void on_delete_PB_clicked();

    void on_chat_PB_clicked();

private:
    Ui::Friend *ui;
};

#endif // FRIEND_H
