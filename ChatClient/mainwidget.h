#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "messageeditarea.h"
#include "messageshowarea.h"
#include "searchfriendarea.h"
#include "sessionfriendarea.h"

#include <QPushButton>
#include <QWidget>
#include <QMouseEvent>
#include <QLineEdit>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWidget;
}
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

private:
    Ui::MainWidget *ui;
    // static MainWidget* instance;

    MainWidget(QWidget *parent = nullptr);
    MainWidget(const MainWidget&) = delete;
    MainWidget operator=(const MainWidget&) = delete;

    QPoint mousePoint;
    bool mouse_press;
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void closeEvent(QCloseEvent* event) override;
private:
    QWidget* windowLeft;    //左侧导航栏

    QWidget* windowMid;     //中间列表栏
    SearchFriendWidget* searchPage; //中间的搜索朋友区域

    QWidget* windowRight;   //右侧展示区
    QWidget* defaultPage;   //默认界面
    QWidget* chatArea;   //聊天区
    QWidget* userInfoPage; //信息展示区

    //左侧导航栏内容
    QPushButton* userAvatar;    //用户头像
    QPushButton* sessionTabBtn; //会话标签页按钮
    QPushButton* friendTabBtn;  //好友标签页按钮
    QPushButton* applyTabBtn;   //好友申请标签页按钮

    //中间内容
    QLineEdit* searchEdit;      //用户搜索框
    QPushButton* addFriendBtn;  //添加好友按钮

    //右侧元素
    SessionFriendArea* sessionFriendArea;
    QLabel* sessionTitleLabel;
    QPushButton* extraBtn;
    MessageShowArea* messageShowArea;
    MessageEditArea* messageEditArea;

    //系统托盘元素
    bool isBlinking = false;
    QTimer* binkTimer;
    QIcon* normalIcon;
    QIcon* emptyIcon;
    QSystemTrayIcon* trayIcon;

    //激活的标签页
    enum ActiveTab{ SESSION_LIST, FRIEND_LIST, APPLY_LIST };
    ActiveTab activeTab = SESSION_LIST;

public:
    ~MainWidget();
    static MainWidget* getInstance();

    void initMainWindow();  //初始化主窗口的样式布局
    void initLeftWindow();  //初始化左侧窗口布局
    void initMidWindow();   //初始化中间窗口布局
    void initRightWindow(); //初始化右侧窗口布局

    void initSignalSlot();  //安装信号槽
    void initWebsocket();   //初始化 websocket
    void setupSystemTrayIcon(); //安装系统托盘

    void switchTabToSession();  //切换到会话标签页
    void switchTabToFriend();   //切换到好友标签页
    void switchTabToApply();    //切换到申请标签页

    void loadSessionList(); //加载会话列表数据
    void loadFriendList();  //加载好友列表数据
    void loadApplyList();   //加载申请列表数据

    void updateFriendList();
    void updateChatSessionList();
    void updateApplyList();

    void loadRecentMessage(const QString& chatSessionId);
    void updateRecentMessage(const QString& chatSessionId);

    void switchSession(const QString& userId);

    MessageShowArea* getMessageShowArea();
    void showDefault();
    void showChatArea();
    void showUserInfo(const model::UserInfo& userId);

    void launchLater(); //延迟启动，等待数据初始化成功
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void toChatFromSearch(const QString& userId);
    void closing();
};
#endif // MAINWIDGET_H
