#ifndef GROUPSESSIONDETAILDIALOG_H
#define GROUPSESSIONDETAILDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>

class AvatarItem;
class GroupSessionDetailWidget : public QDialog
{
    Q_OBJECT
public:
    GroupSessionDetailWidget(QWidget* parent=nullptr);

    void initData();
    void initMembers(const QString& chatSessionId);

    void addMember(AvatarItem* avatarItem);
private:
    QGridLayout* glayout;
    QLabel* groupNameLabel;

    // 表示当前要添加的 AvatarItem 处在的行和列
    // 由于整个界面上存在 + 这个按钮, 占据了 (0, 0) 位置. 接下来添加 AvatarItem 就要从
    // (0, 1) 位置添加了
    int curRow = 0;
    int curCol = 1;
};

#endif // GROUPSESSIONDETAILDIALOG_H
