#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cstdio>
#include <fstream> 
#include "configuartion.h"
#include "point.h"
#include "segment.h"
#include "segmentfiller.h"
#include "maskoperators.h"
using namespace std;


int main(int argc, char **argv) {
    string win1("Reference"), win2("HSV"), win3("Segmentated");
    cv::namedWindow(win1, cv::WINDOW_NORMAL);
    cv::namedWindow(win2, cv::WINDOW_NORMAL);
    cv::namedWindow(win3, cv::WINDOW_NORMAL);
    
    string dir = "samples";
    const int n_samples=6;
    char sleep;
    char path[100];
    
    vector<Segment> segments;
    int discarded_segments = 0;
    
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
        cv::Mat segmentated_image_mask = ups_segmentate(image);
        discarded_segments +=  separate_segments(segmentated_image_mask, segments);
        cout << "Found segments: \n";
        int k = 0;
        for( auto &s : segments ) {
            cout << k++ << " " <<  s;
            cout << " Moments: ";
            for (int l = 0; l < 4; ++l) {
                for (int j = 0; j < 4; ++j)
                    cout <<  s.moments[l][j] <<  " ";
                
            }cout <<  endl;
        } segments.clear();
        
        cv::imshow(win1, image);
        cv::imshow(win3, segmentated_image_mask);
        cv::waitKey(0);
    }
    

    cv::waitKey(5000);
    return 0;
}
