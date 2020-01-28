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
#include "preprocessing.h"
//#include <cstring>
#include <string>
using namespace std;

const int N_COEFFICIENTS=12;
const std::string shapes[] = {"u", "p", "s", "r"};
constexpr int N_SHAPES = sizeof(shapes)/sizeof(std::string);

class Shape {
public:
    Shape(std::string name_="") : name(name_) {}
    std::string name;
    double mean[N_COEFFICIENTS];
    double stdev[N_COEFFICIENTS];
    float weights[N_COEFFICIENTS];
    //weights
    double get_distance(Segment &s);
    void load();
};

double Shape::get_distance(Segment &s) {
    double dist = 0;
    for(int i=0; i<N_COEFFICIENTS; ++i) {
        double normalized = (s.getIMCoeff(i)-mean[i])/stdev[i];
        dist += weights[i] * normalized * normalized;
    }
    return dist;
}

void Shape::load() {
    std::ifstream file;
    file.open((name + std::string(".shape")).c_str());
    for(int i=0; i<N_COEFFICIENTS; ++i)
        file >> mean[i];
    for(int i=0; i<N_COEFFICIENTS; ++i)
        file >> stdev[i];
    for(int i=0; i<N_COEFFICIENTS; ++i)
        file >> weights[i];
}



int check_for_string(const int n, char ** argv, char const* str) {
    int result = 0;
    for(int i=0; i < n; ++i)
        if((strcmp(argv[i], str) == 0))
            result = i;
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
        cout << "Usage: <command> [options] <path for processing>\nOptions: \n\t-d,--debug - for extra info\n\t-m,--mask - treat input image as a mask to segmentate\n\t-s,--source - for displaying image path as source\n\t-c,--central - for displaying non-zero central moments\n\t-t,--test compare values with opencv builtin HuMoments, requires --debug\n\t-r,--header - display in first row the header for printed values\n\t-o,--output - save segmentation result with given path\n\t--contrast <C> <L> - adjusts contrast and lightness before processing the image C from range <-128:1280>, L from range <-255:255>\n\t--detect - flag to use shape values to detect u, p, s, border shapes\n\nCurrently there are " << N_COEFFICIENTS << " coefficients." << endl;
        return 0;
    }

    ifstream f(argv[argc-1]);
    if(f.good())
        f.close();
    else {
        cout << "Cannot open " << argv[1] << endl;
        return -1;
    }

    int debug =            check_for_string(argc, argv, "-d") + check_for_string(argc, argv, "--debug");
    int mask_as_input =    check_for_string(argc, argv, "-m") + check_for_string(argc, argv, "--mask");
    int display_source =   check_for_string(argc, argv, "-s") + check_for_string(argc, argv, "--source");
    int display_central =  check_for_string(argc, argv, "-c") + check_for_string(argc, argv, "--central");
    int display_header =   check_for_string(argc, argv, "-r") + check_for_string(argc, argv, "--header");
//     bool display_basic =    check_for_string(argc, argv, "-b") || check_for_string(argc, argv, "--basic");
    int test =             check_for_string(argc, argv, "-t") + check_for_string(argc, argv, "--test");
    int output_mask =      check_for_string(argc, argv, "-o") + check_for_string(argc, argv, "--output");
    int improve_contrast = check_for_string(argc, argv, "--contrast");
    int detect = check_for_string(argc, argv, "--detect");
    vector<Segment> segments;
    int discarded_segments = 0;

    // Reading the image
    std::string input_file(argv[argc-1]);
    cv::Mat image = cv::imread(input_file.c_str(), cv::IMREAD_COLOR);
    cv::Mat segmentated_image_mask = image;

    // Segmentation
    if(mask_as_input)
        cv::cvtColor(image, segmentated_image_mask, cv::COLOR_BGR2GRAY);
    else {
        if(improve_contrast) {
            int lightness;
            int contrast;
            if(improve_contrast+2 < argc && sscanf(argv[improve_contrast+2], "%d", &lightness) && sscanf(argv[improve_contrast+1], "%d", &contrast) ) {
                lighten_rgb(image, lightness);
                contrast_rgb(image, contrast);
            } else { 
                std::cerr << "Error in parsing contrast parameters, provide two integer values, see --help for reference\n";
                return 1;
            }
        }
        segmentated_image_mask = ups_segmentate(image);
    }
    
    if(output_mask) {
        std::string output_file = "output_mask.png";//input_file;
        //output_file.insert(output_file.end()-4, "_mask");
        cv::imwrite(output_file.c_str(), segmentated_image_mask);
    }
        

    // Separating segments and calculating moments
    discarded_segments +=  separate_segments(segmentated_image_mask, segments);

    // Displaying segmentation results
    if(debug) cout << "Found segments: \n";
    cout << std::setprecision(12);
    cout.setf(std::ios_base::scientific, std::ios_base::showpos);
    char const *basename = get_basename(argv[argc-1]);
    int k = 0;

    // Tables for non-zero central moments values
    int a[] = {0, 1, 2, 0, 2, 1, 3, 0};
    int b[] = {0, 1, 0, 2, 1, 2, 0, 3};
    constexpr size_t central_moments_num = sizeof(a)/sizeof(int);
    
    // Handle displaying header table for values
    if(display_header) {
        if(display_source) cout << "sourcefile " << "n-th_segment ";
        if(display_central) for(int i=0; i<central_moments_num; ++i) cout << "M" << a[i] << b[i] << " ";
        for(int i=0; i<N_COEFFICIENTS; ++i) cout << "M" << i << " ";
        cout << endl;
    }
    
    if( !detect) {
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
            for(int i=0; i<N_COEFFICIENTS; ++i)
                cout  << s.getIMCoeff(i) << " ";
            cout << endl;
        }
    }
    
    // Comparison with library implementation
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

    if(detect) {
        Shape shp[N_SHAPES];
        for(int i=0; i<N_SHAPES; ++i) {
            shp[i].name = shapes[i];
            shp[i].load();
            if(debug) {
                cout << shp[i].name << endl;
                for(int j=0; j<N_COEFFICIENTS; ++j) {
                    cout << shp[i].mean[j] << " " << shp[i].stdev[j] << " " << shp[i].weights[j] << endl;
                }
            }
        }
        for (int i=0; i<N_SHAPES; ++i) {
            vector<pair<double, Segment*> > order;
            for ( auto &a : segments) {
                order.push_back(make_pair(shp[i].get_distance(a), &a));
            }
            std::sort(order.begin(), order.end());
            cout << "Minimal " << shp[i].name << " distance = " << order[0].first << endl;
            if(debug) {
                cout << "Least distance 3 samples:\n";
                for(int k=0; k<3; ++k) {
                    cout << order[k].first << " ";
                    for(int j=0; j<N_COEFFICIENTS; ++j) {
                        cout << order[k].second->getIMCoeff(j) << " ";
                    } 
                    cout << endl;
                }
            }
            
        }
    }
    
    if(debug) cout << "Discarded segments = " << discarded_segments << endl;
    discarded_segments = 0;
    

    return 0;
}
