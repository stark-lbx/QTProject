/********************************************************************************
** Form generated from reading UI file 'sharefile.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHAREFILE_H
#define UI_SHAREFILE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ShareFile
{
public:
    QHBoxLayout *horizontalLayout;
    QListWidget *listWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *allSelected_PB;
    QPushButton *cancelSelected_PB;
    QSpacerItem *verticalSpacer;
    QPushButton *ok_PB;

    void setupUi(QWidget *ShareFile)
    {
        if (ShareFile->objectName().isEmpty())
            ShareFile->setObjectName("ShareFile");
        ShareFile->resize(662, 538);
        horizontalLayout = new QHBoxLayout(ShareFile);
        horizontalLayout->setObjectName("horizontalLayout");
        listWidget = new QListWidget(ShareFile);
        listWidget->setObjectName("listWidget");
        listWidget->setSelectionMode(QAbstractItemView::MultiSelection);

        horizontalLayout->addWidget(listWidget);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        allSelected_PB = new QPushButton(ShareFile);
        allSelected_PB->setObjectName("allSelected_PB");

        verticalLayout->addWidget(allSelected_PB);

        cancelSelected_PB = new QPushButton(ShareFile);
        cancelSelected_PB->setObjectName("cancelSelected_PB");

        verticalLayout->addWidget(cancelSelected_PB);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        ok_PB = new QPushButton(ShareFile);
        ok_PB->setObjectName("ok_PB");

        verticalLayout->addWidget(ok_PB);


        horizontalLayout->addLayout(verticalLayout);


        retranslateUi(ShareFile);

        QMetaObject::connectSlotsByName(ShareFile);
    } // setupUi

    void retranslateUi(QWidget *ShareFile)
    {
        ShareFile->setWindowTitle(QCoreApplication::translate("ShareFile", "Form", nullptr));
        allSelected_PB->setText(QCoreApplication::translate("ShareFile", "\345\205\250\351\200\211", nullptr));
        cancelSelected_PB->setText(QCoreApplication::translate("ShareFile", "\345\217\226\346\266\210\351\200\211\344\270\255", nullptr));
        ok_PB->setText(QCoreApplication::translate("ShareFile", "\347\241\256\345\256\232\345\210\206\344\272\253", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ShareFile: public Ui_ShareFile {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHAREFILE_H
