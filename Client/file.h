#ifndef FILE_H
#define FILE_H

#include "protocol.h"
#include "sharefile.h"

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class File;
}

class File : public QWidget
{
    Q_OBJECT

public:
    explicit File(QWidget *parent = nullptr);
    ~File();

    QString m_strUserPath;
    QString m_strCurPath;
    QList<FileInfo*> m_pFileInfoList;
    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strUploadFilePath;
    QString m_strSharedFileName;
    ShareFile *m_pShareFile;

    void updateFiles_LW(QList<FileInfo*> pFileList);
    void flushFiles();
    void uploadFile();
private slots:
    void on_mkdir_PB_clicked();

    void on_flush_PB_clicked();

    void on_return_PB_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_rename_PB_clicked();

    void on_move_PB_clicked();

    void on_upload_PB_clicked();

    void on_share_PB_clicked();

private:
    Ui::File *ui;
};

#endif // FILE_H
