#ifndef SELFINFODIALOG_H
#define SELFINFODIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class SelfInfoWidget : public QDialog
{
    Q_OBJECT
public:
    SelfInfoWidget(QWidget* parent = nullptr);

    void initSignalSlot();

    void clickNameSubmitBtn();
    void clickNameSubmitBtnDone();

    void clickDescSubmitBtn();
    void clickDescSubmitBtnDone();

    void clickGetVerifyCodeBtn();

    void clickPhoneSubmitBtn();
    void clickPhoneSubmitBtnDone();

    void clickAvatarBtn();
    void clickAvatarBtnDone();
private:
    QGridLayout* layout;

    QPushButton* avatarBtn;         // 用户头像
    QLabel* idTag;                  // 显示 "序号:"
    QLabel* idLabel;                // 显示 "123456.."

    QLabel* nameTag;                // 显示 "昵称:"
    QLabel* nameLabel;              // 显示 "zhangsan"
    QLineEdit* nameEdit;            // 编辑名字
    QPushButton* nameModifyBtn;     // 修改名字
    QPushButton* nameSubmitBtn;     // 提交修改

    QLabel* descTag;                // 显示 "签名:"
    QLabel* descLabel;              // 显示 "....."
    QLineEdit* descEdit;            // 编辑签名
    QPushButton* descModifyBtn;     // 修改签名
    QPushButton* descSubmitBtn;     // 提交修改

    QLabel* phoneTag;                // 显示 "电话:"
    QLabel* phoneLabel;              // 显示 "178 1234 5678"
    QLineEdit* phoneEdit;            // 编辑电话
    QPushButton* phoneModifyBtn;     // 修改电话
    QPushButton* phoneSubmitBtn;     // 提交修改

    QLabel* verifyCodeTag;           // 显示 "验证码:"
    QLineEdit* verifyCodeEdit;       // 输入验证码
    QPushButton* getVerifyCodeBtn;    // 获取验证码的点击按钮

    QString phoneToChange;  //要修改到的手机号码

    int leftTime = 30;

};

#endif // SELFINFODIALOG_H
