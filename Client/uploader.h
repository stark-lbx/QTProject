#ifndef UPLOADER_H
#define UPLOADER_H

#include "protocol.h"

#include <QObject>

class Uploader : public QObject
{
    Q_OBJECT
public:
    explicit Uploader(QObject *parent = nullptr);
    Uploader(QString strFilePath);
    QString file_path;//要上传的文件路径，构造时初始化
    void start();//开始上传
public slots:
    void uploadFile();
signals:
    void errorSignal(QString err);
    void sendPDU(std::shared_ptr<PDU>);
    void finished();
};

#endif // UPLOADER_H
