/********************************************************************************
** Form generated from reading UI file 'friend.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FRIEND_H
#define UI_FRIEND_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Friend
{
public:
    QHBoxLayout *horizontalLayout;
    QListWidget *friend_LW;
    QVBoxLayout *verticalLayout;
    QPushButton *chat_PB;
    QPushButton *flush_PB;
    QPushButton *delete_PB;
    QSpacerItem *verticalSpacer_4;
    QSpacerItem *verticalSpacer_3;
    QSpacerItem *verticalSpacer_2;
    QSpacerItem *verticalSpacer;
    QPushButton *findUser_PB;
    QPushButton *onlineUser_PB;

    void setupUi(QWidget *Friend)
    {
        if (Friend->objectName().isEmpty())
            Friend->setObjectName("Friend");
        Friend->resize(613, 476);
        horizontalLayout = new QHBoxLayout(Friend);
        horizontalLayout->setObjectName("horizontalLayout");
        friend_LW = new QListWidget(Friend);
        friend_LW->setObjectName("friend_LW");

        horizontalLayout->addWidget(friend_LW);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        chat_PB = new QPushButton(Friend);
        chat_PB->setObjectName("chat_PB");
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(chat_PB->sizePolicy().hasHeightForWidth());
        chat_PB->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(chat_PB);

        flush_PB = new QPushButton(Friend);
        flush_PB->setObjectName("flush_PB");
        sizePolicy.setHeightForWidth(flush_PB->sizePolicy().hasHeightForWidth());
        flush_PB->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(flush_PB);

        delete_PB = new QPushButton(Friend);
        delete_PB->setObjectName("delete_PB");
        sizePolicy.setHeightForWidth(delete_PB->sizePolicy().hasHeightForWidth());
        delete_PB->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(delete_PB);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_4);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        findUser_PB = new QPushButton(Friend);
        findUser_PB->setObjectName("findUser_PB");
        sizePolicy.setHeightForWidth(findUser_PB->sizePolicy().hasHeightForWidth());
        findUser_PB->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(findUser_PB);

        onlineUser_PB = new QPushButton(Friend);
        onlineUser_PB->setObjectName("onlineUser_PB");
        sizePolicy.setHeightForWidth(onlineUser_PB->sizePolicy().hasHeightForWidth());
        onlineUser_PB->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(onlineUser_PB);


        horizontalLayout->addLayout(verticalLayout);


        retranslateUi(Friend);

        QMetaObject::connectSlotsByName(Friend);
    } // setupUi

    void retranslateUi(QWidget *Friend)
    {
        Friend->setWindowTitle(QCoreApplication::translate("Friend", "Form", nullptr));
        chat_PB->setText(QCoreApplication::translate("Friend", "\350\201\212\345\244\251", nullptr));
        flush_PB->setText(QCoreApplication::translate("Friend", "\345\210\267\346\226\260\345\245\275\345\217\213", nullptr));
        delete_PB->setText(QCoreApplication::translate("Friend", "\345\210\240\351\231\244\345\245\275\345\217\213", nullptr));
        findUser_PB->setText(QCoreApplication::translate("Friend", "\346\237\245\346\211\276\347\224\250\346\210\267", nullptr));
        onlineUser_PB->setText(QCoreApplication::translate("Friend", "\345\234\250\347\272\277\347\224\250\346\210\267", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Friend: public Ui_Friend {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRIEND_H
