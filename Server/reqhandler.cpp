#include "reqhandler.h"

#include "OperateDB.h"
#include "server.h"

#include <MyTcpServer.h>
#include <QDir>

// ReqHandler::ReqHandler(std::unique_ptr<PDU> _pdu) {
//     this->pdu = clone(_pdu.get());
// }

ReqHandler::ReqHandler(){}

void ReqHandler::resetPDU(std::unique_ptr<PDU> _pdu)
{
    this->pdu.release();
    this->pdu = clone(_pdu.get());
}



std::unique_ptr<PDU> ReqHandler::regist()
{
    char caUid[32] = {'\0'};memcpy(caUid,pdu->caData,32);
    char caPwd[32] = {'\0'};memcpy(caPwd,pdu->caData+32,32);

    //注册
    QString ret = QString(std::to_string(
                              OperateDB::getInstance().handleRegist(caUid,caPwd)
                              ).c_str());
    if(ret == "1"){
        //注册成功，在服务器建立一个属于自己的目录系统
        QDir dir;
        dir.mkdir(QString("%1/%2").arg(Server::getInstance().m_strRootPath,caUid));
    }

    //响应注册
    std::unique_ptr<PDU> respdu = mkPDU(ret.size());
    respdu->uiMsgType = ENUM_MSG_TYPE_REGISTER_RESPOND;
    memcpy(respdu->caData,caUid,32);
    memcpy(respdu->caData+32,caPwd,32);
    memcpy(respdu->caMsg,ret.toStdString().c_str(),respdu->uiMsgLen);
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::login(QString& m_strLoginName)
{
    char caUid[32] = {'\0'};memcpy(caUid,pdu->caData,32);
    char caPwd[32] = {'\0'};memcpy(caPwd,pdu->caData+32,32);

    //登录
    QString ret = QString(std::to_string(
                              OperateDB::getInstance().handleLogin(caUid,caPwd)
                              ).c_str());
    if(ret.toInt()==1){
        m_strLoginName = QString(caUid);
    }
    //响应登录
    std::unique_ptr<PDU> respdu = mkPDU(ret.size());
    respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
    memcpy(respdu->caData,caUid,32);
    memcpy(respdu->caData+32,caPwd,32);
    memcpy(respdu->caMsg,ret.toStdString().c_str(),respdu->uiMsgLen);
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::findUser()
{
    char caUid[32] ={'\0'};memcpy(caUid,pdu->caData,32);

    //搜索
    QString ret = QString(std::to_string(
                              OperateDB::getInstance().handleFindUser(caUid)
                              ).c_str());

    //响应搜索
    std::unique_ptr<PDU> respdu = mkPDU(ret.size());
    respdu->uiMsgType = ENUM_MSG_TYPE_FINDUSER_RESPOND;
    memcpy(respdu->caData,caUid,32);
    memcpy(respdu->caMsg,ret.toStdString().c_str(),respdu->uiMsgLen);
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::addFriend(){
    char caCurName[32] ={'\0'};
    char caTarName[32] ={'\0'};
    memcpy(caCurName,pdu->caData,32);
    memcpy(caTarName,pdu->caData+32,32);
    // qDebug()<<"curName:"<<caCurName<<"\t tarName:"<<caTarName;

    //添加好友
    QString ret = QString(std::to_string(
                              OperateDB::getInstance().handleAddFriend(caCurName,caTarName)
                              ).c_str());
    // qDebug()<<"addFriend ret: "<<ret;
    if(ret.toInt()==1){
        //转发消息
        pdu->uiMsgType = ENUM_MSG_TYPE_ADD_RESEND;
        MyTcpServer::getInstance().resend(caTarName,std::move(pdu));
    }

    //响应添加
    std::unique_ptr<PDU> respdu = mkPDU(ret.size());
    respdu->uiMsgType=ENUM_MSG_TYPE_ADDFRIEND_RESPOND;
    memcpy(respdu->caData,caCurName,32);
    memcpy(respdu->caData+32,caTarName,32);
    memcpy(respdu->caMsg,ret.toStdString().c_str(),respdu->uiMsgLen);
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::delFriend()
{
    char caCurName[32]={'\0'};
    char caTarName[32]={'\0'};
    memcpy(caCurName,pdu->caData,32);
    memcpy(caTarName,pdu->caData+32,32);

    QString ret = QString(std::to_string(
                              OperateDB::getInstance().handleDelFriend(caCurName,caTarName)
                              ).c_str());

    //响应删除
    std::unique_ptr<PDU> respdu = mkPDU(ret.size());
    respdu->uiMsgType=ENUM_MSG_TYPE_DELFRIEND_RESPOND;
    memcpy(respdu->caData,caCurName,32);
    memcpy(respdu->caData+32,caTarName,32);
    memcpy(respdu->caMsg,ret.toStdString().c_str(),respdu->uiMsgLen);
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::agreeAdd()
{
    // qDebug()<<"===========agreeAdd===========";

    char caCurName[32] ={'\0'};//主动请求用户
    char caTarName[32] ={'\0'};//被动请求用户
    memcpy(caCurName,pdu->caData,32);
    memcpy(caTarName,pdu->caData+32,32);

    bool ret = OperateDB::getInstance().handleAgreeAdd(caCurName,caTarName);

    //响应同意
    std::unique_ptr<PDU> respdu = mkPDU(sizeof(ret));
    respdu->uiMsgType = ENUM_MSG_TYPE_AGREEADD_RESPOND;
    memcpy(respdu->caData,caCurName,32);
    memcpy(respdu->caData+32,caTarName,32);
    memcpy(respdu->caMsg,&ret,respdu->uiMsgLen);
    MyTcpServer::getInstance().resend(caCurName,clone(respdu.get()));
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::getFriendList()
{
    char caCurName[32]={'\0'};
    memcpy(caCurName,pdu->caData,32);
    QStringList result = OperateDB::getInstance().handleGetFriendList(caCurName);
    std::unique_ptr<PDU> respdu = mkPDU(result.size()*32);
    respdu->uiMsgType = ENUM_MSG_TYPE_GETFRIENDLIST_RESPOND;

    for(int i=0;i<result.size();++i){
        memcpy(respdu->caMsg+i*32,
               result.at(i).toStdString().c_str(),
               32);
    }
    // int i=0;
    // foreach(QString item,result){
    //     memcpy(respdu->caMsg+i*32,
    //            item.toStdString().c_str(),
    //            32);
    //     ++i;
    // }
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::chat()
{
    char caTarName[32] ={'\0'};
    memcpy(caTarName,pdu->caData+32,32);

    //修改类型,转发消息
    std::unique_ptr<PDU> respdu(clone(pdu.get()));
    respdu->uiMsgType = ENUM_MSG_TYPE_CHAT_RESPOND;
    MyTcpServer::getInstance().resend(caTarName,std::move(respdu));
    return nullptr;
}

std::unique_ptr<PDU> ReqHandler::mkdir()
{
    char caNewDir[32] = {'\0'};
    memcpy(caNewDir,pdu->caData,32);
    QString strCurPath = pdu->caMsg;

    QDir dir;
    bool ret = true;
    if(!dir.exists(strCurPath))ret =false;
    else{
        QString strNewPath =QString("%1/%2").arg(strCurPath,caNewDir);
        if(dir.exists(strNewPath))ret = false;
        else ret = dir.mkdir(strNewPath);
    }
    std::unique_ptr<PDU> respdu = mkPDU();
    respdu->uiMsgType = ENUM_MSG_TYPE_MKDIR_RESPOND;
    memcpy(respdu->caData,&ret,sizeof(ret));
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::flushFiles()
{
    QString strCurPath = pdu->caMsg;
    //qDebug()<< "ReqHandler(flushFiles:)当前路径" <<strCurPath;
    QDir dir(strCurPath);
    QFileInfoList fileInfoList = dir.entryInfoList();
    if(!fileInfoList.empty())fileInfoList.removeFirst();//删除目录文件.
    if(!fileInfoList.empty())fileInfoList.removeFirst();//删除目录文件..

    qDebug() <<"ReqHandler(flushFiles:)当前目录下的数目"<<fileInfoList.size();
    std::unique_ptr<PDU> respdu =
        mkPDU(sizeof(FileInfo)*fileInfoList.size());

    respdu->uiMsgType = ENUM_MSG_TYPE_FLUSHFILES_RESPOND;
    for(int i=0;i < fileInfoList.size();i++){
        FileInfo* pFileInfo = (FileInfo*)(respdu->caMsg) + i;
        memcpy( pFileInfo->caName,
               fileInfoList.at(i).fileName().toStdString().c_str(),32);

        if(fileInfoList.at(i).isDir()){
            pFileInfo->iType = ENUM_FILE_TYPE_DIR;//目录文件
        }else{
            pFileInfo->iType = ENUM_FILE_TYPE_REG;//普通文件
        }
    }
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::moveFile()
{
    int iSrcLen=0,iDstLen=0;
    memcpy(&iSrcLen,pdu->caData,sizeof(int));
    memcpy(&iDstLen,pdu->caData+32,sizeof(int));
    qDebug() <<"iSrcLen: "<< iSrcLen
             <<"iDstLen: "<< iDstLen;

    char* pSrcPath = new char[iSrcLen+1];
    memset(pSrcPath,'\0',iSrcLen+1);
    memcpy(pSrcPath,pdu->caMsg,iSrcLen);
    qDebug() << pSrcPath;

    char* pDstPath = new char[iDstLen+1];
    memset(pDstPath,'\0',iDstLen+1);
    memcpy(pDstPath,pdu->caMsg+iSrcLen,iDstLen);
    qDebug() << pDstPath;

    bool ret = QFile::rename(pSrcPath,pDstPath);
    qDebug()<<"move file result: "<< ret ;

    std::unique_ptr<PDU> respdu = mkPDU();
    respdu->uiMsgType = ENUM_MSG_TYPE_MOVEFILE_RESPOND;
    memcpy(respdu->caData,&ret,sizeof(ret));
    qDebug()<<respdu->caData;

    delete[] pSrcPath;  pSrcPath=nullptr;
    delete[] pDstPath;  pDstPath=nullptr;
    return respdu;

}

std::unique_ptr<PDU> ReqHandler::uploadFileInit()
{
    char caFileName[32] = {'\0'};
    memcpy(caFileName,pdu->caData,32);
    m_iUploadFileSize = 0;
    memcpy(&m_iUploadFileSize,pdu->caData+32,sizeof(qint64));

    m_fUploadFile.setFileName(QString("%1/%2").arg(
        pdu->caMsg,caFileName));
    bool ret = m_fUploadFile.open(QIODevice::WriteOnly);
    m_iUploadReceived = 0;
    std::unique_ptr<PDU> respdu = mkPDU();
    respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_INIT_RESPOND;
    memcpy(respdu->caData,&ret,sizeof(bool));
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::uploadFileData()
{
    m_fUploadFile.write(pdu->caMsg,pdu->uiMsgLen);
    m_iUploadReceived+=pdu->uiMsgLen;
    if(m_iUploadReceived < m_iUploadFileSize)return nullptr;

    m_fUploadFile.close();
    std::unique_ptr<PDU> respdu = mkPDU();
    respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_DATA_RESPOND;
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::shareFile()
{
    //文件的路径、发送者
    char caCurName[32] = {'\0'};
    int iFriendNum = 0;
    memcpy(caCurName,pdu->caData,32);
    memcpy(&iFriendNum,pdu->caData+32,sizeof(int));
    qDebug()<<"ReqHandler::shareFile iFriendNum:"<<iFriendNum;

    //caMsg的长度为pdu消息长度减好友长度
    std::unique_ptr<PDU> resendpdu = mkPDU(pdu->uiMsgLen - iFriendNum*32);
    resendpdu->uiMsgType = pdu->uiMsgType;//转发的类型是share file resquest
    memcpy(resendpdu->caData,caCurName,32);
    memcpy(resendpdu->caMsg,pdu->caMsg+iFriendNum*32,pdu->uiMsgLen-iFriendNum*32);
    char caRecvName[32] = {'\0'};
    for(int i=0;i<iFriendNum;i++){
        memcpy(caRecvName,pdu->caMsg+i*32,32);
        MyTcpServer::getInstance().resend(caRecvName,clone(resendpdu.get()));
    }

    std::unique_ptr<PDU> respdu = mkPDU();
    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
    return respdu;
}

std::unique_ptr<PDU> ReqHandler::shareFileAgree()
{
    //从caMsg获取要分享的文件路径，取出文件名，拼接新的完整路径
    QString strShareFilePath =  pdu->caMsg;
    int index = strShareFilePath.lastIndexOf('/');
    QString strFileName = strShareFilePath.right(strShareFilePath.size()-index-1);
    QString strRecvPath = QString("%1/%2/%3").arg(
        Server::getInstance().m_strRootPath,
        pdu->caData,
        strFileName);

    //使用QFileInfo判断是否为文件、是否存在，进行复制
    QFileInfo fileInfo(strShareFilePath);
    bool ret = true;
    if(!fileInfo.isNativePath())ret =false;
    else if(fileInfo.isFile()){
        ret = QFile::copy(strShareFilePath,strRecvPath);
    }else{
        //复制目录
        ret = copyDir(strShareFilePath,strRecvPath);
    }

    std::unique_ptr<PDU> respdu = mkPDU();
    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
    memcpy(respdu->caData,&ret,sizeof(bool));
    return respdu;
}


bool ReqHandler::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);
    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();
    bool ret = true;
    QString srcTmp;
    QString destTmp;
    for (int i=0; i<fileInfoList.size(); i++) {
        if (fileInfoList[i].isFile()) {
            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            if (!QFile::copy(srcTmp, destTmp)) {
                ret = false;
            }
        } else if (fileInfoList[i].isDir()) {
            if (fileInfoList[i].fileName() == QString(".") ||
                fileInfoList[i].fileName() == QString("..")) {
                continue;
            }
            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            if (!copyDir(srcTmp, destTmp)) {
                ret = false;
            }
        }
    }
    return ret;
}













