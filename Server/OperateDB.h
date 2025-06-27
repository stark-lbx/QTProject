#ifndef OPERATEDB_H
#define OPERATEDB_H

#include <QObject>
#include <QSqlDatabase>

class OperateDB : public QObject
{
    Q_OBJECT
public:
    ~OperateDB();
    static OperateDB& getInstance();
    // void Connect();

    int handleRegist(const char* uid,const char* pwd);//注册
    int handleLogin(const char* uid, const char* pwd);//登录
    int handleFindUser(const char* uid);//查找用户
    int handleAddFriend(const char* curName,const char* tarName);//添加好友
    bool handleAgreeAdd(const char* curName,const char* tarName);//同意添加
    void handleOffline(const char* uid);//下线
    QStringList handleGetFriendList(const char *uid);//获取好友列表

    int handleDelFriend(const char* curName,const char* tarName);//删除好友
private:
    // QSqlDatabase m_db;//数据库对象
    QSqlDatabase getDatabaseConnection();//从连接池获取一个连接

    explicit OperateDB(QObject *parent = nullptr);

    OperateDB(const OperateDB& o)=delete;
    OperateDB& operator=(const OperateDB& o) = delete;

    OperateDB(OperateDB&& o)=delete;
    OperateDB& operator=(OperateDB&& o) = delete;
signals:
};

#endif // OPERATEDB_H
