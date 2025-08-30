#ifndef SESSIONFRIENDAREA_H
#define SESSIONFRIENDAREA_H

#include <QScrollArea>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "model/data.h"

class QLabel;

enum ItemType{
    SessionItemType,
    FriendItemType,
    ApplyItemType
};

/////////////////////////////////////////////////
/// 整个滚动区域的实现
/////////////////////////////////////////////////

class SessionFriendArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit SessionFriendArea(QWidget *parent = nullptr);

    // 清空该区域中所有的 Item
    void clear();

    // 添加一个 item 到该区域中
    void addItem(ItemType itemType, const QString& id, const QIcon& avatar, const QString& name, const QString& text);

    // 选中某个 item
    void clickItem(int index);


    // QVBoxLayout* getContainerLayout(){
    //     return dynamic_cast<QVBoxLayout*>(container->layout());
    // }
private:
    // 后续往 container 内部的 layout 中添加元素， 就能触发 QScrollArea 的滚动效果
    QWidget* container;
signals:
};

/////////////////////////////////////////////////
/// 滚动区域中的 Item 的实现
/////////////////////////////////////////////////

class SessionFriendItem : public QWidget
{
    Q_OBJECT

public:
    // 此处的 owner 就是对应着 SessionFriendArea 对象的指针
    // 如果直接把 SessionFriendItem 添加到 SessionFriendArea 里的布局管理器中
    // 此时可以直接使用 SessionFriendItem 的parent 相关操作，就能找到 SessionFriendArea
    // 此时就不需要额外记录 "owner" 的，实际上是需要把sessionfriendItem添加到Area成员中有一个container成员的layout里
    SessionFriendItem(QWidget *owner, const QIcon& avatar, const QString& name, const QString& text);

    // 为了使得item能够通过指定父类的QSS样式设置本类的样式
    // Qt实现QSS的时候，底层本质都是通过画笔QPainter这一系列操作，画上去的
    // 对于Qt自带的类，自身已经在paintEvent中处理好了相关逻辑
    // 对于自定义的子类，就需要手动完成上述的“绘制”过程
    void paintEvent(QPaintEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void enterEvent(QEnterEvent *event);
    void leaveEvent(QEvent *event);

    void select();  //实现被点击后的动画逻辑
    virtual void active();  //实现被点击后的业务逻辑

    // bool isSelected(){return selected;}
    // QString getName(){return nameLabel->text();}
private:
    QWidget* owner;

    // 当前Item是否处于选中状态
    bool selected;
protected:
    QLabel* messageLabel;
    QLabel* nameLabel;
};

/////////////////////////////////////////////////
/// 会话 Item 的实现
/////////////////////////////////////////////////

class SessionItem : public SessionFriendItem
{
    Q_OBJECT
public:
    SessionItem(QWidget* owner, const QString& chatSessionId,const QIcon& avatar, const QString& name, const QString& lastMessage);

    void active() override;

    void updateLastMessage(const QString& chatSessionId);
private:
    QString chatSessionId;    //当前会话 id

    QString showText;   //最后一条消息的文本预览
};


/////////////////////////////////////////////////
/// 好友 Item 的实现
/////////////////////////////////////////////////

class FriendItem : public SessionFriendItem
{
    Q_OBJECT
public:
    FriendItem(QWidget* owner, const QString& userId,const QIcon& avatar, const QString& name, const QString& description);

    void active() override;
private:
    QString userId;    //好友的 用户id
    model::UserInfo info;
};

class FriendDetail : public QWidget {
    Q_OBJECT
public:
    FriendDetail(const model::UserInfo& userInfo);
};

/////////////////////////////////////////////////
/// 好友申请 Item 的实现
/////////////////////////////////////////////////

class ApplyItem : public SessionFriendItem
{
    Q_OBJECT
public:
    ApplyItem(QWidget* owner, const QString& userId,const QIcon& avatar, const QString& name);

    void active() override;

    void acceptFriendApply();
    void rejectFriendApply();
private:
    QString userId;    //申请人的 userId
};


#endif // SESSIONFRIENDAREA_H
