#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include <time.h> 
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

class Dtmove
{
    Rect ret;
    time_t now_time;
    struct tm *p;
    char fmt_time[100];
    bool occ;
    int times;
    String svtime,path;
    VideoCapture cap;
    vector<vector<Point> > contours;
    Scalar color,mean;
    Mat gray,avg,differ,frame,frameold,bigger,thresh;
    Mat element ;

public:
    Dtmove(int i,String pas);
    int ckcamera();
    void start();
};

Dtmove::Dtmove(int i ,String pas= "./images/")
{
   path =pas;
    cap = VideoCapture(i);
    occ = 0;
    color = Scalar( 0, 255, 0);
    element = getStructuringElement( 0,Size( 3, 3 ), Point(1, 1 ) );

}
int Dtmove::ckcamera()
{
    if(!cap.isOpened())  // check if we succeeded
    {
        cout<<"设备打开错误";
        return -1;
    }
    for(int i=50;i>=0;i--)
    {
        cap >> frame;
        waitKey(10);
    }
    return 1;
}
void Dtmove::start()
{

    ckcamera();
    //初始化背景帧
    cap >> frame;
    cvtColor(frame, avg, COLOR_BGR2GRAY);
    GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
    gray = avg.clone();
    frameold = gray.clone();
    
    for(;;)     //主循环
    {
        cap >> frame;                                        // 获取一帧图像
        cvtColor(frame, gray, COLOR_BGR2GRAY);               //转化为灰度图像
        GaussianBlur(gray, gray, Size(7,7), 1.5, 1.5);       //高斯模糊
        absdiff(gray,avg,differ);                            //比较两幅图片结果放入differ中
        threshold(differ,thresh, 40, 255, THRESH_BINARY);    //根据给出的阈值二值化
        dilate(thresh,bigger, element,Point(-1,-1), 3);      //膨胀图像
        findContours(bigger,contours, RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);  //寻找轮廓
        for(unsigned int j = 0; j < contours.size(); j++ )
        {
            if (contourArea(contours[j])<1000) continue;
            occ = 1;
            ret = boundingRect(contours[j]);
            rectangle(frame,ret,color,2);
        }
        //重新建立背景
        if (occ==0)
        {
            mean = cv::mean(differ); //名称空间重复故使用cv::
            cout<<"m"<<mean[0]<<endl;
            if (mean[0] > 2)
            {
                cap >> avg;
                cvtColor(avg, avg, COLOR_BGR2GRAY);
                GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
            }
        }else
        {
            if (times > 30)
            {
                /*重新建立背景*/
                times = 0;
                absdiff(gray,frameold,frameold); 
                mean = cv::mean(frameold);   //名称空间重复故使用cv::
                cout<<"f"<<mean<<endl;
                if  ( mean[0] < 2 ) 
                {
                    cap >> avg;
                    cvtColor(avg, avg, COLOR_BGR2GRAY);
                    GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
                }
                frameold = gray.clone();
            }
            times++;
            occ = 0;
        }
        imshow("frame", frame);
        imshow("avg", avg);
        imshow("thresh", thresh);
        now_time = time(NULL);
        p=localtime(&now_time);
        strftime(fmt_time, sizeof(fmt_time), "%Y_%m_%d_%H_%M_%S", p);
        if((waitKey(20)&0xFF) == 115)
        {
            svtime = path + format("%s.jpg",fmt_time);
            cout << svtime << endl;
            imwrite(svtime,frame);
        }
        if((waitKey(20)&0xFF) == 113) break;
    }

    cap.release();
    destroyAllWindows();
}
int main( int argc, char** argv ){
    Dtmove dt(1);
    dt.start();
}
