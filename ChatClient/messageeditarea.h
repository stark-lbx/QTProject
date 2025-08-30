#ifndef MESSAGEEDITAREA_H
#define MESSAGEEDITAREA_H

#include <QPushButton>
#include <QWidget>
#include <QPlainTextEdit>
#include <QLabel>
#include <QTimer>

#include "model/data.h"

class MessageEditArea : public QWidget
{
    Q_OBJECT
public:
    explicit MessageEditArea(QWidget *parent = nullptr);

    void clear();
private:
    QPushButton* sendImageBtn;
    QPushButton* sendFileBtn;
    QPushButton* sendSpeechBtn;
    QPushButton* showHistoryBtn;

    QPlainTextEdit* textEdit;
    QPushButton* sendTextBtn;

    QTimer* timer;
    const int MAX_RECORD_TIME = 60;
    int leftTime = MAX_RECORD_TIME;
    void stopTimer(){
        timer->stop();
        leftTime = MAX_RECORD_TIME;    //重置倒计时、重置文本
        tipLabel->setText("正在录音中, 松开发送");
    }
    bool recordIsAutoStopped = false;
    QLabel* tipLabel;

    void initSignalSlot();
    void sendTextMessage();
    void sendImageMessage();
    void sendFileMessage();

    void recordSpeechMessage();
    void sendSpeechMessage();

    void addMyselfMessage(model::MessageType messageType, const QByteArray& content, const QString& extraInfo);
    void addOthersMessage(const model::Message& message);
};

#endif // MESSAGEEDITAREA_H
