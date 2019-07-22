#include "Preview.h"
#include "ui_Preview.h"
#include <QDebug>
#include <QStringList>
Preview::Preview(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Preview)
{
	ui->setupUi(this);

	
}

Preview::~Preview()
{
	delete ui;
}
void Preview::on_comboBoxcameraconnect_currentIndexChanged(QString)
{
	qDebug() << "相机连接";

	QString curStr = ui->comboBoxcameraconnect->currentText();
	qDebug() << curStr;
}
void Preview::on_comboBoxpreview_currentIndexChanged(QString)
{
	qDebug() << "实时预览";
	QString curStr = ui->comboBoxpreview->currentText();
	qDebug() << curStr;
}
void Preview::on_comboBoxcapture_currentIndexChanged(QString)
{
	qDebug() << "图像采集";
	QString curStr = ui->comboBoxcapture->currentText();
	qDebug() << curStr;
}
void Preview::on_comboBoxautofocus_currentIndexChanged(QString)
{
	qDebug() << "自动聚焦";
	QString curStr = ui->comboBoxautofocus->currentText();
	qDebug() << curStr;
}
void Preview::on_comboBoxiso_currentIndexChanged(QString)
{
	qDebug() << "ISO设置";
	QString curStr = ui->comboBoxiso->currentText();
	qDebug() << curStr;
}
