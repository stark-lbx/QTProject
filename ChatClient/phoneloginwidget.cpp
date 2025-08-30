#include "phoneloginwidget.h"

#include "loginwidget.h"
#include "mainwidget.h"
#include "common/toast.h"
#include "model/datacenter.h"

#include <QGridLayout>
#include <QRegularExpression>

PhoneLoginWidget::PhoneLoginWidget(QWidget *parent, bool isLoginMode)
    : QWidget{parent}
    , isLoginMode(isLoginMode)
{
    // 1. 设置本窗口的基本属性
    this->setFixedSize(350, 475);
    this->setWindowTitle("墨鸿");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setStyleSheet("QWidget { background-color: rgb(238, 245, 249); }");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 2. 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    // layout->setSpacing(0);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(20);
    layout->setContentsMargins(50,50,50,50);
    layout->setAlignment(Qt::AlignTop);
    this->setLayout(layout);

    // 3.创建标题
    titleLabel = new QLabel();
    titleLabel->setText("登录");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(50);
    titleLabel->setStyleSheet("QLabel { font-size: 40px; font-weight: 600; }");


    QString editStyle = R"(
        QLineEdit {
            border: none;
            border-radius: 10px;
            font-size: 20px;
            background-color: rgb(240,240,240);
            padding-left: 10px;
        }
        QLineEdit:focus {
            border: 2px solid rgb(69,209,136);
            outline: none;
            background-color: rgb(255,255,255);
            /*caret-color: rgb(69,209,136);*/
        }
    )";
    // 4.创建手机号输入框
    phoneEdit = new QLineEdit();
    phoneEdit->setFixedHeight(40);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    phoneEdit->setPlaceholderText("输入手机号");
    phoneEdit->setStyleSheet(editStyle);

    // 5.创建验证码输入框
    verifyCodeEdit = new QLineEdit();
    verifyCodeEdit->setFixedHeight(40);
    verifyCodeEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    verifyCodeEdit->setPlaceholderText("输入验证码");
    verifyCodeEdit->setStyleSheet(editStyle);


    QString codeStyle = R"(
        QPushButton {
            border: none;
            border-radius: 10px;
            background-color: transparent;
            color: black;
        }
        QPushButton:hover {
            color: rgb(7, 129, 216);
            text-decoration: underline;
            padding-bottom: 4px;
        }
        QPushButton:pressed {
            color: rgb(14, 82, 153);
            text-decoration: underline;
            padding-bottom: 4px;
        }
        QPushButton:disabled {
            color: rgb(157, 157, 157);
            background-color: rgb(225, 225, 225);
        }
    )";
    // 6.创建获取验证码按钮
    sendVerifyCodeBtn = new QPushButton();
    sendVerifyCodeBtn->setFixedSize(100,40);
    sendVerifyCodeBtn->setText("发送验证码");
    sendVerifyCodeBtn->setStyleSheet(codeStyle);


    QString submitStyle =R"(
        QPushButton {
            border: none;
            border-radius: 10px;
            background-color: rgb(7, 193, 96);
            color: white;
        }
        QPushButton:hover {
            background-color: rgb(6, 183, 91);
        }
        QPushButton:pressed {
            background-color: rgb(6, 174, 86);
        }
    )";
    // 7.创建提交按钮
    submitBtn = new QPushButton();
    submitBtn->setText("登录");
    submitBtn->setFixedHeight(40);
    submitBtn->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    submitBtn->setStyleSheet(submitStyle);
    submitBtn->setEnabled(false);


    QString whiteBtnStyle = R"(
        QPushButton {
            border: none;
            border-radius: 10px;
            background-color: transparent;
            color: black;
        }
        QPushButton:hover {
            color: rgb(7, 129, 216);
            text-decoration: underline;
            padding-bottom: 4px;
        }
        QPushButton:pressed {
            color: rgb(14, 82, 153);
            text-decoration: underline;
            padding-bottom: 4px;
        }
    )";
    // 8.创建 "切换到用户名" 模式按钮
    userModeBtn = new QPushButton();
    userModeBtn->setFixedSize(100, 40);
    userModeBtn->setText("用户名登陆");
    userModeBtn->setStyleSheet(whiteBtnStyle);

    // 9. 创建切换到注册模式的按钮
    switchModeBtn = new QPushButton();
    switchModeBtn->setFixedSize(100, 40);
    switchModeBtn->setText("注册");
    switchModeBtn->setStyleSheet(whiteBtnStyle);

    // 10. 添加到布局管理器
    layout->addWidget(titleLabel, 0, 2);
    layout->addWidget(phoneEdit, 1, 0, 1, 5);
    layout->addWidget(verifyCodeEdit, 2, 0, 1, 4);
    layout->addWidget(sendVerifyCodeBtn, 2, 4);
    layout->addWidget(submitBtn, 3, 0, 1, 5);
    layout->addWidget(userModeBtn, 4, 0);
    layout->addWidget(switchModeBtn, 4, 4);


    // 12. 处理信号槽
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &PhoneLoginWidget::countOut);
    connect(phoneEdit, &QLineEdit::textEdited, this, &PhoneLoginWidget::checkSbumitEnable);
    connect(verifyCodeEdit, &QLineEdit::textEdited, this, &PhoneLoginWidget::checkSbumitEnable);

    connect(switchModeBtn, &QPushButton::clicked, this, &PhoneLoginWidget::switchMode);
    connect(userModeBtn, &QPushButton::clicked, this, &PhoneLoginWidget::switchLoginMode);

    connect(sendVerifyCodeBtn, &QPushButton::clicked, this, &PhoneLoginWidget::sendVerifyCode);
    connect(submitBtn, &QPushButton::clicked, this, &PhoneLoginWidget::clickSubmitBtn);
    // qDebug() << isLoginMode;
    if(isLoginMode == false){
        // 当前模式不是登录模式, 要设置到注册模式
        titleLabel->setText("注册");
        submitBtn->setText("注册");
        userModeBtn->setText("用户名注册");
        switchModeBtn->setText("登录");
    }
}

void PhoneLoginWidget::countOut()
{
    if(this->leftTime <= 1){
        this->sendVerifyCodeBtn->setEnabled(true);
        this->sendVerifyCodeBtn->setText("获取验证码");
        this->leftTime = 30;

        timer->stop();
        return;
    }
    --(this->leftTime);
    this->sendVerifyCodeBtn->setText(QString("%1s").arg(this->leftTime));
    if(sendVerifyCodeBtn->isEnabled()){
        sendVerifyCodeBtn->setEnabled(false);
    }
}

void PhoneLoginWidget::checkSbumitEnable(const QString &)
{
    if( phoneEdit->text().trimmed().isEmpty() ||
        verifyCodeEdit->text().trimmed().isEmpty() ){
        // 如果二者任一输入为空、则无法点击
        submitBtn->setEnabled(false);
    } else {
        // 否则按钮设置为可以点击
        submitBtn->setEnabled(true);
    }
}

void PhoneLoginWidget::clickSubmitBtn()
{
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    const QString& phone = this->currentPhone;
    const QString& verifyCode = verifyCodeEdit->text();

    if(isLoginMode){
        // 登录
        connect(dataCenter, &DataCenter::phoneLoginDone, this, &PhoneLoginWidget::phoneLoginDone);
        dataCenter->phoneLoginAsync(phone, verifyCode);
    } else {
        // 注册
        connect(dataCenter, &DataCenter::phoneRegisterDone, this, &PhoneLoginWidget::phoneRegisterDone);
        dataCenter->phoneRegisterAsync(phone, verifyCode);
    }
}

void PhoneLoginWidget::phoneLoginDone(bool ok, const QString &reason)
{
    if(!ok){
        Toast::showMessage(QString("登陆失败(%1)").arg(reason), false);
        return;
    } else {
        MainWidget::getInstance()->launchLater();
        // connect(model::DataCenter::getInstance(), &model::DataCenter::loginDone, this, [=](){this->close();});
        this->close();
        return;
    }
}

void PhoneLoginWidget::phoneRegisterDone(bool ok, const QString &reason)
{
    if(!ok){
        Toast::showMessage(QString("注册失败(%1)").arg(reason), false);
    } else {
        Toast::showMessage("注册成功", false);
        this->switchMode();                 //切换到登录界面
    }
    verifyCodeEdit->clear();                //清空输入的验证码，等待重新输入
}

/**
 * @brief 验证手机号格式是否正确
 * @param phone 待验证的手机号字符串
 * @return 格式正确返回true，否则返回false
 *
 * 支持的号段包括：
 * 130-139：中国移动、中国联通、中国电信
 * 145、147：中国联通、中国移动
 * 150-159：三大运营商
 * 166：中国联通
 * 170、171、173、175、176、177、178：不同运营商
 * 180-189：三大运营商
 * 198、199：中国移动、中国电信
 */
bool PhoneLoginWidget::isPhoneNumberValid(const QString &phone)
{
    // 空字符串直接返回无效
    if (phone.trimmed().isEmpty()) {
        return false;
    }

    // 手机号正则表达式
    // ^1 表示以1开头
    // ( 3[0-9] | 4[57] | 5[0-35-9] | 6[6] | 7[0135-8] | 8[0-9] | 9[89] ) 匹配运营商号段
    // \\d{8} 匹配后面8位数字
    QRegularExpression phoneRegExp("^1(3[0-9]|4[57]|5[0-35-9]|6[6]|7[0135-8]|8[0-9]|9[89])\\d{8}$");

    // 严格匹配整个字符串
    return phoneRegExp.match(phone).hasMatch();
}

void PhoneLoginWidget::switchMode()
{
    if(isLoginMode){
        // 当前模式是登录模式, 要切换到注册模式
        titleLabel->setText("注册");
        submitBtn->setText("注册");
        userModeBtn->setText("用户名注册");
        switchModeBtn->setText("登录");
    } else {
        // 当前模式是注册模式, 要切换到登陆模式
        titleLabel->setText("登录");
        submitBtn->setText("登录");
        userModeBtn->setText("用户名登录");
        switchModeBtn->setText("注册");
    }
    isLoginMode = !isLoginMode;
}

void PhoneLoginWidget::switchLoginMode()
{
    LoginWidget* loginWidget = new LoginWidget(nullptr, this->isLoginMode);
    loginWidget->show();

    // 关闭当前窗口
    this->close();
}

void PhoneLoginWidget::sendVerifyCode()
{
    // 1.获取到验证码
    QString phone = this->phoneEdit->text();
    if(!isPhoneNumberValid(phone)){
        Toast::showMessage("非法手机号, 无法发送验证码", false);
        return; // 手机号校验器：正则表达式
    }
    currentPhone = phone;

    // 2.发送网络请求，发送验证码
    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    connect(dataCenter, &DataCenter::getVerifyCodeDone,
            this, &PhoneLoginWidget::sendVerifyCodeDone, Qt::UniqueConnection);
    dataCenter->getVerifyCodeAsync(currentPhone);

    // 3.开启定时器, 开始倒计时
    timer->start(1000);
    this->sendVerifyCodeBtn->setEnabled(false);
    this->sendVerifyCodeBtn->setText(QString("%1s").arg(this->leftTime));
}

void PhoneLoginWidget::sendVerifyCodeDone()
{
    Toast::showMessage("验证码已经发送", false);
}











