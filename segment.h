/*
 * Author: Artur Dobrogowski
 * 2020-01-04
 */

#ifndef SEGMENT_H
#define SEGMENT_H
#include "point.h"
#include "configuartion.h"
#include <opencv2/core/core.hpp>


/**
 * @todo write docs
 */
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
    void add_point(Coord x, Coord y);
    friend ostream& operator<<(ostream& ostr, const Segment& seg);
    MomentType get_area() { return moments[0][0]; }
    Point get_mass_center() { return Point((moments[0][0]/2 + moments[1][0])/moments[0][0],  (moments[0][0]/2 + moments[0][1])/moments[0][0]); }
    Point get_bbox_center() { return (start+end)/2; }
};

#endif // SEGMENT_H
