#include "sessiondetaildialog.h"

#include "choosefrienddialog.h"
#include "common/debug.h"
#include "common/toast.h"
#include "model/datacenter.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QEvent>
#include <QMouseEvent>

//////////////////////////////////////
/// 表示一个头像 + 一个名字的组合控件
//////////////////////////////////////

AvatarItem::AvatarItem(const QIcon &avatar, const QString &name)
{
    // 1. 设置自身属性
    this->setFixedSize(70,80);

    // 2. 创建布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->setAlignment(Qt::AlignHCenter);
    this->setLayout(layout);

    // 3.创建头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(45, 45);
    avatarBtn->setIconSize(QSize(45,45));
    avatarBtn->setIcon(avatar);
    avatarBtn->setStyleSheet("QPushButton { border: none; }");

    // 4.创建名字
    nameLabel = new QLabel();
    nameLabel->setText(name);
    QFont font("微软雅黑", 12);
    nameLabel->setFont(font);
    nameLabel->setAlignment(Qt::AlignCenter);

    // 5.对名字作一个截断
    //(1)约定一个最大的宽度
    const int MAX_WIDTH = 65;
    //(2)针对文本尺寸进行测量
    // QFontMetrics metrics(nameLabel->text());
    QFontMetrics metrics(font);
    //(3)计算总宽度, 如果大于MAX_WIDTH更小，不需要截断
    int totWidth = metrics.horizontalAdvance(name);
    //(4)否则截断末尾多于的部分，替换为"..."
    if(totWidth >= MAX_WIDTH){
        QString tail = "…";
        int tailWidth = metrics.horizontalAdvance(tail);/*测量...的宽度*/
        int availableWidth = MAX_WIDTH - tailWidth;/*截断后的宽度*/
        int avaliableSize = name.size() * ((double)availableWidth / totWidth);/*换算后的字符个数*/
        QString newName = name.left(avaliableSize);/*按照字符个数截取字符*/
        nameLabel->setText(newName + tail);
    }

    // 6.
    layout->addWidget(avatarBtn, 0,Qt::AlignHCenter);
    layout->addWidget(nameLabel, 0,Qt::AlignHCenter);

    // connect(avatarBtn, &QPushButton::clicked, this,[=](){});
}



//////////////////////////////////////
/// 表示 "单聊会话详细" 窗口
//////////////////////////////////////

SessionDetailWidget::SessionDetailWidget(const model::UserInfo& userInfo, QWidget* parent)
    :QDialog(parent)
    ,userInfo(userInfo)
{
    // 1. 设置基本属性
    this->setWindowTitle("会话详情");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setFixedSize(300, parent->height());
    this->setStyleSheet("QWidget { background-color: rgb(245, 245, 245); }");
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->move(parent->x()+parent->width(), parent->y());

    this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);// 设置窗口标志为Popup

    // 2. 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(50, 0, 50, 0);
    layout->setAlignment(Qt::AlignCenter);
    this->setLayout(layout);

    // 3. 添加 "创建群聊" 按钮
    AvatarItem* createGroupBtn = new AvatarItem(QIcon(":/resource/image/cross.png"), "添加");
    layout->addWidget(createGroupBtn, 0, 0);

    // 4. 添加当前用户的信息 (临时构造的假数据)
#if TEST_UI
    AvatarItem* currentUser = new AvatarItem(QIcon(":/resource/image/defaultAvatar.png"), "张三123456");
    layout->addWidget(currentUser, 0, 1);
#else
    // 单聊界面，默认添加对方头像和昵称
    AvatarItem* currentUser = new AvatarItem(userInfo.avatar, userInfo.nickname);
    layout->addWidget(currentUser, 0, 1);
#endif

    // 5. 添加 "删除好友" 按钮
    deleteFriendBtn = new QPushButton();
    deleteFriendBtn->setFixedHeight(50);
    deleteFriendBtn->setText("删除好友");
    QString style = R"(
        QPushButton {
            border: 1px solid rgb(90, 90, 90);
            border-radius: 5px;
        }
        QPushButton:hover {
            color: rgb(255, 255, 255);
            background-color: rgb(237, 76, 76); /*  hover时变红 */
        }
        QPushButton:pressed {
            color: rgb(255, 255, 255);
            background-color: rgb(225, 73, 73);
        }
    )";
    deleteFriendBtn->setStyleSheet(style);
    layout->addWidget(deleteFriendBtn, 1, 0, 1, 3);

    // 6.添加信号槽处理点击 "创建群聊" 按钮
    connect(createGroupBtn->getAvatar(), &QPushButton::clicked,this,[=](){
        ChooseFriendDialog* chooseFriendDialog = new ChooseFriendDialog(this);
        chooseFriendDialog->exec();
        // chooseFriendDialog->show();
    });
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
    });
}












