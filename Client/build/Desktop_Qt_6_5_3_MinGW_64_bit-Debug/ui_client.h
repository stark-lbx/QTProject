/********************************************************************************
** Form generated from reading UI file 'client.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENT_H
#define UI_CLIENT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Client
{
public:
    QVBoxLayout *verticalLayout;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_3;
    QSpacerItem *verticalSpacer_2;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *uid_LE;
    QLabel *label_2;
    QLineEdit *pwd_LE;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *register_PB;
    QSpacerItem *horizontalSpacer;
    QPushButton *login_PB;
    QSpacerItem *horizontalSpacer_3;

    void setupUi(QWidget *Client)
    {
        if (Client->objectName().isEmpty())
            Client->setObjectName("Client");
        Client->setWindowModality(Qt::NonModal);
        Client->resize(324, 303);
        Client->setMinimumSize(QSize(324, 303));
        Client->setMaximumSize(QSize(324, 303));
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("QIcon::ThemeIcon::AddressBookNew")));
        Client->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(Client);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        label_3 = new QLabel(Client);
        label_3->setObjectName("label_3");
        label_3->setMinimumSize(QSize(300, 150));
        label_3->setMaximumSize(QSize(300, 150));
        label_3->setFrameShape(QFrame::StyledPanel);
        label_3->setPixmap(QPixmap(QString::fromUtf8(":/img/background01.png")));
        label_3->setScaledContents(true);
        label_3->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(label_3);


        verticalLayout->addLayout(verticalLayout_2);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        label = new QLabel(Client);
        label->setObjectName("label");
        QFont font;
        font.setPointSize(12);
        label->setFont(font);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        uid_LE = new QLineEdit(Client);
        uid_LE->setObjectName("uid_LE");
        uid_LE->setFont(font);

        gridLayout->addWidget(uid_LE, 0, 1, 1, 1);

        label_2 = new QLabel(Client);
        label_2->setObjectName("label_2");
        label_2->setFont(font);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        pwd_LE = new QLineEdit(Client);
        pwd_LE->setObjectName("pwd_LE");
        pwd_LE->setFont(font);
        pwd_LE->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(pwd_LE, 1, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        register_PB = new QPushButton(Client);
        register_PB->setObjectName("register_PB");
        QFont font1;
        font1.setPointSize(10);
        register_PB->setFont(font1);
        register_PB->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 170, 255);"));

        horizontalLayout->addWidget(register_PB);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        login_PB = new QPushButton(Client);
        login_PB->setObjectName("login_PB");
        login_PB->setFont(font1);
        login_PB->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 170, 255);"));

        horizontalLayout->addWidget(login_PB);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(Client);

        QMetaObject::connectSlotsByName(Client);
    } // setupUi

    void retranslateUi(QWidget *Client)
    {
        Client->setWindowTitle(QCoreApplication::translate("Client", "\347\231\273\345\275\225/\346\263\250\345\206\214", nullptr));
        label_3->setText(QString());
        label->setText(QCoreApplication::translate("Client", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        label_2->setText(QCoreApplication::translate("Client", "\345\257\206    \347\240\201\357\274\232", nullptr));
        register_PB->setText(QCoreApplication::translate("Client", "\346\263\250\345\206\214", nullptr));
        login_PB->setText(QCoreApplication::translate("Client", "\347\231\273\345\275\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Client: public Ui_Client {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENT_H
