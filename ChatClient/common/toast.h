#ifndef TOAST_H
#define TOAST_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QPainter>
#include <QScreen>
#include <QApplication>
#include <QFontMetrics>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QListWidget>
#include <QSystemTrayIcon>

#include "../sessionfriendarea.h"
////////////////////////////////////////////////////////////////
/// 一个弹窗工厂
////////////////////////////////////////////////////////////////

class Toast : public QDialog
{
    Q_OBJECT
public:
    // 此处不需要指定父窗口，全局通知的父窗口就是 桌面
    Toast(const QString& text, bool play);

    // 显示交互信息: 并不需要手动 new 对象，通过showMessage来弹出窗口
    static void showMessage(const QString& text, bool play = false);

    ////////////////////////////////////////////////////////////////
    /// 下面的静态函数，只是通过Toast访问，但是实际没有与Toast的构造方法交互
    ///     1. 删除好友确认弹窗
    ///     2. 退出程序确认弹窗
    ////////////////////////////////////////////////////////////////

    // 删除好友确认弹窗
    static bool deleteFriendBox(const QString& userNickname);

    // 退出程序确认弹窗
    static bool quitApplicationBox();

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
private:
    QLabel *m_label;

    QMediaPlayer* player;
    QAudioOutput* audioOutput;
};


#endif // TOAST_H


// #ifndef TOAST_H
// #define TOAST_H
//
// #include <QDialog>
// #include <QWidget>
//
// class Toast : public QDialog
// {
//     Q_OBJECT
// public:
//     // 此处不需要指定父窗口，全局通知的父窗口就是 桌面
//     Toast(const QString& text);
//
//     // 并不需要手动new 对象，通过showMessage来弹出窗口
//     static void showMessage(const QString& text);
//
// protected:
//     void paintEvent(QPaintEvent *event) override;
// };
//
// #endif // TOAST_H
