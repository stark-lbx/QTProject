#ifndef USERINFODIALOG_H
#define USERINFODIALOG_H

#include <QDialog>
#include <QWidget>
#include <QLabel>

#include "model/data.h"
using model::UserInfo;
class UserInfoWidget : public QDialog
{
    Q_OBJECT
public:
    UserInfoWidget(const UserInfo& user, QWidget* parent = nullptr);

private:
    // 保存对应的 UserInfo 对象
    const UserInfo userInfo;

    QPushButton* avatarBtn;

    QLabel* idTag;
    QLabel* idLabel;

    QLabel* nameTag;
    QLabel* nameLabel;

    QLabel* phoneTag;
    QLabel* phoneLabel;

    QPushButton* applyBtn;
    QPushButton* sendMessageBtn;
    QPushButton* deleteFriendBtn;

    void initSignalSlot();
};

#endif // USERINFODIALOG_H
