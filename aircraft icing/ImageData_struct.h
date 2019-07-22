#ifndef DE_H
#define DE_H
//#include "QString"
#include <QString>
//导入外部库
#ifdef _DEBUG
//#pragma comment(lib,"opencv_world330d.lib")
//#include <opencv2/opencv.hpp>
#else
//#pragma comment(lib,"opencv_world330.lib")
#endif //_DEBUG


struct ImageData
{
	QString Imagename;	//图片名字
	QString time;		//图片存储时间
						//时间保存格式"yyyy_MM_dd hh:mm:ss:zzz"
	int id;				//图片序列号
	QString Camera;		//图片来源相机
	//ImageData() {}
};

#endif // DE_H
