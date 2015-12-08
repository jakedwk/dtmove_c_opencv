#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex> 

#define SERVPORT 5788
#define BACKLOG 10
#define MAXSIZE 921600
#define ADDRESS "192.168.0.116"
//#define ADDRESS "127.0.0.1"
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
    char key;
    String svtime,path;  //保存路径与文件名
    VideoCapture cap;
    vector<vector<Point> > contours;
    Scalar color,mean;
    Mat gray,avg,differ,frame,frameold,bigger,thresh;
    Mat element ;
    int sockfd,client_fd,new_server_socket;
    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    struct sockaddr_in their_addr;
    vector<uchar> buff;//buffer for coding
    vector<int> param;
    unsigned char buf[MAXSIZE],sbuf[MAXSIZE],newbuf;
    unsigned int datasize;
    int freeman;
    struct timeval tpstart,tpend; 
    float timeuse; 
    uint vsize;
    vector<uchar> vbuf;
    mutex mtx;

public:
    Dtmove();
    ~Dtmove();
    int ckcamera();
    void socketinit();
    int accept_m();
    void senddata();
    void recvdata();
    void recvall(int socket,void *ptr,uint len);
    void sender();
    void server();
    void server_send(int new_socket);
    void server_receive(int new_socket);
    void start(int i ,String pas);
    void client();
    void clientinit();
};

Dtmove::Dtmove()
{
    param = vector<int>(2);
    freeman = 0;
    vsize = 0;
}
Dtmove::~Dtmove()
{
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
void Dtmove::clientinit()
{
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    their_addr.sin_family = AF_INET; /* host byte order */
    their_addr.sin_port = htons(SERVPORT); /* short, network byte order */
    their_addr.sin_addr.s_addr =inet_addr(ADDRESS); 
    bzero(&(their_addr.sin_zero),8); /* zero the rest of the struct */
    if(connect(sockfd,(struct sockaddr *)&their_addr,sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }
}

void Dtmove::socketinit()
{
    //创建套接字
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
        perror("socket create failed!");
        exit(1);
    }

    //绑定端口地址
    my_addr.sin_family      = AF_INET;
    my_addr.sin_port        = htons(SERVPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero),8);
    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind error!");
        exit(1);
    }

    //监听端口  
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen error");
        exit(1);
    }
}
int Dtmove::accept_m()
{
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    int new_server_socket = accept(sockfd,(struct sockaddr*)&client_addr,&length);
    if ( new_server_socket < 0)
    {
        printf("Server Accept Failed!\n");
        exit(1);  
    }
    return new_server_socket;
}    

void Dtmove::senddata()
{
    uint pic,lst;
    uchar *dataptr = NULL;
        //发送
    lst = vsize%1920;
    pic = vsize/1920;
    send(sockfd,&vsize,4,0);
    for(uint i=0;i<pic;i++)
    {
        send(sockfd,dataptr,1920,0);
        dataptr+=1920;
    }
    if(lst)
        send(sockfd,dataptr,lst,0); 
}
void Dtmove::recvdata()
{
    uint num=0;
    int numbytes;
    uchar *dataptr = NULL;
    if ((numbytes=recv(sockfd,&vsize, 4, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    vbuf.resize(vsize);
    dataptr = &vbuf[0];
    while(1)
    {
        if ((numbytes=recv(sockfd,dataptr, vsize-num, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        dataptr+=numbytes;
        num+=numbytes;
        if(num==vsize) break;
    }
}
void Dtmove::recvall(int socket,void *ptr,uint len)
{
    int numbytes;
    uchar *dataptr = (uchar *)ptr;
    while(len)
    {
        if ((numbytes=recv(socket,dataptr, len, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        if(numbytes == 0) break;
        cout<<"recving!"<<endl;
        dataptr+=numbytes;
        len-=numbytes;
    }
}
void Dtmove::server_send(int new_socket)
{
    mtx.lock();
    senddata();
    mtx.unlock();
}
void Dtmove::server_receive(int new_socket)
{
    mtx.lock();
    recvdata();
    mtx.unlock();
}
void Dtmove::server()
{
    uint linker=0;
    vector<std::thread> threads;
    vector<int> sockets;
    socketinit();
    while(1)
    {
        cout<<"run!"<<endl;
        sockets.push_back(accept_m());
        cout<<"accepted!"<<endl;
        recvall(sockets.back(),&linker,4);
        cout<<"recived!"<<endl;
        cout<<linker<<endl;
        if(linker == 0x05)
            threads.push_back(std::thread(&Dtmove::server_receive,this,sockets.back()));
        if(linker == 0x50)
            threads.push_back(std::thread(&Dtmove::server_send,this,sockets.back()));
        if(linker == 0x55) break;

    }
    std::for_each(sockets.begin(),sockets.end(),close);
    std::for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join));

}


void Dtmove::sender()
{
    param[0]=CV_IMWRITE_JPEG_QUALITY;
    param[1]=95;
    //gettimeofday(&tpstart,NULL);
    imencode(".jpg",frame,vbuf,param);
    //gettimeofday(&tpend,NULL); 
    vsize = vbuf.size();
    senddata();
}

void Dtmove::client()
{
    int sorr=0x50;
    Mat img(Size(640,480),CV_8UC3) ;
    clientinit();
    send(sockfd,&sorr,4,0);
    while(1)
    {
        recvdata();
        img = imdecode(Mat(vbuf),CV_LOAD_IMAGE_COLOR);
        imshow("img",img);
        key = waitKey(1);
        if (key == 'q') break;
    }
    close(sockfd);
}
void Dtmove::start(int i ,String pas= "./images/")
{
    int sorr=0x05;
    clientinit();
    send(sockfd,&sorr,4,0);
    path =pas;
    cap = VideoCapture(i);
    cap >> frame;
    datasize = frame.rows*frame.cols*frame.channels();
    occ = 0;
    color = Scalar( 0, 255, 0);
    element = getStructuringElement( 0,Size( 3, 3 ), Point(1, 1 ) );
    ckcamera();
    //初始化背景帧
    cap >> frame;
    cvtColor(frame, avg, COLOR_BGR2GRAY);
    GaussianBlur(avg, avg, Size(7,7), 1.5, 1.5);
    gray = avg.clone();
    frameold = gray.clone();
    
    while(1)    //主循环
    {
        cap >> frame;                                        // 获取一帧图像
        cvtColor(frame, gray, COLOR_BGR2GRAY);               //转化为灰度图像
        GaussianBlur(gray, gray, Size(7,7), 1.5, 1.5);       //高斯模糊
        absdiff(gray,avg,differ);                            //比较两幅图片结果放入differ中
        threshold(differ,thresh, 40, 255, THRESH_BINARY);    //根据给出的阈值二值化
        dilate(thresh,bigger, element,Point(-1,-1), 1);      //膨胀图像
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
            times = 0;
            mean = cv::mean(differ); //名称空间重复故使用cv::
            //cout<<"m"<<mean[0]<<endl;
            if (mean[0] > 2)
            {
                avg = gray.clone();
            }
        }else
        {
            if (times > 30)
            {
                /*重新建立背景*/
                times = 0;
                absdiff(gray,frameold,frameold); 
                mean = cv::mean(frameold);   //名称空间重复故使用cv::
                //cout<<"f"<<mean[0]<<endl;
                if  ( mean[0] < 2 ) 
                {
                    avg = gray.clone();
                }
                frameold = gray.clone();
            }
            times++;
            occ = 0;
        }
        //显示图片
        sender();
        //imshow("frame", frame);
        //imshow("avg", avg);
        //imshow("thresh", thresh);
        //imshow("differ", differ);

        //key = waitKey(25)&0xFF;
        //if (key == 's')
        //{
            //now_time = time(NULL);
            //p=localtime(&now_time);
            //strftime(fmt_time, sizeof(fmt_time), "%Y_%m_%d_%H_%M_%S", p);
            //svtime = path + format("%s.jpg",fmt_time);
            //cout << svtime << endl;
            //imwrite(svtime,frame);
        //}
        //if(key == 'q') break;
        //timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
        //tpend.tv_usec-tpstart.tv_usec; 
        //timeuse/=1000000; 
        //cout<<timeuse<<endl;
    }

    //destroyWindow("frame");
    cap.release();
    close(sockfd);

}
int main( int argc, char** argv ){
    Dtmove dt;
    //dt.start(-1);
    //dt.client();
    dt.server();
}
