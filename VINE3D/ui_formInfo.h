/********************************************************************************
** Form generated from reading UI file 'formInfo.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORMINFO_H
#define UI_FORMINFO_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>

QT_BEGIN_NAMESPACE

class Ui_formInfo
{
public:
    QLabel *label;
    QProgressBar *progressBar;

    void setupUi(QDialog *formInfo)
    {
        if (formInfo->objectName().isEmpty())
            formInfo->setObjectName(QStringLiteral("formInfo"));
        formInfo->resize(432, 175);
        label = new QLabel(formInfo);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 20, 361, 71));
        QFont font;
        font.setPointSize(10);
        label->setFont(font);
        progressBar = new QProgressBar(formInfo);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setGeometry(QRect(10, 120, 421, 21));
        progressBar->setValue(24);

        retranslateUi(formInfo);

        QMetaObject::connectSlotsByName(formInfo);
    } // setupUi

    void retranslateUi(QDialog *formInfo)
    {
        formInfo->setWindowTitle(QApplication::translate("formInfo", "Info", 0));
        label->setText(QApplication::translate("formInfo", "label", 0));
    } // retranslateUi

};

namespace Ui {
    class formInfo: public Ui_formInfo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORMINFO_H
