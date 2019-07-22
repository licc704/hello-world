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
	QString file_path = QFileDialog::getExistingDirectory(this, "��ѡ�񹤳̱���·��...", fileSavePath);
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
		dir.mkpath(Projectpath);        //�����༶Ŀ¼
		dir.mkpath(Projectpath + "/SLR1");//�����༶Ŀ¼
		dir.mkpath(Projectpath + "/SLR2");//�����༶Ŀ¼
		dir.mkpath(Projectpath + "/HSC1");//�����༶Ŀ¼
		dir.mkpath(Projectpath + "/HSC2");//�����༶Ŀ¼
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