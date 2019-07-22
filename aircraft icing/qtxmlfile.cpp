#include "qtxmlfile.h"
#include <QFile>
//#include <QXmlStreamWriter>
#include <QDebug>
#include <QDir>
#include <QDomNode>

QtXmlfile::QtXmlfile()
{

}

QtXmlfile::~QtXmlfile()
{

}

bool QtXmlfile::CreatXmlfile(QString filename)
{
	QDir dir(filename);
	QFile file(filename);
	if (!file.open(QFile::WriteOnly | QFile::Text)) { // 只写模式打开文件
		qDebug() << QString("Cannot write file %1(%2).").arg(filename).arg(file.errorString());
		return false;
	}
	QDomDocument doc;
	QDomProcessingInstruction instruction;
	instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"GB2312\"");
	doc.appendChild(instruction);
	QDomElement root = doc.createElement("Airplane");
	doc.appendChild(root);
	QDomElement path_element = doc.createElement("Path");
	path_element.setAttribute("filepath", filename);
	if (!dir.cdUp())
	{
		return false;
	}
	path_element.setAttribute("path", dir.path());
	root.appendChild(path_element);

	//QDomElement camera_element = doc.createElement("Path");
	QDomElement camera_element = doc.createElement("SLR1");
	root.appendChild(camera_element);
	camera_element = doc.createElement("SLR2");
	root.appendChild(camera_element);
	camera_element = doc.createElement("HSC1");
	root.appendChild(camera_element);
	camera_element = doc.createElement("HSC2");
	root.appendChild(camera_element);
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	return true;
}

bool QtXmlfile::InsertElement(QString filename, QString parentElement, ImageData imageData)
{
	//读取xml文件
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
		qDebug() << "xml file is not exist error...";
		return false;
	}
	QDomDocument doc;
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) {
		qDebug() << "add setcontent error...";
		file.close();
		return false;
	}
	file.close();
	QDomElement root = doc.documentElement();
	if (root.tagName() != "Airplane")
	{
		qDebug() << "root.tagname != Airplane error...";
		return false;
	}

	// 创建图片信息节点
	QDomElement element_camera = *ImageData2Element(imageData);
	//QDomElement element_camera = doc.createElement(imageData.Camera);
	//element_camera.setAttribute("id",QString::number(imageData.id));//QString::number(imageData.id));
	//QDomElement element_name = doc.createElement("image");
	//element_name.setAttribute("name",imageData.Imagename);
	//element_name.setAttribute("time",imageData.time);
	//element_camera.appendChild(element_name);

	QDomNode node = root.firstChild();
	while (!node.isNull())
	{
		if (node.isElement())
		{
			QDomElement parent_element = node.toElement();
			if (parent_element.tagName() == parentElement)
			{
				//node.appendChild(element_camera);
				parent_element.appendChild(element_camera);
				if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))//|QIODevice::Append)
					qDebug() << "open for add error!";
				QTextStream out(&file);
				doc.save(out, 4);
				file.close();
				return true;
			}
		}
		node = node.nextSibling();
	}
	qDebug() << "import xml nope is not find error...";

	return false;
}

bool QtXmlfile::ReadProjectfile(QString filename, QString parentElement, QVector<ImageData>& data)
{
	//读取xml文件
	if (data.size() > 0)
	{
		data.clear();
	}
	ImageData imageData;
	data.empty();
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
		qDebug() << "xml file is not exist error...";
		return false;
	}
	QDomDocument doc;
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) {
		qDebug() << "add setcontent error...";
		file.close();
		return false;
	}
	file.close();
	QDomElement root = doc.documentElement();
	QDomElement parent_element = root.firstChildElement(parentElement);
	if (parent_element.isNull())
	{
		qDebug() << "There is no parent element";
		return false;
	}
	QDomNodeList nodeList = parent_element.childNodes();
	if (nodeList.size() == 0)
	{
		qDebug() << "There is no child element";
		return false;
	}
	for (int i = 0; i < nodeList.count(); i++)
	{
		//QDomNode nodechild = nodeList.at(i);
		QDomElement imgElement = nodeList.at(i).toElement();

		data.push_back(Element2ImageData(imgElement));
	}
	return true;
}




/*删除parentElement节点下的指定节点
*filename xml file
*/
bool QtXmlfile::DeleteElement(QString filename, QString parentElement, ImageData imageData)
{
	//读取xml文件
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
		qDebug() << "xml file is not exist error...";
		return false;
	}
	QDomDocument doc;
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) {
		qDebug() << "add setcontent error...";
		file.close();
		return false;
	}
	file.close();

	QDomElement root = doc.documentElement();
	QDomElement parent_element = root.firstChildElement(parentElement);

	if (parent_element.isNull())
	{
		qDebug() << "There is no parent element";
		return false;
	}
	QDomNodeList nodeList = parent_element.childNodes();
	if (nodeList.size() == 0)
	{
		qDebug() << "There is no child element";
		return false;
	}
	for (int i = 0; i < nodeList.count(); i++)
	{
		QDomElement imgElement = nodeList.at(i).toElement();
		if (imageData.id == imgElement.attribute("id", "-1").toInt())
		{
			parent_element.removeChild(imgElement);
		}
	}
	return true;
}

QDomElement* QtXmlfile::ImageData2Element(ImageData imageData)
{
	QDomElement* element = new QDomElement();
	element->setTagName(imageData.Camera);
	element->setAttribute("id", QString::number(imageData.id));//QString::number(imageData.id));
	QDomElement element_name;
	element_name.setTagName("image");
	element_name.setAttribute("name", imageData.Imagename);
	element_name.setAttribute("time", imageData.time);
	element->appendChild(element_name);
	return element;
}

ImageData QtXmlfile::Element2ImageData(QDomElement element)
{
	QDomElement namechild = element.firstChildElement("image");
	ImageData imageData;
	imageData.Camera = element.tagName();
	imageData.id = element.attribute("id", "1").toInt();
	imageData.Imagename = namechild.attribute("name", "1");
	imageData.time = namechild.attribute("time", "1");
	return imageData;
}