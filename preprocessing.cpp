#include "preprocessing.h"

constexpr int MAXPIX = 255;

uchar add_overflow(uchar pix, int value) {
	if (pix + value > MAXPIX)
		pix = MAXPIX;
	else if (pix + value < 0) {
		pix = 0;
	}
	else pix = pix + value;
	return pix;
}


uchar mul_overflow(uchar pix, float value) {
	value = (float)pix * value;
	if (value > MAXPIX)
		value = MAXPIX;
	else if (value < 0) {
		value = 0;
	}
	return static_cast<uchar>(value);
}

cv::Mat& lighten_rgb(cv::Mat& I, int degree){
  CV_Assert(I.depth() != sizeof(uchar));
  switch(I.channels())  {
  case 1:
	  for (int i = 0; i < I.rows; ++i)
		  for (int j = 0; j < I.cols; ++j)
			  I.at<uchar>(i, j) = add_overflow(I.at<uchar>(i,j), degree);
    break;
  case 3:
    cv::Mat_<cv::Vec3b> _I = I;
    for( int i = 0; i < I.rows; ++i)
        for( int j = 0; j < I.cols; ++j ){
            _I(i,j)[0] = add_overflow(_I(i, j)[0], degree);
            _I(i,j)[1] = add_overflow(_I(i, j)[1], degree);
            _I(i,j)[2] = add_overflow(_I(i, j)[2], degree);
        }
    I = _I;
    break;
  }
  return I;
}

cv::Mat& contrast_rgb(cv::Mat& I, int degree) {
	CV_Assert(I.depth() != sizeof(uchar));
	float cval = 1 + (float)degree / 128;
	switch (I.channels()) {
	case 1:
		for (int i = 0; i < I.rows; ++i)
			for (int j = 0; j < I.cols; ++j)
				I.at<uchar>(i, j) = mul_overflow(I.at<uchar>(i, j), cval);
		break;
	case 3:
		cv::Mat_<cv::Vec3b> _I = I;
		for (int i = 0; i < I.rows; ++i)
			for (int j = 0; j < I.cols; ++j) {
				_I(i, j)[0] = mul_overflow(_I(i, j)[0], cval);
				_I(i, j)[1] = mul_overflow(_I(i, j)[1], cval);
				_I(i, j)[2] = mul_overflow(_I(i, j)[2], cval);
			}
		I = _I;
		break;
	}
	return I;
}
