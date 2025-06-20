/********************************************************************************
** Form generated from reading UI file 'formlogin.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORMLOGIN_H
#define UI_FORMLOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>

QT_BEGIN_NAMESPACE

class Ui_FormLogin
{
public:
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *FormLogin)
    {
        if (FormLogin->objectName().isEmpty())
            FormLogin->setObjectName("FormLogin");
        FormLogin->resize(320, 240);
        buttonBox = new QDialogButtonBox(FormLogin);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setGeometry(QRect(10, 200, 301, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        retranslateUi(FormLogin);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, FormLogin, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, FormLogin, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(FormLogin);
    } // setupUi

    void retranslateUi(QDialog *FormLogin)
    {
        FormLogin->setWindowTitle(QCoreApplication::translate("FormLogin", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FormLogin: public Ui_FormLogin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORMLOGIN_H
