/********************************************************************************
** Cameraset generated from reading UI file 'cameraset.ui'
**
** Created by: Qt User Interface Compiler version 5.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CAMERASET_H
#define UI_CAMERASET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Cameraset
{
public:

    void setupUi(QWidget *Cameraset)
    {
        if (Cameraset->objectName().isEmpty())
            Cameraset->setObjectName(QStringLiteral("Cameraset"));
        Cameraset->resize(400, 300);

        retranslateUi(Cameraset);

        QMetaObject::connectSlotsByName(Cameraset);
    } // setupUi

    void retranslateUi(QWidget *Cameraset)
    {
        Cameraset->setWindowTitle(QApplication::translate("Cameraset", "Cameraset", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Cameraset: public Ui_Cameraset {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAMERASET_H
