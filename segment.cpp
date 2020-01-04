/*
 * Author: Artur Dobrogowski
 * 2020-01-04
 */

#include "segment.h"
#include <cstring>

ostream& operator<<(ostream& ostr, const Segment& seg) {
    return ostr << "S(bbox:" << seg.start <<  "-" << seg.end << ")";
}


void Segment::add_point(Coord x, Coord y) {
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
    memset(moments, 0, sizeof(MomentType)*16);
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
