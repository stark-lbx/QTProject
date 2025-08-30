#ifndef PHONELOGINWIDGET_H
#define PHONELOGINWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class PhoneLoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PhoneLoginWidget(QWidget *parent = nullptr, bool isLoginMode = true);

private:
    bool isLoginMode;
    int leftTime = 30;
    QTimer* timer;
    QString currentPhone = "";    // 记录是使用哪个手机号发送的验证码

    QLabel* titleLabel;
    QLineEdit* phoneEdit;
    QLineEdit* verifyCodeEdit;
    QPushButton* sendVerifyCodeBtn;

    QPushButton* submitBtn;
    QPushButton* switchModeBtn;
    QPushButton* userModeBtn;

    void countOut();
    void switchMode();
    void switchLoginMode();

    void sendVerifyCode();
    void sendVerifyCodeDone();

    void checkSbumitEnable(const QString&);
    void clickSubmitBtn();
    void phoneLoginDone(bool ok, const QString& reason);
    void phoneRegisterDone(bool ok, const QString& reason);

    bool isPhoneNumberValid(const QString& phone);
};

#endif // PHONELOGINWIDGET_H
