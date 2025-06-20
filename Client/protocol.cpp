

#include "protocol.h"
std::unique_ptr<PDU> clone(PDU* self){
    uint msg_len = self->uiMsgLen;

    std::unique_ptr<PDU> pdu = mkPDU(msg_len);
    pdu->uiMsgLen = self->uiMsgLen;
    pdu->uiMsgType = self->uiMsgType;
    pdu->uiPDULen = self->uiPDULen;
    memcpy(pdu->caData,self->caData,64);
    if(msg_len!=0)memcpy(pdu->caMsg,self->caMsg,msg_len);

    return pdu;
}
/*
struct PDU{
    uint uiPDULen;//协议数据总长度
    uint uiMsgLen;//消息内容长度
    uint uiMsgType;//消息类型
    char caData[64];//默认大小的内容容器
    char caMsg[];//柔性大小的内容容器
};
*/
std::unique_ptr<PDU> mkPDU(uint msg_len)
{
    if(msg_len>0) msg_len++;//给msg预留一个\0的位置，防止乱码出现
    PDU *p = (PDU*)malloc(sizeof(PDU)+msg_len);
    std::unique_ptr<PDU> pdu(p);
    pdu->uiPDULen = sizeof(PDU) + msg_len;
    pdu->uiMsgLen = msg_len;
    // pdu->uiMsgType = ENUM_MSG_TYPE_MIN;
    memset(pdu->caData,'\0',64);
    if(msg_len!=0)memset(pdu->caMsg,'\0',msg_len);

    // free(p);
    p=nullptr;
    return pdu;
}

// PDU *mkPDU(uint msg_len)
// {
//     msg_len++;//给msg预留一个\0的位置，防止乱码出现
//     PDU* pdu = new PDU();
//     pdu->uiPDULen = sizeof(PDU) + msg_len;
//     pdu->uiMsgLen = msg_len;
//     return pdu;
// }
