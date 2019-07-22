#include "mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include "Preview.h"
#include "Cameraset.h"
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}
MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_SelectPath_clicked()
{
	QString fileSavePath = ui->textBrowser_savepath->toPlainText();
	QString file_path = QFileDialog::getExistingDirectory(this, "请选择工程保存路径...", fileSavePath);
	if (file_path.isEmpty())
	{
		return;
	}
	else
	{
		ui->textBrowser_savepath->setText(file_path);
		qDebug() << file_path << endl;
	}
}

void MainWindow::on_actionNew_project_triggered()
{
	QString Projectpath;
	Projectpath = ui->textBrowser_savepath->toPlainText() + "/" + ui->textBrowser_date->toPlainText() + "/" + ui->textBrowser_projectname->toPlainText();
	QDir dir(Projectpath);
	if (!dir.exists())
	{
		dir.mkpath(Projectpath);        //创建多级目录
		dir.mkpath(Projectpath + "/SLR1");//创建多级目录
		dir.mkpath(Projectpath + "/SLR2");//创建多级目录
		dir.mkpath(Projectpath + "/HSC1");//创建多级目录
		dir.mkpath(Projectpath + "/HSC2");//创建多级目录
	}
	QtXmlfile* pxmlfile = new QtXmlfile;
	pxmlfile->CreatXmlfile(Projectpath + "/" + ui->textBrowser_projectname->toPlainText() + ".xml");
}
void MainWindow::on_actionSLR_Preview_triggered()
{
	Preview* pPreview = new Preview;
	pPreview->show();
}

void MainWindow::on_actionCameraSet_triggered()
{
	Cameraset* pCameraset = new Cameraset;
	pCameraset->show();
}