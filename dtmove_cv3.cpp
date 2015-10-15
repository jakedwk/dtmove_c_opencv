#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

int main( int argc, char** argv ){
    Rect ret;
    vector<vector<Point> > contours;
    Mat gray,avg,differ,frame,frameold,bigger,thresh;
    Scalar color = Scalar( 0, 255, 0),mean;
    Mat element = getStructuringElement( 0,Size( 3, 3 ), Point(1, 1 ) );
    bool occ = 0;
    int times = 0;
    VideoCapture cap(1);

    if(!cap.isOpened())  // check if we succeeded
        return -1;
    for(int i=50;i>=0;i--)
    {
        cap >> frame;
        waitKey(30);
    }

    cap >> frame;
    cvtColor(frame, avg, COLOR_BGR2GRAY);
    cvtColor(frame, differ, COLOR_BGR2GRAY);
    cvtColor(frame, bigger, COLOR_BGR2GRAY);
    cvtColor(frame, frameold, COLOR_BGR2GRAY);
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
    GaussianBlur(gray, gray, Size(7,7), 1.5, 1.5);
    namedWindow("differ",1);
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
        imshow("bigger",bigger);
        findContours(bigger,contours, RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
        for(unsigned int j = 0; j < contours.size(); j++ )
        {
            if (contourArea(contours[j])<1000) continue;
            occ = 1;
            ret = boundingRect(contours[j]);
            rectangle(frame,ret,color,2);
            if (times > 3)
            {
                times = 0;
                absdiff(gray,frameold,frameold); 
                mean = cv::mean(frameold);   
                if  ( mean[0] < 2 ) 
                {
                    cout <<"frameold" << mean[0] << endl;
                    cap >> avg;
                    cvtColor(avg, avg, COLOR_BGR2GRAY);
                    GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
                }
            }
            times++;
        }
        mean = cv::mean(differ);
        if ((occ == 0)&(mean[0] > 2))
        {
            cout <<"differ" << mean[0] << endl;
            cap >> avg;
            cvtColor(avg, avg, COLOR_BGR2GRAY);
            GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
        }
        occ = 0;
        imshow("avg", avg);
        imshow("frame", frame);
        imshow("gray", bigger);
        if((waitKey(20)&0xFF) == 113) break;
    }

    cap.release();
    destroyWindow("differ");
    return 0;
}
