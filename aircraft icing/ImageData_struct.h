#ifndef DE_H
#define DE_H
//#include "QString"
#include <QString>
//�����ⲿ��
#ifdef _DEBUG
//#pragma comment(lib,"opencv_world330d.lib")
//#include <opencv2/opencv.hpp>
#else
//#pragma comment(lib,"opencv_world330.lib")
#endif //_DEBUG


struct ImageData
{
	QString Imagename;	//ͼƬ����
	QString time;		//ͼƬ�洢ʱ��
						//ʱ�䱣���ʽ"yyyy_MM_dd hh:mm:ss:zzz"
	int id;				//ͼƬ���к�
	QString Camera;		//ͼƬ��Դ���
	//ImageData() {}
};

#endif // DE_H
