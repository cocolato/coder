/////This is a code of face_test and communction with arduino.
/////The face detection code is mainly used by opencv to process image information.
/////The basic idea of face detection is to train and invoke cascaded classifiers based on face samples.
/////The part communicating with arduino is based on the serial communication function in Windows.
/////The face detection part is completed by Mao Yuxi, the serial communication part was completed by Tang Hengxuan.
/////Shall we begin!

//---------------------------------【头文件、命名空间包含部分】----------------------------
//		描述：包含程序所使用的头文件和命名空间
//-------------------------------------------------------------------------------------------------
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "stdlib.h"
#include "windows.h"
#include <iostream>
#include <stdio.h>
#include <ctime>
using namespace std;
using namespace cv;
//--------------------------------------------------------------------------------------------------


//-----------------------------【三个自定义函数的声明】---------------------------------------
int serial_connect(HANDLE& hCom, wchar_t* serialport, int baudrate);  //串口连接函数
int Write(HANDLE hCom, char* lpOutBuffer, int length);                //写串口的函数
void Delay(int time);                                               //延时函数、、、emmm貌似没有起到理想的效果
//-------------------------------------------------------------------------------------------------


//--------------------------------【全局变量声明】----------------------------------------------
//		描述：声明全局变量
//-------------------------------------------------------------------------------------------------
//注意，需要把"haarcascade_frontalface_alt.xml"和"haarcascade_eye_tree_eyeglasses.xml"这两个文件复制到工程路径下
String face_cascade_name = "haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
string window_name = "Capture - Face detection";
RNG rng(12345);
int signl;
HANDLE hCom;//串口句柄
char* p = "1";// 与OpenCV结合在一起时，则改为char* p=judge，其中judge为字符型变量（judge为1慢慢变亮，为0慢慢变暗）
char* q = "2";// 与OpenCV结合在一起时，则改为char* p=judge，其中judge为字符型变量（judge为1慢慢变亮，为0慢慢变暗）

//int connectstate = serial_connect(hCom, L"COM7", CBR_9600);//连接到COM6，波特率9600（这里的COM视具体情况改变数字）
//--------------------------------------------------------------------------------------


//-----------------------------------【main( )函数】--------------------------------------------
//		描述：控制台应用程序的入口函数，我们的程序从这里开始
//-------------------------------------------------------------------------------------------------
int main(void)
{
	VideoCapture capture;    //VideoCapture是OpenCV中的一个class，这句代码的意思是对capture实例化
							 //见http://blog.csdn.net/weicao1990/article/details/53379881
	Mat frame;               //定义一个Mat类型的叫frame的变量
	Mat frame_gray;
	std::vector<Rect> faces;
	//-- 1. 加载级联（cascades）
	if (!face_cascade.load(face_cascade_name)) { printf("--(!)Error loading\n"); return -1; };
	if (!eyes_cascade.load(eyes_cascade_name)) { printf("--(!)Error loading\n"); return -1; };
	//-- 2. 读取视频并进行人脸检测
	capture.open(0);
	//if (capture.isOpened())   //如果摄像头打开了，就进行识别、串口等操作,现在不同判断了....减轻算法速度压力
	//{
		for (;;)              //无头无尾的循环，让人脸识别一直进行下去
		{
			capture >> frame;
			//-- 3. 对当前帧使用分类器
				cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
				equalizeHist(frame_gray, frame_gray);
			//--4. 人脸检测
				face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
				for (size_t i = 0; i < faces.size(); i++)
				{
					Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
					ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(255, 0, 255), 2, 8, 0);
				}
				imshow(window_name, frame);
				int c = waitKey(10);
				if ((char)c == 'c') { break; }
		    //--5.执行判断语句
				serial_connect(hCom, L"COM9", CBR_9600);
			    if (faces.size() > 0)
				{	
					Write(hCom, q, 8);  //q是1，表示亮
					CloseHandle(hCom);
				}
				else
				{
					Write(hCom, p, 8);
					CloseHandle(hCom);
				}
				//Delay(2 * 1000);
//======================================================【分割线】==================================================================
			}
	return 0;
}
//---------------------------【三个自定义函数】---------------------------------------------------
void Delay(int time)        //time*1000为秒数 
{
	clock_t now = clock();
	while (clock() - now   <   time);
}
int serial_connect(HANDLE& hCom, wchar_t* serialport, int baudrate) {
	hCom = CreateFile(serialport, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);//打开串口，读写方式，同步方式
	if (hCom == (HANDLE)-1) return -1;
	SetupComm(hCom, 1024, 1024); //输入缓冲区和输出缓冲区的大小都是1024 
	COMMTIMEOUTS TimeOuts; //设定读超时
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000; //设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(hCom, &TimeOuts); //设置超时
	DCB dcb;//通过dcb结构配置串口
	GetCommState(hCom, &dcb);
	dcb.BaudRate = baudrate;//设置波特率
	dcb.fParity = 0; // 指定奇偶校验使能。若此成员为1，允许奇偶校验检查 …
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY; //指定奇偶校验方法。此成员可以有下列值： EVENPARITY 偶校验 NOPARITY 无校验 MARKPARITY 标记校验 ODDPARITY 奇校验
	dcb.StopBits = ONESTOPBIT; //指定停止位的位数。此成员可以有下列值： ONESTOPBIT 1位停止位 TWOSTOPBITS 2位停止位
	SetCommState(hCom, &dcb);
	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);
	return 0;
}
int Write(HANDLE hCom, char* lpOutBuffer, int length)
{//串口发送，同步写串口
	DWORD dwBytesWrite = length;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	BOOL bWriteStat;
	ClearCommError(hCom, &dwErrorFlags, &ComStat);
	bWriteStat = WriteFile(hCom, lpOutBuffer, dwBytesWrite, &dwBytesWrite, NULL);
	if (!bWriteStat) {
		return -1;
	}
	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	return 0;
}
//------------------------------------------------------------------------------------------------
