/*
* Author: Artur Dobrogowski
* 2020-01-04
*/

#include "segment.h"
#include <cstring>
#include <cmath>

ostream& operator<<(ostream& ostr, const Segment& seg) {
    return ostr << "S(bbox:" << seg.start <<  "-" << seg.end << ")";
}


void Segment::addPoint(Coord x, Coord y, BasicMomentType f) {
    if (x > end.x)
        end.x = x;
    else if (x < start.x)
        start.x = x;

    if (y > end.y)
        end.y = y;
    else if (y < start.y)
        start.y = y;

    updateMoments(x, y, f);
}


Segment::Segment(cv::Mat o, Coord ox, Coord oy) : origin(o) {
    memset(m, 0, sizeof(BasicMomentType)*16);
    memset(M, 0, sizeof(MomentType)*16);
    start = end = sample = Point(ox, oy);
    circumference = 0;
}


void Segment::updateMoments(Coord x, Coord y, BasicMomentType f) {
    m[0][0] += 1*f;
    m[1][0] += x*f;
    m[0][1] += y*f;
    m[1][1] += x*y*f;
    BasicMomentType xx = x*static_cast<BasicMomentType>(x);
    BasicMomentType yy = y*static_cast<BasicMomentType>(y);
    m[2][0] += xx*f;
    m[2][1] += xx*y*f;
    m[1][2] += x*yy*f;
    m[0][2] += yy*f;
    m[3][0] += xx*x*f;
    m[0][3] += yy*y*f;
}


void Segment::updateMomentsCentralMoments() {
    M[0][0] = m[0][0];
    MomentType cx, cy;
    cx = m[1][0] / M[0][0];
    cy = m[0][1] / M[0][0];
    M[0][1] = M[1][0] = 0;
    M[1][1] = m[1][1] - m[1][0]*m[0][1] / M[0][0];
    M[2][0] = m[2][0] - m[1][0]*m[1][0] / M[0][0];
    M[0][2] = m[0][2] - m[0][1]*m[0][1] / M[0][0];
    M[2][1] = m[2][1] - 2*m[1][1]*cx - m[2][0]*cy + 2*m[0][1]*cx*cx;
    M[1][2] = m[1][2] - 2*m[1][1]*cy - m[0][2]*cx + 2*m[1][0]*cy*cy;
    M[3][0] = m[3][0] - 3*m[2][0]*cx + 2*m[1][0]*cx*cx;
    M[0][3] = m[0][3] - 3*m[0][2]*cy + 2*m[0][1]*cy*cy;
}

/*!
 * Function calculating n-th moment invariant. Invariant numbers 0-6 are the same as in OpenCV, based on:
 * Analysis of Hu’s Moment Invariants on Image Scaling and Rotation, Zhihu Huang, Jinsong Leng, Edith Cowan University, 2010
 * Invariant numbers 7-10 are invariants 7-10 from other source
 */
MomentType Segment::getIMCoeff(short n) {
    MomentType M3012 = M[3][0] + M[1][2];
    MomentType M2103 = M[2][1] + M[0][3];

    if(static_cast<MomentType>(m[0][0]) != M[0][0])
        updateMomentsCentralMoments();

    switch(n) {
        case 0:
            // (M20 + M02) / m00^2
            return (M[2][0] + M[0][2]) / pow(M[0][0], 2);
        case 1:
            // [(M20 - M02)^2 + 4*M11^2] / m00^4
            return (pow(M[2][0] - M[0][2], 2) + 4*pow(M[1][1], 2)) / pow(M[0][0], 4);
        case 2:
            // [(M30 - 3M12)^2 + (3M21 - M03)^2] / m00^5
            return (
                pow(M[3][0] - 3*M[1][2], 2) + pow(3*M[2][1] - M[0][3], 2)
            ) / pow(M[0][0], 5);
        case 3:
            // [(M30 + M12)^2 + (M21 + M03)^2] / m00^5
            return (pow(M3012, 2) + pow(M2103, 2)) / pow(M[0][0], 5);
        case 4:
            // {(M30 - 3M12)(M30 + M12)*[(M30 + M12)^2 - 3(M21 + M03)^2] + (3M21 - M03)(M21 + M03)[3*(M30 + M12)^2 - (M21 + M03)^2]} / M00^100
            return (
                (  M[3][0] - 3*M[1][2])*(M3012)*(  pow(M3012, 2) - 3*pow(M2103, 2)) +
                (3*M[2][1] -   M[0][3])*(M2103)*(3*pow(M3012, 2) -   pow(M2103, 2))
            ) / pow(M[0][0], 10);
        case 5:
            // {(M20 - M02) * [(M30 + M12)^2 - (M21 + M03)^2] + 4*M11*(M30 + M12)*(M21 + M03)} / M00^7
            return (
                (M[2][0] - M[0][2]) * (pow((M3012), 2) - pow(M2103, 2)) +
                4*M[1][1]*(M3012)*(M2103)
            ) / pow(M[0][0], 7);
        case 6:
            // {(3M21 - M03)*(M30 + M12)*[(M30 + M12)^2 - 3(M21 + M03)^2] - (M30 - 3M12)(M21 + M03)[3*(M30 + M12)^2 - (M21 + M03)^2]} / M00^100
            return (
                (3*M[2][1] -   M[0][3])*(M3012)*(  pow(M3012, 2) - 3*pow(M2103, 2)) -
                (  M[3][0] - 3*M[1][2])*(M2103)*(3*pow(M3012, 2) -   pow(M2103, 2))
            ) / pow(M[0][0], 10);
        case 7:
            // (M20*M02 - M11^2) / M00^4
            return (M[2][0]*M[0][2] - pow(M[1][1], 2)) / pow(M[0][0], 4);
        case 8:
            // (M30*M12 + M21*M03 - M12^2 - M21^2) / M00^5
            return (
                M[3][0]*M[1][2] + M[2][1]*M[0][3] -
                pow(M[1][2], 2) - pow(M[2][1], 2)
            ) / pow(M[0][0], 5);
        case 9:
            // {M20*(M21*M03 - M12^2) + M02*(M03*M12 - M21^2) - M11*(M30*M03 - M21*M12)} / M00^7
            return (
                M[2][0] * (M[2][1]*M[0][3] - pow(M[1][2], 2)) +
                M[0][2] * (M[0][3]*M[1][2] - pow(M[2][1], 2)) -
                M[1][1] * (M[3][0]*M[0][3] - M[2][1]*M[1][2])
            ) / pow(M[0][0], 7);
        case 10:
            // {(M30*M03 - M12*M21)^2 - 4*(M30*M12 - M21^2) * (M03*M21 - M12)} / M00^10
            return (
                pow(M[3][0]*M[0][3] - M[1][2]*M[2][1], 2) -
                4*(M[3][0]*M[1][2] - pow(M[2][1], 2)) * (M[0][3]*M[2][1] - M[1][2])
            ) / pow(M[0][0], 10);
        case 11:
            // Malinowska coefficient 
            return static_cast<double>(circumference)/(2*sqrtf(M_PI*m[0][0]/255)) - 1;
        default:
            break;
    }
    return 0;
}
