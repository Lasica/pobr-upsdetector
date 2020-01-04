/*
 * Author: Artur Dobrogowski
 * 2020-01-04
 */

#ifndef SEGMENTFILLER_H
#define SEGMENTFILLER_H
#include "segment.h"

/**
 * Class defining functor for DFS search of non-zero mask values to integrate as single segment using recursive operator()
 */
class SegmentFiller {
    unsigned char *mask_ptr;
    Segment &segment;
    int n_rows;
    int n_cols;
    
public:
    SegmentFiller(cv::Mat &vm, Segment &s);
    void operator()(Coord x, Coord y); 
};
#endif // SEGMENTFILLER_H
