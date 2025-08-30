#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    ,websocketServer(WebsocketServer::getInstance())
{
    ui->setupUi(this);
    this->layout()->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    this->setWindowTitle("ChatServerMock: 测试服务器(信息推送器界面)");
    this->setFixedWidth(300);
    this->move(0,0);

    const QSize btnSize{250,50};
    ui->sendTextMessage->setFixedSize(btnSize);
    ui->sendImageMessage->setFixedSize(btnSize);
    ui->sendFileMessage->setFixedSize(btnSize);
    ui->sendSpeechMessage->setFixedSize(btnSize);

    ui->notifyFriendRemove->setFixedSize(btnSize);
    ui->notifyFriendApply->setFixedSize(btnSize);
    ui->notifyFriendAgree->setFixedSize(btnSize);
    ui->notifyFriendReject->setFixedSize(btnSize);

    ui->notifyNewChatSession->setFixedSize(btnSize);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_sendTextMessage_clicked()
{
    emit websocketServer->sendTextResp();
}

void Widget::on_sendImageMessage_clicked()
{
    emit websocketServer->sendImageResp();
}


void Widget::on_sendFileMessage_clicked()
{
    emit websocketServer->sendFileResp();
}


void Widget::on_sendSpeechMessage_clicked()
{
    emit websocketServer->sendSpeechResp();
}


void Widget::on_notifyFriendRemove_clicked()
{
    emit websocketServer->sendFriendRemove();
}


void Widget::on_notifyFriendApply_clicked()
{
    emit websocketServer->sendAddFriendApply();
}


void Widget::on_notifyFriendAgree_clicked()
{
    emit websocketServer->sendAddFriendProcess(true);
}


void Widget::on_notifyFriendReject_clicked()
{
    emit websocketServer->sendAddFriendProcess(false);
}


void Widget::on_notifyNewChatSession_clicked()
{
    emit websocketServer->sendCreateChatSession();
}



