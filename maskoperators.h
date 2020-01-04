/*
 * Author: Artur Dobrogowski
 * 2020-01-04
 */

#ifndef MASKOPERATORS_H
#define MASKOPERATORS_H
#include "segment.h"
#include <vector>

/**
 * @todo write docs
 */

int separate_segments(const cv::Mat &src, std::vector<Segment> &container,  int tresh=16);
cv::Mat ups_segmentate(cv::Mat src);


#endif // MASKOPERATORS_H
