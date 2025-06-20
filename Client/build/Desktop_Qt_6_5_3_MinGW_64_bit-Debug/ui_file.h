/********************************************************************************
** Form generated from reading UI file 'file.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILE_H
#define UI_FILE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_File
{
public:
    QHBoxLayout *horizontalLayout;
    QListWidget *listWidget;
    QGridLayout *gridLayout;
    QPushButton *download_PB;
    QPushButton *upload_PB;
    QPushButton *rename_PB;
    QPushButton *share_PB;
    QPushButton *delfile_PB;
    QPushButton *mkdir_PB;
    QPushButton *deldir_PB;
    QPushButton *move_PB;
    QPushButton *flush_PB;
    QPushButton *return_PB;

    void setupUi(QWidget *File)
    {
        if (File->objectName().isEmpty())
            File->setObjectName("File");
        File->resize(564, 219);
        horizontalLayout = new QHBoxLayout(File);
        horizontalLayout->setObjectName("horizontalLayout");
        listWidget = new QListWidget(File);
        listWidget->setObjectName("listWidget");

        horizontalLayout->addWidget(listWidget);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        download_PB = new QPushButton(File);
        download_PB->setObjectName("download_PB");

        gridLayout->addWidget(download_PB, 5, 1, 1, 1);

        upload_PB = new QPushButton(File);
        upload_PB->setObjectName("upload_PB");

        gridLayout->addWidget(upload_PB, 4, 1, 1, 1);

        rename_PB = new QPushButton(File);
        rename_PB->setObjectName("rename_PB");

        gridLayout->addWidget(rename_PB, 5, 0, 1, 1);

        share_PB = new QPushButton(File);
        share_PB->setObjectName("share_PB");

        gridLayout->addWidget(share_PB, 3, 1, 1, 1);

        delfile_PB = new QPushButton(File);
        delfile_PB->setObjectName("delfile_PB");

        gridLayout->addWidget(delfile_PB, 4, 0, 1, 1);

        mkdir_PB = new QPushButton(File);
        mkdir_PB->setObjectName("mkdir_PB");

        gridLayout->addWidget(mkdir_PB, 0, 0, 1, 1);

        deldir_PB = new QPushButton(File);
        deldir_PB->setObjectName("deldir_PB");

        gridLayout->addWidget(deldir_PB, 3, 0, 1, 1);

        move_PB = new QPushButton(File);
        move_PB->setObjectName("move_PB");

        gridLayout->addWidget(move_PB, 1, 1, 1, 1);

        flush_PB = new QPushButton(File);
        flush_PB->setObjectName("flush_PB");

        gridLayout->addWidget(flush_PB, 0, 1, 1, 1);

        return_PB = new QPushButton(File);
        return_PB->setObjectName("return_PB");

        gridLayout->addWidget(return_PB, 1, 0, 1, 1);


        horizontalLayout->addLayout(gridLayout);


        retranslateUi(File);

        QMetaObject::connectSlotsByName(File);
    } // setupUi

    void retranslateUi(QWidget *File)
    {
        File->setWindowTitle(QCoreApplication::translate("File", "Form", nullptr));
        download_PB->setText(QCoreApplication::translate("File", "\344\270\213\350\275\275\346\226\207\344\273\266", nullptr));
        upload_PB->setText(QCoreApplication::translate("File", "\344\270\212\344\274\240\346\226\207\344\273\266", nullptr));
        rename_PB->setText(QCoreApplication::translate("File", "\351\207\215\345\221\275\345\220\215", nullptr));
        share_PB->setText(QCoreApplication::translate("File", "\345\210\206\344\272\253\346\226\207\344\273\266", nullptr));
        delfile_PB->setText(QCoreApplication::translate("File", "\345\210\240\351\231\244\346\226\207\344\273\266", nullptr));
        mkdir_PB->setText(QCoreApplication::translate("File", "\345\210\233\345\273\272\346\226\207\344\273\266\345\244\271", nullptr));
        deldir_PB->setText(QCoreApplication::translate("File", "\345\210\240\351\231\244\346\226\207\344\273\266\345\244\271", nullptr));
        move_PB->setText(QCoreApplication::translate("File", "\347\247\273\345\212\250\346\226\207\344\273\266", nullptr));
        flush_PB->setText(QCoreApplication::translate("File", "\345\210\267\346\226\260\346\226\207\344\273\266", nullptr));
        return_PB->setText(QCoreApplication::translate("File", "\350\277\224\345\233\236\344\270\212\344\270\200\347\272\247", nullptr));
    } // retranslateUi

};

namespace Ui {
    class File: public Ui_File {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILE_H
