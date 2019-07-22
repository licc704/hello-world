#ifndef QTXMLFILE_H
#define QTXMLFILE_H
//#include "QString"
#include "ImageData_struct.h"
#include <QVector>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QtXml/QDomProcessingInstruction>
#include <QString>
class QtXmlfile
{
public:
	QtXmlfile();
	~QtXmlfile();
	bool CreatXmlfile(QString filename);
	bool InsertElement(QString filename, QString parentElement, ImageData imageData);
    bool ReadProjectfile(QString filename, QString parentElement, QVector<ImageData>& data);
	bool DeleteElement(QString filename, QString parentElement, ImageData imageData);


	QDomElement* ImageData2Element(ImageData imageData);

	ImageData Element2ImageData(QDomElement element);
};

#endif // QTXMLFILE_H
