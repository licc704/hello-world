#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma execution_character_set("UTF-8")
#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "qtxmlfile.h"
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	 MainWindow(QWidget *parent = Q_NULLPTR);
	 ~MainWindow();

private slots:
		void on_actionNew_project_triggered();
		void on_SelectPath_clicked();
	//	void on_actionOpen_project_triggered();
	    void on_actionCameraSet_triggered();
		void on_actionSLR_Preview_triggered();
private:
	Ui::MainWindow *ui;
	void init();

	
};

#endif // MAINWINDOW_H