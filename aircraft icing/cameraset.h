#ifndef CAMERASET_H
#define CAMERASET_H
#pragma execution_character_set("UTF-8")
#include <QtWidgets/QMainWindow>

//#include <QWidget>

//#include "ui_Cameraset.h"

namespace Ui {
	class Cameraset;
} // namespace Ui

class Cameraset : public QWidget
{
	Q_OBJECT

public:
	explicit Cameraset(QWidget *parent = 0);
	~Cameraset();

private:
	Ui::Cameraset *ui;



};

#endif // CAMERASET_H
