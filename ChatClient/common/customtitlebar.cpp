#include "customtitlebar.h"

#include <QColor>
#include <QPushButton>
#include <QRgb>
CustomTitleBar::CustomTitleBar(QWidget *parent)
    : QWidget{parent}
{
    // 设置基本属性
    this->setFixedHeight(40);   //固定高度
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    this->setWindowFlag(Qt::FramelessWindowHint);
    // this->setStyleSheet("QWidget { border-top-left-radius: 10px; border-top-right-radius: 10px; } ");

    // 设置布局
    hlayout = new QHBoxLayout(this);
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0,0,0,0);
    this->setLayout(hlayout);

    // 设置标题
    titleLabel = new QLabel();
    titleLabel->setText("自定义标题");
    titleLabel->setFixedHeight(40);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("customTitle");
    titleLabel->setStyleSheet("#customTitle {background-color: transparent;}");
    QFont font;
    // font.setFamily("Microsoft YaHei");
    font.setPointSize(12);
    font.setWeight(QFont::Normal);
    titleLabel->setFont(font);
}

CustomTitleBar *CustomTitleBar::makeAddFriendPageTitleBar(QWidget *parent)
{
    CustomTitleBar* bar = new CustomTitleBar(parent);
    bar->setTitleText("添加朋友");
    bar->setBackgroundColor(QColor(255, 255, 255));
    bar->setFontColor(QColor(22, 22, 22));

    bar->addEmptyButton();
    bar->hlayout->addWidget(bar->titleLabel, 1, Qt::AlignHCenter);
    bar->addCloseButton();

    bar->updateUI();
    return bar;
}

CustomTitleBar *CustomTitleBar::makeHistoryMessagePageTitleBar(QWidget *parent, const QString &nickname)
{
    CustomTitleBar* bar = new CustomTitleBar(parent);
    bar->setTitleText(QString("与\"%1\"的聊天记录").arg(nickname));
    bar->setBackgroundColor(QColor(255, 255, 255));
    bar->setFontColor(QColor(22, 22, 22));

    bar->addEmptyButton();
    bar->addEmptyButton();
    bar->hlayout->addWidget(bar->titleLabel, 1, Qt::AlignHCenter);
    bar->addMinimumButton();
    bar->addCloseButton();

    bar->updateUI();
    return bar;
}


void CustomTitleBar::setBackgroundColor(const QColor &bgColor)
{
    QString style = QString("QWidget {background-color: rgb(%1, %2, %3);}")
                        .arg(bgColor.red())
                        .arg(bgColor.green())
                        .arg(bgColor.blue());
    this->setStyleSheet(style);
}

void CustomTitleBar::setFontColor(const QColor &fontColor)
{
    QString style = QString("QLabel {color: rgb(%1, %2, %3);}")
                        .arg(fontColor.red())
                        .arg(fontColor.green())
                        .arg(fontColor.blue());
    this->titleLabel->setStyleSheet(style);
}

void CustomTitleBar::setTitleText(const QString &titleText)
{
    titleLabel->setText(titleText);
}

void CustomTitleBar::addEmptyButton()
{
    QPushButton* emptyButton = new ClickThroughButton(qobject_cast<QWidget*>(this->parent()));
    emptyButton->setFixedSize(40,40);
    emptyButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    emptyButton->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                border: none;
            })");

    hlayout->addWidget(emptyButton, 0, Qt::AlignRight);
}

void CustomTitleBar::addMinimumButton()
{
    QPushButton* minimumButton = new QPushButton("-");
    minimumButton->setFixedSize(40,40);
    minimumButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    minimumButton->setStyleSheet(R"(
            QPushButton {
                color: rgb(76, 76, 76);
                background-color: transparent;
                border: none;
                font-size: 30px;
                font-weight: 300;
            }
            QPushButton:hover {
                color: rgb(64, 64, 64);
                background-color: rgb(225, 225, 225);
            })");

    connect(minimumButton, &QPushButton::clicked,
            qobject_cast<QWidget*>(this->parent()), &QWidget::hide);
    hlayout->addWidget(minimumButton, 0, Qt::AlignRight);
}

void CustomTitleBar::addCloseButton()
{
    QPushButton* closeButton = new QPushButton("×");
    closeButton->setFixedSize(40,40);
    closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    closeButton->setStyleSheet(R"(
            QPushButton {
                color: rgb(76, 76, 76);
                background-color: transparent;
                border: none;
                font-size: 30px;
                font-weight: 300;
            }
            QPushButton:hover {
                color: rgb(255, 255, 255);
                background-color: rgb(237, 76, 76); /*  hover时变红 */
            })");

    connect(closeButton, &QPushButton::clicked,
            qobject_cast<QWidget*>(this->parent()), &QWidget::close);
    hlayout->addWidget(closeButton, 0, Qt::AlignRight);
}

void CustomTitleBar::updateUI()
{
    // this->repaint();
    this->update();
}
