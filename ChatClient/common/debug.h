#ifndef DEBUG_H
#define DEBUG_H

#include <QString>
#include <QFileInfo>

// 测试UI, 显示构造的假数据
#define TEST_UI 0

// 测试群组会话详细窗口
#define TEST_GROUP_SESSION_DETAIL 0

// 测试跳过登录窗口
#define TEST_SKIP_LOGIN 1

// 测试网络连通性
#define TEST_NETWORK 0

// 从网络获取数据
#define LOAD_DATA_FROM_NETWORK 0

//是否连接测试服务器
#define CONNECT_TEST_SERVER 0

//是否部署
#define DEPOLY 0


// // 辅助打印日志的宏定义
// static inline QString getFileName(const QString& path){
//     QFileInfo fileInfo(path);
//     return fileInfo.fileName();
// }
//
// // 封装一个 宏 作为打印日志的方式
// #define TAG QString("[%1:%2]").arg(model::getFileName(__FILE__),QString::number(__LINE__))
//
// // 使用 .noquote() 设置 qDebug() 针对字符串不输出 ""
// #define LOG() qDebug().noquote() << TAG
#endif // DEBUG_H
