/*
 * Author: Artur Dobrogowski
 * 2020-01-04
 */

#ifndef CONFIGUARTION_H
#define CONFIGUARTION_H
#include <iostream>

using std::ostream;

/**
 * @todo write docs
 */
typedef double BasicMomentType;
typedef double MomentType;
typedef int Coord;

const Coord x_translation[] = {-1, +1,  0,  0};
const Coord y_translation[] = { 0,  0, -1, +1};


#endif // CONFIGUARTION_H
