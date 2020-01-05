/*
 * Author: Artur Dobrogowski
 * 2020-01-05
 */

#include <iostream>
#include <iomanip>
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

char const * get_basename(char const *path) {
    int n = strlen(path);
    while(n--) {
        if(path[n] == '/')
            break;
    }
    if(n < 0)
        return path;
    else
        return path+n+1;
}

int main(int argc, char **argv) {

    if(check_for_string(argc, argv, "-h") || check_for_string(argc, argv, "--help") || argc < 2) {
        cout << "Usage: <command> [options] <path for processing>\nOptions: \n\t-d,--debug - for extra info\n\t-m,--mask - treat input image as a mask to segmentate\n\t-s,--source - for displaying image path as source\n\t-c,--central - for displaying non-zero central moments\n\t-t,--test compare values with opencv builtin HuMoments, requires --debug\n\t-r,--header - display in first row the header for printed values" << endl;
        return 0;
    }

    ifstream f(argv[argc-1]);
    if(f.good())
        f.close();
    else {
        cout << "Cannot open " << argv[1] << endl;
        return -1;
    }

    bool debug =            check_for_string(argc, argv, "-d") || check_for_string(argc, argv, "--debug");
    bool mask_as_input =    check_for_string(argc, argv, "-m") || check_for_string(argc, argv, "--mask");
    bool display_source =   check_for_string(argc, argv, "-s") || check_for_string(argc, argv, "--source");
    bool display_central =  check_for_string(argc, argv, "-c") || check_for_string(argc, argv, "--central");
    bool display_header =  check_for_string(argc, argv, "-r") || check_for_string(argc, argv, "--header");
//     bool display_basic =    check_for_string(argc, argv, "-b") || check_for_string(argc, argv, "--basic");
    bool test =             check_for_string(argc, argv, "-t") || check_for_string(argc, argv, "--test");

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
//     if(debug) cout << std::setw(14);
    cout << std::setprecision(12);
    cout.setf(std::ios_base::scientific, std::ios_base::showpos);
//     std::ios printState(nullptr);
//     printState.copyfmt(std::cout);
    char const *basename = get_basename(argv[argc-1]);
    int k = 0;

    // Tables for non-zero central moments values
    int a[] = {0, 1, 2, 0, 2, 1, 3, 0};
    int b[] = {0, 1, 0, 2, 1, 2, 0, 3};
    constexpr size_t central_moments_num = sizeof(a)/sizeof(int);
    if(display_header) {
        if(display_source) cout << "sourcefile " << "n-th_segment ";
        if(display_central) for(int i=0; i<central_moments_num; ++i) cout << "M" << a[i] << b[i] << " ";
        for(int i=0; i<11; ++i) cout << "M" << i << " ";
        cout << endl;
    }
    for( auto &s : segments ) {
        if(display_source) cout << basename << " " << k++ << " ";
        if(debug) cout << s << endl;
        if(debug) {
            cout << "Basic moments: " << endl;
            for (int l = 0; l < 4; ++l) {
                for (int j = 0; j < 4; ++j)
                    cout <<  s.m[l][j] <<  " ";
            } cout <<  endl;
        }
        if(display_central) {
            s.updateMomentsCentralMoments();
            if(debug) cout << "Central moments: \n";
            for (int i=0; i<central_moments_num; ++i)
                cout  << s.M[a[i]][b[i]] << " ";
        }
        if(debug) cout << " Invariants: " << endl;
        for(int i=0; i<=9; ++i)
            cout  << s.getIMCoeff(i) << " ";
        cout << endl;
    }

    if(test && debug) {
        cout << "Library calculated HuMoments\n";
        // Calculate Moments
        cv::Moments moments = cv::moments(segmentated_image_mask, false);

        // Calculate Hu Moments
        double huMoments[7];
        cv::HuMoments(moments, huMoments);
        for(int i=0; i<7; ++i)
            cout  << huMoments[i] << " ";
        cout << endl;
        cout << "Difference between HuMoments and invariants of 1st segment:\n";
        for(int i=0; i<7; ++i)
            cout  << huMoments[i]-segments[0].getIMCoeff(i) << " ";
        cout << endl;
    }
    if(debug) cout << "Discarded segments = " << discarded_segments << endl;
    discarded_segments = 0;

    return 0;
}
