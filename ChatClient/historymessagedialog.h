#ifndef HISTORYMESSAGEDIALOG_H
#define HISTORYMESSAGEDIALOG_H

#include <QDateTimeEdit>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>

#include "model/data.h"
#include "common/customtitlebar.h"
using model::Message;


/////////////////////////////////////////////////////////
/// 历史消息记录的某一项
/////////////////////////////////////////////////////////

class HistoryItem : public QWidget{
    Q_OBJECT
public:
    HistoryItem() {}

    static HistoryItem* makeHistoryItem(const Message& message);
protected:
    void paintEvent(QPaintEvent* event)override;
};



/////////////////////////////////////////////////////////
/// 历史记录 界面类
/////////////////////////////////////////////////////////
#include "common/roundeddialog.h"
class HistoryMessageWidget : public RoundedDialog
{
    Q_OBJECT
public:
    HistoryMessageWidget(const QString& chatSessionId, QWidget* parent=nullptr);

    // 在窗口中添加一个历史消息
    void addHistoryMessage(const Message& messgae);
    void initScrollArea(QVBoxLayout* layout);
    // 清空窗口内的所有历史消息
    void clear();

    void clickSearchBtn();
    void clickSearchBtnDone(const QString& chatSessionId);

protected:
    bool mouse_press = false;
    QPoint mousePoint;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // 持有所有历史消息结果的容器对象
    QWidget* container;

    QLineEdit* searchEdit;
    QRadioButton* keyRadioBtn;
    QRadioButton* timeRadioBtn;
    QDateTimeEdit* begTimeEdit;
    QDateTimeEdit* endTimeEdit;

    CustomTitleBar* titleBar;
    const QString currentChatSessionId;
};


/////////////////////////////////////////////////////////
/// 图片
/////////////////////////////////////////////////////////

// class ImageButton : public QPushButton {
// public:
//     ImageButton(const QString& fileId, const QByteArray& content);
//     void updateUI(const QString& fileId, const QByteArray& content);

// private:
//     QString fileId;
// };


/////////////////////////////////////////////////////////
/// 文件
/////////////////////////////////////////////////////////

// class FileLabel : public QLabel {
// public:
//     FileLabel(const QString& fileId, const QString& fileName);

//     void getContentDone(const QString& fileId, const QByteArray& fileContent);

//     // 通过这个函数, 来处理鼠标点击操作.
//     void mousePressEvent(QMouseEvent* event) override;

// private:
//     QString fileId;
//     QByteArray content;
//     QString fileName;
//     bool loadDone = false;
// };


/////////////////////////////////////////////////////////
/// 语音
/////////////////////////////////////////////////////////

// class SpeechLabel : public QLabel {
// public:
//     SpeechLabel(const QString& fileId);

//     void getContentDone(const QString& fileId, const QByteArray& content);

//     // 通过这个函数处理鼠标点击
//     void mousePressEvent(QMouseEvent* event) override;

// private:
//     QString fileId;
//     QByteArray content;
//     bool loadDone = false;
// };



#endif // HISTORYMESSAGEDIALOG_H
