#include "client.h"
#include "index.h"
#include "sharefile.h"
#include "ui_sharefile.h"

ShareFile::ShareFile(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShareFile)
{
    ui->setupUi(this);
}

ShareFile::~ShareFile()
{
    delete ui;
}

void ShareFile::updateFriend_LW()
{
    ui->listWidget->clear();
    QListWidget* friend_LW = Index::getInstance().getFriend()->getFriend_LW();
    QStringList friendList;
    for(int i=0;i<friend_LW->count();i++){
        friendList.append(friend_LW->item(i)->text());
    }
    ui->listWidget->addItems(friendList);
}

void ShareFile::on_allSelected_PB_clicked()
{
    for(int i=0;i < ui->listWidget->count();i++){
        ui->listWidget->item(i)->setSelected(true);
    }
}


void ShareFile::on_cancelSelected_PB_clicked()
{
    for(int i=0;i < ui->listWidget->count();i++){
        ui->listWidget->item(i)->setSelected(false);
    }
}


void ShareFile::on_ok_PB_clicked()
{
    //发送caData：当前用户名、好友数量
    //发送caMsg：文件完整路径、好友名
    QString strCurName = Client::getInstance().m_strLoginName;
    QString strFilePath = Index::getInstance().getFile()->m_strSharedFileName;
    //获取选中的好友元素
    QList<QListWidgetItem*> pItems = ui->listWidget->selectedItems();
    //构建PDU
    int iFriendNum = pItems.size();
    std::unique_ptr<PDU> pdu = mkPDU(
        iFriendNum*32 + strFilePath.toStdString().size());
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    memcpy(pdu->caData,strCurName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,&iFriendNum,sizeof(int));

    //循环放入好友名
    for(int i=0;i<iFriendNum;i++){
        memcpy(pdu->caMsg+i*32,pItems[i]->text().toStdString().c_str(),32);
    }
    memcpy(pdu->caMsg+iFriendNum*32,strFilePath.toStdString().c_str(),
           strFilePath.toStdString().size());

    Client::getInstance().sendMsg(std::move(pdu));
}




















