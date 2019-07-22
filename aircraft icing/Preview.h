#ifndef PREVIEW_H
#define PREVIEW_H
#include <QtWidgets/QMainWindow>
#pragma execution_character_set("UTF-8")
#include <QWidget>
//#include <QWidget>
#include "NKCamera.h"
#include "ui_Preview.h"
#include <QTimer>

namespace Ui {
	class Preview ;
} // namespace Ui

class Preview : public QWidget
{
	Q_OBJECT

public:
	
	explicit Preview(QWidget *parent = 0);
	~Preview();
	
private:
	Ui::Preview *ui;
//	QTimer * m_timerNKGrapImage;
//	QTimer * m_timerNKViewImageImage;
	NKCamera m_NKCamera;
private slots:
	void on_comboBoxcameraconnect_currentIndexChanged(QString);
	void on_comboBoxpreview_currentIndexChanged(QString);
	void on_comboBoxcapture_currentIndexChanged(QString);
	void on_comboBoxautofocus_currentIndexChanged(QString);
	void on_comboBoxiso_currentIndexChanged(QString);
	
};

#endif // PREVIEW_H


