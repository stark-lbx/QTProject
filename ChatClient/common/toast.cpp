#include "Toast.h"
#include "../mainwidget.h"

#include <QMessageBox>
#include <QPushButton>
#include <QWidget>
#include <QDialog>

Toast::Toast(const QString &text, bool play)
    :audioOutput(nullptr)
    ,player(nullptr)
{
    if(play) {
        audioOutput = new QAudioOutput;
        audioOutput->setVolume(100);    //音量: 0-100

        player = new QMediaPlayer;
        player->setAudioOutput(audioOutput);
        player->setSource(QUrl("qrc:/resource/audio/information.mp3"));

        connect(player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error){
            LOG() << "音频播放错误: "<< player->errorString();
        });
        connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status){
            if(status == QMediaPlayer::LoadedMedia){
                LOG() << "音频加载成功, 准备播放";
            } else if(status == QMediaPlayer::InvalidMedia){
                LOG() << "音频文件无效或无法加载";
            }
        });
    }


    // 1. 基本窗口设置
    setFixedSize(200, 100); // 设置初始大小（将在 showEvent 中调整）
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground); // 启用半透明背景
    setAttribute(Qt::WA_ShowWithoutActivating); // 不抢夺焦点
    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);


    // 3. 创建布局和标签
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    this->setLayout(layout);

    m_label = new QLabel(this);
    m_label->setText(text);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_label->setStyleSheet("QLabel {"
                           "font-size: 32px;"
                           "color: white;"
                           "background-color: transparent;"
                           "padding: 15px;"  // 标签内边距
                           "}");
    layout->addWidget(m_label);


    // 4. 定时关闭（2秒）
    // QTimer::singleShot(2000, this, &Toast::close);
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=](){
        timer->stop();
        this->close();
    });
    timer->start(1000 * 2);
}

void Toast::paintEvent(QPaintEvent *event)
{
    (void) event;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

    // 绘制半透明圆角背景
    QBrush brush(QColor(0, 0, 0, 180)); // 默认背景色：黑色，70%不透明度
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);

    // 绘制圆角矩形（比窗口小1像素，避免边缘锯齿）
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 20, 20);
}

void Toast::showEvent(QShowEvent *event)
{
    (void) event;

    // 1. 确保字体已完全加载
    QApplication::processEvents();

    // 2. 计算文本实际需要的尺寸
    QFontMetrics metrics(m_label->font());
    int textWidth = metrics.horizontalAdvance(m_label->text());
    int textHeight = metrics.height();

    // 3. 计算窗口最终尺寸（包含内边距）
    int width = qMax(textWidth + 50 * 2, 200);
    int height = qMax(textHeight + 30 * 2, 100);

    // 4. 设置窗口固定大小
    setFixedSize(width, height);

    // 5. 定位到屏幕底部中央
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width) / 2;
        int y = screenGeometry.height() - height - 100;
        move(x, y);
    }

    // 6. 更新布局确保标签正确显示
    layout()->activate();
    updateGeometry();
}

void Toast::showMessage(const QString &text, bool play)
{
    Toast* toast = new Toast(text, play);
    if(play){
        // QApplication::beep();    //不静音, 那就响铃
        // 等待媒体加载后再播放（避免文件未加载完成）
        connect(toast->player, &QMediaPlayer::mediaStatusChanged, toast, [toast](QMediaPlayer::MediaStatus status) {
            if (status == QMediaPlayer::LoadedMedia) {
                toast->player->play();
            }
        });
    }
    toast->show();
}

bool Toast::deleteFriendBox(const QString &userNickname)
{
    QWidget* caller = MainWidget::getInstance();
    const int boxWidth = 350;
    const int boxHeight = 230;

    // 整体设置
    QDialog* msgBox = new QDialog();
    msgBox->setWindowTitle("删除联系人");
    msgBox->setWindowFlag(Qt::FramelessWindowHint);
    msgBox->setFixedSize(boxWidth, boxHeight);

    int x = caller->x() + (caller->width() - boxWidth)/2;
    int y = caller->y() + (caller->height() - boxHeight)/2;
    msgBox->move(x, y);

    msgBox->setStyleSheet(R"(
    QDialog {
        background-color: rgb(247, 247, 247);
        border: 1px solid #e0e0e0;
        border-radius: 10px;
    }
    QLabel {
        color: rgb(158, 158, 158);
        font-size: 18px;
    }
    )");

    // 布局创建
    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->setContentsMargins(30,40,30,40);
    mainLayout->setVerticalSpacing(0);
    mainLayout->setHorizontalSpacing(2);
    msgBox->setLayout(mainLayout);

    // 组件创建
    //      1.创建标题
    QLabel* titleLabel = new QLabel();
    titleLabel->setFixedSize(120,40);
    titleLabel->setText("删除联系人");
    titleLabel->setStyleSheet("QLabel {background-color: transparent; color: rgb(22, 22, 22); font-size: 20px;}");
    titleLabel->setAlignment(Qt::AlignTop | Qt:: AlignHCenter);
    mainLayout->addWidget(titleLabel,0,3,1,5);

    //      2.创建文本
    QLabel* contentLabel = new QLabel();
    contentLabel->setText(QString("删除 %1 后,\n将同时删除与该联系人的聊天记录。").arg(userNickname));
    contentLabel->setStyleSheet("QLabel {background-color: transparent; color: rgb(158, 158, 158); font-size: 18px;}");
    contentLabel->setAlignment(Qt::AlignTop | Qt:: AlignHCenter);
    mainLayout->addWidget(contentLabel,1,0,1,12);

    //      3.创建按钮
    QString deleteBtnStyle = R"(
    QPushButton {
        color: #fa5151;
        background-color: #eaeaea;
        border: 1px solid #e0e0e0;
        border-radius: 4px;
        padding: 8px 16px;   /* 内部间距，影响按钮大小 */
        font-size: 18px;
    }
    QPushButton:hover { background-color: #e4e4e4; }
    QPushButton:pressed { background-color: #dedede; }
    )";

    QString cancelBtnStyle = R"(
    QPushButton {
        color: #181818;
        background-color: #eaeaea;
        border: 1px solid #e0e0e0;
        border-radius: 4px;
        padding: 8px 16px;
        font-size: 18px;
    }
    QPushButton:hover { background-color: #e4e4e4; }
    QPushButton:pressed { background-color: #dedede; }
    )";

    QPushButton* deleteBtn = new QPushButton("删除");
    QPushButton* cancelBtn = new QPushButton("取消");
    deleteBtn->setFixedSize(140,35);
    cancelBtn->setFixedSize(140,35);
    deleteBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cancelBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    deleteBtn->setStyleSheet(deleteBtnStyle);
    cancelBtn->setStyleSheet(cancelBtnStyle);

    mainLayout->addWidget(deleteBtn, 2,0,1,6);
    mainLayout->addWidget(cancelBtn, 2,6,1,6);

    // 获取点击按钮的情况并返回
    bool ensure = false;
    connect(deleteBtn, &QPushButton::clicked, msgBox, [=, &ensure](){
        ensure = true;
        msgBox->deleteLater();
    });
    connect(cancelBtn, &QPushButton::clicked, msgBox, [=, &ensure](){
        ensure = false;
        msgBox->deleteLater();
    });

    msgBox->exec(); // 阻塞
    return ensure;
}

bool Toast::quitApplicationBox()
{
    const int boxWidth = 200;
    const int boxHeight = 150;

    // 整体设置
    QDialog* msgBox = new QDialog();
    msgBox->setWindowTitle("退出程序");
    msgBox->setWindowFlag(Qt::FramelessWindowHint);
    msgBox->setFixedSize(boxWidth, boxHeight);
    msgBox->setStyleSheet(R"(
    QDialog {
        background-color: rgb(247, 247, 247);
        border: 1px solid #e0e0e0;
        border-radius: 10px;
    }
    QLabel {
        color: rgb(158, 158, 158);
        font-size: 18px;
    }
    )");

    // 布局创建
    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->setContentsMargins(10,20,10,20);
    mainLayout->setVerticalSpacing(0);
    mainLayout->setHorizontalSpacing(5);
    msgBox->setLayout(mainLayout);

    // 组件创建
    //      1.创建标题
    QLabel* titleLabel = new QLabel();
    titleLabel->setText("是否退出程序？");
    titleLabel->setStyleSheet("QLabel {background-color: transparent; color: rgb(22, 22, 22); font-size: 20px;}");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel,0,0,1,4);

    //      2.创建按钮
    QString okBtnStyle = R"(
    QPushButton {
        color: #fa5151;
        background-color: #eaeaea;
        border: 1px solid #e0e0e0;
        border-radius: 4px;
        padding: 8px 16px;   /* 内部间距，影响按钮大小 */
        font-size: 18px;
    }
    QPushButton:hover { background-color: #e4e4e4; }
    QPushButton:pressed { background-color: #dedede; }
    )";

    QString noBtnStyle = R"(
    QPushButton {
        color: #181818;
        background-color: #eaeaea;
        border: 1px solid #e0e0e0;
        border-radius: 4px;
        padding: 8px 16px;
        font-size: 18px;
    }
    QPushButton:hover { background-color: #e4e4e4; }
    QPushButton:pressed { background-color: #dedede; }
    )";

    QPushButton* okBtn = new QPushButton("确定");
    QPushButton* noBtn = new QPushButton("取消");
    okBtn->setStyleSheet(okBtnStyle);
    noBtn->setStyleSheet(noBtnStyle);
    mainLayout->addWidget(okBtn, 1,0,1,2);
    mainLayout->addWidget(noBtn, 1,2,1,2);

    // 获取点击按钮的情况并返回
    bool ensure = false;
    connect(okBtn, &QPushButton::clicked, msgBox, [=, &ensure](){
        ensure = true;
        msgBox->deleteLater();
    });
    connect(noBtn, &QPushButton::clicked, msgBox, [=, &ensure](){
        ensure = false;
        msgBox->deleteLater();
    });

    msgBox->exec(); // 阻塞
    return ensure;
}
