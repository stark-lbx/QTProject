#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "verifycodewidget.h"
class LoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr, bool isLoginMode = true);

private:
    bool isLoginMode;

    QLabel* titleLabel;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLineEdit* verifyCodeEdit;
    VerifyCodeWidget* verifyCodeWidget;

    QPushButton* submitBtn;     //登录or注册 按钮
    QPushButton* phoneModeBtn;
    QPushButton* switchModeBtn; //切换为注册模式

    void checkSubmitEnable(const QString&);
    void toLoginMode();
    void toRegisterMode();
    void switchMode();
    void switchLoginMode();

    void submit();
    void userLoginDone(bool ok, const QString& reason);
    void userRegisterDone(bool ok, const QString& reason);
};

#endif // LOGINWIDGET_H
