#ifndef MESSAGESHOWAREA_H
#define MESSAGESHOWAREA_H

#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "model/data.h"

// .h 文件中不宜进行 using namespace xxxx
using model::Message;

//////////////////////////////////////////////
/// 表示消息展示区
//////////////////////////////////////////////

class MessageShowArea : public QScrollArea
{
    Q_OBJECT
public:
    MessageShowArea();

    //尾插
    void addMessage(bool isLeft, const Message& message);

    //头插
    void addFrontMessage(bool isLeft, const Message& message);

    //清空消息
    void clear();

    // 滚动到末尾
    void scrollToEnd();

private:
    QWidget* container;
    QWidget* spacerWidget; // 占位控件
};


//////////////////////////////////////////////
/// 表示一条消息
/// 这个里面要同时支持 文本消息、图片消息、文件消息、语音消息
/// 当前只考虑文本消息，后续添加其它
//////////////////////////////////////////////

class MessageItem : public QWidget
{
    Q_OBJECT
public:
    // 消息是否是左侧消息
    MessageItem(bool isLeft);

    // 通过工厂方法 创建MessageItem实例
    static MessageItem* makeMessageItem(bool isLeft, const Message& message);

    // 添加工厂函数
    static QWidget* makeTextMessageItem(bool isLeft, const QString& text);
    static QWidget* makeImageMessageItem(bool isLeft, const QString& fileId, const QByteArray& content);
    static QWidget* makeFileMessageItem(bool isLeft, const model::Message& message);
    static QWidget* makeSpeechMessageItem(bool isLeft, const model::Message& message);

private:
    bool isLeft;
};

//////////////////////////////////////////////
/// 创建类表示 "文本消息" 正文部分
//////////////////////////////////////////////

class MessageContentLabel : public QWidget{
    Q_OBJECT
public:
    MessageContentLabel(const QString& text, bool isLeft);;

    QLabel* getLabel(){return label;}
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QLabel* label;
    bool isLeft;
};

//////////////////////////////////////////////
/// 创建类表示 "图片消息" 正文部分
//////////////////////////////////////////////

class MessageImageLabel : public QWidget {
    Q_OBJECT
public:
    MessageImageLabel(const QString& fileId, const QByteArray& content, bool isLeft);

    void updateUI(const QString& fileId, const QByteArray& content);
protected:
    void paintEvent(QPaintEvent *event)override;
private:
    QPushButton* imageBtn;

    QString fileId;     //图片的文件fileId
    QByteArray content; //图片的二进制数据
    bool isLeft;        //是否是左侧消息
};

//////////////////////////////////////////////
/// 创建类表示 "文件消息" 正文部分
//////////////////////////////////////////////

class MessageFileLabel : public QWidget {
    Q_OBJECT
public:
    MessageFileLabel(const QString& fileId, const QByteArray& content, const QString& fileName, bool isLeft);
    void updateUI(const QString& fileId, const QByteArray& fileContent);

    void saveAsFile();
    void openFile();
protected:
    void mousePressEvent(QMouseEvent* event)override;
private:
    QLabel* fileName;               //文件的名字
    QLabel* fileSize;               //文件的大小
    QPushButton* fileIcon;          //文件的图标

    QString fileId;                 //文件的fileId
    QString filePath = "";          //文件保存到的位置
    QByteArray content;             //文件的二进制内容

    bool isLeft;                    //是否是左侧消息
    bool loadContentDone = false;   //数据是否加载完毕
};


//////////////////////////////////////////////
/// 创建类表示 "语音消息" 正文部分
///     语音消息处理时
///     1) 录制语音
///     2) 播放语音
///     3) 网络传输语音消息
///     4) 语音转文字（第三方sdk: 百度语音）
//////////////////////////////////////////////

class MessageSpeechLabel : public QWidget{
    Q_OBJECT
public:
    // onlySpeechLabel代表的是，转文字不可用，如果为true时，只需要将contextMenuEvent给过滤掉即可
    MessageSpeechLabel(const QString& fileId, const QByteArray& content, bool isLeft, bool onlySpeechLabel = false);

    void adjustLabelSize();
    void updateUI(const QString& fileId, const QByteArray& content);

    void play();
    void stop();
    void playDone();

    void speechConvertText();
    void speechConvertTextDone(const QString& fileId, const QString& text);
protected:
    void paintEvent(QPaintEvent* event)override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mousePressEvent(QMouseEvent* event)override;
    void mouseReleaseEvent(QMouseEvent* event)override;
private:           
    QLabel* showLabel;                  //语音界面
    QLabel* textLabel;                  //语音转成的文本、文本部分

    QString fileId;                 //文件的fileId
    QByteArray content;             //文件的二进制内容
    bool isLeft;
    bool loadContentDone = false;   //是否加载了语音内容
    bool isConvertText = false;     //语音是否已经转成文字
    bool isPlaying = false;         //语音是否正在播放

    int m_duration = 1;   //语音时长
    QColor rightColor = QColor(149, 236, 105);          //浅绿
    QColor pressedColor_right = QColor(134, 212, 94);   //深绿
    QColor leftColor = QColor(255, 255, 255);           //白色
    QColor pressedColor_left = QColor(229, 229, 229);   //灰色
    bool mouse_press = false;       //鼠标是否按下
    bool onlySpeechLabel;
};

//////////////////////////////////////////////
/// 链接消息类型:（也是Clickable类型的消息）
///     即该消息点击后跳转到一定的页面
///         页面可能是外部网页
///         页面可能是内置页面
/// 该类型可以根据内外页面分为两类拓展:
///     外部界面: 超链接、
///     内部界面: 聊天记录转发、红包消息 等
//////////////////////////////////////////////



#endif // MESSAGESHOWAREA_H
