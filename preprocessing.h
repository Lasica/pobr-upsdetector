/*
 * Author: Artur Dobrogowski
 * 2020-01-26
 */

#ifndef PREPROCESSING_H
#define PREPROCESSING_H
#include "configuration.h"
#include "opencv2/core/core.hpp"

/**
 * adds value to each pixel by degree (-255:255)
 */
cv::Mat& lighten_rgb(cv::Mat& I, int degree);

/**
 * multiples value of each pixel by 1+degree/128 (-128:1280~)
 */
cv::Mat& contrast_rgb(cv::Mat& I, int degree);
#endif // PREPROCESSING_H
