#include "loginwidget.h"

#include "mainwidget.h"
#include "phoneloginwidget.h"
#include "common/toast.h"
#include "model/datacenter.h"

#include <QGridLayout>

LoginWidget::LoginWidget(QWidget *parent ,bool isLoginMode)
    : QWidget{parent}
    ,isLoginMode(isLoginMode)
{
    // 1. 设置本窗口的基本属性
    this->setFixedSize(350, 475);
    this->setWindowTitle("墨鸿");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setStyleSheet("QWidget {background-color: rgb(238, 245, 249);}");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 2. 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    // layout->setSpacing(0);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(20);
    layout->setContentsMargins(50,50,50,50);
    layout->setAlignment(Qt::AlignTop);
    this->setLayout(layout);

    // 3. 创建标题
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
    // 4. 创建用户名输入框
    usernameEdit = new QLineEdit();
    usernameEdit->setFixedHeight(40);
    usernameEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    usernameEdit->setPlaceholderText("输入用户名");
    usernameEdit->setMaxLength(18);
    usernameEdit->setStyleSheet(editStyle);

    // 5. 创建密码输入框
    passwordEdit = new QLineEdit();
    passwordEdit->setFixedHeight(40);
    passwordEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    passwordEdit->setPlaceholderText("输入密码");
    passwordEdit->setMaxLength(16);
    passwordEdit->setStyleSheet(editStyle);
    passwordEdit->setEchoMode(QLineEdit::Password);

    // 6. 创建验证码输入框
    verifyCodeEdit = new QLineEdit();
    verifyCodeEdit->setFixedHeight(40);
    verifyCodeEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    verifyCodeEdit->setPlaceholderText("输入验证码");
    verifyCodeEdit->setStyleSheet(editStyle);

    // 7. 创建显示验证码图片的控件
    verifyCodeWidget = new VerifyCodeWidget(this);
    verifyCodeWidget->setFixedHeight(40);
    verifyCodeWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    QString submitStyle =R"(
        QPushButton {
            border: none;
            border-radius: 10px;
            background-color: rgb(7, 193, 96);
            color: rgb(255, 255, 255);
            font-size: 20px;
        }
        QPushButton:hover {
            background-color: rgb(6, 183, 91);
        }
        QPushButton:pressed {
            background-color: rgb(6, 174, 86);
        }
        QPushButton:disabled {
            color: rgb(157, 157, 157);
            background-color: rgb(225, 225, 225);
        }
    )";
    // 8. 创建登录按钮
    submitBtn = new QPushButton();
    submitBtn->setText("登录");
    submitBtn->setFixedHeight(40);
    submitBtn->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    submitBtn->setStyleSheet(submitStyle);
    submitBtn->setEnabled(false);   //初始状态不可点击


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
    // 9. 创建切换到手机号登录按钮
    phoneModeBtn = new QPushButton();
    phoneModeBtn->setFixedSize(100, 40);
    phoneModeBtn->setText("手机号登陆");
    phoneModeBtn->setStyleSheet(whiteBtnStyle);

    // 10. 创建切换到注册模式的按钮
    switchModeBtn = new QPushButton();
    switchModeBtn->setFixedSize(100, 40);
    switchModeBtn->setText("注册");
    switchModeBtn->setStyleSheet(whiteBtnStyle);

    // 11. 添加到布局管理器
    layout->addWidget(titleLabel, 0, 2);
    layout->addWidget(usernameEdit, 1, 0, 1, 5);
    layout->addWidget(passwordEdit, 2, 0, 1, 5);
    layout->addWidget(verifyCodeEdit, 3, 0, 1, 4);

    layout->addWidget(verifyCodeWidget, 3, 4);

    layout->addWidget(submitBtn, 4, 0, 1, 5);
    layout->addWidget(phoneModeBtn, 5, 0);
    layout->addWidget(switchModeBtn, 5, 4);



    // 12. 处理信号槽
    connect(switchModeBtn, &QPushButton::clicked, this, &LoginWidget::switchMode);
    connect(phoneModeBtn, &QPushButton::clicked, this, &LoginWidget::switchLoginMode);
    connect(submitBtn, &QPushButton::clicked, this, &LoginWidget::submit);

    connect(usernameEdit, &QLineEdit::textEdited, this, &LoginWidget::checkSubmitEnable);
    connect(passwordEdit, &QLineEdit::textEdited, this, &LoginWidget::checkSubmitEnable);
    connect(verifyCodeEdit, &QLineEdit::textEdited, this, &LoginWidget::checkSubmitEnable);

    // qDebug() << isLoginMode;
    if(!isLoginMode){
        // 当前模式不是登录模式, 要设置到注册模式
        titleLabel->setText("注册");
        submitBtn->setText("注册");
        phoneModeBtn->setText("手机号注册");
        switchModeBtn->setText("登录");
    }
}

void LoginWidget::switchMode()
{
    // LOG() << "cur:" <<isLoginMode<<", switch to:" << !isLoginMode;
    if(isLoginMode) {
        // 如果当前是登录模式, 那么就切换到注册模式
        this->toRegisterMode();
    } else {
        // 如果当前是注册模式，那么就切回到登陆模式
        this->toLoginMode();
    }
}

void LoginWidget::switchLoginMode()
{
    // Toast::showMessage("该功能尚未开发, 请耐心等待"); return;

    PhoneLoginWidget* phoneLoginWidget = new PhoneLoginWidget(nullptr, this->isLoginMode);
    phoneLoginWidget->show();

    // 关闭当前窗口
    this->close();
}

void LoginWidget::submit()
{
    const QString& username = usernameEdit->text();
    const QString& password = passwordEdit->text();
    const QString& verifycode = verifyCodeEdit->text();
    if(!verifyCodeWidget->checkVerifyCode(verifycode)){
        verifyCodeEdit->clear();
        verifyCodeWidget->refreshVerifyCode();  //重新刷新
        Toast::showMessage("验证码错误", false);
        return;
    }
    // 当三者有任意一个没有输入时，按钮不可点击, 无须判断账号密码是否都输入了

    using model::DataCenter;
    DataCenter* dataCenter = DataCenter::getInstance();
    if(isLoginMode){
        // 提交到服务器查询是否存在这个用户, [并生成唯一的 loginSessionId 保存到数据中心DataCenter中(这个操作不在这写)]
        connect(dataCenter, &DataCenter::userLoginDone, this, &LoginWidget::userLoginDone);
        dataCenter->userLoginAsync(username, password);
    } else {
        // 注册, 向服务器提交注册申请, 如果已经存在该用户, 则注册失败
        connect(dataCenter, &DataCenter::userRegisterDone, this, &LoginWidget::userRegisterDone);
        dataCenter->userRegisterAsync(username, password);
    }

}

void LoginWidget::userLoginDone(bool ok, const QString &reason)
{
    if(!ok){
        Toast::showMessage(QString("登陆失败(%1)").arg(reason), false);
    } else {
        MainWidget::getInstance()->launchLater();
        // connect(model::DataCenter::getInstance(), &model::DataCenter::loginDone, this, [=](){this->close();});
        this->close();
    }
}

void LoginWidget::userRegisterDone(bool ok, const QString &reason)
{
    if(!ok){
        Toast::showMessage(QString("注册失败(%1)").arg(reason), false);
    } else {
        this->toLoginMode();                     //切换到登录界面
        Toast::showMessage("注册成功", false);
    }
    verifyCodeWidget->refreshVerifyCode();  //刷新验证码
    verifyCodeEdit->clear();                //清空输入的验证码，等待重新输入
}

void LoginWidget::checkSubmitEnable(const QString&)
{
    if( usernameEdit->text().trimmed().isEmpty() ||
        passwordEdit->text().trimmed().isEmpty() ||
        verifyCodeEdit->text().trimmed().isEmpty() ){
        // 如果三者任一输入为空、则无法点击
        submitBtn->setEnabled(false);
    } else {
        // 否则按钮设置为可以点击
        submitBtn->setEnabled(true);
    }
}

void LoginWidget::toLoginMode()
{
    titleLabel->setText("登录");
    submitBtn->setText("登录");
    phoneModeBtn->setText("手机号登录");
    switchModeBtn->setText("注册");

    isLoginMode = true;
}

void LoginWidget::toRegisterMode()
{
    titleLabel->setText("注册");
    submitBtn->setText("注册");
    phoneModeBtn->setText("手机号注册");
    switchModeBtn->setText("登录");

    isLoginMode = false;
}
















