/********************************************************************************
** Preview generated from reading UI file 'Preview.ui'
**
** Created by: Qt User Interface Compiler version 5.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PREVIEW_H
#define UI_PREVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QStringList>
#include <QString>
QT_BEGIN_NAMESPACE

class Ui_Preview
{
public:
    QComboBox *comboBoxcameraconnect;
    QComboBox *comboBoxpreview;
    QComboBox *comboBoxcapture;
    QComboBox *comboBoxautofocus;
    QComboBox *comboBoxiso;
    QLabel *labelcameraconnect;
    QLabel *labelpreview;
    QLabel *labelcapture;
    QLabel *labelautofocus;
    QLabel *labeliso;
    QComboBox *comboBox;

    void setupUi(QWidget *Preview)
    {
        if (Preview->objectName().isEmpty())
            Preview->setObjectName(QStringLiteral("Preview"));
        Preview->resize(645, 434);
        comboBoxcameraconnect = new QComboBox(Preview);
        comboBoxcameraconnect->setObjectName(QStringLiteral("comboBoxcameraconnect"));
        comboBoxcameraconnect->setGeometry(QRect(90, 40, 69, 22));
        comboBoxpreview = new QComboBox(Preview);
        comboBoxpreview->setObjectName(QStringLiteral("comboBoxpreview"));
        comboBoxpreview->setGeometry(QRect(90, 90, 69, 22));
        comboBoxcapture = new QComboBox(Preview);
        comboBoxcapture->setObjectName(QStringLiteral("comboBoxcapture"));
        comboBoxcapture->setGeometry(QRect(90, 150, 69, 22));
        comboBoxautofocus = new QComboBox(Preview);
        comboBoxautofocus->setObjectName(QStringLiteral("comboBoxautofocus"));
        comboBoxautofocus->setGeometry(QRect(90, 210, 69, 22));
        comboBoxiso = new QComboBox(Preview);
        comboBoxiso->setObjectName(QStringLiteral("comboBoxiso"));
        comboBoxiso->setGeometry(QRect(90, 280, 69, 22));
        labelcameraconnect = new QLabel(Preview);
        labelcameraconnect->setObjectName(QStringLiteral("labelcameraconnect"));
        labelcameraconnect->setGeometry(QRect(10, 42, 54, 20));
        labelpreview = new QLabel(Preview);
        labelpreview->setObjectName(QStringLiteral("labelpreview"));
        labelpreview->setGeometry(QRect(10, 92, 54, 20));
        labelcapture = new QLabel(Preview);
        labelcapture->setObjectName(QStringLiteral("labelcapture"));
        labelcapture->setGeometry(QRect(10, 152, 54, 20));
        labelautofocus = new QLabel(Preview);
        labelautofocus->setObjectName(QStringLiteral("labelautofocus"));
        labelautofocus->setGeometry(QRect(10, 212, 54, 20));
        labeliso = new QLabel(Preview);
        labeliso->setObjectName(QStringLiteral("labeliso"));
        labeliso->setGeometry(QRect(10, 282, 54, 20));
        comboBox = new QComboBox(Preview);
        comboBox->setObjectName(QStringLiteral("comboBox"));
        comboBox->setGeometry(QRect(320, 140, 69, 22));

		//定义字符串列表
		QStringList str1;
		QStringList str2;
		QStringList str3;
		QStringList str4;
		QStringList str5;
		str1 << "On" << "Off";
		str2 << "On" << "Off";
		str3 << "On" << "Off";
		str4 << "On" << "Off";
		str5 << "On" << "Off";
		//将字符串列表绑定QComboBox 控件 
		comboBoxcameraconnect->addItems(str1);
		comboBoxpreview->addItems(str2);
		comboBoxcapture->addItems(str3);
		comboBoxautofocus->addItems(str4);
		comboBoxiso->addItems(str5);

	//	QString curStr1 = comboBoxcameraconnect->currentText();

        retranslateUi(Preview);

        QMetaObject::connectSlotsByName(Preview);
    } // setupUi

    void retranslateUi(QWidget *Preview)
    {
        Preview->setWindowTitle(QApplication::translate("Preview", "Preview", Q_NULLPTR));
        labelcameraconnect->setText(QApplication::translate("Preview", "\347\233\270\346\234\272\350\277\236\346\216\245", Q_NULLPTR));
        labelpreview->setText(QApplication::translate("Preview", "\345\256\236\346\227\266\351\242\204\350\247\210", Q_NULLPTR));
        labelcapture->setText(QApplication::translate("Preview", "\345\233\276\345\203\217\351\207\207\351\233\206", Q_NULLPTR));
        labelautofocus->setText(QApplication::translate("Preview", "\350\207\252\345\212\250\350\201\232\347\204\246", Q_NULLPTR));
        labeliso->setText(QApplication::translate("Preview", "ISO", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Preview: public Ui_Preview {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PREVIEW_H
