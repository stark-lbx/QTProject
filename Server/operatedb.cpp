#include "operatedb.h"


#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
OperateDB &OperateDB::getInstance()
{
    static OperateDB instance;
    return instance;
}

OperateDB::~OperateDB()
{
    m_db.close();
}

OperateDB::OperateDB(QObject *parent)
    : QObject{parent}
{
    // //查看当前可用驱动
    // QStringList drivers = QSqlDatabase::drivers();
    // for(auto driver:drivers){
    //     qDebug()<<driver<<" ";
    // }

    //通过静态函数构造一个m_db,在析构函数中自动关闭
    m_db = QSqlDatabase::addDatabase("QMYSQL");//参数：驱动名
}

void OperateDB::Connect(){

    //配置并连接
    m_db.setHostName("localhost");      //主机名称
    m_db.setPort(3306);                 //端口号
    m_db.setDatabaseName("qtprogram");  //数据库名称
    m_db.setUserName("root");           //用户名
    m_db.setPassword("2396573637");     //密码

    if(m_db.open()){
        qDebug()<<"OperateDB(Connect):\n\t open/connect the database successfully";
    }else{
        //输出报错信息需要头文件QSqlError
        qDebug()<<"OperateDB(Connect):\n\t open/connect the database error: "<<m_db.lastError().text();
    }

}

int OperateDB::handleRegist(const char *uid, const char *pwd)
{
    if(uid==nullptr || pwd==nullptr){
        return 0;
    }
    QString sqlStr;//sql语句
    QSqlQuery q;//执行对象

    sqlStr = QString("select * from user_info where name = '%1';").arg(uid);
    if(!q.exec(sqlStr)){
        return 0;//QString("sql execute error(select)");
    }
    if(q.next()){
        return -1;//QString("name have already existed");
    }

    sqlStr = QString("insert into user_info(name,pwd) values('%1','%2');").arg(uid,pwd);
    if(!q.exec(sqlStr)){
        return 0;//QString("sql execute error(insert)");
    }
    return 1;//QString("register successfully");
}

int OperateDB::handleLogin(const char *uid, const char *pwd)
{
    if(uid==nullptr || pwd==nullptr){
        return 0;//QString("Illegal empty content");
    }
    QString sqlStr;//sql语句
    QSqlQuery q;//执行对象
    sqlStr = QString("select * from user_info where name = '%1' and pwd = '%2';").arg(uid,pwd);
    if(!q.exec(sqlStr)){
        return 0;//QString("sql execute error(select)");
    }
    if(!q.next()){
        return -1;//QString("username or password is error");
    }


    //修改登录在线状态
    sqlStr = QString("update user_info set online = '1' where name = '%1';").arg(uid);
    if(!q.exec(sqlStr)){
        return 0;//QString("sql excute error(update)");
    }
    return 1;//QString("login successfully");
}

int OperateDB::handleFindUser(const char *uid)
{
    if(uid==nullptr) return 0;//"查找失败";

    QString sqlStr = QString("select online from user_info where name='%1'").arg(uid);
    //qDebug() <<"sql: 查询用户是否在线"<<sqlStr;

    QSqlQuery q;
    if(!q.exec(sqlStr)){
        return 0;//"查找失败";
    }
    if(q.next()){
        int ret = q.value(0).toInt();
        if(ret==0)return -1;//"该用户不在线";
        else return 1;//"该用户在线";
    }
    return -2;//"该用户不存在";
}

int OperateDB::handleAddFriend(const char *curName, const char *tarName)
{
    if(curName==nullptr || tarName==nullptr)return 0;
    if(QString(curName)==QString(tarName))return -1;//不可添加自己为好友

    QString sqlStr = QString(R"(
            select * from friend
            where(
                user_id = (select id from user_info where name='%1')
                and
                friend_id = (select id from user_info where name='%2')
            ) or (
                user_id = (select id from user_info where name='%2')
                and
                friend_id = (select id from user_info where name='%1')
            )
            )").arg(curName,tarName);

    // qDebug()<<"OperateDB(handleAddFriend):"
    //          <<"\t sql:查询是否是好友"<<sqlStr;

    QSqlQuery q;
    if(!q.exec(sqlStr))return 0;//执行sql失败
    if(q.next())return -1;//对方已是您的好友

    sqlStr = QString("select online from user_info where name='%1'").arg(tarName);//再次确认对方是否在线
    if(!q.exec(sqlStr))return 0;//执行sql失败
    if(!q.next() || !q.value(0).toInt())return -2;//用户不存在或者下线了

    return 1;//添加请求成功
}

bool OperateDB::handleAgreeAdd(const char *curName, const char *tarName)
{
    if(curName==nullptr || tarName==nullptr)return 0;
    QString sqlStr = QString(R"(
        insert into friend(user_id,friend_id)
            select u1.id, u2.id
            from user_info u1, user_info u2
            where (u1.name='%1' and u2.name='%2')
                or (u1.name='%2' and u2.name='%1')
    )").arg(curName,tarName);

    QSqlQuery q;
    return q.exec(sqlStr);
}

void OperateDB::handleOffline(const char *uid)
{
    if(uid == nullptr)return;
    QString sql = QString("update user_info set online=0 where name = '%1'").arg(uid);

    QSqlQuery().exec(sql);
    qDebug()<<"sql: 用户"+QString(uid)+"已下线";
}

QStringList OperateDB::handleGetFriendList(const char *uid)
{
    QStringList result;
    if(uid==nullptr)return result;

    QString sqlStr = QString(R"(
                    select name from user_info
                    where id in(
                        select friend_id from friend where user_id =
                        (select id from user_info where name='%1')
                        union
                        select user_id from friend where friend_id =
                        (select id from user_info where name='%1')
                    )
                    )").arg(uid);
    QSqlQuery q;
    q.exec(sqlStr);
    while(q.next()){
        result.append(q.value(0).toString());
    }
    return result;
}

int OperateDB::handleDelFriend(const char *curName, const char *tarName)
{
    if(curName==nullptr || tarName==nullptr)return 0;
    QString sqlStr = QString(R"(
            select * from friend
            where(
                user_id = (select id from user_info where name='%1')
                and
                friend_id = (select id from user_info where name='%2')
            ) or (
                user_id = (select id from user_info where name='%2')
                and
                friend_id = (select id from user_info where name='%1')
            )
            )").arg(curName,tarName);
    QSqlQuery q;
    if(!q.exec(sqlStr))return 0;//执行sql失败
    if(!q.next())return -1;//对方已不是你的好友

    sqlStr = QString(R"(
            delete from friend
            where(
                user_id = (select id from user_info where name='%1')
                and
                friend_id = (select id from user_info where name='%2')
            ) or (
                user_id = (select id from user_info where name='%2')
                and
                friend_id = (select id from user_info where name='%1')
            )
            )").arg(curName,tarName);

    if(!q.exec(sqlStr))return 0;//执行sql失败
    return 1;
}
