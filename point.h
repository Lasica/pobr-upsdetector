/*
 * Author: Artur Dobrogowski
 * 2020-01-04
 */

#ifndef POINT_H
#define POINT_H
#include "configuration.h"

/**
 * @todo write docs
 */

struct Point {
public:
    Coord x, y;
    Point();
    Point(Coord x_, Coord y_);
    Point operator+(Point other);
    Point operator-(Point other);
    Point operator-();
    Point operator/(Coord scalar);
    Point operator*(Coord scalar);
};

std::ostream& operator<<(std::ostream& ostr, const Point& pt);

#endif // POINT_H
