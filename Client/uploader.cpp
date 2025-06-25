#include "uploader.h"

#include <QFile>
#include <QThread>

Uploader::Uploader(QObject *parent)
    : QObject{parent}
{}

Uploader::Uploader(QString strFilePath)
    :file_path(strFilePath)
{}

void Uploader::start()
{
    QThread* thread = new QThread;//创建一个线程对象
    this->moveToThread(thread);//将当前线程移动到指定线程对象中

    connect(thread,&QThread::started, this,&Uploader::uploadFile);
    connect(this, &Uploader::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished,thread, &QThread::deleteLater);

    thread->start();
}

void Uploader::uploadFile()
{
    //构建QFile对象并打开
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly)){
        emit errorSignal("上传文件失败");
        emit finished();
        return;
    }

    //构建pdu，循环读取文件内容并发送
    while(true){
        auto pdu = mkPDU(4096);//std::unique_ptr<PDU>
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST;
        qint64 ret = file.read(pdu->caMsg,4096);

        if(ret==0)break;//读取完毕
        if(ret<0){
            emit errorSignal("读取文件失败");
            break;
        }
        pdu->uiMsgLen = ret;
        pdu->uiPDULen = ret + sizeof(PDU);
        emit sendPDU(std::move(pdu));
    }

    file.close();
    emit finished();
}
