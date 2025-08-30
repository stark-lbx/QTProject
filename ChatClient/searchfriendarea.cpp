#include "searchfriendarea.h"

#include "searchuserdialog.h"
#include "mainwidget.h"
#include "common/toast.h"
#include "common/debug.h"
#include "model/datacenter.h"

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyleOption>

//////////////////////////////////////
/// 分割符
//////////////////////////////////////
Separator::Separator(const QString &title)
    :QWidget(nullptr)
{
    // 1.设置基本属性
    this->setFixedHeight(40);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setStyleSheet("QWidget {background-color: rgb(231,231,231);}");

    // 2.设置布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(15,0,15,0);
    layout->setAlignment(Qt::AlignLeft);
    this->setLayout(layout);

    // 3.创建分隔符的Label
    QLabel* titleLabel = new QLabel();
    titleLabel->setFixedHeight(40);
    titleLabel->setText(title);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleLabel->setStyleSheet("QLabel { "
                              "font-size: 15px; "
                              "color: rgb(158, 158, 158);"
                              "border: none;"
                              "border-bottom: 1px solid rgb(249, 249, 249);"
                              "background-color: transparent; "
                              "}");

    layout->addWidget(titleLabel);
}

void Separator::paintEvent(QPaintEvent *event)
{
    (void) event;
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this) ;
}



//////////////////////////////////////
/// 一个特殊的显示结果
///     点击后跳到外部搜索结果中
//////////////////////////////////////

LinkedItem::LinkedItem(const QString &keyword)
{
    // 1. 设置基本属性
    this->setFixedHeight(80);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setStyleSheet("QWidget {background-color: rgb(231,231,231);}");

    // 2. 创建布局管理器
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(15, 0, 15, 0);
    this->setLayout(layout);

    // 3. 创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(50, 50);
    avatarBtn->setIconSize(QSize(50, 50));
    avatarBtn->setIcon(QIcon(":/resource/image/friend_add.png"));

    // 4. 创建昵称
    QLabel* nameLabel = new QLabel();
    nameLabel->setFixedHeight(40);		// 整个 Item 高度是 80. 昵称和个性签名各自占一半.
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    nameLabel->setStyleSheet("QLabel { font-size: 18px; background-color: transparent; }");
    nameLabel->setText("网络查找:");

    // 5. 创建内容标签
    contentLabel = new QLabel();
    contentLabel->setFixedHeight(40);
    contentLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    contentLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    contentLabel->setStyleSheet("QLabel { font-size: 18px; background-color: transparent; color: rgb(7, 193, 96);}");
    contentLabel->setText(keyword);

    // 6.

    // 7. 把上述内容, 添加到布局管理器中
    layout->addWidget(avatarBtn);
    layout->addWidget(nameLabel);
    layout->addWidget(contentLabel);

    // 8. 连接信号槽
    // connect(this, &QWidget::clicked, this, &ResultItem::clickThis);
    // 信号槽没有click信号，重写鼠标点击事件
}

void LinkedItem::clickThis()
{
    AddFriendDialog* addFriendDialog = AddFriendDialog::getInstance();
    // 将当前搜索框内的搜索文本传输到addFriendDialog中
    addFriendDialog->setSearchKey(this->contentLabel->text());
    addFriendDialog->clickSearch();
    if(addFriendDialog->isHidden()) addFriendDialog->show();
}

void LinkedItem::updateContent(const QString &keyword)
{
    this->contentLabel->setText(keyword);
    this->update();
}

void LinkedItem::paintEvent(QPaintEvent *event)
{
    (void) event;
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this) ;
}

void LinkedItem::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        clickThis();
        return;
    }
    QWidget::mousePressEvent(event);
}

//////////////////////////////////////
/// 表示一个好友搜索的结果
//////////////////////////////////////

ResultItem::ResultItem(const UserInfo &userInfo)
    :userInfo(userInfo)
{
    // 1. 设置基本属性
    this->setFixedHeight(80);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setStyleSheet("QWidget {background-color: rgb(231,231,231);}");

    // 2. 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(15);
    layout->setContentsMargins(15, 0, 15, 0);
    this->setLayout(layout);

    // 3. 创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(50, 50);
    avatarBtn->setIconSize(QSize(50, 50));
    avatarBtn->setIcon(userInfo.avatar);

    // 4. 创建昵称
    QLabel* nameLabel = new QLabel();
    nameLabel->setFixedHeight(40);		// 整个 Item 高度是 80. 昵称和个性签名各自占一半.
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    nameLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: 600; background-color: transparent; }");
    nameLabel->setText(userInfo.nickname);

    // 5. 创建个性签名
    QLabel* descLabel = new QLabel();
    descLabel->setFixedHeight(40);
    descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    descLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    descLabel->setStyleSheet("QLabel { background-color: transparent; }");
    descLabel->setText(userInfo.description);

    // 6.1 创建删除好友按钮
    removeBtn = new QPushButton();
    removeBtn->setFixedSize(40, 40);
    removeBtn->setText("删除");
    QString btnStyle = "QPushButton { border: none; background-color: rgb(237, 76, 76); color: rgb(255, 255, 255); border-radius: 5px;} ";
    btnStyle += "QPushButton:pressed { background-color: rgb(200, 200, 200); }";
    removeBtn->setStyleSheet(btnStyle);

    // 6.2 创建去聊天按钮
    tochatBtn = new QPushButton();
    tochatBtn->setFixedSize(40,40);
    tochatBtn->setText("聊天");
    QString chatStyle = "QPushButton { border: none; background-color: rgb(7, 183, 91); color: rgb(255, 255, 255); border-radius: 5px;} ";
    tochatBtn->setStyleSheet(chatStyle);

    // 7. 把上述内容, 添加到布局管理器中
    layout->addWidget(avatarBtn, 0, 0, 2, 1);
    layout->addWidget(nameLabel, 0, 1);
    layout->addWidget(descLabel, 1, 1);
    layout->addWidget(removeBtn, 0, 2, 2, 1);
    layout->addWidget(tochatBtn, 0, 3, 2, 1);

    // 8. 连接信号槽
    connect(removeBtn, &QPushButton::clicked, this, &ResultItem::clickRemoveButton);
    connect(tochatBtn, &QPushButton::clicked, this, &ResultItem::clickTochatButton);
}

void ResultItem::clickRemoveButton()
{
    // 删除好友逻辑
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

    // 2. 设置按钮为禁用状态
    removeBtn->setEnabled(false);
    removeBtn->setText("已删");
    removeBtn->setStyleSheet("QPushButton {background-color: rgb(200, 200, 200);}");
}

void ResultItem::clickTochatButton()
{
    emit MainWidget::getInstance()->toChatFromSearch(userInfo.userId);
}

void ResultItem::paintEvent(QPaintEvent *event)
{
    (void) event;
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this) ;
}


#if SearchFriendWidgetVersion == 1
//////////////////////////////////////
/// 整个搜索好友的窗口
//////////////////////////////////////

SearchFriendWidget::SearchFriendWidget(QWidget *parent)
    :QWidget(parent)
{
    // 1. 设置基本属性
    // this->setFixedSize(500, 500);
    // this->setWindowTitle("添加好友");
    // this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    // this->setAttribute(Qt::WA_DeleteOnClose);	// 不要忘记这个属性!!!
    this->setWindowFlags(Qt::Popup);
    this->setStyleSheet("QDialog {background-color: rgb(247,247,247); }");

    // 2. 添加布局管理器
    layout = new QGridLayout();
    layout->setContentsMargins(0,30,0,0);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(20);
    this->setLayout(layout);

    // 3. 创建搜索框
    searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(30);
    searchEdit->setPlaceholderText("按手机号/用户序号/昵称搜索");
    searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QString style = "QLineEdit { "
                    "border: none; "
                    "border-radius: 5px; "
                    "background-color: rgb(226, 226, 226); "
                    "padding-left: 5px;"
                    "}";
    style += "QLineEdit:focus { "
             "border: 2px solid rgb(69, 209, 136); "
             "outline: none; "
             "background-color: rgb(255,255,255); "
             "/*caret-color: rgb(69, 209, 136);*/"
             "}";
    searchEdit->setStyleSheet(style);

    // 初始化滚动区
    sItem = new LinkedItem();
    initResultArea();

    // 为了灵活控制边距，只影响搜索+按钮这一行，不影响下方列表行
    // 创建空白的 widget 填充到布局管理器中
    QWidget* spacer1 = new QWidget();
    spacer1->setFixedWidth(15);
    QWidget* spacer2 = new QWidget();
    spacer2->setFixedWidth(15);

    layout->addWidget(spacer1,0,0);
    layout->addWidget(searchEdit, 0, 1);
    layout->addWidget(spacer2,0,2);
    // layout->addWidget(scrollArea,1,0,1,5);

// 构造假的数据, 验证界面效果
#if TEST_UI
    QIcon avatar(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 20; ++i) {
        // new 出来这个对象, 再往 addResult 中添加. FriendResultItem 中持有了 UserInfo 的 const 引用. 需要确保引用是有效的引用
        UserInfo* userInfo = new UserInfo();
        userInfo->userId = QString::number(1000 + i);
        userInfo->nickname = "张三" + QString::number(i);
        userInfo->description = "这是一段个性签名";
        userInfo->avatar = avatar;
        this->addResult(*userInfo);
    }
#endif

    // 6. 连接信号槽
    connect(searchEdit, &QLineEdit::textChanged, this, [=](const QString& keyword){
        this->clear();
        if(keyword.trimmed().isEmpty()){
            return;
        }
        sItem->updateContent(keyword);

        // 获取好友数据
        using model::DataCenter;
        DataCenter* dataCenter = DataCenter::getInstance();
        QList<model::UserInfo>* friendList = dataCenter->getFriendList();

        // 加载UI界面
        // this->addSeparator("联系人");
        for(const auto& f: *friendList){
            if(this->containKeyWord(f, keyword)){
                this->addResult(f);
            }
        }
    });
}

void SearchFriendWidget::initResultArea()
{
    // 1. 创建滚动区域对象
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal {height: 0;} ");
    scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar:vertical {width: 2px; background-color: rgb(255, 255, 255);}");
    scrollArea->setStyleSheet("QScrollArea { border: none; }");
    layout->addWidget(scrollArea,1,0,1,5);

    // 2. 创建 QWidget
    resultContainer = new QWidget();
    resultContainer->setObjectName("resultContainer");
    resultContainer->setStyleSheet("#resultContainer { background-color: rgb(255, 255, 255); } ");
    scrollArea->setWidget(resultContainer);

    // 3. 给这个 QWidget 里面添加元素, 需要给它创建垂直的布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    resultContainer->setLayout(vlayout);

    // this->addSeparator("更多");
    resultContainer->layout()->addWidget(sItem);
}

void SearchFriendWidget::addResult(const UserInfo &userInfo)
{
    ResultItem* item = new ResultItem(userInfo);
    resultContainer->layout()->addWidget(item);
}

void SearchFriendWidget::addSeparator(const QString &showText)
{
    Separator* separator = new Separator(showText);
    resultContainer->layout()->addWidget(separator);
}

void SearchFriendWidget::clear()
{
    // 从后往前遍历
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(resultContainer->layout());
    for (int i = layout->count() - 1; i > 0; --i) {
        QLayoutItem* layoutItem = layout->takeAt(i);
        if (layoutItem == nullptr || layoutItem->widget() == nullptr) {
            continue;
        }
        // 删除这里面持有的元素
        delete layoutItem->widget();
    }
    // i==0:
    sItem->updateContent("");
}

bool SearchFriendWidget::containKeyWord(const UserInfo &userInfo, const QString &keyword)
{
    // 关键词 属于范围: userId、desc、nickname、phone
    return userInfo.userId.contains(keyword)
           || userInfo.description.contains(keyword)
           || userInfo.nickname.contains(keyword)
           || userInfo.phone.contains(keyword);
}

void SearchFriendWidget::release()
{
    this->clear();
    searchEdit->clear();
    searchEdit->setFocus(Qt::ActiveWindowFocusReason); //立马获取焦点
}
#elif SearchFriendWidgetVersion == 2

//////////////////////////////////////
/// 整个搜索好友的窗口 - version2.0
//////////////////////////////////////

SearchFriendWidget::SearchFriendWidget(QWidget *parent)
    :QWidget(parent)
{
    // 1.设置基本属性
    this->setWindowFlags(Qt::Popup);
    this->setStyleSheet("QDialog {background-color: rgb(247,247,247); }");

    // 2. 添加布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(0,30,0,0);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(20);
    this->setLayout(layout);

    // 3. 创建搜索框
    searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(30);
    searchEdit->setPlaceholderText("按手机号/用户序号/昵称搜索");
    searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QString style = "QLineEdit { "
                    "border: none; "
                    "border-radius: 5px; "
                    "background-color: rgb(226, 226, 226); "
                    "padding-left: 5px;"
                    "}";
    style += "QLineEdit:focus { "
             "border: 2px solid rgb(69, 209, 136); "
             "outline: none; "
             "background-color: rgb(255,255,255); "
             "/*caret-color: rgb(69, 209, 136);*/"
             "}";
    searchEdit->setStyleSheet(style);


    // 4.初始化listWidget
    listWidget = new QListWidget();
    listWidget->setSpacing(0);
    listWidget->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal {height: 0;}");
    listWidget->verticalScrollBar()->setStyleSheet("QScrollBar:vertical {width: 2px; background-color: rgb(255,255,255);}");

    // 5.创建一个listWidgetItem添加到widget中
    QListWidgetItem* listWidgetItem = new QListWidgetItem;
    linkedItem = new LinkedItem();
    listWidgetItem->setSizeHint(QSize(this->width(), linkedItem->height()));
    listWidget->addItem(listWidgetItem);
    listWidget->setItemWidget(listWidgetItem, linkedItem);


    // 为了灵活控制边距，只影响搜索+按钮这一行，不影响下方列表行
    // 创建空白的 widget 填充到布局管理器中
    QWidget* spacer1 = new QWidget();
    spacer1->setFixedWidth(15);
    QWidget* spacer2 = new QWidget();
    spacer2->setFixedWidth(15);

    layout->addWidget(spacer1,0,0);
    layout->addWidget(searchEdit, 0, 1);
    layout->addWidget(spacer2,0,2);
    layout->addWidget(listWidget,1,0,1,3);

    // 6. 连接信号槽
    connect(searchEdit, &QLineEdit::textChanged, this, &SearchFriendWidget::searchEditTextChanged);
}

void SearchFriendWidget::clear()
{
    listWidget->clear();

    linkedItem = new LinkedItem();
    QListWidgetItem* listWidgetItem = new QListWidgetItem;
    listWidgetItem->setSizeHint(QSize(this->width(), linkedItem->height()));
    listWidget->addItem(listWidgetItem);
    listWidget->setItemWidget(listWidgetItem, linkedItem);
}

void SearchFriendWidget::release()
{
    this->clear();
    searchEdit->clear();
    searchEdit->setFocus(Qt::ActiveWindowFocusReason);
}

void SearchFriendWidget::addResult(const UserInfo &userInfo)
{
    QListWidgetItem* listWidgetItem = new QListWidgetItem;
    ResultItem* resultItem = new ResultItem(userInfo);
    listWidgetItem->setSizeHint(QSize(this->width(), resultItem->height()));

    listWidget->addItem(listWidgetItem);
    listWidget->setItemWidget(listWidgetItem, resultItem);
}

bool SearchFriendWidget::containKeyWord(const UserInfo &userInfo, const QString &keyword)
{
    // 关键词 属于范围: userId、desc、nickname、phone
    QString cleanKeyword = keyword.trimmed().toLower();
    return userInfo.userId.contains(cleanKeyword, Qt::CaseInsensitive)
           || userInfo.nickname.contains(cleanKeyword, Qt::CaseInsensitive)
           || userInfo.phone.contains(cleanKeyword);
}

void SearchFriendWidget::searchEditTextChanged(const QString &keyword)
{
    QTimer::singleShot(300, this, [=](){
        this->clear();
        if(keyword.trimmed().isEmpty()){
            return;
        }
        linkedItem->updateContent(keyword);

        // 获取好友数据
        using model::DataCenter;
        DataCenter* dataCenter = DataCenter::getInstance();
        QList<model::UserInfo>* friendList = dataCenter->getFriendList();

        // 加载UI界面
        for(const auto& f: *friendList){
            if(this->containKeyWord(f, keyword)){
                this->addResult(f);
            }
        }
    });
}


#endif
