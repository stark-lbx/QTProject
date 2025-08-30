#ifndef SESSIONDETAILDIALOG_H
#define SESSIONDETAILDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QWidget>

#include "model/data.h"
//////////////////////////////////////
/// 表示一个头像 + 一个名字的组合控件
//////////////////////////////////////

class AvatarItem : public QWidget{
    Q_OBJECT
public:
    AvatarItem(const QIcon& avatar, const QString& name);

    QPushButton* getAvatar() {
        return avatarBtn;
    }
private:
    QPushButton* avatarBtn;
    QLabel* nameLabel;
};



//////////////////////////////////////
/// 表示 "单聊会话详细" 窗口
//////////////////////////////////////

class SessionDetailWidget : public QDialog
{
    Q_OBJECT
public:
    SessionDetailWidget(const model::UserInfo& userInfo, QWidget* parent = nullptr);

private:
    QPushButton* deleteFriendBtn;
    model::UserInfo userInfo;
};

#endif // SESSIONDETAILDIALOG_H
