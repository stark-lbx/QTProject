#include "mainwidget.h"
#include "./ui_mainwidget.h"
#include "userinfodialog.h"
#include "loginwidget.h"

#include "common/debug.h"
#include "common/toast.h"
#include "model/datacenter.h"
#include "selfinfodialog.h"
#include "sessiondetaildialog.h"
#include "groupsessiondetaildialog.h"
#include "searchuserdialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QProgressBar>
#include <QVBoxLayout>


MainWidget::~MainWidget()
{
    delete ui;
}

MainWidget *MainWidget::getInstance()
{
    // if(instance==nullptr) instance = new MainWidget();
    static MainWidget* instance = new MainWidget();
    return instance;
}

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    this->installEventFilter(this);
    this->setWindowTitle("MyChat");  //设置标题
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));    //设置标题
    this->setMinimumWidth(875);     //设置最小宽度
    this->setMinimumHeight(625);    //设置最小高度
    this->setWindowFlag(Qt::FramelessWindowHint);
    // this->setAttribute(Qt::WA_DeleteOnClose);

    // 初始化窗口布局
    initMainWindow();
    initLeftWindow();
    initMidWindow();
    initRightWindow();

    // 安装信号槽
    initSignalSlot();
    // 初始化websocket
    initWebsocket();
    // 安装系统托盘
    setupSystemTrayIcon();
}

void MainWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        mouse_press = true;
        mousePoint = event->globalPosition().toPoint() - this->pos();
    }
    else if(event->button()==Qt::RightButton){
        QWidget::mousePressEvent(event);
    }
}

void MainWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(mouse_press){
        if(this->isMaximized()) {
            this->showNormal();
        }
        move(event->globalPosition().toPoint() - mousePoint);
    }
}

void MainWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mouse_press = false;
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    // 取消绑定 this 与 this 的信号槽
    disconnect(this, &MainWidget::closing, this, nullptr);
    // 所有的单例模式都应该关闭
    AddFriendDialog::getInstance()->close();
    QWidget::closeEvent(event);
}

void MainWidget::initMainWindow()
{
    // 1.初始化最左侧导航栏
    windowLeft = new QWidget();
    windowLeft->setFixedWidth(65);
    windowLeft->setStyleSheet("QWidget { background-color: rgb(46, 46, 46); }"); //newVersion: rgb(237, 237, 237)

    // 2.初始化中间列表界面
    //      a) 初始化中间会话or好友or申请列表
    windowMid = new QWidget();
    windowMid->setFixedWidth(280);
    windowMid->setStyleSheet("QWidget { background-color: rgb(247, 247, 247); }");
    //      b) 初始化中间搜索结果展示区
    searchPage = new SearchFriendWidget();
    searchPage->setFixedWidth(280);
    searchPage->setStyleSheet("QWidget { background-color: rgb(247, 247, 247); }");

    // 3.初始化右侧展示界面
    windowRight = new QWidget();
    windowRight->setMinimumWidth(525);
    windowRight->setStyleSheet("QWidget { background-color: rgb(237, 237, 237); }");

    // 4.布局管理添加界面（水平）
    //      a) 初始化布局管理器
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setSpacing(0);//内部元素间隔距离
    layout->setContentsMargins(0,0,0,0);//元素距离四个边界的距离
    this->setLayout(layout);
    //      b) 添加左侧界面
    layout->addWidget(windowLeft);
    //      c) 添加中间界面
    layout->addWidget(windowMid);
    layout->addWidget(searchPage);
    //      d) 添加右侧界面
    layout->addWidget(windowRight);

    // 5. 设置可见性
    //      a) 左侧可见内容
    windowLeft->show();
    //      b) 中间可见内容
    windowMid->show();
    searchPage->hide();
    //      c) 右侧可见内容
    windowRight->show();
}

void MainWidget::initLeftWindow()
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(20);//按钮间距设置为20
    layout->setContentsMargins(0,45,0,15);//第一个按钮距离顶部的距离设置为45
    windowLeft->setLayout(layout);

    // 添加用户头像
    userAvatar = new QPushButton();
    userAvatar->setFixedSize(45,45);
    userAvatar->setIconSize(QSize(45,45));
    userAvatar->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    layout->addWidget(userAvatar, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 添加会话标签页按钮
    sessionTabBtn = new QPushButton();
    sessionTabBtn->setFixedSize(45,45);
    sessionTabBtn->setIconSize(QSize(25, 25));
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_active.png"));
    sessionTabBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    layout->addWidget(sessionTabBtn, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 添加好友标签页按钮
    friendTabBtn = new QPushButton();
    friendTabBtn->setFixedSize(45,45);
    friendTabBtn->setIconSize(QSize(25, 25));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_inactive.png"));
    friendTabBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    layout->addWidget(friendTabBtn, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 申请好友标签页按钮
    applyTabBtn = new QPushButton();
    applyTabBtn->setFixedSize(45,45);
    applyTabBtn->setIconSize(QSize(25, 25));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_inactive.png"));
    applyTabBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    layout->addWidget(applyTabBtn, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 最下方添加空白区域，将按钮挤上去
    layout->addStretch(30);

    // diy: 添加退出按钮
    QPushButton* exitTabBtn = new QPushButton();
    exitTabBtn->setFixedSize(45,45);
    exitTabBtn->setIconSize(QSize(25, 25));
    exitTabBtn->setIcon(QIcon(":/resource/image/exit.png"));
    exitTabBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
        }
        QPushButton:hover { background-color: rgb(251, 115, 115); }
        QPushButton:pressed { background-color: rgb(225, 72, 72); }
    )");
    layout->addWidget(exitTabBtn, 1, Qt::AlignBottom | Qt::AlignHCenter);
    connect(exitTabBtn, &QPushButton::clicked, this, [=](){
        // 退出登录-切换账户
        LoginWidget* login = new LoginWidget();
        login->show();
        this->close();
    });
}

void MainWidget::initMidWindow()
{
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(0,30,0,0);   //距离上方30px的边距，其它方向不要边距
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(20);
    windowMid->setLayout(layout);

    // 搜索框，点击后就转移另一个界面了
    searchEdit = new QLineEdit(this);
    searchEdit->installEventFilter(this);//安装事件过滤器
    searchEdit->setFixedHeight(30);
    searchEdit->setPlaceholderText("搜索");
    searchEdit->setStyleSheet("QLineEdit { border-radius: 5px; background-color: rgb(226,226,226); padding-left: 5px;}");

    // 添加好友按钮，点击后打开全网搜索好友的窗体
    addFriendBtn = new QPushButton();
    addFriendBtn->setFixedSize(30,30);
    addFriendBtn->setIcon(QIcon(":/resource/image/cross.png"));
    QString style = R"(
        QPushButton{
            border-radius: 5px;
            background-color: rgb(226, 226, 226);
        }
        QPushButton:pressed { background-color: rgb(209, 209, 209); }
    )";
    addFriendBtn->setStyleSheet(style);

    // 列表区域
    sessionFriendArea = new SessionFriendArea();

    // 为了灵活控制边距，只影响搜索+按钮这一行，不影响下方列表行
    // 创建空白的 widget 填充到布局管理器中
    QWidget* spacer1 = new QWidget;
    spacer1->setFixedWidth(15);
    QWidget* spacer2 = new QWidget;
    spacer2->setFixedWidth(15);
    QWidget* spacer3 = new QWidget;
    spacer3->setFixedWidth(15);

    layout->addWidget(spacer1, 0, 0);
    layout->addWidget(searchEdit, 0, 1);
    layout->addWidget(spacer2, 0, 2);
    layout->addWidget(addFriendBtn, 0, 3);
    layout->addWidget(spacer3, 0, 4);
    layout->addWidget(sessionFriendArea, 1, 0, 1, 5);
}

void MainWidget::initRightWindow()
{
    // 初始化界面
    //      a) 初始化默认展示背景
    defaultPage = new QWidget();
    defaultPage->setMinimumWidth(525);
    defaultPage->setStyleSheet("QWidget { background-color: rgb(237, 237, 237); }");
    //      b) 初始化右侧聊天消息展示区和消息编辑区
    chatArea = new QWidget();
    chatArea->setMinimumWidth(525);
    chatArea->setStyleSheet("QWidget { background-color: rgb(237, 237, 237); }");
    //      c) 初始化用户信息展示界面
    userInfoPage = new QWidget();
    userInfoPage->setMinimumWidth(525);
    userInfoPage->setStyleSheet("QWidget { background-color: rgb(237,237,237); }");
    /*
     * 总的布局为垂直布局，上方一个固定的toolBar
     *      tooBar的布局是一个水平布局，水平排放各种按钮
     *      主页面是垂直布局的另一个窗体
     */

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    windowRight->setLayout(mainLayout);

    QWidget* toolBar = new QWidget;
    toolBar->setFixedHeight(40);
    toolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    toolBar->setStyleSheet("QWidget {background-color: rgb(237, 237, 237);}");
    {
        QHBoxLayout* itemLayout = new QHBoxLayout;
        itemLayout->setContentsMargins(0,0,0,0);
        itemLayout->setSpacing(0);
        toolBar->setLayout(itemLayout);
        itemLayout->addStretch(1);

        QString style = "QPushButton {border: none; background-color: transparent; font-size: 18px;}";
        QString buttonStyle = style + "QPushButton:hover {background-color: rgb(225, 225, 225);}";
        buttonStyle += "QPushButton:pressed {background-color: rgb(213, 213, 213);}";

        QPushButton* minBtn = new QPushButton("-");
        minBtn->setFixedSize(60,40);
        minBtn->setStyleSheet(buttonStyle);
        itemLayout->addWidget(minBtn);
        connect(minBtn, &QPushButton::clicked, this, &MainWidget::showMinimized);

        QPushButton* maxBtn = new QPushButton("□");
        maxBtn->setFixedSize(60,40);
        maxBtn->setStyleSheet(buttonStyle);
        itemLayout->addWidget(maxBtn);
        connect(maxBtn, &QPushButton::clicked, this, [=](){
            if(this->isMaximized()){
                // 当前是最大化状态, 恢复正常位置
                this->showNormal();
            } else {
                // 当前不是最大化状态,最大化
                this->showMaximized();
            }
        });

        QString quitBtnStyle = style + "QPushButton:hover {background-color: rgb(237, 76, 76);}";
        quitBtnStyle += "QPushButton:pressed {background-color: rgb(225, 73, 73);}";
        QPushButton* quitBtn = new QPushButton("×");
        quitBtn->setFixedSize(60,40);
        quitBtn->setStyleSheet(quitBtnStyle);
        itemLayout->addWidget(quitBtn);
        connect(quitBtn, &QPushButton::clicked, this, &MainWidget::closing);
    }
    mainLayout->addWidget(toolBar);

    //////////////////////////////////////////////////
    /// 默认界面展示
    //////////////////////////////////////////////////
    QPushButton* iconBtn = new QPushButton;
    iconBtn->setStyleSheet("QPushButton { border: none; background-color: transparent;}");
    iconBtn->setIcon(QIcon(":/resource/image/logo.png"));

    QVBoxLayout* default_layout = new QVBoxLayout;
    default_layout->setSpacing(0);
    default_layout->setContentsMargins(0,0,0,0);
    default_layout->addWidget(iconBtn);

    defaultPage->setLayout(default_layout);
    mainLayout->addWidget(defaultPage);

    //////////////////////////////////////////////////
    /// 消息展示区、消息编辑区
    //////////////////////////////////////////////////

    // 1.创建右侧窗口的布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setAlignment(Qt::AlignTop);
    chatArea->setLayout(vlayout);

    // 2.创建上方标题栏
    QWidget* titleWidget = new QWidget();
    titleWidget->setFixedHeight(40);
    titleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleWidget->setObjectName("titleWidget");
    titleWidget->setStyleSheet("#titleWidget { "
                               "border-bottom: 1px solid rgb(213, 213, 213); "
                               "border-left: 1px solid rgb(213, 213, 213); }");
    vlayout->addWidget(titleWidget);

    // 3.给标题栏添加label和一个按钮
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(15,0,15,0);
    titleWidget->setLayout(hlayout);

    /*QLabel* */sessionTitleLabel = new QLabel();
    sessionTitleLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sessionTitleLabel->setTextFormat(Qt::RichText); //设置为富文本格式
    sessionTitleLabel->setOpenExternalLinks(false); //禁用自动打开外部链接
    sessionTitleLabel->setTextInteractionFlags(Qt::TextBrowserInteraction); //启用链接交互
#if TEST_UI
    //为了测试临时增加，实际内容来自服务器
    sessionTitleLabel->setText("会话标题");
#endif
    hlayout->addWidget(sessionTitleLabel, 1, Qt::AlignLeft);

    /* QPushButton* */ extraBtn = new QPushButton();
    extraBtn->setFixedSize(40,40);
    extraBtn->setIconSize(QSize(40,40));
    extraBtn->setIcon(QIcon(":/resource/image/more.png"));
    extraBtn->setStyleSheet("QPushButton {border:none; background-color: transparent;} QPushButton:pressed {background-color: rgb(220,220,220);}");
    hlayout->addWidget(extraBtn, 0, Qt::AlignRight);

    // 4.添加消息展示区
    messageShowArea = new MessageShowArea();
    vlayout->addWidget(messageShowArea);

    // 5.添加消息编辑区
    messageEditArea = new MessageEditArea();
    vlayout->addWidget(messageEditArea, 0, Qt::AlignBottom);

    mainLayout->addWidget(chatArea);
    //////////////////////////////////////////////////
    /// 用户信息展示区
    //////////////////////////////////////////////////
    QVBoxLayout* info_layout = new QVBoxLayout;
    info_layout->setSpacing(0);
    info_layout->setContentsMargins(60,35,60,35);
    // 通过userInfoPage->layout()->remove, addWidget

    userInfoPage->setLayout(info_layout);
    mainLayout->addWidget(userInfoPage);

    // 右侧真正初始可见内容
    defaultPage->show();
    chatArea->hide();
    userInfoPage->hide();
}

void MainWidget::initSignalSlot()
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();

    ////////////////////////////////////////
    /// 连接信号槽，处理标签页按钮切换的问题
    ////////////////////////////////////////
    connect(sessionTabBtn,&QPushButton::clicked,this,&MainWidget::switchTabToSession);
    connect(friendTabBtn,&QPushButton::clicked,this,&MainWidget::switchTabToFriend);
    connect(applyTabBtn,&QPushButton::clicked,this,&MainWidget::switchTabToApply);

    ////////////////////////////////////////
    /// 点击自己头像，显示个人主页
    ////////////////////////////////////////
    connect(userAvatar, &QPushButton::clicked,this,[=](){
        SelfInfoWidget* selfInfoWidget = new SelfInfoWidget(this);
        selfInfoWidget->show(); //弹出模态
    });

    ////////////////////////////////////////
    /// 点击会话详情按钮，显示会话详细信息
    ////////////////////////////////////////
    connect(sessionTitleLabel, &QLabel::linkActivated, this, [=](const QString& link){
        emit extraBtn->clicked();
    });
    connect(extraBtn, &QPushButton::clicked,this,[=](){
        model::ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionById(dataCenter->getCurrentChatSessionId());
        if(chatSessionInfo == nullptr){
            Toast::showMessage("当前未选中任何会话",false);
            return;
        }

        bool isSingleChat = (chatSessionInfo->userId != ""); // test
        if(isSingleChat){
            model::UserInfo* userInfo = dataCenter->findFriendByUserId(chatSessionInfo->userId);
            if(userInfo == nullptr){
                Toast::showMessage("单聊会话对应用户不存在",false);
                return;
            }
            // 单聊，弹出这个窗口
            SessionDetailWidget* sessionDetailWidget = new SessionDetailWidget(*userInfo, this);
            sessionDetailWidget->show(); //弹出非模态
        }else{
            // 群聊，弹出另一个窗口
            GroupSessionDetailWidget* groupSessionDetailWidget = new GroupSessionDetailWidget(this);
            groupSessionDetailWidget->show();
        }
    });


    /////////////////////////////////////////////
    /// 点击添加好友按钮, 弹出添加好友窗口
    /////////////////////////////////////////////
    connect(addFriendBtn, &QPushButton::clicked,this,[this](){
        AddFriendDialog* addFriendDialog = AddFriendDialog::getInstance();
        addFriendDialog->setSearchKey("");
        if(addFriendDialog->isHidden()) addFriendDialog->show();
    });


    /////////////////////////////////////////////
    /// 从搜索界面来到聊天界面
    /////////////////////////////////////////////
    connect(this, &MainWidget::toChatFromSearch, this, [=](const QString& userId){
        searchPage->hide();
        windowMid->show();
        this->switchSession(userId);
    });

    /////////////////////////////////////////////
    /// 获取个人信息
    ///     DataCenter::提供一个具体的方法，来获取到具体的数据
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::getMyselfDone, this, [=](){
        // 从datacenter拿到响应结果的myself，把里面的头像取出来，显式出来
        auto myself = dataCenter->getMyself();
        userAvatar->setIcon(myself->avatar);
    });
    dataCenter->getMyselfAsync();


    /////////////////////////////////////////////
    /// 获取好友列表
    /////////////////////////////////////////////
    // 提供一个具体的方法，来获取到具体的数据
    loadFriendList();

    /////////////////////////////////////////////
    /// 获取会话列表
    /////////////////////////////////////////////
    loadSessionList();

    /////////////////////////////////////////////
    /// 获取申请列表
    /////////////////////////////////////////////
    loadApplyList();


    /////////////////////////////////////////////
    /// 发送消息功能
    ///     messageEditArea: sendTextBtn()
    ///     1. 处理 发送按钮 点击操作
    ///     2. 把输入框中的内容, 通过网络传输给服务器
    ///     3. 发送消息成功, 消息展示区 显示刚才发的消息
    ///     4. 清空输入框的文本消息
    /////////////////////////////////////////////

    /////////////////////////////////////////////
    /// 处理修改头像
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::changeAvatarDone, this, [=](){
        userAvatar->setIcon(dataCenter->getMyself()->avatar);
    });

    /////////////////////////////////////////////
    /// 处理删除好友
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::deleteFriendDone, this, [=](){
        // 更新会话列表和好友列表
        this->updateFriendList();
        this->updateChatSessionList();
        this->showDefault();
        LOG() << "删除好友完成";
        // Toast::showMessage("删除好友完成");
    });


    /////////////////////////////////////////////
    /// 清空当前选中会话页面
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::clearCurrentSession, this, [=](){
        sessionTitleLabel->setText("");
        messageShowArea->clear();
        messageEditArea->clear();
        LOG() << "清空当前会话完成";
    });

    /////////////////////////////////////////////
    /// 处理发送好友申请
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::addFriendApplyDone, this, [=](){
        Toast::showMessage("好友申请已发送",false);
    });

    /////////////////////////////////////////////
    /// 处理接收到了一个好友申请
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::receiveFriendApplyDone, this, [=](){
        Toast::showMessage("收到了一条好友申请, 注意查看", true);
        this->updateApplyList();
    });

    /////////////////////////////////////////////
    /// 处理同意好友申请
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::acceptFriendApplyDone, this, [=](){
        this->updateApplyList();    //删除申请项
        this->updateFriendList();   //添加好友项
        Toast::showMessage("已成功添加好友",false);
    });

    /////////////////////////////////////////////
    /// 处理好友申请结果的推送数据
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::receiveFriendProcessDone, this, [=](const UserInfo& userInfo,bool agree){
        if(agree){
            // 同意
            this->updateFriendList();
            Toast::showMessage(QString("%1(%2)同意了你的好友申请").arg(userInfo.nickname).arg(userInfo.userId),false);
        }else{
            // 拒绝
            Toast::showMessage(QString("%1(&2)拒绝了你的好友申请").arg(userInfo.nickname).arg(userInfo.userId),false);
        }
    });

    /////////////////////////////////////////////
    /// 处理拒绝好友申请
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::rejectFriendApplyDone, this, [=](){
        this->updateApplyList();
        LOG()<<"好友申请已经拒绝";
    });

    /////////////////////////////////////////////
    /// 处理创建群聊的响应信号
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::createGroupChatSessionDone, this, [=](){
        // Toast::showMessage("创建群聊会话请求成功发送!");
        LOG() << "创建群聊会话请求成功发送!";
    });
    connect(dataCenter, &model::DataCenter::receiveSessionCreateDone, this, [=](){
        Toast::showMessage(QString("加入了一个新群聊"), false);
        this->updateChatSessionList();  //更新会话列表
    });

    /////////////////////////////////////////////
    /// 接收一条好友消息, 主界面需要闪烁系统托盘
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::trayIconBlink, this, [=](){
        isBlinking = true;
        binkTimer->start();
        // trayIcon->showMessage("通知", "你有一条新的消息", normalIcon, 3000);
    });
    connect(dataCenter, &model::DataCenter::trayIconBlinkDone, this, [=](){
        isBlinking = false;
        binkTimer->stop();
        trayIcon->setIcon(*normalIcon);
    });
}

void MainWidget::initWebsocket()
{
    model::DataCenter::getInstance()->initWebsocket();    //初始化 websocket
}

void MainWidget::switchTabToSession()
{
    // 1.记录当前切换到了哪个标签页
    activeTab = SESSION_LIST;
    // 2.调整图标显示情况，把对应的按钮设置为激活状态，另外两个设置为invalid
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_active.png"));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_inactive.png"));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_inactive.png"));
    // 3.在主窗口的中间部分，加载出会话列表数据
    this->loadSessionList();
    this->showChatArea();
}

void MainWidget::switchTabToFriend()
{
    activeTab = FRIEND_LIST;
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_active.png"));
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_inactive.png"));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_inactive.png"));
    this->loadFriendList();
    this->showDefault();
}

void MainWidget::switchTabToApply()
{
    activeTab = APPLY_LIST;
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_active.png"));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_inactive.png"));
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_inactive.png"));
    this->loadApplyList();
    this->showDefault();
}


void MainWidget::loadSessionList()
{
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    if(dataCenter->getChatSessionList() != nullptr){
        updateChatSessionList();
    } else {
        connect(dataCenter, &DataCenter::getChatSessionListDone,
                this, &MainWidget::updateChatSessionList,
                Qt::UniqueConnection);
        dataCenter->getChatSessionListAsync();
    }
}

void MainWidget::loadFriendList()
{
    using model::DataCenter;
    // 好友列表是在dataCenter中存储的
    // 首先判定DataCenter中是否有数据了，如果有就从本地加载
    // 如果没有, 从服务器获取
    DataCenter* dataCenter = DataCenter::getInstance();
    if(dataCenter->getFriendList() != nullptr){
        // 从内存加载列表
        updateFriendList();
    } else {
        // 从网络加载数据
        // Qt::UniqueConnection: 确保唯一绑定一次。
        connect(dataCenter, &DataCenter::getFriendListDone,
                this, &MainWidget::updateFriendList,
                Qt::UniqueConnection);
        dataCenter->getFriendListAsync();
    }
}

void MainWidget::loadApplyList()
{
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    if(dataCenter->getApplyList() != nullptr){
        updateApplyList();
    } else {
        connect(dataCenter, &DataCenter::getApplyListDone,
                this, &MainWidget::updateApplyList,
                Qt::UniqueConnection);
        dataCenter->getApplyListAsync();
    }
}

void MainWidget::updateFriendList()
{
    if(activeTab != FRIEND_LIST) {
        // 如果当前标签页不是好友列表, 就不渲染任何数据到界面上
        return;
    }
    using model::DataCenter;
    DataCenter* dc = DataCenter::getInstance();
    QList<model::UserInfo>* friendList = dc->getFriendList();

    // 清空一下之前界面上的数据
    sessionFriendArea->clear();

    // 遍历好友，添加到界面上
    for(const auto& f: *friendList){
        sessionFriendArea->addItem(
            FriendItemType,
            f.userId,f.avatar,f.nickname,f.description
            );
    }
}

void MainWidget::updateChatSessionList()
{
    if(activeTab != SESSION_LIST) {
        // 如果当前标签页不是会话列表, 就不渲染任何数据到界面上
        return;
    }
    using model::DataCenter;
    DataCenter* dc = DataCenter::getInstance();
    QList<model::ChatSessionInfo>* chatSessionList = dc->getChatSessionList();
    // LOG() << chatSessionList->count();

    sessionFriendArea->clear();

    for (const auto& c : *chatSessionList) {
        if (c.lastMessage.messageType == model::TEXT_TYPE) {
            sessionFriendArea->addItem(SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, c.lastMessage.content);
        } else if (c.lastMessage.messageType == model::IMAGE_TYPE) {
            sessionFriendArea->addItem(SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, "[图片]");
        } else if (c.lastMessage.messageType == model::FILE_TYPE) {
            sessionFriendArea->addItem(SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, "[文件]");
        } else if (c.lastMessage.messageType == model::SPEECH_TYPE) {
            sessionFriendArea->addItem(SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, "[语音]");
        } else {
            // LOG() << "错误的消息类型! messageType=" << c.lastMessage.messageType;
            sessionFriendArea->addItem(SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, "");
        }
    }
}

void MainWidget::updateApplyList()
{
    if(activeTab != APPLY_LIST) {
        // 如果当前标签页不是申请列表, 就不渲染任何数据到界面上
        return;
    }
    using model::DataCenter;
    DataCenter* dc = DataCenter::getInstance();
    QList<UserInfo>* applyList = dc->getApplyList();

    sessionFriendArea->clear();

    for (const auto& u : *applyList) {
        sessionFriendArea->addItem(ApplyItemType, u.userId, u.avatar, u.nickname, "");
    }
}


void MainWidget::loadRecentMessage(const QString &chatSessionId)
{
    using model::DataCenter;
    // 判断本地内存是否有对应的会话消息
    DataCenter* dataCenter = DataCenter::getInstance();
    if(dataCenter->getRecentMessageList(chatSessionId) != nullptr){
        // 拿着本地数据更新界面
        updateRecentMessage(chatSessionId);
    } else {
        // 本地没有数据，从网络加载
        connect(dataCenter, &DataCenter::getRecentMessageListDone,
                this, &MainWidget::updateRecentMessage,
                Qt::UniqueConnection);
        dataCenter->getRecentMessageListAsync(chatSessionId, true);
    }

}

void MainWidget::updateRecentMessage(const QString &chatSessionId)
{
    using model::DataCenter;
    // 1.拿到该会话的最近消息列表
    DataCenter* dataCenter = DataCenter::getInstance();
    auto* recentMessageList = dataCenter->getRecentMessageList(chatSessionId);

    // 2.清空原有界面上显示的消息列表
    messageShowArea->clear();

    // 3.根据当前拿到的消息列表, 显示到界面上
    // 可以头插、也可以尾插、这里使用头插实现
    // 主要是因为消息列表，用户先看到的是最近消息，也是末尾消息
    for(int i=recentMessageList->size()-1; i>=0; --i){
        const Message& message = recentMessageList->at(i);
        bool isLeft = (message.sender.userId != dataCenter->getMyself()->userId);
        messageShowArea->addFrontMessage(isLeft, message);
    }

    // 4.设置会话标题
    model::ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionById(chatSessionId);
    if(chatSessionInfo != nullptr){
        // 把会话的名字显示到界面上
        QString showText = QString("<a href=\"extraBtn\" style=\"text-decoration: none; color: #161616;\">%1</a>")
                               .arg(chatSessionInfo->chatSessionName);
        sessionTitleLabel->setText(showText);

        QString style = QString("QLabel { width: %1px; height: %2px; font-size: 22px; }")
                            .arg(sessionTitleLabel->sizeHint().width())
                            .arg(sessionTitleLabel->sizeHint().height());
        sessionTitleLabel->setStyleSheet(style);
    }

    // 5.保存当前选中的会话是哪一个
    dataCenter->setCurrentChatSessionId(chatSessionId);

    // 6.自动把滚动条滚动到末尾
    // 由于步骤3.的绘制是异步的，所以此处
    // 可能会有一个中间状态
    // 所以这里不一定会滚动到最后
    // 那么就要在scrollToEnd()中考虑这个问题
    messageShowArea->scrollToEnd();
    this->showChatArea();
}

void MainWidget::switchSession(const QString &userId)
{
    using model::DataCenter;
    using model::ChatSessionInfo;

    // 1) 在会话列表中, 先找到对应的会话元素
    DataCenter* dataCenter = DataCenter::getInstance();
    ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionByUserId(userId);
    if(chatSessionInfo == nullptr){
        // 正常来讲, 每个好友, 都会有一个对应的会话(哪怕从来没有聊过天)
        // 添加好友时, 就会自动创建会话
        LOG() << "[严重错误] 当前选中的好友, 对应的会话不存在!";
        return;
    }

    // 2) 把选中的会话置顶 - 把这个会话信息放到整个会话列表的第一个位置
    //                    后续显示时, 就是按照列表的顺序, 从前往后显示的
    dataCenter->topChatSessionInfo(*chatSessionInfo);

    // 3) 切换到会话列表标签页
    this->switchTabToSession();

    // 4) 在消息展示区, 加载出对应会话的最近消息
    sessionFriendArea->clickItem(0);
}

MessageShowArea *MainWidget::getMessageShowArea()
{
    return messageShowArea;
}

void MainWidget::showDefault(){
    // 右侧显式默认页
    userInfoPage->hide();
    chatArea->hide();
    defaultPage->show();
}

void MainWidget::showChatArea(){
    if(model::DataCenter::getInstance()->getCurrentChatSessionId()==""){
        this->showDefault();
    } else {
        // 右侧显式消息区
        defaultPage->hide();
        userInfoPage->hide();
        chatArea->show();
    }
}

void MainWidget::showUserInfo(const model::UserInfo& userInfo)
{
    // 先清空原来的展示区
    QVBoxLayout* info_layout = dynamic_cast<QVBoxLayout*>(userInfoPage->layout());
    for(int i=info_layout->count()-1; i >= 0; --i){
        QLayoutItem* item = info_layout->takeAt(i);
        if(item->widget()){
            delete item->widget();
        }
    }

    // 展示信息当前信息
    QWidget* userInfoWidget = new FriendDetail(userInfo);
    info_layout->addWidget(userInfoWidget);

    defaultPage->hide();
    chatArea->hide();
    userInfoPage->show();
}


void MainWidget::launchLater()
{
    // Toast::showMessage("Hello, Welcome to MyChat", true);

    // 1.创建主窗体
    QDialog* progressDialog = new QDialog();
    progressDialog->setFixedSize(400,150);
    progressDialog->setWindowFlag(Qt::FramelessWindowHint);

    // 2.创建布局
    QVBoxLayout* layout = new QVBoxLayout(progressDialog);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    // 3.创建说明标签
    QLabel* label = new QLabel("Welcom to MyChat", progressDialog);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("QLabel {background-color: transparent; font-size: 20px;}");
    layout->addWidget(label);

    // 4.创建进度条
    QProgressBar* progressBar = new QProgressBar(progressDialog);
    progressBar->setFixedHeight(100);
    progressBar->setRange(0, 100);      //0-100的范围
    progressBar->setValue(0);           //初始值
    progressBar->setTextVisible(true);  //显示百分比文本

    const QString& style = R"(
        /* 进度条整体样式 */
        QProgressBar {
            background-color: rgb(255,255,255);
            border: 1px solid rgb(225, 225, 225);
            border-radius: 8px;     /* 圆角 */
            text-align: center;     /* 文本居中 */
            height: 12px;           /* 进度条高度 */
            min-width: 200px;       /* 最小宽度 */
            font-size: 24px;
            font-weight: 500;
        }
        /* 进度条填充部分样式 */
        QProgressBar::chunk {
            background-color: rgb(7, 193, 96);      /* 主体颜色 */
            border-radius: 7px;                     /* 与整体圆角匹配 */
        }
    )";
    progressBar->setStyleSheet(style);
    layout->addWidget(progressBar);

    // 5.创建定时器控制进度
    QTimer* timer = new QTimer(progressDialog);
    timer->setInterval(2000 / 100);    //更新频率时间分成100份
    int currentProgress = 0;
    connect(timer, &QTimer::timeout, [=, &currentProgress](){
        currentProgress++;
        if(currentProgress > 100){
            currentProgress = 100;
            timer->stop();
            timer->deleteLater();

            this->show();   //显示窗体
            progressDialog->close();
        }

        progressBar->setValue(currentProgress);
    });
    timer->start();
    progressDialog->exec();
}

void MainWidget::setupSystemTrayIcon()
{
    // 1.确保应用程序在关闭最后一个窗口时不退出
    qApp->setQuitOnLastWindowClosed(false);
    // 2.创建系统托盘图标
    trayIcon = new QSystemTrayIcon(qApp);
    normalIcon = new QIcon(":/resource/image/logo.png");
    emptyIcon = new QIcon();//用于闪烁的空白图标
    trayIcon->setIcon(*normalIcon);
    trayIcon->setToolTip(this->toolTip());

    // 3.创建右键菜单
    QMenu* trayMenu = new QMenu();
    QAction* quitAction = new QAction("退出程序", trayMenu);
    QObject::connect(quitAction, &QAction::triggered, this, [=](){
        if(Toast::quitApplicationBox()){
            trayIcon->hide();
            this->close();
            qApp->quit();
        }
    });
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);

    // 4.左键点击托盘显示/隐藏窗口
    QObject::connect(trayIcon, &QSystemTrayIcon::activated, this,
                     [=](QSystemTrayIcon::ActivationReason reason)
                     {
                         if(reason == QSystemTrayIcon::Trigger){
                             //左键被点击
                             if(this->isVisible()){
                                 this->hide();
                             } else {
                                 this->showNormal();
                                 this->raise();
                             }
                         }
                     });

    // 5.实现消息闪烁功能
    bool* currentState = new bool(false);
    binkTimer = new QTimer(qApp);
    binkTimer->setInterval(500);    //500ms切换一次
    QObject::connect(binkTimer, &QTimer::timeout, this, [=](){
        *currentState = !(*currentState);
        trayIcon->setIcon( (*currentState)? (*emptyIcon):(*normalIcon) );
    });


    bool* isFirst = new bool(true);
    QObject::connect(this, &MainWidget::closing, this, [=](){
        this->hide();   //隐藏而非关闭
        if(*isFirst){   //只有程序启动后的第一次隐藏才显示
            trayIcon->showMessage("提示", "程序已最小化到系统托盘", *normalIcon, 500);
            *isFirst = false;
        }
    });
    trayIcon->show();
}

bool MainWidget::eventFilter(QObject *watched, QEvent *event) {
    // 1. 处理编辑框获得焦点事件
    if (watched == searchEdit && event->type() == QEvent::FocusIn) {
        // qDebug() << "搜索框获得焦点，显示搜索窗口";
        searchPage->release();  //显示前需要清理一下界面
        searchPage->show();

        windowMid->hide();
        return true;
    }

    // 2. 处理鼠标点击事件（判断是否需要隐藏搜索窗口）
    if (event->type() == QEvent::MouseButtonPress) {
        // qDebug()<<"press mouse";

        // 只有当搜索窗口可见时才需要判断
        if (searchPage->isVisible()) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint globalPos = mouseEvent->globalPos();  // 使用全局坐标

            // 将全局坐标转换为控件本地坐标
            QPoint searchEditLocal = searchEdit->mapFromGlobal(globalPos);
            QPoint searchPageLocal = searchPage->mapFromGlobal(globalPos);

            // 判断点击是否在控件区域内
            bool isInSearchEdit = searchEdit->rect().contains(searchEditLocal);
            bool isInsearchPage = searchPage->rect().contains(searchPageLocal);

            // 如果点击在外部，则隐藏搜索窗口
            if (!isInSearchEdit && !isInsearchPage) {
                // qDebug() << "点击外部区域，隐藏搜索窗口";
                searchPage->hide();
                windowMid->show();

                // 清除编辑框焦点（可选）
                if (searchEdit->hasFocus()) {
                    searchEdit->clearFocus();
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}








