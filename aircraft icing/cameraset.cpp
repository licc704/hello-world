#include "Cameraset.h"
#include "ui_Cameraset.h"

Cameraset::Cameraset(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Cameraset)
{
	ui->setupUi(this);
}

Cameraset::~Cameraset()
{
	delete ui;
}