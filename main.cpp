#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cstdio>
#include <fstream> 
using namespace std;

typedef long long MomentType;
typedef int Coord;

const Coord x_translation[] = {-1, +1,  0,  0};
const Coord y_translation[] = { 0,  0, -1, +1};

struct Point {
public:
    Coord x, y;
    Point();
    Point(Coord x_, Coord y_);
};

Point::Point() {
    x = y = -1;
}

Point::Point(Coord x_, Coord y_) : x(x_), y(y_) {
}


ostream& operator<<(ostream& ostr, const Point& pt) {
    return ostr <<  "(" << pt.x << "," << pt.y << ")";
}


class Segment {
public:
    cv::Mat origin;
    Point start;
    Point end;
    Point sample;
    MomentType moments[4][4];
//     int circumference;
    
    void update_moments(Coord x, Coord y);
    Segment(cv::Mat o, Coord ox, Coord oy);
    inline void add_point(Coord x, Coord y);
    friend ostream& operator<<(ostream& ostr, const Segment& seg);
};


ostream& operator<<(ostream& ostr, const Segment& seg) {
    return ostr << "S(bbox:" << seg.start <<  "-" << seg.end << ")";
}


inline void Segment::add_point(Coord x, Coord y) {
    if (x > end.x)
        end.x = x;
    else if (x < start.x)
        start.x = x;
    
    if (y > end.y)
        end.y = y;
    else if (y < start.y)
        start.y = y;
    
    update_moments(x, y);
}


Segment::Segment(cv::Mat o, Coord ox, Coord oy) : origin(o) {
    memset(moments, 0, sizeof(MomentType)*12);
    start = end = sample = Point(ox, oy);
}


void Segment::update_moments(Coord x, Coord y) {
    moments[0][0] += 1;
    moments[1][0] += x;
    moments[0][1] += y;
    moments[1][1] += x*y;
    int xx = x*static_cast<int>(x);
    int yy = y*static_cast<int>(y);
    moments[2][0] += xx;
    moments[2][1] += xx*y;
    moments[1][2] += x*yy;
    moments[0][2] += yy;
    moments[3][0] += xx*x;
    moments[0][3] += yy*y;
}


// Class defining functor for DFS search of non-zero mask values to integrate as single segment using recursive operator()
class SegmentFiller {
    unsigned char *mask_ptr;
    Segment &segment;
    int n_rows;
    int n_cols;
    
public:
    SegmentFiller(cv::Mat &vm, Segment &s);
    void operator()(Coord x, Coord y); 
};

SegmentFiller::SegmentFiller(cv::Mat &vm, Segment &s) : segment(s) {
    mask_ptr = vm.ptr<unsigned char>(0);
    n_rows = vm.rows;
    n_cols = vm.cols;
}

    
void SegmentFiller::operator()(Coord x, Coord y) {
    // Process the pixel as segment fragment
    segment.add_point(x, y);
    *(mask_ptr + n_cols * x + y) = 0;
    for(int i=0; i < 4; ++i) {
        const Coord new_x = x_translation[i] + x;
        const Coord new_y = y_translation[i] + y;
        
        // Validate coordinates
        if(new_x > 0 && new_x < n_rows && new_y > 0 && new_y < n_cols)
            
            // If mask not yet processed
            if (*(mask_ptr + n_cols * new_x + new_y) > 0)
                
                // Process pixel of new coordinates as segment fragment
                this->operator()(new_x, new_y);
    }
}


void separate_segments(const cv::Mat &src, vector<Segment> &container) {
    int nRows = src.rows;
    int nCols = src.cols;
    unsigned char *pvis;
    
    cv::Mat visited = src.clone();
    pvis = visited.ptr<unsigned char>(0);
    
    for(int i = 0; i < nRows; ++i) {
        
        for (int j = 0; j < nCols; ++j) {
            
            if(*(pvis + i*nCols + j) > 0) {
                Segment s = Segment(src, i, j);
                SegmentFiller sfm = SegmentFiller(visited, s);
                sfm(i, j);
                container.push_back(s);
            }
        }
    }
}


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
    
    vector<Segment> segments;
    
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
        separate_segments(segmentated_image_mask, segments);
        
        cv::imshow(win1, image);
        cv::imshow(win3, segmentated_image_mask);
        cv::waitKey(0);
    }
    
    cout << "Found segments: \n";
    for( auto &s : segments ) {
        cout << s << endl;
        cout << "Moments: " << endl;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j)
                cout <<  s.moments[i][j] <<  " ";
            cout <<  endl;
        }
    }
    cv::waitKey(5000);
    return 0;
}
