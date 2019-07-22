/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNew_project;
    QAction *actionOpen_project;
    QAction *actionSave_as;
    QAction *actionCameraSet;
    QAction *actionFirstPage;
    QAction *actionLastPage;
    QAction *actionPreviousPage;
    QAction *actionNextPage;
    QAction *actionSLR_1;
    QAction *actionSLR_2;
    QAction *actionSLR_3;
    QAction *actionSLR_4;
    QAction *actionSLR_Preview;
    QAction *actionSLR_Stop_View;
    QAction *actionSLR_Start_Capture;
    QAction *actionSLR_Stop_Capture;
    QAction *actionHSC_1;
    QAction *actionHSC_7;
    QAction *actionHSC_8;
    QAction *actionHSC_9;
    QAction *actionHSC_Preview;
    QAction *actionHSC_Stop_View_2;
    QAction *actionHSC_Start_Capture_2;
    QAction *actionHSC_Stop_Capture_2;
    QAction *actionAll_Camara;
    QAction *actionAll_HCS_Camera;
    QWidget *centralWidget;
    QLabel *label_savepath;
    QLabel *label_date;
    QTextBrowser *textBrowser_savepath;
    QPushButton *SelectPath;
    QTextBrowser *textBrowser_date;
    QLabel *label_project;
    QTextBrowser *textBrowser_projectname;
    QLabel *label_slr_preview;
    QLabel *label_hsc_preview;
    QWidget *camera_slr_1;
    QWidget *camera_slr_2;
    QWidget *camara_hsc_1;
    QWidget *camera_hsc_2;
    QMenuBar *menuBar;
    QMenu *menu;
    QMenu *menuReview;
    QMenu *menu_2;
    QMenu *menu_3;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(820, 678);
        actionNew_project = new QAction(MainWindow);
        actionNew_project->setObjectName(QStringLiteral("actionNew_project"));
        QIcon icon;
        icon.addFile(QStringLiteral("myImages/control/199-Add-File.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionNew_project->setIcon(icon);
        actionOpen_project = new QAction(MainWindow);
        actionOpen_project->setObjectName(QStringLiteral("actionOpen_project"));
        QIcon icon1;
        icon1.addFile(QStringLiteral("myImages/control/126-File.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen_project->setIcon(icon1);
        actionSave_as = new QAction(MainWindow);
        actionSave_as->setObjectName(QStringLiteral("actionSave_as"));
        QIcon icon2;
        icon2.addFile(QStringLiteral("myImages/control/2-XML.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_as->setIcon(icon2);
        actionCameraSet = new QAction(MainWindow);
        actionCameraSet->setObjectName(QStringLiteral("actionCameraSet"));
        QIcon icon3;
        icon3.addFile(QStringLiteral("myImages/control/38-Settings.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCameraSet->setIcon(icon3);
        actionFirstPage = new QAction(MainWindow);
        actionFirstPage->setObjectName(QStringLiteral("actionFirstPage"));
        QIcon icon4;
        icon4.addFile(QStringLiteral("myImages/control/102-Left-Round.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionFirstPage->setIcon(icon4);
        actionLastPage = new QAction(MainWindow);
        actionLastPage->setObjectName(QStringLiteral("actionLastPage"));
        QIcon icon5;
        icon5.addFile(QStringLiteral("myImages/control/47-Right-Round.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLastPage->setIcon(icon5);
        actionPreviousPage = new QAction(MainWindow);
        actionPreviousPage->setObjectName(QStringLiteral("actionPreviousPage"));
        QIcon icon6;
        icon6.addFile(QStringLiteral("myImages/control/103-Left-Arrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPreviousPage->setIcon(icon6);
        actionNextPage = new QAction(MainWindow);
        actionNextPage->setObjectName(QStringLiteral("actionNextPage"));
        QIcon icon7;
        icon7.addFile(QStringLiteral("myImages/control/48-Right-Arrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionNextPage->setIcon(icon7);
        actionSLR_1 = new QAction(MainWindow);
        actionSLR_1->setObjectName(QStringLiteral("actionSLR_1"));
        actionSLR_2 = new QAction(MainWindow);
        actionSLR_2->setObjectName(QStringLiteral("actionSLR_2"));
        actionSLR_3 = new QAction(MainWindow);
        actionSLR_3->setObjectName(QStringLiteral("actionSLR_3"));
        actionSLR_4 = new QAction(MainWindow);
        actionSLR_4->setObjectName(QStringLiteral("actionSLR_4"));
        actionSLR_Preview = new QAction(MainWindow);
        actionSLR_Preview->setObjectName(QStringLiteral("actionSLR_Preview"));
        QIcon icon8;
        icon8.addFile(QStringLiteral("myImages/SLR/Eject.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSLR_Preview->setIcon(icon8);
        actionSLR_Stop_View = new QAction(MainWindow);
        actionSLR_Stop_View->setObjectName(QStringLiteral("actionSLR_Stop_View"));
        QIcon icon9;
        icon9.addFile(QStringLiteral("myImages/SLR/Stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSLR_Stop_View->setIcon(icon9);
        actionSLR_Start_Capture = new QAction(MainWindow);
        actionSLR_Start_Capture->setObjectName(QStringLiteral("actionSLR_Start_Capture"));
        QIcon icon10;
        icon10.addFile(QStringLiteral("myImages/SLR/Play.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSLR_Start_Capture->setIcon(icon10);
        actionSLR_Stop_Capture = new QAction(MainWindow);
        actionSLR_Stop_Capture->setObjectName(QStringLiteral("actionSLR_Stop_Capture"));
        QIcon icon11;
        icon11.addFile(QStringLiteral("myImages/SLR/Pause.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSLR_Stop_Capture->setIcon(icon11);
        actionHSC_1 = new QAction(MainWindow);
        actionHSC_1->setObjectName(QStringLiteral("actionHSC_1"));
        actionHSC_7 = new QAction(MainWindow);
        actionHSC_7->setObjectName(QStringLiteral("actionHSC_7"));
        actionHSC_8 = new QAction(MainWindow);
        actionHSC_8->setObjectName(QStringLiteral("actionHSC_8"));
        actionHSC_9 = new QAction(MainWindow);
        actionHSC_9->setObjectName(QStringLiteral("actionHSC_9"));
        actionHSC_Preview = new QAction(MainWindow);
        actionHSC_Preview->setObjectName(QStringLiteral("actionHSC_Preview"));
        QIcon icon12;
        icon12.addFile(QStringLiteral("myImages/HSC/button_eject.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHSC_Preview->setIcon(icon12);
        actionHSC_Stop_View_2 = new QAction(MainWindow);
        actionHSC_Stop_View_2->setObjectName(QStringLiteral("actionHSC_Stop_View_2"));
        QIcon icon13;
        icon13.addFile(QStringLiteral("myImages/HSC/button_stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHSC_Stop_View_2->setIcon(icon13);
        actionHSC_Start_Capture_2 = new QAction(MainWindow);
        actionHSC_Start_Capture_2->setObjectName(QStringLiteral("actionHSC_Start_Capture_2"));
        QIcon icon14;
        icon14.addFile(QStringLiteral("myImages/HSC/button_play.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHSC_Start_Capture_2->setIcon(icon14);
        actionHSC_Stop_Capture_2 = new QAction(MainWindow);
        actionHSC_Stop_Capture_2->setObjectName(QStringLiteral("actionHSC_Stop_Capture_2"));
        QIcon icon15;
        icon15.addFile(QStringLiteral("myImages/HSC/button_pause.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHSC_Stop_Capture_2->setIcon(icon15);
        actionAll_Camara = new QAction(MainWindow);
        actionAll_Camara->setObjectName(QStringLiteral("actionAll_Camara"));
        QIcon icon16;
        icon16.addFile(QStringLiteral("myImages/SLR/canon_128.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAll_Camara->setIcon(icon16);
        actionAll_HCS_Camera = new QAction(MainWindow);
        actionAll_HCS_Camera->setObjectName(QStringLiteral("actionAll_HCS_Camera"));
        QIcon icon17;
        icon17.addFile(QStringLiteral("myImages/HSC/camera_128.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAll_HCS_Camera->setIcon(icon17);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        label_savepath = new QLabel(centralWidget);
        label_savepath->setObjectName(QStringLiteral("label_savepath"));
        label_savepath->setGeometry(QRect(33, 29, 51, 31));
        label_date = new QLabel(centralWidget);
        label_date->setObjectName(QStringLiteral("label_date"));
        label_date->setGeometry(QRect(360, 40, 54, 12));
        textBrowser_savepath = new QTextBrowser(centralWidget);
        textBrowser_savepath->setObjectName(QStringLiteral("textBrowser_savepath"));
        textBrowser_savepath->setGeometry(QRect(105, 30, 151, 31));
        SelectPath = new QPushButton(centralWidget);
        SelectPath->setObjectName(QStringLiteral("SelectPath"));
        SelectPath->setGeometry(QRect(270, 30, 75, 31));
        textBrowser_date = new QTextBrowser(centralWidget);
        textBrowser_date->setObjectName(QStringLiteral("textBrowser_date"));
        textBrowser_date->setGeometry(QRect(390, 30, 131, 31));
        label_project = new QLabel(centralWidget);
        label_project->setObjectName(QStringLiteral("label_project"));
        label_project->setGeometry(QRect(560, 30, 51, 31));
        textBrowser_projectname = new QTextBrowser(centralWidget);
        textBrowser_projectname->setObjectName(QStringLiteral("textBrowser_projectname"));
        textBrowser_projectname->setGeometry(QRect(610, 30, 151, 31));
        label_slr_preview = new QLabel(centralWidget);
        label_slr_preview->setObjectName(QStringLiteral("label_slr_preview"));
        label_slr_preview->setGeometry(QRect(150, 80, 101, 31));
        label_hsc_preview = new QLabel(centralWidget);
        label_hsc_preview->setObjectName(QStringLiteral("label_hsc_preview"));
        label_hsc_preview->setGeometry(QRect(560, 80, 101, 31));
        camera_slr_1 = new QWidget(centralWidget);
        camera_slr_1->setObjectName(QStringLiteral("camera_slr_1"));
        camera_slr_1->setGeometry(QRect(70, 130, 331, 131));
        camera_slr_2 = new QWidget(centralWidget);
        camera_slr_2->setObjectName(QStringLiteral("camera_slr_2"));
        camera_slr_2->setGeometry(QRect(69, 320, 331, 131));
        camara_hsc_1 = new QWidget(centralWidget);
        camara_hsc_1->setObjectName(QStringLiteral("camara_hsc_1"));
        camara_hsc_1->setGeometry(QRect(479, 129, 281, 121));
        camera_hsc_2 = new QWidget(centralWidget);
        camera_hsc_2->setObjectName(QStringLiteral("camera_hsc_2"));
        camera_hsc_2->setGeometry(QRect(479, 320, 301, 131));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 820, 23));
        menu = new QMenu(menuBar);
        menu->setObjectName(QStringLiteral("menu"));
        menuReview = new QMenu(menu);
        menuReview->setObjectName(QStringLiteral("menuReview"));
        menu_2 = new QMenu(menuBar);
        menu_2->setObjectName(QStringLiteral("menu_2"));
        menu_3 = new QMenu(menuBar);
        menu_3->setObjectName(QStringLiteral("menu_3"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menuBar->addAction(menu_2->menuAction());
        menuBar->addAction(menu_3->menuAction());
        menu->addAction(actionNew_project);
        menu->addAction(actionOpen_project);
        menu->addAction(actionSave_as);
        menu->addSeparator();
        menu->addAction(actionCameraSet);
        menu->addAction(menuReview->menuAction());
        menuReview->addAction(actionFirstPage);
        menuReview->addAction(actionLastPage);
        menuReview->addAction(actionPreviousPage);
        menuReview->addAction(actionNextPage);
        menu_2->addAction(actionAll_Camara);
        menu_2->addAction(actionSLR_1);
        menu_2->addAction(actionSLR_2);
        menu_2->addAction(actionSLR_3);
        menu_2->addAction(actionSLR_4);
        menu_2->addSeparator();
        menu_2->addAction(actionSLR_Preview);
        menu_2->addAction(actionSLR_Stop_View);
        menu_2->addAction(actionSLR_Start_Capture);
        menu_2->addAction(actionSLR_Stop_Capture);
        menu_3->addAction(actionAll_HCS_Camera);
        menu_3->addAction(actionHSC_1);
        menu_3->addAction(actionHSC_7);
        menu_3->addAction(actionHSC_8);
        menu_3->addAction(actionHSC_9);
        menu_3->addSeparator();
        menu_3->addAction(actionHSC_Preview);
        menu_3->addAction(actionHSC_Stop_View_2);
        menu_3->addAction(actionHSC_Start_Capture_2);
        menu_3->addAction(actionHSC_Stop_Capture_2);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionNew_project);
        mainToolBar->addAction(actionOpen_project);
        mainToolBar->addAction(actionSave_as);
        mainToolBar->addAction(actionCameraSet);
        mainToolBar->addAction(actionAll_Camara);
        mainToolBar->addAction(actionSLR_Preview);
        mainToolBar->addAction(actionSLR_Stop_View);
        mainToolBar->addAction(actionSLR_Start_Capture);
        mainToolBar->addAction(actionSLR_Stop_Capture);
        mainToolBar->addAction(actionAll_HCS_Camera);
        mainToolBar->addAction(actionHSC_Preview);
        mainToolBar->addAction(actionHSC_Stop_View_2);
        mainToolBar->addAction(actionHSC_Start_Capture_2);
        mainToolBar->addAction(actionHSC_Stop_Capture_2);
        mainToolBar->addAction(actionPreviousPage);
        mainToolBar->addAction(actionNextPage);
        mainToolBar->addAction(actionFirstPage);
        mainToolBar->addAction(actionLastPage);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "mainwindow", Q_NULLPTR));
        actionNew_project->setText(QApplication::translate("MainWindow", "New", Q_NULLPTR));
        actionOpen_project->setText(QApplication::translate("MainWindow", "Open", Q_NULLPTR));
        actionSave_as->setText(QApplication::translate("MainWindow", "Save", Q_NULLPTR));
        actionCameraSet->setText(QApplication::translate("MainWindow", "CameraSet", Q_NULLPTR));
        actionFirstPage->setText(QApplication::translate("MainWindow", "First", Q_NULLPTR));
        actionLastPage->setText(QApplication::translate("MainWindow", "Last", Q_NULLPTR));
        actionPreviousPage->setText(QApplication::translate("MainWindow", "Previous", Q_NULLPTR));
        actionNextPage->setText(QApplication::translate("MainWindow", "Next", Q_NULLPTR));
        actionSLR_1->setText(QApplication::translate("MainWindow", "\345\215\225\345\217\215\347\233\270\346\234\2721", Q_NULLPTR));
        actionSLR_2->setText(QApplication::translate("MainWindow", "\345\215\225\345\217\215\347\233\270\346\234\2722", Q_NULLPTR));
        actionSLR_3->setText(QApplication::translate("MainWindow", "\345\215\225\345\217\215\347\233\270\346\234\2723", Q_NULLPTR));
        actionSLR_4->setText(QApplication::translate("MainWindow", "\345\215\225\345\217\215\347\233\270\346\234\2724", Q_NULLPTR));
        actionSLR_Preview->setText(QApplication::translate("MainWindow", "SLR  Preview", Q_NULLPTR));
        actionSLR_Stop_View->setText(QApplication::translate("MainWindow", "SLR  Stop View", Q_NULLPTR));
        actionSLR_Start_Capture->setText(QApplication::translate("MainWindow", "SLR  Start Capture ", Q_NULLPTR));
        actionSLR_Stop_Capture->setText(QApplication::translate("MainWindow", "SLR  Stop Capture", Q_NULLPTR));
        actionHSC_1->setText(QApplication::translate("MainWindow", "\351\253\230\351\200\237\347\233\270\346\234\2721", Q_NULLPTR));
        actionHSC_7->setText(QApplication::translate("MainWindow", "\351\253\230\351\200\237\347\233\270\346\234\2722", Q_NULLPTR));
        actionHSC_8->setText(QApplication::translate("MainWindow", "\351\253\230\351\200\237\347\233\270\346\234\2723", Q_NULLPTR));
        actionHSC_9->setText(QApplication::translate("MainWindow", "\351\253\230\351\200\237\347\233\270\346\234\2724", Q_NULLPTR));
        actionHSC_Preview->setText(QApplication::translate("MainWindow", "HSC Preview", Q_NULLPTR));
        actionHSC_Stop_View_2->setText(QApplication::translate("MainWindow", "HSC  Stop View", Q_NULLPTR));
        actionHSC_Start_Capture_2->setText(QApplication::translate("MainWindow", "HSC Start Capture ", Q_NULLPTR));
        actionHSC_Stop_Capture_2->setText(QApplication::translate("MainWindow", "HSC   Stop Capture", Q_NULLPTR));
        actionAll_Camara->setText(QApplication::translate("MainWindow", "\345\205\250\351\203\250\345\215\225\345\217\215\347\233\270\346\234\272", Q_NULLPTR));
        actionAll_HCS_Camera->setText(QApplication::translate("MainWindow", "\345\205\250\351\203\250\351\253\230\351\200\237\347\233\270\346\234\272", Q_NULLPTR));
        label_savepath->setText(QApplication::translate("MainWindow", "\345\255\230\345\202\250\350\267\257\345\276\204", Q_NULLPTR));
        label_date->setText(QApplication::translate("MainWindow", "\346\227\245\346\234\237", Q_NULLPTR));
        SelectPath->setText(QApplication::translate("MainWindow", "\346\233\264\346\224\271\350\267\257\345\276\204", Q_NULLPTR));
        label_project->setText(QApplication::translate("MainWindow", "\345\267\245\347\250\213\345\220\215", Q_NULLPTR));
        label_slr_preview->setText(QApplication::translate("MainWindow", "\345\215\225\345\217\215\347\233\270\346\234\272\351\242\204\350\247\210", Q_NULLPTR));
        label_hsc_preview->setText(QApplication::translate("MainWindow", "\351\253\230\346\225\260\347\233\270\346\234\272\351\242\204\350\247\210", Q_NULLPTR));
        menu->setTitle(QApplication::translate("MainWindow", "\347\263\273\347\273\237\350\256\276\347\275\256", Q_NULLPTR));
        menuReview->setTitle(QApplication::translate("MainWindow", "Review", Q_NULLPTR));
        menu_2->setTitle(QApplication::translate("MainWindow", "\345\215\225\345\217\215\347\233\270\346\234\272", Q_NULLPTR));
        menu_3->setTitle(QApplication::translate("MainWindow", "\351\253\230\351\200\237\347\233\270\346\234\272", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
