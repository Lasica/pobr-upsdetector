/*
 * Author: Artur Dobrogowski
 * 2020-01-04
 */

#include "point.h"
#include <ostream>
using std::ostream;

Point Point::operator+(Point other) { 
    return Point(x+other.x,  y+other.y);
}


Point Point::operator-(Point other) { 
    return Point(x-other.x,  y-other.y); 
}


Point Point::operator-() { 
    return Point(-x,  -y); 
}


Point Point::operator/(Coord scalar) { 
    return Point(x/scalar,  y/scalar); 
}


Point Point::operator*(Coord scalar) { 
    return Point(x*scalar,  y*scalar); 
}

Point::Point() {
    x = y = -1;
}


Point::Point(Coord x_, Coord y_) : x(x_), y(y_) { }


ostream& operator<<(ostream& ostr, const Point& pt) {
    return ostr <<  "(" << pt.x << "," << pt.y << ")";
}
