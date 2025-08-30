#ifndef SEARCHFRIENDAREA_H
#define SEARCHFRIENDAREA_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <QListWidget>

#include "model/data.h"
using model::UserInfo;

#define SearchFriendWidgetVersion 2
//////////////////////////////////////
/// 分割符
//////////////////////////////////////
class Separator: public QWidget {
    Q_OBJECT
public:
    Separator(const QString& title = "");
protected:
    void paintEvent(QPaintEvent *event) override;
};

//////////////////////////////////////
/// 一个特殊的显示结果
///     点击后跳到外部搜索结果中
//////////////////////////////////////
class LinkedItem : public QWidget{
    Q_OBJECT
public:
    LinkedItem(const QString& keyword = "");
    void clickThis();

    void updateContent(const QString& keyword);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    QLabel* contentLabel;
};

//////////////////////////////////////
/// 表示一个好友搜索的结果
//////////////////////////////////////
class ResultItem : public QWidget {
    Q_OBJECT
public:
    ResultItem(const UserInfo& userInfo);

    void clickRemoveButton();
    void clickTochatButton();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPushButton* removeBtn;
    QPushButton* tochatBtn;
    const UserInfo& userInfo;  
};




#if SearchFriendWidgetVersion == 1
//////////////////////////////////////
/// 整个搜索好友的窗口
//////////////////////////////////////
class SearchFriendWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchFriendWidget(QWidget *parent = nullptr);

    // 初始化结果显示区
    void initResultArea();

    // 清空界面上所有的好友结果
    void clear();
    void release();

    // 往窗口中新增一个好友搜索结果
    void addResult(const UserInfo& userInfo);
    void addSeparator(const QString& showText);

    // 查看用户是否含有某个关键词
    bool containKeyWord(const UserInfo& userInfo, const QString& keyword);
private:
    QLineEdit* searchEdit;
    // 必有的搜索结果项
    LinkedItem* sItem;

    // 整个窗口总的网格布局
    QGridLayout* layout;

    // 保存搜索好友的结果
    QWidget* resultContainer;
};


#elif SearchFriendWidgetVersion == 2
//////////////////////////////////////
/// 整个搜索好友的窗口 - version2.0
//////////////////////////////////////
class SearchFriendWidget : public QWidget {
    Q_OBJECT
public:
    explicit SearchFriendWidget(QWidget *parent = nullptr);

    // 清空界面上所有的好友结果
    void clear();
    void release();

    // 往窗口中新增一个好友搜索结果
    void addResult(const UserInfo& userInfo);

    // 查看用户是否含有某个关键词
    bool containKeyWord(const UserInfo& userInfo, const QString& keyword);
private:
    QLineEdit* searchEdit;
    // 必有的搜索结果项
    LinkedItem* linkedItem;

    QListWidget* listWidget;

    void searchEditTextChanged(const QString& keyword);
};
#endif  // Version1

#endif // SEARCHFRIENDAREA_H
