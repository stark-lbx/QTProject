#include "dbconnectionpool.h"

#include <QDateTime>
#include <QSqlQuery>
#include <QThread>
#include <QUuid>

DBConnectionPool &DBConnectionPool::getInstance()
{
    static DBConnectionPool instance;
    return instance;
}

QSqlDatabase DBConnectionPool::openConnection()
{
    QMutexLocker locker(&m_mutex);//打开连接前加锁，避免多个地方获取同一个连接

    QString connectionName;
    QDateTime startTime = QDateTime::currentDateTime();//开始连接时间

    //尝试从可用连接中获取
    while(!m_availableConnections.empty()){
        connectionName = m_availableConnections.dequeue();//取出一个可用连接
        QSqlDatabase db = QSqlDatabase::database(connectionName);

        //检查连接是否有效
        if(isConnectionValid(db)){
            m_usedConnections.enqueue(connectionName);
            m_connectionTime[connectionName] = QDateTime::currentDateTime();
            //qDebug() << "Reuse Connection:"<<connectionName;
            return db;
        }else{
            //连接无效
            QSqlDatabase::removeDatabase(connectionName);
            // qDebug() << "Remove invalid Connection:"<<connectionName;
        }
    }

    //还没达到最大连接数,新建一个
    while(m_usedConnections.size() < m_maxSize){
        connectionName = QString("Connnection_%1").arg(QUuid::createUuid().toString());
        QSqlDatabase db = createConnection(connectionName);

        if(db.isOpen()){
            //如果成功打开
            m_usedConnections.enqueue(connectionName);
            m_connectionTime[connectionName] = QDateTime::currentDateTime();
            qDebug() << "Create new conncetion" <<connectionName;
            return db;
        }
    }

    //达到最大连接数了
    qDebug()<<"current database connected have been maxSize";
    qDebug()<<"waiting for available connection ...";

    //等待一段时间
    while(true){
        //等待时将锁释放
        locker.unlock();
        QThread::msleep(500);
        locker.relock();


        //尝试从可用连接中获取
        while(!m_availableConnections.empty()){
            connectionName = m_availableConnections.dequeue();//取出一个可用连接
            QSqlDatabase db = QSqlDatabase::database(connectionName);

            //检查连接是否有效
            if(isConnectionValid(db)){
                m_usedConnections.enqueue(connectionName);
                m_connectionTime[connectionName] = QDateTime::currentDateTime();
                //qDebug() << "Reuse Connection after waiting:"<<connectionName;
                return db;
            }else{
                //连接无效
                QSqlDatabase::removeDatabase(connectionName);
                // qDebug() << "Remove invalid Connection after waiting:"<<connectionName;
            }
        }

        if(startTime.msecsTo(QDateTime::currentDateTime()) > m_timeout){
            qDebug() <<"Get connection timeout!";
            return QSqlDatabase();
        }

    }
}

void DBConnectionPool::releaseConnection(const QString& connectionName)
{
    QMutexLocker locker(&m_mutex);

    if(m_usedConnections.contains(connectionName)){
        m_usedConnections.removeOne(connectionName);
        m_availableConnections.enqueue(connectionName);
    }
}

void DBConnectionPool::destroyPool()
{
    QMutexLocker locker(&m_mutex);

    //关闭所有可用连接
    foreach(QString connectionName, m_availableConnections){
        QSqlDatabase::removeDatabase(connectionName);
    }
    m_availableConnections.clear();

    //关闭所有还没结束的连接
    foreach(QString connectionName, m_usedConnections){
        QSqlDatabase::removeDatabase(connectionName);
    }
    m_usedConnections.clear();

    m_connectionTime.clear();
}
void DBConnectionPool::setDatabaseParam(
    const QString &hostName, const QString &databaseName,
    const QString &userName, const QString &password,
    int port)
{
    m_hostName = hostName;
    m_databaseName = databaseName;
    m_userName = userName;
    m_password = password;
    m_port = port;
}

void DBConnectionPool::setPoolParam(int minSize, int maxSize, int timeout)
{
    m_minSize = minSize;
    m_maxSize = maxSize;
    m_timeout = timeout;
}

void DBConnectionPool::setHeartbeatParam(int interval, const QString &sql)
{
    m_heartbeatInterval = interval;
    m_heartbeatSql = sql;

    m_heartbeatTimer->setInterval(interval);
}

DBConnectionPool::DBConnectionPool(QObject *parent)
    : QObject{parent},m_port(33060),
    m_maxSize(20),m_minSize(5),m_timeout(30000),
    m_heartbeatInterval(6000), m_heartbeatSql("select 1")
{
    m_cleanupTimer = new QTimer(this);//启动清理定时器
    connect(m_cleanupTimer, &QTimer::timeout, this,&DBConnectionPool::cleanup);
    m_cleanupTimer->start(5 *60 *1000);//300000: 5分钟清理一次

    //启动心跳检测
    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer,&QTimer::timeout,this,&DBConnectionPool::heartbeatCheck);
    m_heartbeatTimer->start(m_heartbeatInterval);
}

QSqlDatabase DBConnectionPool::createConnection(const QString &connectionName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL",connectionName);
    db.setHostName(m_hostName);
    db.setDatabaseName(m_databaseName);
    db.setUserName(m_userName);
    db.setPassword(m_password);
    db.setPort(m_port);

    //设置连接选项
    db.setConnectOptions("MYSQL_OPT_CONNECTION_TIMEOUT=5");

    if(!db.open()){
        QSqlDatabase::removeDatabase(connectionName);
        return QSqlDatabase();
    }
    return db;
}

void DBConnectionPool::cleanup()
{
    QMutexLocker locker(&m_mutex);

    QDateTime now = QDateTime::currentDateTime();
    QQueue<QString> tempAvailableConnections;

    //清理可用连接中长时间未使用的连接
    while(!m_availableConnections.empty()){
        QString connectionName = m_availableConnections.dequeue();

        //如果连接超过30分钟未使用，关闭它
        if(m_connectionTime[connectionName].secsTo((now)) > 1800){
            QSqlDatabase::removeDatabase(connectionName);
            m_connectionTime.remove(connectionName);
        }else tempAvailableConnections.enqueue(connectionName);
    }
    m_availableConnections = tempAvailableConnections;

    //确保最小连接
    int connectionToCreate = m_minSize - m_availableConnections.size() - m_usedConnections.size();
    for(int i=0; i < connectionToCreate ; ++i){
        QString connectionName = QString("Connection_%1").arg(QUuid::createUuid().toString());
        QSqlDatabase db = createConnection(connectionName);

        if(db.isOpen()){
            m_availableConnections.enqueue(connectionName);
            m_connectionTime[connectionName] = QDateTime::currentDateTime();
        }
    }
}

bool DBConnectionPool::isConnectionValid(const QSqlDatabase &db)
{
    if(!db.isOpen()){
        return false;
    }

    //执行简单的测试连接
    QSqlQuery query(db);
    return query.exec("select 1");
}

void DBConnectionPool::heartbeatCheck()
{
    QMutexLocker locker(&m_mutex);
    //start checking

    QList<QString> connectionToRecreate;//需要重新建立连接的记录

    foreach(QString connectionName, m_availableConnections){
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if(!isConnectionValid(db)){
            connectionToRecreate.append(connectionName);
        }else continue;
    }

    //重新建立无效连接
    foreach (QString connectionName, connectionToRecreate) {
        //先移除
        m_availableConnections.removeOne(connectionName);
        QSqlDatabase::removeDatabase(connectionName);

        //再建立
        QString newConnectionName = QString("Connection_%1").arg(QUuid::createUuid().toString());
        QSqlDatabase newDB = createConnection(newConnectionName);

        if(newDB.isOpen()){
            m_availableConnections.enqueue(newConnectionName);
            m_connectionTime[newConnectionName] = QDateTime::currentDateTime();
        }

    }
}

DBConnectionPool::~DBConnectionPool()
{
    destroyPool();
}











