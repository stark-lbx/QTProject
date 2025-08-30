#ifndef VERIFYCODEWIDGET_H
#define VERIFYCODEWIDGET_H

#include <QRandomGenerator>
#include <QWidget>

class VerifyCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VerifyCodeWidget(QWidget *parent = nullptr);

    // 通过这个函数, 生成随机的验证码字符串
    QString generateVerifyCode();
    // 把当前验证码显示到界面上
    void refreshVerifyCode();

    // 检验验证码是否匹配
    bool checkVerifyCode(const QString& verifyCode);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
private:
    // 随机数生成器
    QRandomGenerator randomGenerator;
    // 保存验证码的值
    QString verifyCode = "";
signals:
};

#endif // VERIFYCODEWIDGET_H
