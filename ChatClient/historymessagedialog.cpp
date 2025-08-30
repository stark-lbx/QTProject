#include "historymessagedialog.h"

#include "common/debug.h"
#include "common/toast.h"
#include "messageshowarea.h"
#include "model/data.h"
#include "model/datacenter.h"

#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>

////////////////////////////////////////////////////////////////////
/// 表示一个历史消息元素
////////////////////////////////////////////////////////////////////

HistoryItem *HistoryItem::makeHistoryItem(const Message &message)
{
    // 1. 创建出对象
    HistoryItem* item = new HistoryItem();
    item->setMinimumHeight(80);
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    item->setStyleSheet("QWidget {background-color: rgb(237,237,237);}");

    // 2. 创建布局
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(30,5,30,5);
    item->setLayout(mainLayout);

    QGridLayout* layout = new QGridLayout();
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(10);
    layout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(layout);

    // 3. 创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(50, 50);
    avatarBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    avatarBtn->setIconSize(QSize(50, 50));
    // 当前消息发送者的头像
    avatarBtn->setIcon(message.sender.avatar);
    avatarBtn->setStyleSheet("QPushButton { border: none; }");

    // 4. 创建昵称和时间
    QLabel* nameLabel = new QLabel();
    nameLabel->setText(message.sender.nickname + " | " + message.time);
    nameLabel->setFixedHeight(20);   // 高度设置为头像高度的一半
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setStyleSheet("QLabel { color: rgb(158, 158, 158); font-size: 14px;}");

    // 5. 消息内容部分
    QWidget* contentWidget = nullptr;
    if (message.messageType == model::TEXT_TYPE) {
        // 文本消息
        QLabel* label = new QLabel();
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        label->setWordWrap(true);
        label->setText(QString(message.content));
        label->setStyleSheet("QLabel {font-size: 18px; color: rgb(22, 22, 22);}");
        label->adjustSize();	// 设置让 label 能够自动调整大小

        contentWidget = label;
    } else if (message.messageType == model::IMAGE_TYPE) {
        // 图片消息
        contentWidget = new MessageImageLabel(message.fileId, message.content, true);
    } else if (message.messageType == model::FILE_TYPE) {
        // 文件消息
        contentWidget = new MessageFileLabel(message.fileId, message.content, message.fileName, true);
    } else if (message.messageType == model::SPEECH_TYPE) {
        // 语音消息
        auto* speechLabel = new MessageSpeechLabel(message.fileId, message.content, true, true);
        speechLabel->setFixedHeight(45);

        QVBoxLayout* vlayout = new QVBoxLayout();
        vlayout->setContentsMargins(0,10,0,0);  //设置10px上边距
        vlayout->addWidget(speechLabel,0,Qt::AlignTop);

        contentWidget = new QWidget();
        contentWidget->setLayout(vlayout);
    } else {
        LOG() << "错误的消息类型! messageType=" << message.messageType;
    }

    // 6. 把上述控件添加到布局中
    layout->addWidget(avatarBtn, 0, 0, 2, 1);
    layout->addWidget(nameLabel, 0, 1, 1, 1);
    layout->addWidget(contentWidget, 1, 1, 5, 1);

    return item;
}

void HistoryItem::paintEvent(QPaintEvent *event)
{
    (void) event;
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this) ;
}


////////////////////////////////////////////////////////////////////
/// 展示历史消息窗口
////////////////////////////////////////////////////////////////////

HistoryMessageWidget::HistoryMessageWidget(const QString &chatSessionId, QWidget *parent)
    :currentChatSessionId(chatSessionId)
{
    // 1. 设置窗口本身属性
    this->setFixedSize(685, 875);
    this->setWindowTitle("历史消息");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setStyleSheet("QWidget { background-color: rgb(255, 255, 255); }");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 切换标题栏
    this->setWindowFlag(Qt::FramelessWindowHint);
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(2,5,2,5);
    this->setLayout(mainLayout);

    auto* dataCenter = model::DataCenter::getInstance();
    auto* chatSession = dataCenter->findChatSessionById(dataCenter->getCurrentChatSessionId());
    mainLayout->addWidget(titleBar = CustomTitleBar::makeHistoryMessagePageTitleBar(this, chatSession->chatSessionName));


    // 2. 创建布局管理器.
    QGridLayout* layout = new QGridLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(30, 30, 30, 10);
    mainLayout->addLayout(layout);


    // 3. 创建单选按钮
    keyRadioBtn = new QRadioButton();
    timeRadioBtn = new QRadioButton();
    keyRadioBtn->setText("按关键词查询");
    timeRadioBtn->setText("按时间查询");
    // 默认按照关键词查询
    keyRadioBtn->setChecked(true);
    layout->addWidget(keyRadioBtn, 0, 0, 1, 2);
    layout->addWidget(timeRadioBtn, 0, 2, 1, 2);

    // 4. 创建搜索框
    searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(50);
    searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    searchEdit->setPlaceholderText("搜索(仅支持文本消息关键词查询)");
    QString style = "QLineEdit { "
                    "border: 2px solid rgb(232, 232, 232); "
                    "border-radius: 10px; "
                    "background-color: rgb(255,255,255); "
                    "padding-left: 5px;"
                    "}";
    style += "QLineEdit:focus { "
             "border: 2px solid rgb(69, 209, 136); "
             "outline: none; "
             "/*caret-color: rgb(69, 209, 136);*/"
             "}";
    searchEdit->setStyleSheet(style);
    layout->addWidget(searchEdit, 1, 0, 1, 8);

    // 5. 创建搜索按钮
    QPushButton* searchBtn = new QPushButton();
    searchBtn->setFixedSize(50, 50);
    searchBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    searchBtn->setIconSize(QSize(30, 30));
    searchBtn->setIcon(QIcon(":/resource/image/search_black.png"));
    QString btnStyle = "QPushButton { border: 2px solid rgb(232,232,232); background-color: rgb(255, 255, 255); border-radius: 10px; }";
    btnStyle += "QPushButton:pressed { background-color: rgb(220, 220, 220); }";
    searchBtn->setStyleSheet(btnStyle);
    layout->addWidget(searchBtn, 1, 8, 1, 1);

    // 6. 创建时间相关的部分控件, 初始情况下要隐藏
    QLabel* begTag = new QLabel();
    begTag->setText("开始时间");
    QLabel* endTag = new QLabel();
    endTag->setText("结束时间");
    begTimeEdit = new QDateTimeEdit();
    endTimeEdit = new QDateTimeEdit();
    begTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    endTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    begTimeEdit->setFixedHeight(40);
    endTimeEdit->setFixedHeight(40);
    begTag->hide();
    endTag->hide();
    begTimeEdit->hide();
    endTimeEdit->hide();

    // 7. 创建滚动区域
    initScrollArea(mainLayout);

    // 8. 设置槽函数
    connect(keyRadioBtn, &QRadioButton::clicked, this, [=]() {
        // 把时间相关的控件, 隐藏起来
        layout->removeWidget(begTag);
        layout->removeWidget(begTimeEdit);
        layout->removeWidget(endTag);
        layout->removeWidget(endTimeEdit);
        begTag->hide();
        begTimeEdit->hide();
        endTag->hide();
        endTimeEdit->hide();

        // 把关键词搜索框显示加入布局
        layout->addWidget(searchEdit, 1, 0, 1, 8);
        searchEdit->show();
    });

    connect(timeRadioBtn, &QRadioButton::clicked, this, [=]() {
        // 关键词搜索框隐藏并从布局中删除掉.
        layout->removeWidget(searchEdit);
        searchEdit->hide();

        // 把时间相关的控件, 添加到布局中, 并且进行显示.
        layout->addWidget(begTag, 1, 0, 1, 1);
        layout->addWidget(begTimeEdit, 1, 1, 1, 3);
        layout->addWidget(endTag, 1, 4, 1, 1);
        layout->addWidget(endTimeEdit, 1, 5, 1, 3);
        begTag->show();
        begTimeEdit->show();
        endTag->show();
        endTimeEdit->show();
    });


    connect(searchBtn, &QPushButton::clicked, this, &HistoryMessageWidget::clickSearchBtn);

// 构造测试数据
#if TEST_UI
    for (int i = 0; i < 30; ++i) {
        // 注意此处代码和前面的差别.
        // 前面有个代码, UserInfo 必须要 new 出来才能构造. 当时 Item 对象里, 持有了 const UserInfo& , 不是 new 的话
        // 就可能使引用指向的对象失效的.
        // 此处后续的代码, 都是按照传值的方式来使用 message 的内容, 不 new 也行.
        model::UserInfo sender;
        sender.userId = "";
        sender.nickname = "张三" + QString::number(i);
        sender.avatar = QIcon(":/resource/image/defaultAvatar.png");
        sender.description = "";
        sender.phone = "18612345678";
        Message message = Message::makeMessage(model::TEXT_TYPE, "", sender, QString("消息内容" + QString::number(i)).toUtf8(), "");
        this->addHistoryMessage(message);
    }
#endif
}

void HistoryMessageWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        // 检查是否点击在标题栏
        QPoint localPos = event->pos();
        QRect titleBarRect = titleBar->geometry();
        if(titleBarRect.contains(localPos)) {
            mouse_press = true;
            mousePoint = event->globalPosition().toPoint() - this->pos();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void HistoryMessageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(mouse_press){
        this->move(event->globalPosition().toPoint() - mousePoint);
    }
}

void HistoryMessageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mouse_press = false;
}

void HistoryMessageWidget::addHistoryMessage(const Message &message)
{
    HistoryItem* item = HistoryItem::makeHistoryItem(message);
    container->layout()->addWidget(item);
}

void HistoryMessageWidget::clear()
{
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(container->layout());
    for (int i = layout->count() - 1; i >= 0; --i) {
        // 之前使用的是 takeAt. 效果和这个是一样的.
        QWidget* w = layout->itemAt(i)->widget();
        if (w == nullptr) {
            continue;
        }
        layout->removeWidget(w);
        w->deleteLater();
    }
}

void HistoryMessageWidget::clickSearchBtn()
{
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    connect(dataCenter, &DataCenter::searchMessageDone,
            this, &HistoryMessageWidget::clickSearchBtnDone, Qt::UniqueConnection);
    if(keyRadioBtn->isChecked()){
        // 关键词搜索
        const QString& searchKey = searchEdit->text().trimmed();
        if(searchKey.isEmpty()){
            return;
        }
        dataCenter->searchMessageAsync(searchKey);
    } else if(timeRadioBtn->isChecked()){
        // 时间范围搜索
        QDateTime begTime = begTimeEdit->dateTime();
        QDateTime endTime = endTimeEdit->dateTime();
        if(begTime >= endTime) {
            Toast::showMessage("结束时间与开始时间设置不合理", false);
            return;
        }
        dataCenter->searchMessageAsync(begTime, endTime);
    } else {
        // 严重错误, 没有选中任何一个
        LOG() << "[严重错误] 没有选中搜索模式";
        return;
    }
}

void HistoryMessageWidget::clickSearchBtnDone(const QString &chatSessionId)
{
    if(currentChatSessionId != chatSessionId) return;   //不是对应的窗口，忽略信号

    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    // 1.从dataCenter中拿到结果列表
    QList<model::Message>* messageList = dataCenter->getSearchMessageResult(chatSessionId);
    if(messageList == nullptr) return;

    // 2. 显示结果
    this->clear();  //清空搜索结果
    for(const model::Message& m: *messageList){
        this->addHistoryMessage(m);
    }
}

void HistoryMessageWidget::initScrollArea(QVBoxLayout *layout)
{
    // 1. 创建滚动区域对象
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 2px; background-color: rgb(237,237,237); }");
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0; }");
    scrollArea->setStyleSheet("QScrollArea { border: none; }");

    // 2. 创建 QWidget, 持有要加入的新的内容
    container = new QWidget();
    scrollArea->setWidget(container);

    // 3. 创建 container 中的布局管理器.
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    container->setLayout(vlayout);

    // 4. 把滚动区加入到整个 layout 中
    layout->addWidget(scrollArea);
}


////////////////////////////////////////////////////////////////////
/// 展示图片历史消息
////////////////////////////////////////////////////////////////////

// ImageButton::ImageButton(const QString &fileId, const QByteArray &content) : fileId(fileId)
// {
//     this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//     this->setStyleSheet("QPushButton { border: none; }");

//     if (!content.isEmpty()) {
//         // 直接显示到界面上
//         this->updateUI(fileId, content);
//     } else {
//         // 通过网络来获取
//         DataCenter* dataCenter = DataCenter::getInstance();
//         connect(dataCenter, &DataCenter::getSingleFileDone, this, &ImageButton::updateUI);
//         dataCenter->getSingleFileAsync(fileId);
//     }
// }

// // 真实的绘制界面
// void ImageButton::updateUI(const QString &fileId, const QByteArray &content)
// {
//     if (this->fileId != fileId) {
//         return;
//     }
//     // 如果图片尺寸太大, 需要进行缩放.
//     QImage image;
//     image.loadFromData(content);

//     int width = image.width();
//     int height = image.height();
//     if (image.width() >= 300) {
//         // 进行缩放, 缩放之后, 宽度就是固定的 300
//         width = 300;
//         height = ((double)image.height() / image.width()) * width;
//     }
//     this->resize(width, height);
//     this->setIconSize(QSize(width, height));
//     QPixmap pixmap = QPixmap::fromImage(image);
//     this->setIcon(QIcon(pixmap));
// }

////////////////////////////////////////////////////////////////////
/// 展示文件历史消息
////////////////////////////////////////////////////////////////////

// FileLabel::FileLabel(const QString &fileId, const QString &fileName)
//     : fileId(fileId), fileName(fileName)
// {
//     this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//     this->setText("[文件] " + fileName);
//     this->setWordWrap(true);
//     // 自动调整尺寸让能够显示下文字内容
//     this->adjustSize();
//     this->setAlignment(Qt::AlignTop | Qt::AlignLeft);

//     // if (!content.isEmpty()) {
//     //     // 原则上来说, 这个条件不会触发. 因为这里的 content 是来自于请求服务器的 历史消息列表的 接口.
//     //     // 请求这个接口的过程中, 拿到的 Message 对象(不是文本), content 必然为 空, 通过 fileId 做二次请求.
//     //     return;
//     // }

//     // 需要从网络加载数据了
//     DataCenter* dataCenter = DataCenter::getInstance();
//     connect(dataCenter, &DataCenter::getSingleFileDone, this, &FileLabel::getContentDone);
//     dataCenter->getSingleFileAsync(this->fileId);
// }

// void FileLabel::getContentDone(const QString &fileId, const QByteArray &fileContent)
// {
//     if (fileId != this->fileId) {
//         return;
//     }

//     this->content = fileContent;
//     this->loadDone = true;
// }

// void FileLabel::mousePressEvent(QMouseEvent *event)
// {
//     (void) event;
//     if (!this->loadDone) {
//         // 说明数据还没准备好.
//         Toast::showMessage("文件内容加载中, 请稍后尝试!");
//         return;
//     }

//     // 弹出一个对话框, 让用户来选择当前要保存的位置
//     QString filePath = QFileDialog::getSaveFileName(this, "另存为", QDir::homePath(), "*");
//     if (filePath.isEmpty()) {
//         // 用户取消了保存
//         LOG() << "用户取消了保存";
//         return;
//     }

//     writeByteArrayToFile(filePath, content);
// }

////////////////////////////////////////////////////////////////////
/// 展示语音历史消息
////////////////////////////////////////////////////////////////////

// SpeechLabel::SpeechLabel(const QString &fileId) : fileId(fileId)
// {
//     this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//     this->setText("[语音]");
//     this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//     // 这两个操作不太需要了. 此处只有 语音 两个字
//     this->setWordWrap(true);
//     this->adjustSize();

//     DataCenter* dataCenter = DataCenter::getInstance();
//     connect(dataCenter, &DataCenter::getSingleFileDone, this, &SpeechLabel::getContentDone);
//     dataCenter->getSingleFileAsync(fileId);
// }

// void SpeechLabel::getContentDone(const QString &fileId, const QByteArray &content)
// {
//     if (fileId != this->fileId) {
//         return;
//     }
//     this->content = content;
//     this->loadDone = true;
// }

// void SpeechLabel::mousePressEvent(QMouseEvent *event)
// {
//     (void) event;
//     if (!this->loadDone) {
//         Toast::showMessage("文件内容加载中, 稍后重试");
//         return;
//     }
//     SoundRecorder* soundRecorder = SoundRecorder::getInstance();
//     soundRecorder->startPlay(this->content);
// }


