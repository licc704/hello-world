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
	qDebug() << "�������";

	QString curStr = ui->comboBoxcameraconnect->currentText();
	qDebug() << curStr;
}
void Preview::on_comboBoxpreview_currentIndexChanged(QString)
{
	qDebug() << "ʵʱԤ��";
	QString curStr = ui->comboBoxpreview->currentText();
	qDebug() << curStr;
}
void Preview::on_comboBoxcapture_currentIndexChanged(QString)
{
	qDebug() << "ͼ��ɼ�";
	QString curStr = ui->comboBoxcapture->currentText();
	qDebug() << curStr;
}
void Preview::on_comboBoxautofocus_currentIndexChanged(QString)
{
	qDebug() << "�Զ��۽�";
	QString curStr = ui->comboBoxautofocus->currentText();
	qDebug() << curStr;
}
void Preview::on_comboBoxiso_currentIndexChanged(QString)
{
	qDebug() << "ISO����";
	QString curStr = ui->comboBoxiso->currentText();
	qDebug() << curStr;
}
