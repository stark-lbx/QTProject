#include "mainwidget.h"

#include "userinfodialog.h"
#include "common/toast.h"
#include "model/datacenter.h"

#include <QGridLayout>
#include <QPushButton>

UserInfoWidget::UserInfoWidget(const UserInfo& userInfo, QWidget* parent)
    :QDialog(parent),userInfo(userInfo)
{
    // 1.设置基本属性
    this->setFixedSize(400,200);
    this->setWindowTitle("用户详情");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setStyleSheet("QDialog {background-color: rgb(255,255,255);}");

    this->setWindowFlags(Qt::Popup);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->move(QCursor::pos());

    // 2.创建布局管理器
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(40,20,40,20);
    this->setLayout(mainLayout);

    QGridLayout* glayout = new QGridLayout();
    glayout->setVerticalSpacing(10);
    glayout->setHorizontalSpacing(15);
    glayout->setAlignment(Qt::AlignHCenter);
    mainLayout->addLayout(glayout);

    // 3.添加头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(75,75);
    avatarBtn->setIconSize(QSize(75,75));
    avatarBtn->setIcon(userInfo.avatar);

    QString btnStyle = R"(
        QPushButton{
            border: 1px solid rgb(100,100,100);
            border-radius: 5px;
            background-color: rgb(240,240,240);
        }
        QPushButton:pressed {
            background-color: rgb(205,205,205);
        }
    )";
    QString infoStyle = "QLabel {color: rgb(142, 142, 142);}";

    int width = 35;
    int height = 30;

    // 4.添加用户序号
    idTag = new QLabel();
    idTag->setText("序号:");
    idTag->setFixedSize(width, height);
    idTag->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    idLabel = new QLabel();
    idLabel->setText(userInfo.userId);
    idLabel->setFixedHeight(height);
    idLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    idLabel->setStyleSheet(infoStyle);

    // 5.添加用户昵称
    nameTag = new QLabel();
    nameTag->setText("昵称:");
    nameTag->setFixedSize(width, height);
    nameTag->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);


    nameLabel = new QLabel();
    nameLabel->setText(userInfo.nickname);
    nameLabel->setFixedHeight(height);
    nameLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    nameLabel->setStyleSheet(infoStyle);

    // 6.添加用户电话
    phoneTag = new QLabel();
    phoneTag->setText("电话:");
    phoneTag->setFixedSize(width, height);
    phoneTag->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    phoneLabel = new QLabel();
    phoneLabel->setText(userInfo.phone);
    phoneLabel->setFixedHeight(height);
    phoneLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    phoneLabel->setStyleSheet(infoStyle);

    // 7.添加功能按钮
    applyBtn = new QPushButton();
    applyBtn->setText("申请好友");
    applyBtn->setFixedSize(width*2,30);
    applyBtn->setStyleSheet(btnStyle);

    sendMessageBtn = new QPushButton();
    sendMessageBtn->setText("发送消息");
    sendMessageBtn->setFixedSize(width*2,30);
    sendMessageBtn->setStyleSheet(btnStyle);

    deleteFriendBtn = new QPushButton();
    deleteFriendBtn->setText("删除好友");
    deleteFriendBtn->setFixedSize(width*2,30);
    deleteFriendBtn->setStyleSheet(btnStyle);

    // 8.添加到布局管理器
    glayout->addWidget(avatarBtn, 0,0, 3,1);
    glayout->addWidget(idTag, 0,1);
    glayout->addWidget(idLabel, 0,2);
    glayout->addWidget(nameTag, 1,1);
    glayout->addWidget(nameLabel, 1,2);
    glayout->addWidget(phoneTag, 2,1);
    glayout->addWidget(phoneLabel, 2,2,1,3);

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->setSpacing(20);
    hlayout->setContentsMargins(0,0,0,0);
    hlayout->addWidget(applyBtn);
    hlayout->addWidget(sendMessageBtn);
    hlayout->addWidget(deleteFriendBtn);

    mainLayout->addLayout(hlayout);

    // 9. 初始化按钮的禁用关系
    //      判定依据：当前用户的userId，在DataCenter的好友列表中查询
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    UserInfo* userf = dataCenter->findFriendByUserId(userInfo.userId);
    if(userf == nullptr){
        // 不是好友
        // 申请按钮设置为可用、其它默认禁用
        sendMessageBtn->setEnabled(false);
        deleteFriendBtn->setEnabled(false);
    } else {
        // 是好友
        // 申请按钮设置为禁用
        applyBtn->setEnabled(false);
    }

    // 10. 初始化信号槽
    initSignalSlot();
}

void UserInfoWidget::initSignalSlot()
{
    // 发送消息
    connect(sendMessageBtn, &QPushButton::clicked,this,[=](){
        // 拿到主窗口指针, 通过主窗口中, 前面实现的切换到会话功能直接调用
        MainWidget* mainWidget = MainWidget::getInstance();
        mainWidget->switchSession(userInfo.userId);

        // 本窗口关闭
        this->close();
    });


    // 删除好友
    connect(deleteFriendBtn, &QPushButton::clicked, this,[=](){
        // 1. 弹出对话框, 提示用户是否确认
        bool isOk = Toast::deleteFriendBox(userInfo.nickname);
        if(!isOk){
            // pass: 取消删除
            LOG() << "删除好友取消";
            return;
        }   // else: 确认删除, 准备删除好友

        // 2. 发送网络请求, 实现删除好友功能
        using model::DataCenter;
        DataCenter* dataCenter = DataCenter::getInstance();
        // connect(empty); // 主窗口处理
        dataCenter->deleteFriendAsync(userInfo.userId);

        // 3. 关闭本窗口
        this->close();
    });

    // 发送好友申请
    connect(applyBtn, &QPushButton::clicked, this, [=](){
        // 1.发送好友申请
        using model::DataCenter;
        DataCenter* dataCenter = DataCenter::getInstance();
        dataCenter->addFriendApplyAsync(userInfo.userId);

        // 2. 关闭窗口
        this->close();
    });
}















