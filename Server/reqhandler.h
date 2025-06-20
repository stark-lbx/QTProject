#ifndef REQHANDLER_H
#define REQHANDLER_H

#include "protocol.h"

#include <QFile>
class ReqHandler
{
    std::unique_ptr<PDU> pdu;

    QFile m_fUploadFile;
    qint64 m_iUploadTotal;
    qint64 m_iUploadReceived;
    qint64 m_iUploadFileSize;
public:
    // ReqHandler(std::unique_ptr<PDU> _pdu);
    ReqHandler();

    void resetPDU(std::unique_ptr<PDU> _pdu);
    std::unique_ptr<PDU> regist();
    std::unique_ptr<PDU> login(QString& m_strLoginName);
    std::unique_ptr<PDU> findUser();
    std::unique_ptr<PDU> addFriend();
    std::unique_ptr<PDU> delFriend();
    std::unique_ptr<PDU> agreeAdd();
    std::unique_ptr<PDU> getFriendList();
    std::unique_ptr<PDU> chat();

    std::unique_ptr<PDU> mkdir();
    std::unique_ptr<PDU> flushFiles();
    std::unique_ptr<PDU> moveFile();

    std::unique_ptr<PDU> uploadFileInit();
    std::unique_ptr<PDU> uploadFileData();

    std::unique_ptr<PDU> shareFile();
    std::unique_ptr<PDU> shareFileAgree();

private:
    bool copyDir(QString strSrcDir, QString strDestDir);
};

#endif // REQHANDLER_H
