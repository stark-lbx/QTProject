#include "client.h"
#include "file.h"
#include "ui_file.h"
#include "uploader.h"

#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>


File::File(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::File)
{
    ui->setupUi(this);
    m_strUserPath = QString("%1/%2").arg(
        Client::getInstance().m_strRootPath,
        Client::getInstance().m_strLoginName
        );
    m_strCurPath = m_strUserPath;

    this->flushFiles();

    m_pShareFile = new ShareFile();
    m_pShareFile->hide();
}

File::~File()
{
    delete ui;
    delete m_pShareFile;
}

void File::on_mkdir_PB_clicked()
{
    QString strNewDir = QInputDialog::getText(this,"新建文件夹","新建文件夹名: ");
    if(strNewDir.isEmpty()||strNewDir.toStdString().size()>32){
        QMessageBox::information(
            this,
            "新建文件夹",
            "文件夹长度非法");

        return;
    }
    std::unique_ptr<PDU> pdu = mkPDU(m_strCurPath.toStdString().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_MKDIR_REQUEST;
    memcpy(pdu->caData,strNewDir.toStdString().c_str(),32);
    memcpy(pdu->caMsg,m_strCurPath.toStdString().c_str(),m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(std::move(pdu));

    this->flushFiles();
}


void File::on_flush_PB_clicked()
{
    this->flushFiles();
}

void File::flushFiles()
{
    // qDebug()<<m_strCurPath;
    std::unique_ptr<PDU> pdu = mkPDU(m_strCurPath.toStdString().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSHFILES_REQUEST;
    memcpy(pdu->caMsg,m_strCurPath.toStdString().c_str(),m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(std::move(pdu));
}

void File::uploadFile()
{
    /*
     * 优化前：使用阻塞式IO，当前线程write完成后，客户端才能响应其它事件
     * 优化后：多线程非阻塞IO，开启一个后台线程执行上传文件的任务，客户端去执行其它任务
     */

    Uploader* uploader = new Uploader(m_strUploadFilePath);
    connect(uploader, &Uploader::errorSignal,this, &File::errorSlot);
    connect(uploader, &Uploader::sendPDU, &Client::getInstance(), &Client::sendMsg);
    connect(uploader, &Uploader::finished,this,[this](){
        this->on_flush_PB_clicked();
    });
    uploader->start();

    // //创建QFile对象并打开
    // QFile file(m_strUploadFilePath);
    // if(!file.open(QIODevice::ReadOnly)){
    //     QMessageBox::warning(this,"上传文件","上传失败");
    //     return;
    // }

    // std::unique_ptr<PDU> pdu = mkPDU(4096);
    // pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST;
    // while(true){
    //     qint64 ret = file.read(pdu->caData,4096);
    //     if(ret==0)break;
    //     if(ret<0){
    //         QMessageBox::warning(this,"上传文件","读取文件失败");
    //         break;
    //     }
    //     pdu->uiMsgLen = ret;
    //     pdu->uiPDULen = ret + sizeof(PDU);
    //     Client::getInstance().m_tcpSocket.write((char*)pdu.get(),pdu->uiPDULen);

    //     qDebug()<<"file::uploadfile:"
    //              <<"\n\t pdu->uiMsgType: "<<pdu->uiMsgType
    //              <<"\n\t pdu->uiPDULen: "<<pdu->uiPDULen
    //              <<"\n\t pdu->uiMsgLen: "<<pdu->uiMsgLen
    //              <<"\n\t pdu->caData: "<<pdu->caData
    //              <<"\n\t pdu->caData+32: "<<pdu->caData+32
    //              <<"\n\t pdu->caMsg: "<<pdu->caMsg;
    // }
    // QMessageBox::information(this,"上传文件","文件上传完毕");
}

void File::updateFiles_LW(QList<FileInfo *> pFileList)
{
    foreach(FileInfo* pFileInfo,m_pFileInfoList){
        delete pFileInfo;
    }
    ui->listWidget->clear();
    m_pFileInfoList = pFileList;
    foreach(FileInfo* pFileInfo ,pFileList){
        QListWidgetItem* pItem = new QListWidgetItem();

        if(pFileInfo->iType==ENUM_FILE_TYPE_DIR)
            pItem->setIcon(QIcon(QPixmap(":/img/dirType.png")));
        else if(pFileInfo->iType==ENUM_FILE_TYPE_REG)
            pItem->setIcon(QIcon(QPixmap(":/img/regType.png")));

        pItem->setText(pFileInfo->caName);
        ui->listWidget->addItem(pItem);
    }
}




void File::on_return_PB_clicked()
{
    if(m_strCurPath==m_strUserPath) return;
    int index = m_strCurPath.lastIndexOf("/");
    m_strCurPath.remove(index,m_strCurPath.size()-index);
    this->flushFiles();
}

void File::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString strDirName =  item->text();
    foreach(FileInfo* pFileInfo,m_pFileInfoList){
        if( pFileInfo->caName == strDirName
            &&pFileInfo->iType!=ENUM_FILE_TYPE_DIR){
            QMessageBox::warning(
                this,
                "警告",
                "该文件类型不支持查看");
            return;
        }
    }
    m_strCurPath = m_strCurPath+"/"+strDirName;
    this->flushFiles();
}


void File::on_rename_PB_clicked()
{
    QString strCurName = ui->listWidget->currentItem()->text();
    QString strNewName = QInputDialog::getText(this,"更改文件名","修改后的文件名: ");
    if(strNewName==strCurName)return;
    if(strNewName.isEmpty() || strNewName.toStdString().size()>32){
        QMessageBox::information(
            this,
            "更改文件名",
            "文件名长度非法");

        return;
    }

    std::unique_ptr<PDU> pdu = mkPDU(m_strCurPath.toStdString().size()+1);
    pdu->uiMsgType=  ENUM_MSG_TYPE_RENAME_REQUEST;
    memcpy(pdu->caData,strCurName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,strNewName.toStdString().c_str(),32);
    memcpy(pdu->caMsg,m_strCurPath.toStdString().c_str(),m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(std::move(pdu));

    this->flushFiles();
}


void File::on_move_PB_clicked()
{
    //点击按钮
    if(ui->move_PB->text() == "移动文件"){
        QListWidgetItem* pItem = ui->listWidget->currentItem();
        if(pItem==nullptr){
            QMessageBox::warning(
                this,
                "移动文件",
                "请选择要移动的文件");

            return;
        }
        //记录移动的文件
        m_strMoveFileName = pItem->text();
        m_strMoveFilePath = m_strCurPath + "/" + m_strMoveFileName;
        //记录后提示用户选择目标路径
        QMessageBox::information(
            this,
            "移动文件",
            "请切换至目标目录下点击确认");

        //将按钮文字修改
        ui->move_PB->setText("确认/取消");
        return;
    }
    //else: "确认/取消"

    ui->move_PB->setText("移动文件");
    //记录目标路径
    QString tarFilePath = m_strCurPath + "/" + m_strMoveFileName;//补充完整路径
    //如果没有切换路径 避免无效移动
    if(m_strMoveFilePath == tarFilePath)return;

    //提示用户是否确认移动
    int ret = QMessageBox::question(
        this,
        "移动文件",
        QString("请确认是否要将文件移动到 %1/ ?").arg(tarFilePath));

    // QMessageBox msgBox(this);
    // msgBox.setIcon(QMessageBox::Question);
    // msgBox.setWindowTitle("移动文件");
    // msgBox.setText(QString("请确认是否要将文件移动到 %1/ ?").arg(tarFilePath));
    // msgBox.addButton("确认", QMessageBox::YesRole);
    // msgBox.addButton("取消", QMessageBox::NoRole);
    // int ret = msgBox.exec();
    if(ret != 0)return;//没有选择确认

    //如果选择了确认

    int iSrcLen = m_strMoveFilePath.size(); //qDebug()<<iSrcLen;
    int iDstLen = tarFilePath.size();       //qDebug()<<iDstLen;
    std::unique_ptr<PDU> pdu = mkPDU(iSrcLen+iDstLen);
    pdu->uiMsgType = ENUM_MSG_TYPE_MOVEFILE_REQUEST;
    memcpy(pdu->caData, &iSrcLen, 32);        //qDebug()<<pdu->caData;
    memcpy(pdu->caData+32, &iDstLen, 32);     //qDebug()<<pdu->caData+32;
    memcpy(pdu->caMsg,
           QString(m_strMoveFilePath+tarFilePath).toStdString().c_str(),
           iSrcLen+iDstLen);

    Client::getInstance().sendMsg(std::move(pdu));
}


void File::on_upload_PB_clicked()
{
    //通过文件选择弹窗获得想要上传的文件的路径，路径作为File属性存下来
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if(m_strUploadFilePath.isEmpty())return;

    //根据路径取出文件名
    int index =  m_strUploadFilePath.lastIndexOf('/');
    QString strFileName = m_strUploadFilePath.right(
        m_strUploadFilePath.size() - index);
    //通过File获得文件大小
    QFile tmpFile(m_strUploadFilePath);
    qint64 iFileSize = tmpFile.size();
    //构建pdu并发送
    std::unique_ptr<PDU> pdu = mkPDU(m_strCurPath.toStdString().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_INIT_REQUEST;
    memcpy(pdu->caData,strFileName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,&iFileSize,32);
    memcpy(pdu->caMsg,m_strCurPath.toStdString().c_str(),
           m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(std::move(pdu));
}


void File::on_share_PB_clicked()
{
    QListWidgetItem* pItem = ui->listWidget->currentItem();
    if(pItem==nullptr){
        QMessageBox::warning(this,"分享文件","请选中要分享的文件");
        return;
    }

    m_strSharedFileName = pItem->text();
    m_pShareFile->updateFriend_LW();
    if(m_pShareFile->isHidden()) {
        m_pShareFile->show();
    }

}

void File::errorSlot(QString error)
{
    QMessageBox::information(this,"提示",error);
}

