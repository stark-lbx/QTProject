#ifndef DBCONNECTIONPOOL_H
#define DBCONNECTIONPOOL_H

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QSqlDatabase>
#include <QTimer>


/*
    连接池类设计：管理连接、分配、回收
    连接的获取接口：openConnection();
    连接的释放接口：releaseConnection();
    连接状态的监管：监控连接有效性、处理超时和异常
    配置参数：最大连接数、最小连接数、超时时间等
*/

class DBConnectionPool : public QObject
{
    Q_OBJECT
public:
    //获取连接池实例
    static DBConnectionPool& getInstance();

    //获取一个连接
    QSqlDatabase openConnection();
    //释放一个连接-不会真正释放，只是归还到了池中
    void releaseConnection(const QString& connectionName);

    //销毁连接池
    void destroyPool();

    //设置数据库连接参数√
    void setDatabaseParam(const QString &hostName, const QString &databaseName,
                          const QString &userName, const QString &password,
                          int port = 3306);

    //设置连接池的参数√
    void setPoolParam(int minSize, int maxSize, int timeout=30000);


    //设置心跳检测参数
    void setHeartbeatParam(int interval, const QString &sql="select 1");
private:
    explicit DBConnectionPool(QObject *parent = nullptr);
    ~DBConnectionPool();//不由编译器自动释放，让用户手动关闭连接池

    DBConnectionPool(const DBConnectionPool&)=delete;
    DBConnectionPool& operator=(const DBConnectionPool&)=delete;

    //创建一个新的数据库连接
    QSqlDatabase createConnection(const QString &connectionName);
    //清理无效连接
    void cleanup();

    //检查连接是否有效
    bool isConnectionValid(const QSqlDatabase &db);


    //心跳检测函数
    void heartbeatCheck();
private:
    //数据库连接参数
    QString m_hostName;
    QString m_databaseName;
    QString m_userName;
    QString m_password;
    int m_port;

    //连接池配置参数
    int m_maxSize;
    int m_minSize;
    int m_timeout;

    //可用的连接队列和已经使用的连接队列
    QQueue<QString> m_availableConnections;
    QQueue<QString> m_usedConnections;

    //连接时间记录
    QHash<QString, QDateTime> m_connectionTime;

    //线程锁
    QMutex m_mutex;

    //清理定时器
    QTimer *m_cleanupTimer;

    //心跳检测参数
    int m_heartbeatInterval;//心跳间隔（毫秒）
    QString m_heartbeatSql;//心跳检测的SQL
    //心跳检测定时器
    QTimer *m_heartbeatTimer;
signals:
};

#endif // DBCONNECTIONPOOL_H
