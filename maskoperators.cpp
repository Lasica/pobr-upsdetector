/*
 * Author: Artur Dobrogowski
 * 2020-01-04
 */

#include "maskoperators.h"
#include "segmentfiller.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>


int separate_segments(const cv::Mat &src, std::vector<Segment> &container,  int tresh) {
    int nRows = src.rows;
    int nCols = src.cols;
    int discarded_tresh = 0;
    unsigned char *pvis;
    
    cv::Mat visited = src.clone();
    pvis = visited.ptr<unsigned char>(0);
    
    for(int i = 0; i < nRows; ++i) {
        
        for (int j = 0; j < nCols; ++j) {
            
            if(*(pvis + i*nCols + j) > 0) {
                Segment s = Segment(src, i, j);
                SegmentFiller sfm = SegmentFiller(visited, s);
                sfm(i, j);
                if (s.getArea() >= tresh) {
                    container.push_back(s);
                }
                else
                    discarded_tresh++;
            }
        }
    }
    return discarded_tresh;
}


cv::Mat ups_segmentate(cv::Mat src) {
    //void cvtColor(InputArray src, OutputArray dst, int code, int dstCn=0 )
    
    cv::Mat hsvsrc(src.rows, src.cols, CV_8UC3);
    
    // Conversion to HSV
    cv::cvtColor(src, hsvsrc, cv::COLOR_BGR2HSV);
    cv::Mat res(src.rows, src.cols, CV_8UC1);
    
    CV_Assert(src.depth() == CV_8U);
    
    int channels = hsvsrc.channels();
    int nRows = hsvsrc.rows;
    int nCols = hsvsrc.cols;
    
    if (hsvsrc.isContinuous()) {
        nCols *= nRows;
        nRows = 1;
    }

    uchar* p;
    unsigned char* t;
    for(int i = 0; i < nRows; ++i) {
        p = hsvsrc.ptr<uchar>(i);
        t = res.ptr<unsigned char>(i);
        for (int j = 0; j < nCols; ++j) {
            //H +0
            //S +1
            //V +2
            // Segmentation color condition
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
