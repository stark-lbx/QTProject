#ifndef SEARCHUSERDIALOG_H
#define SEARCHUSERDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QWidget>

#include "common/roundeddialog.h"
#include "model/data.h"
#include "common/customtitlebar.h"
using model::UserInfo;

//////////////////////////////////////
/// 表示一个好友搜索的结果
//////////////////////////////////////

class FriendResultItem : public QWidget {
    Q_OBJECT
public:
    FriendResultItem(const UserInfo& userInfo);
    void clickAddBtn();
private:
    const UserInfo& userInfo;

    QPushButton* addBtn;
};


//////////////////////////////////////
/// 整个搜索好友的窗口
//////////////////////////////////////

class AddFriendDialog : public RoundedDialog
{
    Q_OBJECT
public:
    static AddFriendDialog* getInstance();

    // 初始化结果显示区
    void initResultArea();

    // 往窗口中新增一个好友搜索结果
    void addResult(const UserInfo& userInfo);

    // 清空界面上所有的好友结果
    void clear();

    void setSearchKey(const QString &searchKey);

    void clickSearch();
    void clickSearchDone();
protected:
    bool mouse_press = false;
    QPoint mousePoint;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    AddFriendDialog(QWidget* parent=nullptr);

    QLineEdit* searchEdit;

    // 整个窗口总的网格布局
    QGridLayout* layout;

    // 保存搜索好友的结果
    QWidget* resultContainer;

    // 搜索按钮
    QPushButton* searchBtn;
    // 标题栏
    CustomTitleBar* titleBar;
};

#endif // SEARCHUSERDIALOG_H
