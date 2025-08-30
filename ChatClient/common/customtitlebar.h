#ifndef CUSTOMTITLEBAR_H
#define CUSTOMTITLEBAR_H

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>


///////////////////////////////////////////////////////
/// 点击事件穿透按钮: 主要用于对齐标题使用的
///////////////////////////////////////////////////////
class ClickThroughButton : public QPushButton {
    Q_OBJECT
public:
    explicit ClickThroughButton(QWidget *parent) : QPushButton(parent) {}

protected:
    // 重写鼠标按下事件：忽略事件，让事件传递给父控件
    void mousePressEvent(QMouseEvent *event) override {
        event->ignore(); // 不处理事件，允许事件向上传递
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        event->ignore();
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        event->ignore();
    }
};



///////////////////////////////////////////////////////
/// 自定义标题栏
///////////////////////////////////////////////////////
class CustomTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit CustomTitleBar(QWidget *parent);

    static CustomTitleBar* makeAddFriendPageTitleBar(QWidget* parent);
    static CustomTitleBar* makeHistoryMessagePageTitleBar(QWidget* parent, const QString& nickname);
protected:
    void setBackgroundColor(const QColor& bgColor);
    void setFontColor(const QColor& fontColor);
    void setTitleText(const QString& titleText);

    void addEmptyButton();
    void addMinimumButton();
    void addCloseButton();

    void updateUI();
private:
    QHBoxLayout* hlayout;

    QLabel* titleLabel;
signals:
};

#endif // CUSTOMTITLEBAR_H
