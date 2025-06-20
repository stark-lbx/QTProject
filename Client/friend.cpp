#include "client.h"
#include "friend.h"
// #include "index.h"
#include "protocol.h"
#include "ui_friend.h"

#include <QInputDialog>
#include <QMessageBox>

Friend::Friend(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Friend)
{
    ui->setupUi(this);
    m_pChat = new Chat;

    flushFriends();
}

Friend::~Friend()
{
    delete ui;
}

void Friend::updateFriend_LW(QStringList friendList)
{
    ui->friend_LW->clear();
    ui->friend_LW->addItems(friendList);
}

void Friend::flushFriends()
{
    QString strName = Client::getInstance().m_strLoginName;
    std::unique_ptr<PDU> newpdu = mkPDU();
    newpdu->uiMsgType=ENUM_MSG_TYPE_GETFRIENDLIST_REQUEST;
    memcpy(newpdu->caData,strName.toStdString().c_str(),32);
    Client::getInstance().sendMsg(std::move(newpdu));
}

QListWidget *Friend::getFriend_LW()
{
    return ui->friend_LW;
}

void Friend::on_findUser_PB_clicked()
{
    QString searchName = QInputDialog::getText(this,"搜索","用户名");
    //从输入框获取用户名和密码，验证长度是否合法
    if(searchName.size()>32|| searchName.isEmpty()){
        QMessageBox::information(
            this,
            "提示",
            "账号非法长度(err: empty or >32)");

        return;
    }
    std::unique_ptr<PDU> pdu = mkPDU();
    pdu->uiMsgType = ENUM_MSG_TYPE_FINDUSER_REQUEST;
    memcpy(pdu->caData,searchName.toStdString().c_str(),32);

    //测试：发送数据
    qDebug() <<"<==findUser==>Friend(use client sendMsg-details):";
    Client::getInstance().sendMsg(std::move(pdu));
}


void Friend::on_flush_PB_clicked()
{
    flushFriends();
}


void Friend::on_delete_PB_clicked()
{
    QListWidgetItem* pItem = ui->friend_LW->currentItem();
    if(pItem==nullptr) return;

    QString strName = pItem->text();
    std::unique_ptr<PDU> newpdu = mkPDU();
    newpdu->uiMsgType = ENUM_MSG_TYPE_DELFRIEND_REQUEST;
    memcpy(newpdu->caData,Client::getInstance().m_strLoginName.toStdString().c_str(),32);
    memcpy(newpdu->caData+32,strName.toStdString().c_str(),32);
    Client::getInstance().sendMsg(std::move(newpdu));
}


void Friend::on_chat_PB_clicked()
{
    QListWidgetItem* pItem = ui->friend_LW->currentItem();
    if(pItem==nullptr)return;

    if(m_pChat->isHidden()){
        m_pChat->show();
    }
    m_pChat->m_strChatName = pItem->text();
    m_pChat->setWindowTitle(pItem->text());
}

