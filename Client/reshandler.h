#ifndef RESHANDLER_H
#define RESHANDLER_H

#include "protocol.h"

#include <Client.h>


class ResHandler
{
    std::unique_ptr<PDU> pdu;
public:
    ResHandler(std::unique_ptr<PDU> _pdu);
    void regist();
    void login();
    void findUser();
    void addFriend();
    void delFriend();
    void addFriendResend();
    void agreeAdd();
    void loadFriend();
    void chat();

    void mkdir();
    void flushFiles();
    void moveFile();

    void uploadFileInit();

    void shareFile();
    void shareFileResend();
};

#endif // RESHANDLER_H
