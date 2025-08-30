#include "messageshowarea.h"

#include "mainwidget.h"
#include "soundrecorder.h"
#include "userinfodialog.h"
#include "common/debug.h"
#include "model/datacenter.h"

#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMenu>

//////////////////////////////////////////////
/// 表示消息展示区
//////////////////////////////////////////////

MessageShowArea::MessageShowArea() {
    // 1.初始化基本属性
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->setWidgetResizable(true);
    //设置滚动条的样式
    this->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 2px; background-color: rgb(240,240,240);}");
    this->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px;}");
    this->setStyleSheet("QScrollArea { border: none; }");

    // 2.创建一个 container 容器，作为包含内部元素的容器
    container = new QWidget();
    this->setWidget(container);

    // 3.给 container 添加布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    container->setLayout(layout);

    // 4.创建占位控件（用于占据底部剩余空间）
    spacerWidget = new QWidget;
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 允许拉伸
    layout->addWidget(spacerWidget, 1); // 拉伸因子1，占据剩余空间

    // 添加测试构造数据
#if TEST_UI
    model::UserInfo userInfo;
    userInfo.userId = QString::number(0);
    userInfo.nickname = "自己";
    userInfo.phone = "666888";
    userInfo.description = "不敲代码就会噶";
    userInfo.avatar = QIcon(":/resource/image/defaultAvatar.png");

    // 如果输入 '\n' 则会出现Bug
    Message message = Message::makeMessage(model::TEXT_TYPE, "", userInfo,
                                           QString("这是一条测试信息\n这是一条测试信息\n这是一条测试信息\n这是一条测试信息\n这是一条测试信息\n").toUtf8(), "");
    this->addMessage(false, message);

    for(int i=1;i<=30;++i){
        model::UserInfo userInfo;
        userInfo.userId = QString::number(i);
        userInfo.nickname = "张三" + QString::number(i);
        userInfo.phone = "123456789";
        userInfo.description = "从今天开始认真敲代码"+QString::number(i);
        userInfo.avatar = QIcon(":/resource/image/defaultAvatar.png");

        Message message = Message::makeMessage(model::TEXT_TYPE, "", userInfo, QString("这是一条测试信息 %1").arg(i).toUtf8(), "");
        this->addMessage(true,message);
    }
#endif
}

void MessageShowArea::addMessage(bool isLeft, const Message &message)
{
    // 构造 MessageItem 添加到布局管理器中
    MessageItem* messageItem = MessageItem::makeMessageItem(isLeft, message);
    QVBoxLayout* vlayout = dynamic_cast<QVBoxLayout*>(container->layout());
    vlayout->insertWidget (vlayout->count () - 1, messageItem, 0);
}

void MessageShowArea::addFrontMessage(bool isLeft, const Message &message)
{
    // 构造 MessageItem 添加到布局管理器中
    MessageItem* messageItem = MessageItem::makeMessageItem(isLeft,message);

    QVBoxLayout* vlayout = dynamic_cast<QVBoxLayout*>(container->layout());
    vlayout->insertWidget(0, messageItem);
}

void MessageShowArea::clear()
{
    // 遍历布局管理器，删除里面的元素
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(container->layout());
    // 保证占位控件不被删
    for(int i=layout->count()-2; i>=0; --i){
        QLayoutItem* item = layout->takeAt(i);
        if(item != nullptr && item->widget() !=nullptr){
            delete item->widget();
        }
    }
}

void MessageShowArea::scrollToEnd() {
    // 为了使得滚动效果更佳, 能够在界面绘制好后进行滚动条的滚动
    // 该操作加一个"短暂的延时", 避免遇到中间状态的end
    // 1. 拿到滚动区的垂直滚动条
    // 2. 获取到滚动条的最大值
    // 3. 设置滚动条到最大值
    auto* vsb = this->verticalScrollBar();
    if(!vsb) return;

    // 立即尝试滚动, 应对界面已经绘制过的情况, 能够立马奏效
    vsb->setValue(vsb->maximum());


    // 延迟重试, 应对界面未完全绘制的情况
    QTimer::singleShot(500, this, [=](){
        if(vsb->value() < vsb->maximum()) {
            vsb->setValue(vsb->maximum());
        }
    });
}




//////////////////////////////////////////////
/// 表示一条消息
/// 目前里面要同时支持 文本消息、图片消息、文件消息、语音消息
/// 当前只考虑文本消息，后续添加其它
//////////////////////////////////////////////

MessageItem::MessageItem(bool isLeft)
    :isLeft(isLeft)
{}

MessageItem *MessageItem::makeMessageItem(bool isLeft, const Message &message)
{
    // 1.创建对象和布局管理器
    MessageItem* messageItem = new MessageItem(isLeft);
    QGridLayout* layout = new QGridLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(40,10,40,0);
    messageItem->setMinimumHeight(100);
    messageItem->setLayout(layout);

    // 2.创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(40,40);
    avatarBtn->setIconSize(QSize(40,40));
    avatarBtn->setIcon(message.sender.avatar);
    avatarBtn->setStyleSheet("QPushButton { border: none; }");

    // 3.名称|时间显示区
    QLabel* nameLabel = new QLabel();
    nameLabel->setText(message.sender.nickname + " | " + message.time);
    nameLabel->setAlignment(Qt::AlignBottom);
    nameLabel->setStyleSheet("QLabel { font-size: 14px; color: rgb(158, 158, 158);}");

    // 4.创建消息体
    QWidget* contentWidget = nullptr;
    switch(message.messageType){
    case model::TEXT_TYPE:
        contentWidget = makeTextMessageItem(isLeft, message.content);
        break;
    case model::IMAGE_TYPE:
        contentWidget = makeImageMessageItem(isLeft, message.fileId, message.content);
        break;
    case model::FILE_TYPE:
        contentWidget = makeFileMessageItem(isLeft, message);
        break;
    case model::SPEECH_TYPE:
        contentWidget = makeSpeechMessageItem(isLeft, message);
        break;
    case model::INVALId_TYPE:
    default:
        LOG()<<"错误的消息类型! messageType="<<message.messageType;
        contentWidget = new QWidget();
        break;
    }

    // 消息布局|对齐方式
    if(isLeft){
        //左侧消息
        layout->addWidget(avatarBtn,0,0,2,1,Qt::AlignTop | Qt::AlignLeft);
        layout->addWidget(nameLabel,0,1,Qt::AlignLeft);
        layout->addWidget(contentWidget,1,1);
    }else{
        //右侧消息
        layout->addWidget(avatarBtn,0,1,2,1,Qt::AlignTop | Qt::AlignLeft);
        layout->addWidget(nameLabel,0,0,Qt::AlignRight);
        layout->addWidget(contentWidget,1,0);
    }

    // 5. 连接信号槽
    connect(avatarBtn, &QPushButton::clicked, messageItem, [=](){
        MainWidget* mainWidget = MainWidget::getInstance();
        UserInfoWidget* userInfoWidget = new UserInfoWidget(message.sender, mainWidget);
        userInfoWidget->show();
    });

    // 6. 当用户修改昵称后, 同步修改此处的用户昵称 / 用户头像
    if(!isLeft) { //只是针对右侧消息(自己的消息) 做下列操作
        model::DataCenter* dataCenter = model::DataCenter::getInstance();

        connect(dataCenter, &model::DataCenter::changeNicknameDone, messageItem, [=](){
            QString newNickname = dataCenter->getMyself()->nickname;
            // message.sender.nickname = newNickname;
            nameLabel->setText( newNickname + " | " + message.time);
        });

        connect(dataCenter, &model::DataCenter::changeAvatarDone, messageItem, [=](){
            avatarBtn->setIcon(dataCenter->getMyself()->avatar);
        });
    }

    return messageItem;
}

QWidget *MessageItem::makeTextMessageItem(bool isLeft, const QString& text)
{
    MessageContentLabel* messageContentLabel = new MessageContentLabel(text, isLeft);
    return messageContentLabel;
}

QWidget *MessageItem::makeImageMessageItem(bool isLeft, const QString& fileId, const QByteArray& content)
{
    MessageImageLabel* imageContentLabel = new MessageImageLabel(fileId, content, isLeft);
    return imageContentLabel;
}

QWidget *MessageItem::makeFileMessageItem(bool isLeft, const model::Message &message)
{
    MessageFileLabel* fileContentLabel = new MessageFileLabel(message.fileId, message.content,message.fileName,isLeft);
    return fileContentLabel;
}

QWidget *MessageItem::makeSpeechMessageItem(bool isLeft, const model::Message &message)
{
    MessageSpeechLabel* messageLabel = new MessageSpeechLabel(message.fileId,message.content,isLeft);
    return messageLabel;
}



//////////////////////////////////////////////
/// 创建类表示 "文本消息" 正文部分
//////////////////////////////////////////////

MessageContentLabel::MessageContentLabel(const QString &text, bool isLeft)
    :isLeft(isLeft)
{
    //设置一下策略
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->label = new QLabel(this);
    this->label->setText(text);
    this->label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    this->label->setWordWrap(true); // 设置文本自动换行
    this->label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->label->setStyleSheet("QLabel { padding: 0 10px; font-size: 16px; line-height: 1.2; background-color: transparent; }");
    // line-height: 行高就是 文字高度+行间距；此处文字高度设置过了16px，那么真实的行高就是16*1.2=19.2；行间距就是19.2-16=3.2
}

// 这个函数会在该控件被显示时自动调用
//（当最大化、最小化等时 可以自动更改绘制效果）
void MessageContentLabel::paintEvent(QPaintEvent *event)
{
    // 确保消息显示区的尺寸 与 文字内容是匹配的
    // 1）如果文字内容比较少，一行就能表示进去了
    //    整个区域的高度就是一行文字的高度 + 上下边距（比如上下各有10px）
    //    整个区域的宽度就是一行文字的宽度 + 左右边距（比如左右各有20px）
    // 2）如果文字进一步增加了，文本的宽度就要增加
    //    此处约定，文字宽度达到父窗口宽度的60%，就要换行
    // 3）当需要换行的时候
    //    此时的宽度就是父窗口60%的尺寸
    //    此时的高度就需要根据文本的文字的宽度 除以 一行能摆放的宽度 => 得到了行数
    //    根据行数换算高度 => 行数*一行文字的行高 + 上下边距
    // 4）确定范围后，绘制圆角矩形
    //    之后再绘制箭头

    // 1.获取到父元素的宽度
    QObject* obj = this->parent();
    if(!obj->isWidgetType()){
        //当前这个对象的父元素不是预期的QWidget
        //此时不需要后续的任何绘制操作
        LOG() << "超出预期";
        return;
    }
    QWidget* parent = dynamic_cast<QWidget*>(obj);
    int width = parent->width() * 0.6;

    // 2.计算当前文本，如果是一行放置，需要多少宽度
    // QFontMetrics(metrics=度量)帮助我们计算一段文字，对应的占据的尺寸空间
    QFontMetrics metrics(this->label->font());
    int oneRowWidth = metrics.horizontalAdvance(this->label->text());

    // 3.计算总行数(包含了左右各自的20px的边距空隙的)
    int rows = (oneRowWidth / (width-40)) + 1;
    if(rows == 1){
        // 用来控制宽度更少的情况的
        width =oneRowWidth + 40;
    }

    // 4.根据行数计算得到高度(+20表示上下各有10px的边距)
    int height = rows * (this->label->font().pixelSize() * 1.2 ) + 10;

    // 5.绘制圆角矩形和箭头
    QPainter painter(this);
    QPainterPath path;//绘制不规则形状需要
    // 设置抗锯齿: 让绘制的图片看起来更加的细腻、增加系统(CPU/GPU)负担
    painter.setRenderHint(QPainter::Antialiasing);
    if(isLeft){
        painter.setPen(QPen(QColor(255,255,255)));
        painter.setBrush(QColor(255,255,255));
        //悬浮时颜色: 235.235.235

        //绘制时(相对父元素坐标: x,y) - (尺寸: width,height) - (圆角度: xR,yR)
        painter.drawRoundedRect(10, 0, width, height, 10, 10);

        //绘制三角形((10,15), (0,20), (10,25))
        path.moveTo(10,15);
        path.lineTo(0,20);
        path.lineTo(10,25);
        path.closeSubpath();    //绘制线条形成闭合的多边形，才能Brush填充颜色
        painter.drawPath(path);

        this->label->setGeometry(10, 0, width, height);
    }else{
        painter.setPen(QPen(QColor(149, 236, 105)));
        painter.setBrush(QColor(149, 236, 105));
        //悬浮时颜色: 137.217.97

        //绘制时(相对父元素坐标: x,y) - (尺寸: width,height) - (圆角度: xR,yR)
        //left_top_x = MessageContentLabel.width - RoundedRect.width - 10(容纳箭头的预留宽度)
        int lxPos = this->width() - width - 10;
        int rxPos = this->width() - 10;
        painter.drawRoundedRect(lxPos, 0, width, height, 10, 10);

        //绘制三角形((rxPos,15), (rxPos+10,20), (rxPos,25))
        path.moveTo(rxPos,15);
        path.lineTo(this->width(),20);
        path.lineTo(rxPos,25);
        path.closeSubpath();    //绘制线条形成闭合的多边形，才能Brush填充颜色
        painter.drawPath(path);

        this->label->setGeometry(lxPos, 0, width, height);
    }

    // 6.重新设置父元素的高度，确保父元素足够高
    // 确保高度要涵盖之前 nameLabel 的高度 和 留白
    parent->setFixedHeight(height + 50);
}


//////////////////////////////////////////////
/// 创建类表示 "图片消息" 正文部分
/// 1.fileId为"", content为图片内容
///     这说明本地在给服务器发送图片消息
///     此处这个对象是右侧消息
/// 2.fileId不为""，content为空
///     这说明收到了服务器推送的消息/从服务器加载消息列表
///     从服务器拿到的图片消息，响应结果是只包含fileId的
///     想要拿到图片的真实数据，需要再给服务器发送请求
///         : "根据fileId"拿到文件内容
//////////////////////////////////////////////

MessageImageLabel::MessageImageLabel(const QString &fileId, const QByteArray &content, bool isLeft)
    :fileId(fileId), content(content), isLeft(isLeft)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    imageBtn = new QPushButton(this);
    imageBtn->setStyleSheet(" QPushButton { border:none; }");

    using model::DataCenter;
    if (content.isEmpty()) {
        // 此处这个控件, 是针对 "从服务器拿到图片消息" 这种情况.
        // 拿着 fileId, 去服务器获取图片内容
        DataCenter* dataCenter = DataCenter::getInstance();
        connect(dataCenter, &DataCenter::getSingleFileDone, this, &MessageImageLabel::updateUI);
        dataCenter->getSingleFileAsync(fileId);
    }
}

void MessageImageLabel::updateUI(const QString &fileId, const QByteArray &content)
{
    // LOG() <<"check fileId is equal" << (this->fileId == fileId);
    // fileId: 服务器返回的fileId
    if(this->fileId != fileId) return;

    this->content = content;
    this->update(); //获取到content后将图片刷新到界面上
}

void MessageImageLabel::paintEvent(QPaintEvent *event)
{
    (void) event;
    if(!imageBtn->icon().isNull()) return; //如果已经设置了图片，再绘制，就比较慢了

    // 1. 先拿到该元素的父元素, 看父元素的宽度是多少.
    //    此处显示的图片宽度的上限 父元素宽度的 60% .
    QObject* object = this->parent();
    if (!object->isWidgetType()) {
        // 这个逻辑理论上来说是不会存在的.
        return;
    }
    QWidget* parent = dynamic_cast<QWidget*>(object);
    int width = parent->width() * 0.6;

    // 2. 加载二进制数据为图片对象
    QImage image;
    if (content.isEmpty()) {
        // 此时图片的响应数据还没回来.
        // 此处先拿一个 "固定默认图片" 顶替一下.
        QByteArray tmpContent = model::loadFileToByteArray(":/resource/image/image.png");
        image.loadFromData(tmpContent);
    } else {
        // 此处的 load 操作 QImage 能够自动识别当前图片是啥类型的 (png, jpg....)
        image.loadFromData(content);
    }

    // 3. 针对图片进行缩放.
    int height = 0;
    if (image.width() > width) {
        // 发现图片更宽, 就需要把图片缩放一下, 使用 width 作为实际的宽度
        // 等比例缩放.
        height = ((double)image.height() / image.width()) * width;
    } else {
        // 图片本身不太宽, 不需要缩放.
        width = image.width();
        height = image.height();
    }

    // pixmap 只是一个中间变量. QImage 不能直接转成 QIcon, 需要 QPixmap 中转一下
    QPixmap pixmap = QPixmap::fromImage(image);
    // imageBtn->setFixedSize(width, height);
    imageBtn->setIconSize(QSize(width, height));
    imageBtn->setIcon(QIcon(pixmap));

    // 4. 由于图片高度是计算算出来的. 该元素的父对象的高度, 能够容纳下当前的元素.
    //    此处 + 50 是为了能够容纳下 上方的 "名字" 部分. 同时留下一点 冗余 空间.
    parent->setFixedHeight(height + 50);

    // 5. 确定按钮所在的位置.
    //    左侧消息, 和右侧消息, 要显示的位置是不同的.
    if (isLeft) {
        imageBtn->setGeometry(10, 0, width, height);
    } else {
        int leftPos = this->width() - width - 10;
        imageBtn->setGeometry(leftPos, 0, width, height);
    }
}


//////////////////////////////////////////////
/// 创建类表示 "文件消息" 正文部分
//////////////////////////////////////////////

MessageFileLabel::MessageFileLabel(const QString &fId, const QByteArray &fContent, const QString &fName, bool isLeft)
    :isLeft(isLeft), fileId(fId), content(fContent)
{
    // this->setFixedWidth(300);   // width:300, height:100+(取决于文件名字占几行)
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(10,10,10,10); //四周各有10px边距
    this->setLayout(hlayout);

    QWidget* showWidget = new QWidget();
    showWidget->setFixedWidth(300);
    showWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    showWidget->setStyleSheet("QWidget {background-color: rgb(255,255,255);}");

    if(isLeft){
        hlayout->addWidget(showWidget, 0, Qt::AlignLeft);
        hlayout->addStretch();
    } else {
        hlayout->addStretch();
        hlayout->addWidget(showWidget, 0, Qt::AlignRight);
    }

    // 1.设置文件名标签
    this->fileName = new QLabel();
    this->fileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->fileName->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->fileName->setWordWrap(true);
    this->fileName->setText(fName);
    this->fileName->setStyleSheet("QLabel { font-size: 18px; background-color: transparent; color: rgb(22, 22, 22);}");

    // 2.设置文件大小标签
    this->fileSize = new QLabel();
    this->fileSize->setFixedHeight(30);
    this->fileSize->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->fileSize->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->fileSize->setWordWrap(true);
    QString strSize = "???";
    auto fsize = content.size();
    if(fsize<1024) {
        strSize = QString::number(fsize) + "B";
    } else if(fsize<1024*1024){
        strSize = QString::number(fsize/1024.0, 'f', 1) + "K";
    } else if(fsize<1024*1024*1024){
        strSize = QString::number(fsize/1024.0/1024.0, 'f', 1) + "M";
    }
    this->fileSize->setText(strSize);
    this->fileSize->setStyleSheet("QLabel { font-size: 16px; background-color: transparent; color: rgb(168, 168, 168);}");

    // 3.设置文件图标
    this->fileIcon = new QPushButton();
    this->fileIcon->setFixedSize(45, 60);       //or 35*45
    this->fileIcon->setIconSize(QSize(45,60));
    this->fileIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->fileIcon->setIcon(QIcon(":/resource/image/fileIcon.png"));
    this->fileIcon->setStyleSheet("QPushButton {background-color: transparent; border:none;}");

    // 4.设置布局
    QGridLayout* layout = new QGridLayout;
    layout->setContentsMargins(10,10,10,10);  //元素内部四周各有10px间距
    layout->setVerticalSpacing(10);           //元素之间的垂直间隔
    showWidget->setLayout(layout);
    layout->addWidget(fileName, 0,0);
    layout->addWidget(fileSize, 1,0);
    layout->addWidget(fileIcon, 0,1,2,1);

    if(content.isEmpty()){
        model::DataCenter* dataCenter = model::DataCenter::getInstance();
        connect(dataCenter, &model::DataCenter::getSingleFileDone, this, &MessageFileLabel::updateUI);
        dataCenter->getSingleFileAsync(this->fileId);
    } else {
        loadContentDone = true;
    }
}

void MessageFileLabel::updateUI(const QString &fileId, const QByteArray &fileContent)
{
    if(fileId != this->fileId) return;

    this->content = fileContent;
    this->loadContentDone = true;

    QString strSize = "???";
    auto fsize = content.size();
    if(fsize<1024) {
        strSize = QString::number(fsize) + "B";
    } else if(fsize<1024*1024){
        strSize = QString::number(fsize/1024.0, 'f', 1) + "K";
    } else if(fsize<1024*1024*1024){
        strSize = QString::number(fsize/1024.0/1024.0, 'f', 1) + "M";
    }
    this->fileSize->setText(strSize);
    this->update();
}

void MessageFileLabel::saveAsFile()
{
    // 弹出对话框，让用户选择路径
    const QString& file_name = this->fileName->text();
    const QString& file_expand_name = file_name.sliced(file_name.lastIndexOf('.')+1);
    const QString& filter = QString("*.%1").arg(file_expand_name);
    QDir homeDir(QDir::homePath()); // 基于用户主目录
    QString initialFilePath = homeDir.filePath(file_name); // 拼接主目录 + file_name（自动处理路径分隔符）
    QString path = QFileDialog::getSaveFileName(
        this,
        "另存为",
        initialFilePath,  // 替换原来的 QDir::homePath()，传入含默认文件名的路径
        filter
        );
    this->filePath = path;
    model::writeByteArrayToFile(path, this->content);
}

void MessageFileLabel::openFile()
{
    if(filePath.isEmpty()){
        LOG() <<"[错误]文件尚未保存";
        return;
    }
    // 转换为 URL 打开文件
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    QDesktopServices::openUrl(fileUrl);
}

void MessageFileLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        //左键按下 触发'打开'功能
        if(!loadContentDone){
            // 数据尚未加载
            LOG() << "数据尚未加载";
            return;
        }
        // 如果文件尚未保存, 则保存文件
        if(filePath.isEmpty()) this->saveAsFile();
        // 然后无论文件是否保存，都打开文件
        this->openFile();
    } else if(event->button() == Qt::RightButton){
        //右键按下 触发其它弹窗
        // 暂不实现
        QWidget::mousePressEvent(event);
    } else {
        // 其它键按下 交由父元素处理
        QWidget::mousePressEvent(event);
    }
}


//////////////////////////////////////////////
/// 创建类表示 "语音消息" 正文部分
///     语音消息处理时
///     1) 录制语音
///     2) 播放语音
///     3) 网络传输语音消息
///     4) 语音转文字（第三方sdk: 百度语音）
//////////////////////////////////////////////


MessageSpeechLabel::MessageSpeechLabel(const QString &fileId, const QByteArray &content, bool isLeft, bool onlySpeechLabel)
    :fileId(fileId)
    ,content(content)
    ,isLeft(isLeft)
    ,onlySpeechLabel(onlySpeechLabel)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 设置布局
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(10,10,10,10);
    mainLayout->setSpacing(4);
    this->setLayout(mainLayout);

    // 语音部分
    showLabel = new QLabel();
    m_duration = std::max(1, SoundRecorder::getPcmContentDuration(this->content));         //语音时长
    QString result = "";
    for(int i=1; i<= m_duration; ++i) {
        result.append(i%2? "ı":"l");
    }
    if(isLeft) {
        showLabel->setAlignment(Qt::AlignLeft |Qt::AlignVCenter);
        showLabel->setText(QString("▶︎ %1 %2")
                               .arg(result)
                               .arg(QString::number(m_duration) + "\""));
    } else {
        showLabel->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
        showLabel->setText(QString("%2 %1 ◀︎")
                               .arg(result)
                               .arg(QString::number(m_duration) + "\""));
    }
    showLabel->setStyleSheet("QLabel {padding: 0 10px; line-height: 1.2; background-color: transparent; font-size: 16px;}");
    mainLayout->addWidget(showLabel);

    // 文本部分
    textLabel = new QLabel();
    textLabel->setMaximumWidth(400);
    textLabel->setWordWrap(true);   //自动换行
    textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    textLabel->setText("");
    textLabel->setStyleSheet("QLabel {padding: 0 10px; line-height: 1.2; background-color: rgb(255,255,255); font-size: 16px; border-radius: 10px;}");
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setVisible(false);   //初始隐藏
    mainLayout->addWidget(textLabel, 0, Qt::AlignTop | (isLeft? (Qt::AlignLeft):(Qt::AlignRight)));

    // 设置鼠标追踪
    setMouseTracking(true);

    if (this->content.isEmpty()) {
        model::DataCenter* dataCenter = model::DataCenter::getInstance();
        connect(dataCenter, &model::DataCenter::getSingleFileDone, this, &MessageSpeechLabel::updateUI);
        dataCenter->getSingleFileAsync(this->fileId);
        showLabel->setText("[正在加载...]");
    } else {
        // content 不为空, 说明当前的这个数据就是已经现成. 直接就把 表示加载状态的变量设为 true
        this->loadContentDone = true;
    }
}

void MessageSpeechLabel::adjustLabelSize()
{
    // 计算当前文本，如果是一行放置，需要多少宽度
    int width = this->width();
    QFontMetrics metrics(this->textLabel->font());
    int oneRowWidth = metrics.horizontalAdvance(this->textLabel->text());

    // 3.计算总行数(包含了左右各自的20px的边距空隙的)
    int rows = (oneRowWidth / (width-40)) + 1;
    if(rows == 1){
        // 用来控制宽度更少的情况的
        width = oneRowWidth + 40;
    }

    // 4.根据行数计算得到高度(+20表示上下各有10px的边距)
    int height = rows *(this->textLabel->font().pixelSize() * 1.2 ) + 20;

    this->textLabel->setFixedSize(width, height);
    this->update();
}

void MessageSpeechLabel::updateUI(const QString &fileId, const QByteArray &content)
{
    if(this->fileId != fileId) return;

    this->content = content;
    this->loadContentDone = true;
    // 获取语音时长
    m_duration = SoundRecorder::getPcmContentDuration(content);         //语音时长

    // 设置显示结果
    QString result = "";
    for(int i=1; i<= m_duration; ++i) {
        result.append(i%2? "ı":"l");
    }
    if(isLeft) {
        showLabel->setText(QString("▶︎ %1 %2")
                               .arg(result)
                               .arg(QString::number(m_duration) + "\""));
    } else {
        showLabel->setText(QString("%2 %1 ◀︎")
                               .arg(result)
                               .arg(QString::number(m_duration) + "\""));
    }
    this->update();
}

void MessageSpeechLabel::play()
{
    if(!loadContentDone){
        LOG() << "数据尚未加载成功";
        return;
    }
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    connect(soundRecorder, &SoundRecorder::soundPlayDone, this, &MessageSpeechLabel::playDone, Qt::UniqueConnection);
    soundRecorder->startPlay(this->content);
    this->showLabel->setText(QString("[正在播放中(%1\")...]").arg(m_duration));
    isPlaying = true;
}

void MessageSpeechLabel::stop()
{
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    soundRecorder->stopPlay();
    isPlaying = false;
}

void MessageSpeechLabel::playDone()
{
    isPlaying = false;
    QString result = "";
    for(int i=1; i<= m_duration; ++i) {
        result.append(i%2? "ı":"l");
    }
    if(isLeft) {
        showLabel->setText(QString("▶︎ %1 %2")
                               .arg(result)
                               .arg(QString::number(m_duration) + "\""));
    } else {
        showLabel->setText(QString("%2 %1 ◀︎")
                               .arg(result)
                               .arg(QString::number(m_duration) + "\""));
    }
}


void MessageSpeechLabel::speechConvertText()
{
    // 文本未显示, 点击之后: 文本显示
    if(!isConvertText) {
        // 文本从未加载, 从网络开始获取文本
        model::DataCenter* dataCenter = model::DataCenter::getInstance();
        connect(dataCenter, &model::DataCenter::speechConvertTextDone,
                this, &MessageSpeechLabel::speechConvertTextDone, Qt::UniqueConnection);
        dataCenter->speechConvertTextAsync(this->fileId, this->content);
        textLabel->setText("正在转换..."); adjustLabelSize();
    }
    textLabel->show();
    this->update();
}

void MessageSpeechLabel::speechConvertTextDone(const QString &fileId, const QString &text)
{
    if(this->fileId != fileId) return;

    this->isConvertText = true;
    this->textLabel->setVisible(true);
    this->textLabel->setText(text);
    adjustLabelSize();
    this->update();
}

void MessageSpeechLabel::paintEvent(QPaintEvent *event)
{
    (void)event;

    // 1.获取宽度
    int height = (this->showLabel->font().pixelSize() * 1.2 ) + 20;
    int width = QFontMetrics(this->showLabel->font())
                    .horizontalAdvance(this->showLabel->text())+40;

    // 只绘制语音气泡部分
    QPainter painter(this);
    QPainterPath path;  //绘制不规则形状
    painter.setRenderHint(QPainter::Antialiasing);  //设置抗锯齿

    const int radius = 10;  //圆角度
    if(isLeft) {
        QColor fillColor = (mouse_press? pressedColor_left:leftColor);
        painter.setBrush(fillColor);
        painter.setPen(QPen(fillColor));
        painter.drawRoundedRect(10,0, width, height, radius,radius);

        // 绘制三角形
        path.moveTo(10,15);
        path.lineTo(0,20);
        path.lineTo(10,25);
        path.closeSubpath();
        painter.drawPath(path);

        this->showLabel->setGeometry(10, 0, width, height);
        this->textLabel->setGeometry(10, height+5, this->textLabel->width(), this->textLabel->height());
    } else {
        QColor fillColor = (mouse_press? pressedColor_right:rightColor);
        painter.setBrush(fillColor);
        painter.setPen(QPen(fillColor));

        int lxPos = this->width() - width - 10;
        int rxPos = this->width() - 10;
        painter.drawRoundedRect(lxPos, 0, width, height, radius,radius);

        path.moveTo(rxPos,15);
        path.lineTo(this->width(), 20);
        path.lineTo(rxPos,25);
        path.closeSubpath();
        painter.drawPath(path);

        this->showLabel->setGeometry(lxPos, 0, width, height);
        this->textLabel->setGeometry(this->width() - this->textLabel->width() - 10, height+5, this->textLabel->width(), this->textLabel->height());
    }

    // 6.重新设置父元素的高度，确保父元素足够高
    // 确保高度要涵盖之前 showLabel 的高度 和 留白
    QObject* obj = this->parent();
    if(!obj->isWidgetType()){
        //当前这个对象的父元素不是预期的QWidget
        //此时不需要后续的任何绘制操作
        LOG() << "超出预期";
        return;
    }
    QWidget* parent = dynamic_cast<QWidget*>(obj);
    parent->setFixedHeight(height + (this->onlySpeechLabel? 20:50) + (this->textLabel->isVisible()? this->textLabel->height():0));
}

void MessageSpeechLabel::contextMenuEvent(QContextMenuEvent *event)
{
    (void)event;
    if(onlySpeechLabel){ return; }

    QMenu* menu = new QMenu(this);
    QString actionText = textLabel->isVisible()? "收起文本":"语音转文本";
    QAction* action = menu->addAction(actionText);
    QString style = "QMenu { color: rgb(0,0,0); background-color: rgb(255,255,255);}";
    // style += "QMenu::item:hover { border-radius: 10px; padding: 0 10px; background-color: rgb(6, 183, 91);}";
    menu->setStyleSheet(style);
    connect(action, &QAction::triggered, this, [=](){
        if(textLabel->isVisible()) {
            //文本正在显示, 点击之后: 文本隐藏
            this->textLabel->hide();    //收起文字
        } else {
            this->speechConvertText();  //显示转成的文本
            MainWidget::getInstance()->getMessageShowArea()->scrollToEnd();
        }
    });
    menu->exec(event->globalPos());
    delete menu;
}

void MessageSpeechLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        mouse_press = true;
        this->update();
        event->accept();
    } else if(event->button() == Qt::RightButton){
        mouse_press = true;
    }
    QWidget::mousePressEvent(event);
}

void MessageSpeechLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton && mouse_press){
        mouse_press = false;
        update();

        // 检查点击位置是否还在气泡内
        QPoint clickPos = event->pos();
        if(showLabel->geometry().contains(clickPos)){
            // 没有播放就播放, 否则就停止播放
            if(isPlaying) {
                this->stop();
            } else {
                this->play();
            }
        }
        event->accept();
    } else if(event->button() == Qt::RightButton && mouse_press){
        mouse_press = false;
    }
    QWidget::mouseReleaseEvent(event);
}
















