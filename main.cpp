#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cstdio>
#include <fstream> 
using namespace std;


cv::Mat ups_segmentate(cv::Mat src) {
    //void cvtColor(InputArray src, OutputArray dst, int code, int dstCn=0 )
    
    cv::Mat hsvsrc(src.rows, src.cols, CV_8UC3);
    
    cv::cvtColor(src, hsvsrc, cv::COLOR_BGR2HSV);
    
    
    cv::imshow("HSV", hsvsrc);
    cv::Mat res(src.rows, src.cols, CV_8UC1);
    
    CV_Assert(src.depth() == CV_8U);
    
    int channels = hsvsrc.channels();
    int nRows = hsvsrc.rows;
    int nCols = hsvsrc.cols;
    
    if (hsvsrc.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    uchar* p;
    unsigned char* t;
    for(int i = 0; i < nRows; ++i)
    {
        p = hsvsrc.ptr<uchar>(i);
        t = res.ptr<unsigned char>(i);
        for (int j = 0; j < nCols; ++j)
        {
            //H +0
            //S +1
            //V +2
            if (p[j*3] >= 9 && p[j*3] <= 28 // Yellow-Orange Hue
                 && p[j*3+1] > 40 // Quite saturated
                 && p[j*3+2] > 130) // Bright color
                t[j] = 255;
            else
                t[j] = 0;
        }
    }
    
    return res;
} 



int main(int argc, char **argv) {
    string win1("Reference"), win2("HSV"), win3("Segmentated");
    cv::namedWindow(win1, cv::WINDOW_NORMAL);
    cv::namedWindow(win2, cv::WINDOW_NORMAL);
    cv::namedWindow(win3, cv::WINDOW_NORMAL);
    
    string dir = "samples";
    const int n_samples=6;
    char sleep;
    char path[100];
    
    for(int i=0; i<n_samples; ++i) {
        sprintf(path, "../../%s/sample%d.png", dir.c_str(), i+1);
        ifstream f(path);
        if(f.good())
            f.close();
        else {
            cout << "Cannot open " << path << endl;
            continue;
        }
        cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
        cv::Mat segmentated_image = ups_segmentate(image);
        
        cv::imshow(win1, image);
        cv::imshow(win3, segmentated_image);
        cv::waitKey(0);
    }
    cv::waitKey(5000);
    return 0;
}
