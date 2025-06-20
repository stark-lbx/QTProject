#ifndef INDEX_H
#define INDEX_H

#include "file.h"
#include "friend.h"

#include <QWidget>

namespace Ui {
class Index;
}

class Index : public QWidget
{
    Q_OBJECT

public:
    // QStringList friendList;
    static Index& getInstance();
    ~Index();
    Friend* getFriend();
    File* getFile();
private slots:
    void on_friend_PB_clicked();
    void on_file_PB_clicked();

private:
    Ui::Index *ui;
    explicit Index(QWidget *parent = nullptr);

    Index(const Index& o)=delete;
    Index& operator=(const Index& o)=delete;

    Index(Index&& o)=delete;
    Index& operator=(Index&& o)=delete;
};

#endif // INDEX_H
