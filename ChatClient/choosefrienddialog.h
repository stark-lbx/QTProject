#ifndef CHOOSEFRIENDDIALOG_H
#define CHOOSEFRIENDDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

#include "model/data.h"

////////////////////////////////////////////////
/// 选择好友窗口中的一个 元素/好友项
////////////////////////////////////////////////

class ChooseFriendDialog;   //前置声明
class ChooseFriendItem : public QWidget{
    Q_OBJECT
public:
    ChooseFriendItem(ChooseFriendDialog* owner, const QString& userId, const QIcon& avatar, const QString& name, bool checked, bool enable);

    const QString& getUserId() const {return userId;}
    QCheckBox* getCheckBox() {return checkBox;}
protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;
private:
    bool isHover = false;

    QCheckBox* checkBox;
    QPushButton* avatarBtn;
    QLabel* nameLabel;
    ChooseFriendDialog* owner;

    QString userId;
};



////////////////////////////////////////////////
/// 选择好友的窗口
////////////////////////////////////////////////
#include "common/roundeddialog.h"
class ChooseFriendDialog : public RoundedDialog
{
    Q_OBJECT
public:
    ChooseFriendDialog(QWidget* parent=nullptr);

    void initLeft(QHBoxLayout* layout);
    void initRight(QHBoxLayout* layout);

    void addFriend(const QString& userId, const QIcon& avatar, const QString& name, bool checked, bool enable);
    void addSelectedFriend(const QString& userId, const QIcon& avatar, const QString& name, bool enable);
    void deleteSelectedFriend(const QString& userId);
    void clearSearchResult();

    void updateSearchResult(const QString& currentText);
    bool containNicknameKeyword(const model::UserInfo &userInfo, const QString &keyword);

    void clickOkButton();
    QList<QString> generateMemberList();
private:
    // 保存左侧全部好友列表的 QWidget
    QWidget* totalContainer;
    // 保存右侧选择的好友列表的 QWidget
    QWidget* selectedContainer;
};

#endif // CHOOSEFRIENDDIALOG_H
