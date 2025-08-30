#include "choosefrienddialog.h"

#include "common/debug.h"
#include "common/toast.h"
#include "model/datacenter.h"
#include "mainwidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>

////////////////////////////////////////////////
/// 选择好友窗口中的一个 元素/好友项
////////////////////////////////////////////////


ChooseFriendItem::ChooseFriendItem(ChooseFriendDialog* owner , const QString &userId, const QIcon &avatar, const QString &name, bool checked, bool enable)
    :owner(owner)
    ,userId(userId)
{
    // 1.设置控件的基本属性
    this->setFixedHeight(50);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    // 2.设置布局管理器（水平: 左侧、右侧）
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(20, 0, 20, 0);
    this->setLayout(layout);

    // 3.创建复选框
    QString style = R"(
        QCheckBox {
            background-color: transparent;
        }
        QCheckBox::indicator {
            width: 20px;
            height: 20px;
            image: url(:/resource/image/unchecked.png);
        }
        QCheckBox::indicator:checked {
            image: url(:/resource/image/checked.png);
        }
        QCheckBox::indicator:checked:disabled {
            image: url(:/resource/image/dischecked.png);
        }
    )";
    checkBox = new QCheckBox();
    checkBox->setFixedSize(25,25);
    checkBox->setChecked(checked);
    checkBox->setEnabled(enable);
    checkBox->setStyleSheet(style);


    // 4.创建头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(40,40);
    avatarBtn->setIconSize(QSize(40,40));
    avatarBtn->setIcon(avatar);

    // 5.创建姓名
    nameLabel = new QLabel();
    nameLabel->setText(name);
    nameLabel->setStyleSheet("QLabel {background-color: transparent;}");

    // 6.添加到布局管理器
    layout->addWidget(checkBox);
    layout->addWidget(avatarBtn);
    layout->addWidget(nameLabel);

    // 7.连接信号槽
    connect(checkBox, &QCheckBox::toggled, this, [=](bool checked){
        if(checked){
            //勾选了复选框，把当前Item添加到右侧选择区
            owner->addSelectedFriend(userId, avatar,name, true);    //通过勾选获得的Item可用
        }else{
            //取消勾选
            owner->deleteSelectedFriend(userId);
        }
    });
}

void ChooseFriendItem::paintEvent(QPaintEvent *event)
{
    (void) event;
    //根据鼠标的进入状态, 来决定绘制成不同的颜色
    QPainter painter(this);
    if(isHover){
        //绘制深色
        painter.fillRect(this->rect(), QColor(230,230,230));
    }else{
        //绘制成浅色
        painter.fillRect(this->rect(),QColor(247, 247, 247));
    }
}

void ChooseFriendItem::enterEvent(QEnterEvent *event)
{
    (void) event;
    isHover = true;

    this->update(); // this->repaint();
}

void ChooseFriendItem::leaveEvent(QEvent *event)
{
    (void) event;
    isHover = false;

    this->update(); // this->repaint();
}

void ChooseFriendItem::mousePressEvent(QMouseEvent *event)
{
    checkBox->toggle();
}









////////////////////////////////////////////////
/// 选择好友的窗口
////////////////////////////////////////////////

ChooseFriendDialog::ChooseFriendDialog(QWidget* parent)
    :RoundedDialog(parent)
{
    // 1.设置窗口的基本属性
    this->setWindowTitle("发起群聊");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setFixedSize(750,550);
    this->setStyleSheet("QDialog {background-color: rgb(255,255,255);}");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 1.2计算出现位置
    auto* mw = MainWidget::getInstance();
    QRect mainRect = mw->frameGeometry();
    QPoint mainCenter = mainRect.center();

    QRect thisRect = this->frameGeometry();
    int x = mainCenter.x() - thisRect.width()/2;
    int y = mainCenter.y() - thisRect.height()/2;
    this->move(x,y);


    // 2.左右水平布局管理器
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(5,5,5,5);
    layout->setSpacing(0);
    this->setLayout(layout);

    // 3.针对左侧窗口进行初始化
    initLeft(layout);

    // 4.针对右侧窗口进行初始化
    initRight(layout);

    // 5.加载初始数据
    this->updateSearchResult("");
}

void ChooseFriendDialog::initLeft(QHBoxLayout *layout)
{
    // 1.创建左侧布局管理器
    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(0);
    leftLayout->setContentsMargins(0,0,0,20);
    layout->addLayout(leftLayout,1);

    // 2.创建搜索框
    QLineEdit* searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(65);
    searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    searchEdit->setPlaceholderText("搜索");
    searchEdit->setStyleSheet("QLineEdit { "
                            "background-color: rgb(255,255,255); "
                            "padding-left: 10px;"
                            "border: none;"
                            "border-bottom: 2px solid rgb(68, 187, 0);"
                            "border-right: 1px solid rgb(247,247,247);"
                            "}");

    // 3. 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px;}");
    scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 2px; background-color: rgb(255,255,255);}");
    scrollArea->setStyleSheet("QScrollArea {border: none;}");

    // 4. 创建 QWidget 设置到滚动区
    totalContainer = new QWidget();
    totalContainer->setObjectName("totalContainer");
    totalContainer->setStyleSheet("#totalContainer {background-color: rgb(247, 247, 247);}");
    scrollArea->setWidget(totalContainer);

    // 5. 创建左侧子窗口内部的 垂直布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setAlignment(Qt::AlignTop);
    totalContainer->setLayout(vlayout);

    // 6.添加控件到布局
    leftLayout->addWidget(searchEdit);
    leftLayout->addWidget(scrollArea,1);

    // 7.连接信号槽
    connect(searchEdit, &QLineEdit::textChanged, this, &ChooseFriendDialog::updateSearchResult);

// extra. 还需要进一步添加vlayout内部的元素, 才能看到效果
// 此处先构造测试数据
#if TEST_UI
    QIcon defaultAvatar(":/resource/image/defaultAvatar.png");
    for(int i=0;i<25;i++){
        this->addFriend(
            QString::number(1000+i),
            defaultAvatar,
            "张三"+QString::number(i),
            false);
    }
#endif
}

void ChooseFriendDialog::initRight(QHBoxLayout *layout)
{
    // 1.创建右侧的布局管理器
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(0,0,0,20);
    layout->addLayout(gridLayout,1);

    // 2.创建提示label
    QLabel* tipLabel = new QLabel();
    tipLabel->setText("发起群聊: 选择联系人");
    tipLabel->setFixedHeight(65);
    tipLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    tipLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tipLabel->setStyleSheet("QLabel { "
                            "font-size: 16px; "
                            "font-weight: 400; "
                            "background-color: transparent; "
                            "padding-left: 30px;"
                            "padding-top: 5px;"
                            "border: none;"
                            "border-bottom: 2px solid rgb(68, 187, 0);"
                            "}");


    // 3. 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px;}");
    scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 2px; background-color: rgb(255,255,255);}");
    scrollArea->setStyleSheet("QScrollArea {border: none;}");

    // 4. 创建 QWidget 设置到滚动区
    selectedContainer = new QWidget();
    selectedContainer->setObjectName("selectedContainer");
    selectedContainer->setStyleSheet("#selectedContainer {background-color: rgb(255,255,255);}");
    scrollArea->setWidget(selectedContainer);

    // 5. 创建右侧子窗口内部的 垂直布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setAlignment(Qt::AlignTop);
    selectedContainer->setLayout(vlayout);


    QString btnStyle = R"(
        QPushButton {
            border: none;
            border-radius: 5px;
            color: rgb(7, 191, 96);
            background-color: rgb(233, 233, 233);
        }
        QPushButton:hover {
            color: rgb(6, 174, 86);
            background-color: rgb(210, 210, 210);
        }
        QPushButton:pressed {
            color: rgb(5, 164, 81);
            background-color: rgb(198, 198, 198);
        }
    )";
    // 6. 创建右侧窗口底部的按钮
    QPushButton* okBtn = new QPushButton();
    okBtn->setFixedHeight(40);
    okBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    okBtn->setText("完成");
    okBtn->setStyleSheet(btnStyle);


    QPushButton* cancelBtn = new QPushButton();
    cancelBtn->setFixedHeight(40);
    cancelBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    cancelBtn->setText("取消");
    cancelBtn->setStyleSheet(btnStyle);

    // 7.将控件添加到布局中
    gridLayout->addWidget(tipLabel,0,0,1,9);
    gridLayout->addWidget(scrollArea,1,0,1,9);
    gridLayout->addWidget(okBtn,2,1,1,3);
    gridLayout->addWidget(cancelBtn,2,5,1,3);


    // 8.连接信号槽
    connect(cancelBtn, &QPushButton::clicked, this, &ChooseFriendDialog::close);
    connect(okBtn, &QPushButton::clicked, this, &ChooseFriendDialog::clickOkButton);

// 此处数据通过勾选完成
#if TEST_UI
    QIcon defaultAvatar(":/resource/image/defaultAvatar.png");
    for(int i=0;i<25;i++){
        this->addSelectedFriend(QString::number(1000+i),defaultAvatar,"张三"+QString::number(i));
    }
#endif
}

void ChooseFriendDialog::addFriend(const QString& userId,const QIcon &avatar, const QString &name, bool checked, bool enable)
{
    ChooseFriendItem* item = new ChooseFriendItem(this, userId, avatar, name, checked , enable);
    totalContainer->layout()->addWidget(item);
}

void ChooseFriendDialog::addSelectedFriend(const QString& userId,const QIcon &avatar, const QString &name, bool enable)
{
    // LOG() << "\n\t userId=" <<userId
    //       << "\n\t avatar="<<avatar
    //       << "\n\t nickname="<<name;
    ChooseFriendItem* item = new ChooseFriendItem(this, userId, avatar, name, true, enable);
    selectedContainer->layout()->addWidget(item);
}

void ChooseFriendDialog::deleteSelectedFriend(const QString &userId)
{
    // 遍历一下左侧列表, 把左侧列表中对应 item 的 checkBox 勾选状态取消掉.
    QVBoxLayout* vlayoutLeft = dynamic_cast<QVBoxLayout*>(totalContainer->layout());
    for (int i = 0; i < vlayoutLeft->count(); ++i) {
        auto* item = vlayoutLeft->itemAt(i);
        if (item == nullptr || item->widget() == nullptr) {
            continue;
        }
        ChooseFriendItem* chooseFriendItem = dynamic_cast<ChooseFriendItem*>(item->widget());
        if (chooseFriendItem->getUserId() != userId) {
            continue;
        }
        // 取消 checkBox 选中状态
        chooseFriendItem->getCheckBox()->setChecked(false);
    }

    //遍历 selectedContainer
    QVBoxLayout* vlayout = dynamic_cast<QVBoxLayout*>(selectedContainer->layout());
    for(int i = vlayout->count()-1; i>=0 ;--i){
        auto* item = vlayout->itemAt(i);
        if(item==nullptr || item->widget() ==nullptr) continue;

        ChooseFriendItem* chooseFriendItem = dynamic_cast<ChooseFriendItem*>(item->widget());
        if(chooseFriendItem->getUserId() != userId)continue;

        vlayout->removeWidget(chooseFriendItem);
        // if(chooseFriendItem) delete chooseFriendItem;
        chooseFriendItem->deleteLater();
    }
}

void ChooseFriendDialog::clearSearchResult()
{
    QLayout* vlayout = totalContainer->layout();
    // 遍历布局管理器中所有元素, 并依次从布局中删除
    for(int i= vlayout->count()-1; i>=0; --i){
        QLayoutItem* item = vlayout->takeAt(i);
        if(item->widget()){
            // item->widget()->deleteLater();
            delete item->widget();
        }
    }
}

void ChooseFriendDialog::updateSearchResult(const QString &currentText)
{
    // 1.清空上次的显示结果
    this->clearSearchResult();

    // 2.预加载所有好友
    using model::DataCenter;
    DataCenter* dataCenter =DataCenter::getInstance();
    QList<model::UserInfo>* fList = dataCenter->getFriendList();

    // 3.确定当前选择会话信息
    model::ChatSessionInfo* currentChatSessionInfo
        = dataCenter->findChatSessionById(dataCenter->getCurrentChatSessionId());
    if(currentChatSessionInfo == nullptr){
        LOG() << "[严重错误]当前点击的会话信息在数据中心未找到";
        return;
    }

    // 4.如果当前会话是群聊 获取当前会话的用户列表
    // TODO 完成功能"当前会话的用户列表"后实现
    // QList<model::UserInfo>* memberList; // = getCurrentChatSessionMemberList();

    // 5.显示搜索结果
    for(const auto& f: *fList){
        if(containNicknameKeyword(f, currentText)){
            bool isCheck = false;
            bool enable = true;
            if(currentChatSessionInfo->userId == f.userId){
                // 单聊情况下, 默认选中这个用户
                // 此时功能为：创建一个新的会话，选中的用户和自己都将加入
                isCheck = true; //选中状态: 已选中
                enable = false; //可用状态: 不可选中
                this->addSelectedFriend(f.userId, f.avatar, f.nickname, enable);
            }
            // if(currentChatSessionInfo->userId == ""
            //     && memberList->contains(f)){  // getCurrentChatSessionMemberList();
            //     // 群聊情况下 && "当前会话的用户列表"包含这个用户: 代表已经选中、且无法勾选
            //     // 此时功能为：邀请指定用户加入群聊、不创建群聊
            //     isCheck = true;
            //     enable = false;
            // }
            this->addFriend(f.userId, f.avatar, f.nickname, isCheck, enable);
        }
    }
}

bool ChooseFriendDialog::containNicknameKeyword(const model::UserInfo &userInfo, const QString &keyword)
{
    return keyword.trimmed().isEmpty() || userInfo.nickname.contains(keyword.trimmed());
}

void ChooseFriendDialog::clickOkButton()
{
    // 1.获取右侧选中成员的userId列表
    QList<QString> userIdList = this->generateMemberList();
    if(userIdList.size() < 3) {
        Toast::showMessage("当前选中的群聊成员不足3人, 无法创建", false);
        return;
    }

    // 2.发送网络请求, 创建群聊
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    dataCenter->createGroupChatSessionAsync(userIdList);
    this->close();
}

QList<QString> ChooseFriendDialog::generateMemberList()
{
    using model::DataCenter;
    QList<QString> result;
    // 1.将自己添加到结果中
    DataCenter* dataCenter = DataCenter::getInstance();
    if(dataCenter->getMyself() == nullptr){
        LOG()<<"[严重错误]个人信息尚未加载到客户端";
        return result;
    }
    result.push_back(dataCenter->getMyself()->userId);

    // 2.遍历选中列表
    QVBoxLayout* vlayout = dynamic_cast<QVBoxLayout*>(selectedContainer->layout());
    for(int i=0; i<vlayout->count(); ++i){
        QLayoutItem* item = vlayout->itemAt(i);
        if(item==nullptr || item->widget()==nullptr){
            continue;
        }

        ChooseFriendItem* chooseFriendItem = dynamic_cast<ChooseFriendItem*>(item->widget());
        result.push_back(chooseFriendItem->getUserId());
    }
    return result;
}













