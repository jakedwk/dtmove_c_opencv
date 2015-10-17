#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include <time.h> 
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

int main( int argc, char** argv ){
    Rect ret;
    time_t now_time;
    struct tm *p;
    char fmt_time[100];
    String svtime;
    vector<vector<Point> > contours;
    Scalar color = Scalar( 0, 255, 0),mean;
    Mat gray,avg,differ,frame,frameold,bigger,thresh;
    Mat element = getStructuringElement( 0,Size( 3, 3 ), Point(1, 1 ) );
    bool occ = 0;
    int times = 0;
    VideoCapture cap(-1);

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

    cap >> frame;
    cvtColor(frame, avg, COLOR_BGR2GRAY);
    GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
    for(;;)
    {
        frameold = gray.clone();
        cap >> frame; // get a new frame from camera
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        GaussianBlur(gray, gray, Size(7,7), 1.5, 1.5);
        if ( (waitKey(10)&0xFF) == 103)
        {
            cap >> avg;
            cvtColor(avg, avg, COLOR_BGR2GRAY);
            GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
        }
        absdiff(gray,avg,differ);
        threshold(differ,thresh, 40, 255, THRESH_BINARY);
        dilate(thresh,bigger, element,Point(-1,-1), 3);
        findContours(bigger,contours, RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
        for(unsigned int j = 0; j < contours.size(); j++ )
        {
            if (contourArea(contours[j])<1000) continue;
            occ = 1;
            ret = boundingRect(contours[j]);
            rectangle(frame,ret,color,2);
            if (times > 6)
            {
                times = 0;
                absdiff(gray,frameold,frameold); 
                mean = cv::mean(frameold);   
                if  ( mean[0] < 2 ) 
                {
                    //cout <<"frameold" << mean[0] << endl;
                    cap >> avg;
                    cvtColor(avg, avg, COLOR_BGR2GRAY);
                    GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
                }
            }
            times++;
        }
        mean = cv::mean(differ); //名称空间重复故使用cv::
        if ((occ == 0)&(mean[0] > 2))
        {
            cap >> avg;
            cvtColor(avg, avg, COLOR_BGR2GRAY);
            GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
        }
        occ = 0;
        imshow("frame", frame);
        imshow("avg", avg);
        imshow("bigger", bigger);
        now_time = time(NULL);
        //cout<<now_time<<endl;
        p=gmtime(&now_time);
        strftime(fmt_time, sizeof(fmt_time), "%Y_%m_%d_%H_%M_%S", p);
        //cout<<fmt_time<<endl;
        if((waitKey(20)&0xFF) == 115)
        {
            svtime = format("images/%s.jpg",fmt_time);
            cout << svtime << endl;
            imwrite(svtime,frame);
        }
        if((waitKey(20)&0xFF) == 113) break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
