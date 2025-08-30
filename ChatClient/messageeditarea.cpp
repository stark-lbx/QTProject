#include "messageeditarea.h"

#include "common/toast.h"
#include "model/datacenter.h"
#include "historymessagedialog.h"
#include "mainwidget.h"
#include "soundrecorder.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QFileDialog>


MessageEditArea::MessageEditArea(QWidget *parent)
    : QWidget{parent}
{
    // 1.设置必要的属性
    this->setFixedHeight(200); //尺寸
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    this->setStyleSheet("QWidget {border-top: 1px solid rgb(213, 213, 213);}");

    // 2.创建垂直方向的布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(10,0,10,10);
    this->setLayout(vlayout);

    // 3.创建水平方向的布局管理器
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(10,0,0,0);
    hlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    vlayout->addLayout(hlayout);

    // 4.把上方四个按钮添加到水平布局中
    QString btnStyle = R"(
        QPushButton {
            background-color: transparent;
            border: none;
        }
        QPushButton:hover {
            background-color: rgb(225, 225, 225);
        }
        QPushButton:pressed {
            background-color: rgb(213, 213, 213);
        }
    )";
    QSize btnSize(35,35);
    QSize iconSize(25,25);

    sendImageBtn = new QPushButton();
    sendImageBtn->setFixedSize(btnSize);
    sendImageBtn->setIconSize(iconSize);
    sendImageBtn->setStyleSheet(btnStyle);
    sendImageBtn->setIcon(QIcon(":/resource/image/image.png"));
    hlayout->addWidget(sendImageBtn);

    sendFileBtn = new QPushButton();
    sendFileBtn->setFixedSize(btnSize);
    sendFileBtn->setIconSize(iconSize);
    sendFileBtn->setStyleSheet(btnStyle);
    sendFileBtn->setIcon(QIcon(":/resource/image/file.png"));
    hlayout->addWidget(sendFileBtn);

    sendSpeechBtn = new QPushButton();
    sendSpeechBtn->setFixedSize(btnSize);
    sendSpeechBtn->setIconSize(iconSize);
    sendSpeechBtn->setStyleSheet(btnStyle);
    sendSpeechBtn->setIcon(QIcon(":/resource/image/sound.png"));
    hlayout->addWidget(sendSpeechBtn);

    showHistoryBtn = new QPushButton();
    showHistoryBtn->setFixedSize(btnSize);
    showHistoryBtn->setIconSize(iconSize);
    showHistoryBtn->setStyleSheet(btnStyle);
    showHistoryBtn->setIcon(QIcon(":/resource/image/history.png"));
    hlayout->addWidget(showHistoryBtn);

    // 5.添加多行编辑框
    textEdit = new QPlainTextEdit();
    textEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    textEdit->setStyleSheet("QPlainTextEdit {border: none; background-color: transparent; font-size 14px; padding: 10px;}");
    textEdit->verticalScrollBar()->setStyleSheet("QScrollBar:vertical {width: 2px; background-color: rgb(45,45,45); }");
    vlayout->addWidget(textEdit);

    // 6. 添加提示 "正在录制中" 这样的 Label
    tipLabel = new QLabel();
    tipLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    tipLabel->setText("正在录音中, 松开发送");
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setFont(QFont("微软雅黑", 24, 600));
    vlayout->addWidget(tipLabel);
    tipLabel->hide();

    // 7.添加发送文本的按钮
    QString sendStyle = R"(
        QPushButton {
            font-size: 16px;
            border: none;
            border-radius: 5px;
            color: rgb(255, 255, 255);
            background-color: rgb(7, 193, 96);
        }
        QPushButton:hover {
            color: rgb(255, 255, 255);
            background-color: rgb(6, 183, 91);
        }
        QPushButton:pressed {
            color: rgb(255, 255, 255);
            background-color: rgb(6, 174, 86);
        }
        QPushButton:disabled {
            color: rgb(157, 157, 157);
            background-color: rgb(225, 225, 225);
        }
    )";
    sendTextBtn = new QPushButton("发 送");
    sendTextBtn->setFixedSize(120,40);
    sendTextBtn->setStyleSheet(sendStyle);
    sendTextBtn->setEnabled(false);
    vlayout->addWidget(sendTextBtn, 0, Qt::AlignVCenter | Qt::AlignRight);

    timer = new QTimer();
    timer->setInterval(1000);   //1s间隔

    initSignalSlot();
}

void MessageEditArea::clear()
{
    this->textEdit->setPlainText("");
    this->sendTextBtn->setEnabled(false);
}


void MessageEditArea::initSignalSlot()
{
    auto* dataCenter = model::DataCenter::getInstance();    
    /////////////////////////////////////////////
    /// 点击历史记录, 打开历史消息展示窗口
    /////////////////////////////////////////////
    connect(showHistoryBtn, &QPushButton::clicked, this, [=](){
        // model::ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionById(dataCenter->getCurrentChatSessionId());
        // if(chatSessionInfo == nullptr){
        //     Toast::showMessage("当前未选中任何会话");
        //     return;
        // }
        if(dataCenter->getCurrentChatSessionId().isEmpty()){
            Toast::showMessage("当前未选中任何会话", false);
            return;
        }

        HistoryMessageWidget* historyMessageWidget = new HistoryMessageWidget(dataCenter->getCurrentChatSessionId(),this);
        historyMessageWidget->show();
    });

    /////////////////////////////////////////////
    /// 发送完消息后, 应该处理的任务
    /////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::sendMessageDone,
            this, &MessageEditArea::addMyselfMessage);

    /////////////////////////////////////////////
    /// 发送文本消息功能
    ///     messageEditArea: sendTextBtn()
    ///     1. 处理 发送按钮 点击操作
    ///     2. 把输入框中的内容, 通过网络传输给服务器
    ///     3. 发送消息成功, 消息展示区 显示刚才发的消息
    ///     4. 清空输入框的文本消息
    /////////////////////////////////////////////
    connect(sendTextBtn, &QPushButton::clicked, this, &MessageEditArea::sendTextMessage);

    /////////////////////////////////////////////
    /// 文本框的输入 与 按钮的禁用信号槽
    /////////////////////////////////////////////
    connect(textEdit, &QPlainTextEdit::textChanged, sendTextBtn, [=](/*const QString& text*/){
        if(dataCenter->getCurrentChatSessionId() == ""){
            // 没有选中任何会话, 默认禁用
            return;
        }
        // 文本不空则启用；否则禁用
        this->sendTextBtn->setEnabled(!textEdit->toPlainText().trimmed().isEmpty());
    });

    /////////////////////////////////////////////
    /// 接收完消息后, 应该处理的任务
    /////////////////////////////////////////////
    connect(model::DataCenter::getInstance(), &model::DataCenter::receiveMessageDone,
            this, &MessageEditArea::addOthersMessage);

    /////////////////////////////////////////////
    /// 关联发送图片信号槽
    /////////////////////////////////////////////
    connect(sendImageBtn, &QPushButton::clicked, this, &MessageEditArea::sendImageMessage);

    /////////////////////////////////////////////
    /// 关联发送文件信号槽
    /////////////////////////////////////////////
    connect(sendFileBtn, &QPushButton::clicked, this, &MessageEditArea::sendFileMessage);

    /////////////////////////////////////////////
    /// 关联发送语音信号槽
    /////////////////////////////////////////////
    connect(sendSpeechBtn, &QPushButton::pressed, this, &MessageEditArea::recordSpeechMessage);
    connect(sendSpeechBtn, &QPushButton::released, this, [=](){
        if(recordIsAutoStopped){
            recordIsAutoStopped = false;
            return; //已经自动结束, 那么就不再发送了
        }
        this->stopTimer();
        this->sendSpeechMessage();
    });


    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    connect(soundRecorder, &SoundRecorder::soundRecordDone, this, [=](const QString& path){
        // 发送语音
        model::DataCenter* dataCenter = model::DataCenter::getInstance();
        QByteArray content = model::loadFileToByteArray(path);
        if(content.isEmpty()){
            LOG() << "语音文件加载失败";
            return;
        }
        dataCenter->sendSpeechMessageAsync(dataCenter->getCurrentChatSessionId(), content);
    });
    // 设置倒计时
    connect(timer, &QTimer::timeout, this, [=](){
        if(leftTime < 1) {
            recordIsAutoStopped = true;
            sendSpeechMessage();        //发送信息
            stopTimer();
            return;
        }
        if(leftTime <= 10){
            this->tipLabel->setText(QString("%1\"后自动停止录音").arg(leftTime));
        }
        --(leftTime);
    });
}

void MessageEditArea::sendTextMessage()
{
    using model::DataCenter;
    // 1. 先确定当前是否有会话被选中了
    DataCenter* dataCenter = DataCenter::getInstance();
    if(dataCenter->getCurrentChatSessionId().isEmpty()){
        /* 日志 */ LOG() << "当前未选中任何会话, 不会发送任何信息";
        /* 运行 */ Toast::showMessage("当前未选中任何会话",false);
        return;
    }

    // 2. 获取到输入框的内容, 看输入框是否有内容
    // 如果trim操作后的内容 是空, 直接返回
    // trim: 将文本的左右两侧的空白字符去除
    const QString& content = textEdit->toPlainText().trimmed();
    if(content.isEmpty()){
        /* 日志 */ LOG() << "输入框输入信息为空, 不会发送任何信息";
        /* 运行 */ //Toast::showMessage("输入框输入信息为空, 不会发送任何信息");
        return;
    }

    // 3. 清空输入框已有内容
    textEdit->setPlainText("");

    // 4. 通过网络发送数据给服务器
    dataCenter->sendTextMessageAsync(dataCenter->getCurrentChatSessionId(), content);
}

void MessageEditArea::sendImageMessage()
{
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    // 1.判断当前是否有选中的会话
    if(dataCenter->getCurrentChatSessionId().isEmpty()){
        // 没有选中会话
        Toast::showMessage("当前未选中任何会话", false);
        return;
    }

    // 2.选择图片文件
    QString filter = "Image (*.png *.jpg *.jpeg)";
    QString imgPath = QFileDialog::getOpenFileName(this, "选择图片", QDir::homePath(), filter);
    if(imgPath.isEmpty()){
        LOG() << "用户取消选择图片";
        return;
    }

    // 3.读取图片内容
    QByteArray imageContent = model::loadFileToByteArray(imgPath);

    // 4.发送请求
    dataCenter->sendImageMessageAsync(dataCenter->getCurrentChatSessionId(), imageContent);
}

void MessageEditArea::sendFileMessage()
{
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    // 1.判定当前是否选中了会话
    if(dataCenter->getCurrentChatSessionId().isEmpty()){
        Toast::showMessage("没有选中任何会话");
        return;
    }

    // 2.弹出对话框，选择文件
    QString filter = "All (*.*)";
    QString path = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath(), filter);
    if(path.isEmpty()){
        // 取消了选择文件
        LOG() <<"用户选择取消";
        return;
    }

    // 3.读取文件内容
    //      此处暂时不考虑大文件的情况, 比如有的文件, 几百MB、或者几个GB的
    //      针对大文件，编写专门的网络通信接口，实现“分片传输”效果 [客户端:分片、 服务端:组装]
    QByteArray content = model::loadFileToByteArray(path);

    // 4.传输文件, 还需要获取到 文件名
    QFileInfo fileInfo(path);
    const QString& filename = fileInfo.fileName();

    // 5.发送消息
    dataCenter->sendFileMessageAsync(dataCenter->getCurrentChatSessionId(), filename, content);
}

void MessageEditArea::recordSpeechMessage()
{
    // 鼠标按下的时候，开始录音
    // 判断当前是否选中会话
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if(dataCenter->getCurrentChatSessionId().isEmpty()){
        Toast::showMessage("当前未选中任何会话", false);
        return;
    }

    // 切换语音按钮图标
    sendSpeechBtn->setIcon(QIcon(":/resource/image/sound_active.png"));
    tipLabel->show();
    textEdit->hide();

    // 开始录音
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    soundRecorder->startRecord();
    timer->start(); //立即开始
}

void MessageEditArea::sendSpeechMessage()
{
    // 鼠标释放的时候，录音结束, 并且发送语音消息
    // 判断当前是否选中会话
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if(dataCenter->getCurrentChatSessionId().isEmpty()){
        Toast::showMessage("当前未选中任何会话", false);
        return;
    }

    // 切换语音按钮图标
    sendSpeechBtn->setIcon(QIcon(":/resource/image/sound.png"));

    // 停止录制
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    soundRecorder->stopRecord();

    tipLabel->hide();
    textEdit->show();
}

// 针对自己发送消息的操作, 做一个处理, 把自己的消息添加到消息展示区
void MessageEditArea::addMyselfMessage(model::MessageType messageType, const QByteArray &content, const QString &extraInfo)
{
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    const QString& currentChatSessionId = dataCenter->getCurrentChatSessionId();

    // 1.构造一个消息对象
    model::Message message = Message::makeMessage(
        messageType,
        currentChatSessionId,
        *dataCenter->getMyself(),
        content,
        extraInfo);
    dataCenter->addMessage(message);
    // LOG() <<"debug:" << messageType<<'\t'<< currentChatSessionId <<'\t' <<content.size();

    // 2.展示到消息展示区
    MainWidget* mainWidget = MainWidget::getInstance();
    MessageShowArea* messageShowArea = mainWidget->getMessageShowArea();
    messageShowArea->addMessage(false, message);

    // 3.控制消息显示区, 滚动条滚动到末尾
    messageShowArea->scrollToEnd();

    // 4. 发送信号, 通知会话列表, 更新最后一条消息
    emit dataCenter->updateLastMessage(currentChatSessionId);
}

void MessageEditArea::addOthersMessage(const model::Message &message)
{
    LOG() <<"test01";
    // 1.通过界面拿到消息展示区
    MainWidget* mainWidget = MainWidget::getInstance();
    MessageShowArea* messageShowArea = mainWidget->getMessageShowArea();
    LOG() <<"test02";

    // 2.把收到的新的消息, 添加到消息展示区
    messageShowArea->addMessage(true, message);
    LOG() <<"test03";

    // 3.滚动到末尾
    messageShowArea->scrollToEnd();
    LOG() <<"test04";

    // 4.提示一个收到消息 - 当前显示该对话，不予提示
    // Toast::showMessage("收到一条新的消息");
}















