/*
 * Author: Artur Dobrogowski
 * 2020-01-05
 */

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cstdio>
#include <fstream>
#include "configuration.h"
#include "point.h"
#include "segment.h"
#include "segmentfiller.h"
#include "maskoperators.h"
#include <cstring>
using namespace std;

bool check_for_string(const int n, char ** argv, char const* str) {
    bool result = false;
    for(int i=0; i < n; ++i)
        result = result || (strcmp(argv[i], str) == 0);
    return result;
}

int main(int argc, char **argv) {

    if(check_for_string(argc, argv, "-h") || check_for_string(argc, argv, "--help") || argc < 2) {
        cout << "Usage: <command> [options] <path for processing>\nOptions: \n-d,--debug - for extra info\n-m,--mask - treat input image as a mask to segmentate" << endl;
        return 0;
    }

    ifstream f(argv[argc-1]);
    if(f.good())
        f.close();
    else {
        cout << "Cannot open " << argv[1] << endl;
        return -1;
    }

    bool debug = check_for_string(argc, argv, "-d") || check_for_string(argc, argv, "--debug");
    bool mask_as_input = check_for_string(argc, argv, "-m") || check_for_string(argc, argv, "--mask");

    vector<Segment> segments;
    int discarded_segments = 0;

    // Reading the image
    cv::Mat image = cv::imread(argv[argc-1], cv::IMREAD_COLOR);
    cv::Mat segmentated_image_mask = image;

    // Segmentation
    if(mask_as_input)
        cv::cvtColor(image, segmentated_image_mask, cv::COLOR_BGR2GRAY);
    else
        segmentated_image_mask = ups_segmentate(image);

    // Separating segments and calculating moments
    discarded_segments +=  separate_segments(segmentated_image_mask, segments);

    // Displaying segmentation results
    if(debug) cout << "Found segments: \n";
    int k = 0;
    for( auto &s : segments ) {
        if(debug) {
            cout << k++ << " " <<  s;
            cout << " Moments: ";
            for (int l = 0; l < 4; ++l) {
                for (int j = 0; j < 4; ++j)
                    cout <<  s.m[l][j] <<  " ";
            } cout <<  endl;
        }
        if(debug) cout << " Invariants: ";
        for(int i=1; i<=10; ++i)
            cout << s.getIMCoeff(i) << " ";
        cout << endl;
    }
    segments.clear();
    if(debug) cout << "Discarded segments = " << discarded_segments << endl;
    discarded_segments = 0;

    return 0;
}
