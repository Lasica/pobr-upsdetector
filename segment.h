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
    BasicMomentType m[4][4]; // Basic moments
    MomentType M[4][4]; // Central moments
//     int circumference;
    
    void updateMoments(Coord x, Coord y);
    void updateMomentsCentralMoments();
    MomentType getIMCoeff(short n);
    
    Segment(cv::Mat o, Coord ox, Coord oy);
    void addPoint(Coord x, Coord y);
    friend ostream& operator<<(ostream& ostr, const Segment& seg);
    BasicMomentType getArea() { return m[0][0]; }
    Point getMassCenter() { return Point((m[0][0]/2 + m[1][0])/m[0][0],  (m[0][0]/2 + m[0][1])/m[0][0]); }
    Point getBboxCenter() { return (start+end)/2; }
};

#endif // SEGMENT_H
