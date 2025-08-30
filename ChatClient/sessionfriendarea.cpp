#include "sessionfriendarea.h"

#include "mainwidget.h"
#include "soundrecorder.h"
#include "model/data.h"
#include "model/datacenter.h"
#include "common/debug.h"
#include "common/toast.h"

#include <QLabel>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStyleOption>
#include <QPainter>

SessionFriendArea::SessionFriendArea(QWidget *parent)
    : QScrollArea{parent}
{
    // 1. 设置必要的属性
    // 设置了这个属性，才能开启滚动效果
    this->setWidgetResizable(true);
    // 设置滚动条相关的样式
    this->setStyleSheet("QWidget { border: none;}");
    // 垂直方向滚动条
    this->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 2px; background-color: rgb(195, 195, 195); }") ;
    // 水平方向滚动条
    // this->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px; }");
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //关闭水平滚动条

    // 2. 把 widget 创建出来
    container = new QWidget();
    // container->setFixedHeight(280);
    this->setWidget(container);

    // 3. 给 widget 指定布局管理器，以便后续添加元素
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignTop);
    container->setLayout(layout);

    // 4. 简单测试滚动的效果
    // for(int i=0;i<500;i++){
    //     QPushButton* btn = new QPushButton("按钮");
    //     layout->addWidget(btn);
    // }

// 构造出一些临时数据, 用来作为 "界面调试" 依据. 后续要删除掉
#if TEST_UI
    QIcon icon(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 30; ++i) {
        // this->addItem(icon, "张三" + QString::number(i), "最后一条消息" + QString::number(i));

        this->addItem(SessionItemType, QString::number(i), icon, "张三" + QString::number(i), "最后一条消息" + QString::number(i));
        this->addItem(ApplyItemType, QString::number(i), icon, "张三" + QString::number(i), "最后一条消息" + QString::number(i));
    }
#endif
}

void SessionFriendArea::clear()
{
    QLayout* layout = container->layout();
    //遍历布局管理器中所有元素，并依次从布局中删除
    for(int i = layout->count()-1; i>=0; --i){
        //takeAt 就能移除对应下标的元素
        QLayoutItem* item = layout->takeAt(i);
        //如果这个对象指针不空，要释放
        if(item->widget()){
            //正常使用时，new出来添加到布局管理器中的...
            delete item->widget();
            // item->widget()->deleteLater();
        }
    }
}

void SessionFriendArea::addItem(ItemType itemType, const QString &id, const QIcon &avatar, const QString &name, const QString &text)
{
    SessionFriendItem* item = nullptr;
    if(itemType == SessionItemType){
        item = new SessionItem(this,id,avatar,name,text);
    }else if(itemType==FriendItemType){
        item = new FriendItem(this,id,avatar,name,text);
    }else if(itemType==ApplyItemType){
        item = new ApplyItem(this,id,avatar,name);
    }else{
        LOG() <<"错误的ItemType! type="<<itemType;
        return;
    }

    container->layout()->addWidget(item);
}

// void SessionFriendArea::addItem(const QIcon &avatar, const QString &name, const QString &text)
// {
//     SessionFriendItem* item = new SessionFriendItem(this, avatar, name, text);
//     container->layout()->addWidget(item);
// }

void SessionFriendArea::clickItem(int index)
{
    if(index < 0 || index >= container->layout()->count()){
        LOG() << "点击元素的下标超出范围! index="<<index;
        return;
    }
    QLayoutItem* layoutItem = container->layout()->itemAt(index);
    if(layoutItem==nullptr || layoutItem->widget()==nullptr){
        LOG() <<"指定的元素不存在! index="<<index;
        return;
    }
    SessionFriendItem* item = dynamic_cast<SessionFriendItem*>(layoutItem->widget());
    item->select();
}


/////////////////////////////////////////////////
/// 滚动区域中的 Item 的实现
/////////////////////////////////////////////////

SessionFriendItem::SessionFriendItem(QWidget *owner, const QIcon &avatar, const QString &name, const QString &text)
    :owner{owner}
{
    this->setFixedHeight(85);
    this->setStyleSheet("QWidget {background-color: rgb(231,231,231);}");

    //创建网格布局管理器
    QGridLayout* layout = new QGridLayout();
    // layout->setSpacing(0);
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(0);
    layout->setContentsMargins(15,20,15,20);
    this->setLayout(layout);

    //创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(45,45);
    avatarBtn->setIconSize(QSize(45,45));
    avatarBtn->setIcon(avatar);
    avatarBtn->setStyleSheet("QPushButton { background-color: transparent; border: none;}");
    avatarBtn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);//尺寸策略

    //创建名字
    nameLabel = new QLabel();
    nameLabel->setText(name);
    nameLabel->setFixedHeight(25);
    nameLabel->setMaximumWidth(195);
    nameLabel->setStyleSheet("QLabel { font-size: 18px; color: rgb(22,22,22); background-color: transparent; }");
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setAlignment(Qt::AlignTop);

    //创建信息预览 label
    messageLabel = new QLabel();
    messageLabel->setText(text);
    messageLabel->setFixedHeight(20);
    messageLabel->setMaximumWidth(195);
    messageLabel->setStyleSheet("QLabel { font-size: 14px; color: rgb(135, 135, 135); background-color: transparent; }");
    messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    messageLabel->setAlignment(Qt::AlignBottom);

    //头像处于0,0位置，占据2行2列
    layout->addWidget(avatarBtn, 0, 0, 2, 2);
    //名字处于0,2位置
    layout->addWidget(nameLabel, 0, 2);
    //预览处于1,2位置
    layout->addWidget(messageLabel, 1, 2);
}

void SessionFriendItem::paintEvent(QPaintEvent *event)
{
    (void) event;
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this) ;
}

void SessionFriendItem::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) select();
    else {
        event->ignore();
        // popup(); // 弹出一个弹窗，对这个会话进行操作
    }
}

void SessionFriendItem::enterEvent(QEnterEvent *event)
{
    (void) event;
    if(this->selected) return;  //被选中状态不受影响
    this->setStyleSheet("QWidget {background-color: rgb(217,217,217);}");
}

void SessionFriendItem::leaveEvent(QEvent *event)
{
    (void) event;
    if(this->selected) return;  //被选中状态不受影响
    this->setStyleSheet("QWidget {background-color: rgb(231,231,231);}");
}

void SessionFriendItem::select()
{
    // 遍历其它元素
    const QObjectList children = this->parentWidget()->children();
    for(QObject* child : children){
        if(!child->isWidgetType()){
            //判断是否是widget
            continue;
        }
        //确定是widget了，就把这个child强转成sessionfrienditem
        SessionFriendItem* item = dynamic_cast<SessionFriendItem*>(child);
        if(item->selected){
            item->selected = false;
            item->setStyleSheet("QWidget {background-color: rgb(231,231,231);}");
        }
    }

    // 鼠标点击时-修改背景色
    this->setStyleSheet("QWidget {background-color: rgb(197,197,197);}");
    // 也要还原其它元素的背景色
    this->selected = true;

    this->active();
}

void SessionFriendItem::active()
{
    // 父类的active，无需实现
}


/////////////////////////////////////////////////
/// 会话 Item 的实现
/////////////////////////////////////////////////

SessionItem::SessionItem(QWidget *owner, const QString &chatSessionId, const QIcon &avatar, const QString &name, const QString &lastMessage)
    :SessionFriendItem(owner, avatar, name, lastMessage)
    ,chatSessionId(chatSessionId)
    ,showText(lastMessage)
{
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();

    // 处理最后一条消息的信号
    connect(dataCenter, &DataCenter::updateLastMessage, this, &SessionItem::updateLastMessage);

    // 需要显示未读消息数目
    int unread = dataCenter->getUnread(chatSessionId);
    if(unread > 0){
        this->messageLabel->setText(
            QString("[%1条未读] %2")
                .arg(unread)
                .arg(this->showText));
    }
}

//加载会话的历史消息列表
void SessionItem::active()
{
    auto* dataCenter = model::DataCenter::getInstance();
    if(chatSessionId == dataCenter->getCurrentChatSessionId()){
        // 这个Item的id与当前已经选中的id一致:
        // 不再重新加载
        // 甚至可以拓展到: 取消选中
        return;
    }

    // 加载会话历史消息
    // 即会涉及当前内存的数据操作
    // 又会涉及网络通信
    // 还涉及界面更新
    MainWidget* mainWidget = MainWidget::getInstance();
    mainWidget->loadRecentMessage(chatSessionId);

    // 清空未读消息的数目
    dataCenter->clearUnread(chatSessionId);
    // 更新界面的显示
    this->messageLabel->setText(this->showText);
}

void SessionItem::updateLastMessage(const QString &chatSessionId)
{
    // 1.判定chat_session_id是否匹配
    if(this->chatSessionId != chatSessionId) {
        return; /* pass */
        // 每个 SessionItem 都会连接信号槽
        // 当信号来了, 所有的对象的槽函数都会被触发
        // 如果聊天会话ID不一致，说明不是正在发消息的SessionItem
    }

    /* else chatSessionId匹配了, 才真正更新最后一条消息 */
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    // 2.获取最后一条消息
    QList<Message>* messageList = dataCenter->getRecentMessageList(chatSessionId);
    if(messageList == nullptr || messageList->size()==0){
        // 当前会话没有任何消息, 无需更新
        return;
    }
    const Message& lastMessage = messageList->back();

    // [关键修改] 将DataCenter中的chatSessionList中的lastMessage给同步更新了
    QList<model::ChatSessionInfo>* chatSessionList = dataCenter->getChatSessionList();
    for(auto& cs: *chatSessionList) {
        if(cs.chatSessionId == this->chatSessionId) {
            cs.lastMessage = lastMessage;
            break;
        }
    }

    // 3.明确显示的文本内容
    // 文本消息: 直接显示文本
    // 图片消息: 显示"[图片]"
    // 文件消息: 显示"[文件]"
    // 语音消息: 显示"[语音]"
    // QString showText; // 修订为=>this->showText
    if(lastMessage.messageType == model::TEXT_TYPE){
        this->showText = lastMessage.content;
    } else if(lastMessage.messageType == model::IMAGE_TYPE){
        this->showText = QString("[图片]");
    } else if(lastMessage.messageType == model::FILE_TYPE){
        this->showText = QString("[文件] %1").arg(lastMessage.fileName);
    } else if(lastMessage.messageType == model::SPEECH_TYPE){
        this->showText = QString("[语音] %1\"").arg(SoundRecorder::getPcmContentDuration(lastMessage.content));
    } else {
        LOG() << "Error MessageType!";
        this->showText = "";
        return;
    }

    // 4.显示到界面上的
    //  针对这里的逻辑, 后续还需要考虑到 "未读消息"
    //  关于未读消息的处理, 后续编写接收消息时处理

    // 先判断该会话是否是选中会话
    // 如果是，不管未读消息
    // 如果不是，增加未读消息
    if(chatSessionId == dataCenter->getCurrentChatSessionId()){
        this->messageLabel->setText(this->showText);
    } else {
        int unread = dataCenter->getUnread(chatSessionId);
        if(unread > 0){
            this->messageLabel->setText(
                QString("[%1条未读] %2")
                    .arg(unread)
                    .arg(this->showText));
        }
    }
}

/////////////////////////////////////////////////
/// 好友 Item 的实现
/////////////////////////////////////////////////

FriendItem::FriendItem(QWidget *owner, const QString &userId, const QIcon &avatar, const QString &name, const QString &description)
    :SessionFriendItem(owner,avatar,name,description)
    ,userId(userId)
{}

//激活对应的会话列表
void FriendItem::active()
{
    MainWidget* mainWidget = MainWidget::getInstance();
    // version_1 点击好友列表的元素: 必然是要找到这个好友对应的单聊会话
    // mainWidget->switchSession(userId);

    // version_2 显式用户信息
    // 根据userId获取userInfo
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    model::UserInfo* uInfo = dataCenter->findFriendInfoByUserId(userId);
    mainWidget->showUserInfo(*uInfo);
}

/////////////////////////////////////////////////
/// 好友申请 Item 的实现
///     继承自SesionFriendItem
/////////////////////////////////////////////////

ApplyItem::ApplyItem(QWidget *owner, const QString &userId, const QIcon &avatar, const QString &name)
    :SessionFriendItem(owner, avatar, name, "ID: "+userId)
    ,userId(userId)
{
    QGridLayout* layout = dynamic_cast<QGridLayout*>(this->layout());
    layout->setContentsMargins(15,0,15,0);
    // 1. 移除父类的messageLabel,后面重新设置位置
    // layout->removeWidget(nameLabel);
    // layout->removeWidget(messageLabel);
    this->setFixedHeight(this->height()*3/2);

    // 2. 创建两个按钮
    QPushButton* acceptBtn = new QPushButton();
    acceptBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    acceptBtn->setFixedSize(80,25);
    acceptBtn->setText("同意添加");
    acceptBtn->setStyleSheet("QPushButton{ "
                             "border: none; "
                             "border-radius: 5px; "
                             "background-color: rgb(19, 175, 18); "
                             "color: rgb(255,255,255);"
                             "}");

    QPushButton* rejectBtn = new QPushButton();
    rejectBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    rejectBtn->setFixedSize(80,25);
    rejectBtn->setText("拒绝申请");
    rejectBtn->setStyleSheet("QPushButton{ "
                             "border: none; "
                             "border-radius: 5px; "
                             "background-color: rgb(228, 60, 64); "
                             "color: rgb(255,255,255);"
                             "}");

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(0,0,0,0);
    hlayout->setSpacing(20);
    // 3. 添加到布局管理器中
    hlayout->addWidget(rejectBtn);
    hlayout->addWidget(acceptBtn);
    layout->addLayout(hlayout, 2,2);


    // 4.添加信号槽
    connect(acceptBtn, &QPushButton::clicked, this, &ApplyItem::acceptFriendApply);
    connect(rejectBtn, &QPushButton::clicked, this, &ApplyItem::rejectFriendApply);
}

void ApplyItem::active()
{
    //无事发生
}

void ApplyItem::acceptFriendApply()
{
    // 发送网络请求，告知服务器同意
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    dataCenter->accpetFriendApplyAsync(this->userId);
}

void ApplyItem::rejectFriendApply()
{
    // LOG() << "拒绝了";
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    dataCenter->rejectFriendApplyAsync(this->userId);
}


/////////////////////////////////
/// 朋友详细信息展示区
////////////////////////////////

FriendDetail::FriendDetail(const model::UserInfo &userInfo)
{
    // this->setMinimumSize(400,300);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(5);
    layout->setContentsMargins(0,0,0,0);
    this->setLayout(layout);

    // 创建头像
    QPushButton* avatar = new QPushButton;
    avatar->setFixedSize(75,75);
    avatar->setIconSize(QSize(75,75));
    avatar->setIcon(QIcon(userInfo.avatar));
    layout->addWidget(avatar,0,Qt::AlignHCenter);

    // 创建姓名
    QLabel* nickname = new QLabel;
    nickname->setText(QString("昵称: %1").arg(userInfo.nickname));
    layout->addWidget(nickname);

    // 创建ID
    QLabel* uid = new QLabel;
    uid->setText(QString("ID: %1").arg(userInfo.userId));
    layout->addWidget(uid);

    // 创建电话
    QLabel* phone = new QLabel;
    phone->setText(QString("电话: %1").arg(userInfo.phone));
    layout->addWidget(phone);

    // 创建描述
    QLabel* desc = new QLabel;
    desc->setText(QString("描述: %1").arg(userInfo.description));
    desc->setWordWrap(true);    //自动换行
    layout->addWidget(desc);

    // 创建布局
    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(0,0,0,0);
    hlayout->setSpacing(0);
    layout->addLayout(hlayout);

    // 创建按钮
    QPushButton* toChat = new QPushButton;
    toChat->setText("发消息");
    hlayout->addWidget(toChat);

    QPushButton* toDelete = new QPushButton;
    toDelete->setText("删除好友");
    hlayout->addWidget(toDelete);

    connect(toChat, &QPushButton::clicked, this, [=](){
        MainWidget* mainWidget = MainWidget::getInstance();
        mainWidget->switchSession(userInfo.userId);
    });

    connect(toDelete, &QPushButton::clicked, this, [=](){
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

    layout->addStretch(1);
}
