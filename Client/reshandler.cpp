#include "reshandler.h"

#include <Client.h>
#include <Index.h>
#include <qmessagebox.h>

ResHandler::ResHandler(std::unique_ptr<PDU> _pdu) {
    this->pdu = clone(_pdu.get());
}

void ResHandler::regist()
{
    if(pdu.get()==nullptr)return;

    QString response ="";
    int ret = QString(pdu->caMsg).toInt();
    switch(ret){
    case 0:response="注册失败:未定义错误";break;
    case -1:response="注册失败:用户名已存在";break;
    case 1:response="注册成功!";break;
    default:break;
    }
    QMessageBox::information(&Client::getInstance(),"提示",response);
}

void ResHandler::login()
{
    if(pdu.get()==nullptr)return;

    int ret = QString(pdu->caMsg).toInt();
    if(ret != 1){
        QString response ="";
        switch(ret){
        case 0:response="登录失败:未定义错误";break;
        case -1:response="登录失败:用户名或密码错误";break;
        default:break;
        }
        QMessageBox::information(&Client::getInstance(),"提示",response);
    } else {
        //记录登录的用户信息
        char * loginUser = (char*)malloc(sizeof(char)*32);
        memcpy(loginUser,pdu->caData,32);
        Client::getInstance().m_strLoginName = QString(loginUser);

        //打开新的会话，隐藏当前会话
        Index::getInstance().show();
        Client::getInstance().hide();


        free(loginUser);
        loginUser=nullptr;
    }
}

void ResHandler::findUser()
{
    if(pdu.get()==nullptr)return;

    char caName[32]={'\0'};
    memcpy(caName,pdu->caData,32);//保存对方姓名

    int ret = QString(pdu->caMsg).toInt();
    if(ret!=1){
        QString response ="";
        switch(ret){
        case 0:response="查找失败:未定义错误";break;
        case -1:response="查找失败:用户不在线";break;
        case -2:response="查找失败:用户不存在";break;
        default:break;
        }
        QMessageBox::information(&Index::getInstance(),"查找用户",response);
    }
    else{
        QMessageBox msgBox(&Client::getInstance());
        msgBox.setWindowTitle("查找用户");
        msgBox.setText("查找成功!");
        msgBox.addButton("添加好友",QMessageBox::YesRole);
        msgBox.addButton("取消",QMessageBox::NoRole);
        int choice_ret = msgBox.exec();

        if(choice_ret == 0){
            std::unique_ptr<PDU> newpdu = mkPDU();
            newpdu->uiMsgType = ENUM_MSG_TYPE_ADDFRIEND_REQUEST;
            memcpy(newpdu->caData,Client::getInstance().m_strLoginName.toStdString().c_str(),32);
            memcpy(newpdu->caData+32,caName,32);

            Client::getInstance().sendMsg(std::move(newpdu));
        }
    }
}

void ResHandler::addFriend()
{
    if(pdu.get()==nullptr)return;

    int ret = QString(pdu->caMsg).toInt();
    if(ret!=1){
        QString response = "";
        switch(ret){
        case 0:response="添加失败:未定义的错误";break;
        case -1:response="添加失败:对方已是您的好友";break;
        case -2:response="添加失败:用户已注销或已下线";break;
        default:break;
        }
        QMessageBox::information(&Index::getInstance(),"添加好友(失败)",response);
    }else{
        QString response="已成功发送请求,等待对方同意";
        QMessageBox::information(&Index::getInstance(),"添加好友",response);
    }
}

void ResHandler::delFriend()
{
    if(pdu.get()==nullptr)return;
    char caTarName[32] ={'\0'};
    memcpy(caTarName,pdu->caData+32,32);

    QString ret(pdu->caMsg);
    QMessageBox msgBox(&Index::getInstance());
    msgBox.setWindowTitle("删除好友");

    if(ret.toInt() == 1) msgBox.setText(QString("成功删除好友 '%1' ").arg(caTarName));
    else if(ret.toInt() == -1) msgBox.setText(QString("删除失败,对方已不是你的好友"));
    else msgBox.setText(QString("删除失败,发生未知错误"));

    msgBox.exec();
    Index::getInstance().getFriend()->flushFriends();
}

void ResHandler::addFriendResend()
{
    if(pdu.get()==nullptr)return;

    char caCurName[32] ={'\0'};
    memcpy(caCurName,pdu->caData,32);
    int ret = QMessageBox::question(
        &Index::getInstance(),
        "添加好友",
        QString("是否同意 %1 的好友请求?").arg(caCurName));

    if(ret != QMessageBox::Yes) return;
    Index::getInstance().getFriend()->flushFriends();

    std::unique_ptr<PDU> resend_pdu = mkPDU();
    resend_pdu->uiMsgType = ENUM_MSG_TYPE_AGREEADD_REQUEST;
    memcpy(resend_pdu->caData,pdu->caData,64);
    Client::getInstance().sendMsg(std::move(resend_pdu));
}

void ResHandler::agreeAdd()
{
    if(pdu.get()==nullptr)return;
    char caTarName[32] = {'\0'};
    memcpy(caTarName,pdu->caData+32,32);
    QMessageBox::information(
        &Index::getInstance(),
        "添加好友",
        QString(" '%1' 已同意的你的好友请求").arg(caTarName));

    Index::getInstance().getFriend()->flushFriends();
}

void ResHandler::loadFriend()
{
    if(pdu.get()==nullptr)return;

    QStringList friendList;
    int len = pdu->uiMsgLen/32;
    for(int i=0;i<len;i++){
        char name[32]={'\0'};
        memcpy(name,(pdu->caMsg+(i*32)),32);
        friendList.append(QString(name));
    }
    Index::getInstance().getFriend()->updateFriend_LW(friendList);
}

void ResHandler::chat()
{
    Chat* c =Index::getInstance().getFriend()->m_pChat;
    if(c->isHidden()){
        c->show();
    }
    char caChatName[32] = {'\0'};
    memcpy(caChatName,pdu->caData,32);
    c->m_strChatName = caChatName;
    c->setWindowTitle(caChatName);
    c->updateShow_TE(pdu->caMsg);
}

void ResHandler::mkdir()
{
    bool ret;
    memcpy(&ret,pdu->caData,sizeof(ret));

    if(ret){
        QMessageBox::information(
            &Index::getInstance(),
            "创建文件夹",
            "创建文件夹成功");
        flushFiles();
    }
    else {
        QMessageBox::information(
            &Index::getInstance(),
            "创建文件夹",
            "创建文件夹失败");
    }
}
void ResHandler::flushFiles()
{
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);
    // qDebug()<<"iCount:"<<iCount;

    QList<FileInfo*> pFileList;
    for(int i=0;i<iCount;i++){
        FileInfo* pFileInfo = new FileInfo;
        memcpy(pFileInfo->caName,
               pdu->caMsg+i*sizeof(FileInfo),sizeof(FileInfo));
        pFileList.append(pFileInfo);
    }

    Index::getInstance().getFile()->updateFiles_LW(pFileList);
}

void ResHandler::moveFile()
{
    bool ret;
    memcpy(&ret,pdu->caData,sizeof(ret));
    qDebug()<<pdu->caData;
    qDebug()<<ret;

    if(bool(ret) == true) {
        Index::getInstance().getFile()->flushFiles();
    }else{
        QMessageBox::information(
            &Index::getInstance(),
            "移动文件",
            "移动文件失败");
    }
}

void ResHandler::uploadFileInit()
{
    bool ret;
    memcpy(&ret,pdu->caData,sizeof(bool));
    if(ret) Index::getInstance().getFile()->uploadFile();//此处调用实际的传输数据
    else QMessageBox::information(&Index::getInstance(),"上传文件","传输文件失败");
}

void ResHandler::shareFile()
{
    QMessageBox::information(
        &Index::getInstance(),
        "分享文件",
        "文件已分享");
}

void ResHandler::shareFileResend()
{
    QString strSharePath = QString(pdu->caMsg);
    int index = strSharePath.lastIndexOf('/');
    QString strFileName = strSharePath.right(strSharePath.size()-index-1);
    int ret = QMessageBox::question(
        &Index::getInstance(),
        "分享文件",
        QString("%1 分享文件：%2\n是否同意接收"
                ).arg(pdu->caData,strFileName));
    if(ret!=QMessageBox::Yes)return;

    std::unique_ptr<PDU> respdu = mkPDU(pdu->uiMsgLen);
    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_AGREE_REQUEST;
    memcpy(respdu->caData,Client::getInstance().m_strLoginName.toStdString().c_str(),32);
    memcpy(respdu->caMsg,pdu->caMsg,pdu->uiMsgLen);

    Client::getInstance().sendMsg(std::move(respdu));
}

















