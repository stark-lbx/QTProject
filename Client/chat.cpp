#include "chat.h"
#include "protocol.h"
#include "ui_chat.h"

#include <Client.h>

Chat::Chat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Chat)
{
    ui->setupUi(this);
}

Chat::~Chat()
{
    delete ui;
}

void Chat::updateShow_TE(QString strMsg)
{
    ui->show_TE->appendPlainText(strMsg);
}

void Chat::on_send_PB_clicked()
{
    //获取发送新消息内容并清空
    QString strMsg = ui->input_LE->text();
    if(strMsg.isEmpty())return;
    ui->input_LE->clear();

    //构建pdu
    //[sender recver putin caData]
    //[message putin caMsg]
    std::unique_ptr<PDU> pdu = mkPDU(strMsg.toStdString().size());
    pdu->uiMsgType = ENUM_MSG_TYPE_CHAT_REQUEST;
    memcpy(pdu->caData,Client::getInstance().m_strLoginName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,this->m_strChatName.toStdString().c_str(),32);
    memcpy(pdu->caMsg,strMsg.toStdString().c_str(),pdu->uiMsgLen);
    Client::getInstance().sendMsg(std::move(pdu));
}

