/********************************************************************************
** Form generated from reading UI file 'index.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INDEX_H
#define UI_INDEX_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <friend.h>
#include "file.h"

QT_BEGIN_NAMESPACE

class Ui_Index
{
public:
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QPushButton *friend_PB;
    QPushButton *file_PB;
    QStackedWidget *stackedWidget;
    File *filePage;
    Friend *friendPage;

    void setupUi(QWidget *Index)
    {
        if (Index->objectName().isEmpty())
            Index->setObjectName("Index");
        Index->resize(657, 451);
        horizontalLayout = new QHBoxLayout(Index);
        horizontalLayout->setObjectName("horizontalLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        friend_PB = new QPushButton(Index);
        friend_PB->setObjectName("friend_PB");
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(friend_PB->sizePolicy().hasHeightForWidth());
        friend_PB->setSizePolicy(sizePolicy);
        QFont font;
        font.setFamilies({QString::fromUtf8("\346\245\267\344\275\223")});
        font.setPointSize(16);
        friend_PB->setFont(font);
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("QIcon::ThemeIcon::AddressBookNew")));
        friend_PB->setIcon(icon);

        verticalLayout->addWidget(friend_PB);

        file_PB = new QPushButton(Index);
        file_PB->setObjectName("file_PB");
        sizePolicy.setHeightForWidth(file_PB->sizePolicy().hasHeightForWidth());
        file_PB->setSizePolicy(sizePolicy);
        file_PB->setFont(font);
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("QIcon::ThemeIcon::FolderOpen")));
        file_PB->setIcon(icon1);

        verticalLayout->addWidget(file_PB);


        horizontalLayout->addLayout(verticalLayout);

        stackedWidget = new QStackedWidget(Index);
        stackedWidget->setObjectName("stackedWidget");
        filePage = new File();
        filePage->setObjectName("filePage");
        stackedWidget->addWidget(filePage);
        friendPage = new Friend();
        friendPage->setObjectName("friendPage");
        stackedWidget->addWidget(friendPage);

        horizontalLayout->addWidget(stackedWidget);


        retranslateUi(Index);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Index);
    } // setupUi

    void retranslateUi(QWidget *Index)
    {
        Index->setWindowTitle(QCoreApplication::translate("Index", "\345\234\250\347\272\277\350\201\212\345\244\251-\344\270\273\351\241\265\351\235\242", nullptr));
        friend_PB->setText(QCoreApplication::translate("Index", "\345\245\275\n"
"\345\217\213", nullptr));
        file_PB->setText(QCoreApplication::translate("Index", "\346\226\207\n"
"\344\273\266", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Index: public Ui_Index {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INDEX_H
