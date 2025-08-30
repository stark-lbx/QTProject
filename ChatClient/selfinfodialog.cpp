#include "selfinfodialog.h"

#include "common/debug.h"
#include "common/toast.h"
#include "model/data.h"
#include "model/datacenter.h"

#include <QFileDialog>
#include <QMouseEvent>
#include <QPointer>

SelfInfoWidget::SelfInfoWidget(QWidget* parent)
    :QDialog(parent)
{
    // 1.设置整个窗口的属性
    // this->setWindowTitle("个人信息");
    // this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setFixedSize(400,200);
    this->setStyleSheet("QDialog {background-color: rgb(255,255,255);}");
    this->setFocusPolicy(Qt::StrongFocus);

    // 窗体模式
    // setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint /*| Qt::WindowStaysOnTopHint*/);
    setWindowFlags(Qt::Popup);
    // 窗口关闭时，自动销毁对话框对象
    this->setAttribute(Qt::WA_DeleteOnClose);
    // 把窗口移动到鼠标当前位置
    this->move(QCursor::pos());

    // 2. 创建布局管理器
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(2,2,2,2);
    this->setLayout(mainLayout);

    layout = new QGridLayout();
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(5);
    layout->setContentsMargins(20,20,20,0);
    layout->setAlignment(Qt::AlignTop);
    mainLayout->addLayout(layout);

    // 3.创建头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(75,75);
    avatarBtn->setIconSize(QSize(75,75));
    // avatarBtn->setIcon(QIcon(":/resource/image/defaultAvatar.png"));
    avatarBtn->setStyleSheet("QPushButton {border: none; background-color: transparent;}");
    layout->addWidget(avatarBtn,0,0,3,1);

    QString labelStyle = "QLabel {font-size: 14px; font-weight: 800;}";
    QString btnStyle = R"(
        QPushButton {
            border: none;
            background-color: transparent;
        }
        QPushButton:pressed {
            background-color: rgb(210,210,210);
        }
    )";
    QString editStyle = "QLineEdit {rgb(225, 225, 225); border-radius; padding: 2px; }";

    const int height = 30;
    // 4.添加用户id的显示
    idTag = new QLabel();
    idTag->setFixedSize(50, height);
    idTag->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    idTag->setText("序号:");
    idTag->setStyleSheet(labelStyle);

    idLabel = new QLabel();
    idLabel->setFixedHeight(height);
    idLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);


    // 5.添加用户的名字显示
    nameTag = new QLabel();
    nameTag->setFixedSize(50, height);
    nameTag->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    nameTag->setText("昵称:");
    nameTag->setStyleSheet(labelStyle);

    nameLabel = new QLabel();
    nameLabel->setFixedHeight(height);
    nameLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    nameEdit = new QLineEdit();
    nameEdit->setFixedHeight(height);
    nameEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    nameEdit->setStyleSheet(editStyle);
    nameEdit->hide();

    nameModifyBtn = new QPushButton();
    nameModifyBtn->setFixedSize(70,25);
    nameModifyBtn->setIconSize(QSize(25,25));
    nameModifyBtn->setIcon(QIcon(":/resource/image/modify.png"));
    nameModifyBtn->setStyleSheet(btnStyle);

    nameSubmitBtn = new QPushButton();
    nameSubmitBtn->setFixedSize(70,25);
    nameSubmitBtn->setIconSize(QSize(25,25));
    nameSubmitBtn->setIcon(QIcon(":/resource/image/submit.png"));
    nameSubmitBtn->setStyleSheet(btnStyle);
    nameSubmitBtn->hide();


    // 6.添加用户的签名显示
    descTag = new QLabel();
    descTag->setFixedSize(50, height);
    descTag->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    descTag->setText("签名:");
    descTag->setStyleSheet(labelStyle);

    descLabel = new QLabel();
    descLabel->setFixedHeight(height);
    descLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    descEdit = new QLineEdit();
    descEdit->setFixedHeight(height);
    descEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    descEdit->setStyleSheet(editStyle);
    descEdit->hide();

    descModifyBtn = new QPushButton();
    descModifyBtn->setFixedSize(70,25);
    descModifyBtn->setIconSize(QSize(25,25));
    descModifyBtn->setIcon(QIcon(":/resource/image/modify.png"));
    descModifyBtn->setStyleSheet(btnStyle);

    descSubmitBtn = new QPushButton();
    descSubmitBtn->setFixedSize(70,25);
    descSubmitBtn->setIconSize(QSize(25,25));
    descSubmitBtn->setIcon(QIcon(":/resource/image/submit.png"));
    descSubmitBtn->setStyleSheet(btnStyle);
    descSubmitBtn->hide();


    // 7.添加用户的电话显示
    phoneTag = new QLabel();
    phoneTag->setFixedSize(50, height);
    phoneTag->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    phoneTag->setText("电话:");
    phoneTag->setStyleSheet(labelStyle);

    phoneLabel = new QLabel();
    phoneLabel->setFixedHeight(height);
    phoneLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    phoneEdit = new QLineEdit();
    phoneEdit->setFixedHeight(height);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    phoneEdit->setStyleSheet(editStyle);
    phoneEdit->hide();

    phoneModifyBtn = new QPushButton();
    phoneModifyBtn->setFixedSize(70,25);
    phoneModifyBtn->setIconSize(QSize(25,25));
    phoneModifyBtn->setIcon(QIcon(":/resource/image/modify.png"));
    phoneModifyBtn->setStyleSheet(btnStyle);

    phoneSubmitBtn = new QPushButton();
    phoneSubmitBtn->setFixedSize(70,25);
    phoneSubmitBtn->setIconSize(QSize(25,25));
    phoneSubmitBtn->setIcon(QIcon(":/resource/image/submit.png"));
    phoneSubmitBtn->setStyleSheet(btnStyle);
    phoneSubmitBtn->hide();

    // 8.添加验证码
    verifyCodeTag = new QLabel();
    verifyCodeTag->setFixedSize(50,height);
    verifyCodeTag->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    verifyCodeTag->setText("验证码:");
    verifyCodeTag->setStyleSheet(labelStyle);
    verifyCodeTag->hide();

    verifyCodeEdit = new QLineEdit();
    verifyCodeEdit->setFixedHeight(height);
    verifyCodeEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    verifyCodeEdit->setStyleSheet(editStyle);
    verifyCodeEdit->hide();

    getVerifyCodeBtn = new QPushButton();
    getVerifyCodeBtn->setFixedSize(75,25);
    getVerifyCodeBtn->setText("获取验证码");
    getVerifyCodeBtn->setStyleSheet("QPushButton { border: none; background-color: transparent; } QPushButton:pressed { background-color: rgb(231, 231, 231); }");
    getVerifyCodeBtn->hide();


    // 添加到布局管理器(第 0 列被头像占用了，下列元素都从第 1 列开始)
    layout->addWidget(idTag,0,1);
    layout->addWidget(idLabel,0,2,1,2);

    layout->addWidget(nameTag,1,1);
    layout->addWidget(nameLabel,1,2);
    layout->addWidget(nameEdit,1,2);
    layout->addWidget(nameModifyBtn,1,3);
    layout->addWidget(nameSubmitBtn,1,3);

    layout->addWidget(descTag,2,1);
    layout->addWidget(descLabel,2,2);
    layout->addWidget(descEdit,2,2);
    layout->addWidget(descModifyBtn,2,3);
    layout->addWidget(descSubmitBtn,2,3);

    layout->addWidget(phoneTag,3,1);
    layout->addWidget(phoneLabel,3,2);
    layout->addWidget(phoneEdit,3,2);
    layout->addWidget(phoneModifyBtn,3,3);
    layout->addWidget(phoneSubmitBtn,3,3);

    layout->addWidget(verifyCodeTag,4,1);
    layout->addWidget(verifyCodeEdit,4,2);
    layout->addWidget(getVerifyCodeBtn,4,3);


    // 10.连接信号槽
    initSignalSlot();

    // 11.加载数据到界面上
#if TEST_UI
    avatarBtn->setIcon(QIcon(":/resource/image/defaultAvatar.png"));
    idLabel->setText("123456");
    nameLabel->setText("Stark");
    descLabel->setText("这是一个签名: 不敲代码就会噶");
    phoneLabel->setText("17812345678");

// verifyCodeTag->show();
// verifyCodeEdit->show();
// getVerifyCodeBtn->show();
#else
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    model::UserInfo* myself = dataCenter->getMyself();
    if(myself != nullptr){
        // 把个人信息, 显示到界面上
        avatarBtn->setIcon(myself->avatar);
        idLabel->setText(myself->userId);
        nameLabel->setText(myself->nickname);
        descLabel->setText(myself->description);
        phoneLabel->setText(myself->phone);
    } else {
        /* 日志 */ LOG() << "个人信息加载失败";
    }
#endif
}

void SelfInfoWidget::initSignalSlot()
{
    ///////////////////////////////////////////////////////////////
    /// 修改按钮槽函数
    ///////////////////////////////////////////////////////////////

    connect(nameModifyBtn, &QPushButton::clicked,this,[=](){
        nameLabel->hide();
        nameModifyBtn->hide();

        nameEdit->setText(nameLabel->text());
        nameEdit->show();
        nameSubmitBtn->show();
    });
    connect(descModifyBtn, &QPushButton::clicked,this,[=](){
        descLabel->hide();
        descModifyBtn->hide();

        descEdit->setText(descLabel->text());
        descEdit->show();
        descSubmitBtn->show();
    });
    connect(phoneModifyBtn, &QPushButton::clicked,this,[=](){
        phoneLabel->hide();
        phoneModifyBtn->hide();

        phoneEdit->setText(phoneLabel->text());
        phoneEdit->show();
        phoneSubmitBtn->show();

        verifyCodeTag->show();
        verifyCodeEdit->show();
        getVerifyCodeBtn->show();
    });


    ///////////////////////////////////////////////////////////////
    /// 确认按钮槽函数
    ///////////////////////////////////////////////////////////////

    connect(nameSubmitBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickNameSubmitBtn);
    connect(descSubmitBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickDescSubmitBtn);

    // 获取验证码按钮
    connect(getVerifyCodeBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickGetVerifyCodeBtn);
    connect(phoneSubmitBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickPhoneSubmitBtn);

    // 修改头像
    connect(avatarBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickAvatarBtn);
}

void SelfInfoWidget::clickNameSubmitBtn()
{
    // 1. 从输入框拿到修改后的昵称
    const QString& nickname = nameEdit->text();
    if(nickname.isEmpty()) return;

    // 2. 发送网络请求
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    connect(dataCenter, &DataCenter::changeNicknameDone,
            this, &SelfInfoWidget::clickNameSubmitBtnDone,
            Qt::UniqueConnection);
    dataCenter->changeNicknameAsync(nickname);

}

void SelfInfoWidget::clickNameSubmitBtnDone()
{
    nameLabel->setText(nameEdit->text());

    nameEdit->hide();
    nameLabel->show();

    nameSubmitBtn->hide();
    nameModifyBtn->show();
}

void SelfInfoWidget::clickDescSubmitBtn()
{
    // 1. 从输入框拿到修改后的昵称
    const QString& desc = descEdit->text();
    if(desc.isEmpty()) return;

    // 2. 发送网络请求
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    connect(dataCenter, &DataCenter::changeDescriptionDone,
            this, &SelfInfoWidget::clickDescSubmitBtnDone,
            Qt::UniqueConnection);
    dataCenter->changeDescriptionAsync(desc);
}

void SelfInfoWidget::clickDescSubmitBtnDone()
{
    descLabel->setText(descEdit->text());

    descEdit->hide();
    descLabel->show();

    descSubmitBtn->hide();
    descModifyBtn->show();
}

// 给修改后的手机号发送验证码
void SelfInfoWidget::clickGetVerifyCodeBtn()
{
    //1. 获取到输入框中的手机号
    const QString& phone = phoneEdit->text();
    if(phone.isEmpty()){
        return;
    }

    // 2.给服务器发起请求
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::getVerifyCodeDone, this, [=](){
        // 无需多做，只需要提示已经发送验证码

        // 这里一旦提示，那么selfInfoWidget这个弹框将被Toast弹窗关闭
        Toast::showMessage("短信验证码已经发送", false);

        // 4. 点击获取验证码按钮暂时禁用，并显示倒计时，避免频繁发送短信的请求
        // int* leftTime = new int(30); // 设为成员变量, 因为点击提交按钮还要用
        this->getVerifyCodeBtn->setEnabled(false);
        this->getVerifyCodeBtn->setText(QString("%1s").arg(this->leftTime));

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [=](){
            if(this->leftTime <= 1){
                this->getVerifyCodeBtn->setEnabled(true);
                this->getVerifyCodeBtn->setText("获取验证码");

                timer->stop();
                timer->deleteLater();
                return;
            }
            --(this->leftTime);
            this->getVerifyCodeBtn->setText(QString("%1s").arg(this->leftTime));
        });
        timer->start(1000);
    });
    dataCenter->getVerifyCodeAsync(phone);

    // 3.把刚才发送请求的号码保存起来
    //      后续点击提交按钮，修改电话，从这个保存的变量读取
    this->phoneToChange = phone;

    /*
     * 客户端发送请求
     * 1) 服务器首先 生成一个键值对:
     *  key: verifyCodeId   字符串
     *  value: verifyCode   字符串
     * 2) 调用第三方SDK实现发送短信的功能
     *  阿里云
     *  腾讯云
     *  ...
     * 3) 服务器返回verifyCodeId到客户端
     *
     * 后续客户端提交验证码时, 就会把verifyCodeId和短信收到的验证码一起发给服务器, 服务器进行验证
     * 服务器: 需要保存好这对键值对，以备后续验证，通过Redis进行保存，很方便实现过期功能
     */
}

void SelfInfoWidget::clickPhoneSubmitBtn()
{
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();

    // 1.判定验证码是否已经收到
    QString verifyCodeId = dataCenter->getVerifyCodeId();
    if(verifyCodeId.isEmpty()) {
        LOG() << "服务器尚未返回响应! 稍后重试";
        Toast::showMessage("服务器尚未返回验证码! 稍后重试", false);
        return;
    }

    // 如果当前已经拿到了verifyCodeId, 可以清空DataCenter中存储的值
    // 确保下次点击提交时, 上述逻辑仍然有效
    dataCenter->resetVerifyCodeId("");

    // 2.获取用户输入的验证码
    const QString& verifyCode = verifyCodeEdit->text();
    if(verifyCode.isEmpty()){
        LOG() << "验证码不可为空";
        Toast::showMessage("验证码不可为空",false);
        return;
    }

    // 3.发送请求, 把当前验证码信息发送给服务器
    connect(dataCenter, &DataCenter::changePhoneDone,
            this, &SelfInfoWidget::clickPhoneSubmitBtnDone,
            Qt::UniqueConnection);
    dataCenter->changePhoneAsync(this->phoneToChange, verifyCodeId, verifyCode);

    // 4.停止获取验证码的倒计时
    leftTime = 1;
}

void SelfInfoWidget::clickPhoneSubmitBtnDone()
{
    phoneLabel->setText(model::DataCenter::getInstance()->getMyself()->phone);

    phoneEdit->hide();
    phoneLabel->show();

    phoneSubmitBtn->hide();
    phoneModifyBtn->show();

    // 关于验证码的控件
    verifyCodeTag->hide();
    verifyCodeEdit->clear();
    verifyCodeEdit->hide();
    getVerifyCodeBtn->setText("发送频繁");
    getVerifyCodeBtn->hide();
}


void SelfInfoWidget::clickAvatarBtn()
{
    this->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    // 1. 弹出对话框, 选择图片
    QString filter = "Image Files (*.png *.jpg *.jpeg *.bmp)";
    QString imagePath = QFileDialog::getOpenFileName(this, "选择头像", QDir::homePath(), filter);
    // QFileDialog* fileDialog = new QFileDialog(this);
    // QString imagePath = fileDialog->getOpenFileName(this, "选择头像", QDir::homePath(), filter);
    if(imagePath.isEmpty()){
        LOG() << "用户未选择任何头像";
        return;
    }

    // 2. 根据路径读取到图片的内容
    QByteArray imageBytes = model::loadFileToByteArray(imagePath);

    // 3. 发送请求，修改头像
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::changeAvatarDone,
            this, &SelfInfoWidget::clickAvatarBtnDone,
            Qt::UniqueConnection);
    dataCenter->changeAvatarAsync(imageBytes);

}



void SelfInfoWidget::clickAvatarBtnDone()
{
    // 理论上执行不到这步，因为this已经关闭了，然后自动disconnect了这个信号槽
    // 如果采用非Popup的形式，这步还是有必要的
    model::DataCenter* dc = model::DataCenter::getInstance();
    avatarBtn->setIcon(QIcon(dc->getMyself()->avatar));
}








