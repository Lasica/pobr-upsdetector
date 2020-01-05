#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
using namespace std;

const uchar MAXPIX = 255;

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

typedef std::tuple<float, float> Orientation;

Orientation get_orientation(cv::Mat& I) {
	CV_Assert(I.depth() != sizeof(uchar));
	
	Orientation mass_center, bbox_center;

    int channels = I.channels();
    int nRows = I.rows;
    int nCols = I.cols * channels;

    // if (I.isContinuous())
    // {
    //     nCols *= nRows;
    //     nRows = 1;
    // }

    int i,j;
    uchar* p;
	int xm, xM, ym, yM, pix_count=0;
	long xmc, ymc;
	xmc = ymc = xM = yM = 0;
	xm = ym = ~0u>>1;
	for( int i = 0; i < nRows; ++i) {
		p = I.ptr<uchar>(i);
		for( int j = 0; j < nCols; j+=channels ){ // 0 <- chosen channel
			if(*p == 0u) {
				if(i > xM) xM = i;
				if(i < xm) xm = i;
				if(j > yM) yM = j;
				if(j < ym) ym = j;
				xmc += i;
				ymc += j;
				++pix_count;
			}
			p += channels;
		}
	}
	yM /= channels;
	ym /= channels;
	ymc /= channels;
	mass_center = make_tuple(static_cast<float>(xmc)/pix_count,  static_cast<float>(ymc)/pix_count);
	bbox_center = make_tuple((xM+xm)/2.0, (yM+ym)/2.0);
	printf("mass center: < %f, %f>\nbbox center: < %f, %f>\n", get<0>(mass_center), get<1>(mass_center), get<0>(bbox_center), get<1>(bbox_center));

	mass_center = make_tuple(get<0>(mass_center)-get<0>(bbox_center), get<1>(mass_center)-get<1>(bbox_center));

	//float len = sqrt(get<0>(mass_center)*get<0>(mass_center) + get<1>(mass_center)*get<1>(mass_center));

	//mass_center = make_tuple(get<0>(mass_center)/len, get<1>(mass_center)/len);

	return mass_center;
}

cv::Mat& contrast_rgb(cv::Mat& I, int degree) {
	CV_Assert(I.depth() != sizeof(uchar));
	float cval = 1 + (float)degree / 100;
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

cv::Mat& greyscale_rgb(cv::Mat& I) {
	CV_Assert(I.depth() != sizeof(uchar));
	switch (I.channels()) {
	case 3:
		cv::Mat_<cv::Vec3b> _I = I;
		for (int i = 0; i < I.rows; ++i)
			for (int j = 0; j < I.cols; ++j) {
				short lightness = ((short)_I(i, j)[0] + (short)_I(i, j)[1] + (short)_I(i, j)[2]) / 3;
				_I(i, j)[0] = lightness;
				_I(i, j)[1] = lightness;
				_I(i, j)[2] = lightness;
			}
		I = _I;
		break;
	}
	return I;
}

cv::Mat tresh_by_channel(cv::Mat& I, int channel, unsigned char tresh_l, unsigned char tresh_h) {
	CV_Assert(I.depth() != sizeof(uchar));
	cv::Mat  res(I.rows, I.cols, CV_8UC3);

	switch (I.channels()) {
	case 3:
		cv::Mat_<cv::Vec3b> _I(I);
		cv::Mat_<cv::Vec3b> _R(res);
		for (int i = 0; i < I.rows; ++i)
			for (int j = 0; j < I.cols; ++j) {
				int T = (1-(_I(i, j)[channel] >= tresh_l && _I(i, j)[channel] <= tresh_h)) * 255;
				_R(i, j)[0] = T;
				_R(i, j)[1] = T;
				_R(i, j)[2] = T;
			}
		res = _R;
		break;
	}
	return res;
}



void print_histogram(cv::Mat& I, uint bins) {
	CV_Assert(I.depth() != sizeof(uchar));
	if (bins < 1 || bins > 255) {
		std::cout << "Wrong bin count\n";
		return;
	}
	int *bin = new int[bins];
	for (int i = 0; i < bins; ++i)
		bin[i] = 0;

	float tresh = (MAXPIX+1) / bins;
	switch (I.channels()) {
	case 3:
		cv::Mat_<cv::Vec3b> _I = I;
		for (int i = 0; i < I.rows; ++i)
			for (int j = 0; j < I.cols; ++j) {
				float lightness = ((short)_I(i, j)[0] + (short)_I(i, j)[1] + (short)_I(i, j)[2]) / 3;
				++bin[(uchar)(lightness/tresh)];
			}
		break;
	}
	int sum = 0;
	for (int i = 0; i < bins; ++i) {
		std::cout << "bin[" << i << "]: " << bin[i] << std::endl;
		sum += bin[i];
	}

	std::cout << "Sum: " << sum << std::endl;
	delete bin;
	return;
}

cv::Mat rankFilter(cv::Mat& I,  int size, int rank){
    CV_Assert(I.depth() != sizeof(uchar));
    cv::Mat  res(I.rows, I.cols, CV_8UC3);
	//array<tuple<float, unsigned char>, 9> ranking();

	vector<pair<float, int>> ranking((2*size+1)*(2 * size + 1));
	//std::array<pair<float, unsigned char>, 9> ranking = { 0 };
	pair<float, unsigned char> t;
	
	const int C = I.cols, R = I.rows;
	cout << C << " " << R;
    switch(I.channels())  {
    case 3:
        cv::Mat_<cv::Vec3b> _I(I);
        cv::Mat_<cv::Vec3b> _R(res);

        for( int i = size; i < I.rows-size; ++i)
            for( int j = size; j < I.cols-size; ++j ){
				int ri = 0;
				for (int k = i-size; k <= i+size; ++k) {
					for (int l = j-size; l <= j+size; ++l) {
						float lumi = (_I(k, l)[0] + _I(k, l)[1] + _I(k, l)[2]) / 3.0f;
						ranking[ri++] = make_pair(lumi, k*R + l);
					}
				}
				sort(ranking.begin(), ranking.end());
				int rank_r = ranking[rank].second / R;
				int rank_c = ranking[rank].second % R;
				for (int a = 0; a < 3; ++a)
					_R(i, j)[a] = _I(rank_r, rank_c)[a];
            }
        res = _R;
        break;
    }
    return res;
}

int calc_surface(cv::Mat& I, int channel=0) {
	CV_Assert(I.depth() != sizeof(uchar));
	int acc = 0;
	switch (I.channels()) {
	case 3:
		cv::Mat_<cv::Vec3b> _I = I;
		for (int i = 0; i < I.rows; ++i)
			for (int j = 0; j < I.cols; ++j) {
				//short lightness = ((short)_I(i, j)[0] + (short)_I(i, j)[1] + (short)_I(i, j)[2]) / 3;
				acc += _I(i, j)[channel] == 0;
			}
		I = _I;
		break;
	}
	return acc;
}

int calc_delim(cv::Mat& I, int channel = 0) {
	CV_Assert(I.depth() != sizeof(uchar));
	int acc = 0;

	int x_translation[] = {0,1,0,-1};//{ 0, 1, 1, 1, 0, -1, -1, -1 };
	int y_translation[] = {1,0,-1,0};//{ 1, 1, 0, -1, -1, -1, 0, 1 };
	const int K = sizeof(x_translation) / sizeof(int);
	int recolor_channel = (channel + 1) % 3;

	switch (I.channels()) {
	case 3:
		cv::Mat_<cv::Vec3b> _I = I;
		for (int i = 1; i < I.rows-1; ++i)
			for (int j = 1; j < I.cols-1; ++j) {
				char conditions = 0;
				if (_I(i, j)[channel] == 0) {
					for (int t = 0; t < K; ++t)
						conditions += _I(i, j)[channel] == _I(i + x_translation[t], j + y_translation[t])[channel];
				}
				if (conditions % K != 0) {
					++acc;
					_I(i, j)[recolor_channel] = 255;
				}
			}
		I = _I;
		break;
	}
	return acc;
}

#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

template<unsigned int P, unsigned int Q> 
long calc_m(cv::Mat& I) {
	CV_Assert(I.depth() != sizeof(uchar));
	long acc = 0;

	switch (I.channels()) {
	case 3:
		cv::Mat_<cv::Vec3b> _I = I;
		for (int i = 0; i < I.rows; ++i)
			for (int j = 0; j < I.cols; ++j) {
				int x = 1;
				int y = 1;
#pragma unroll
				for (int p = 0; p < P; ++p)
					x *= i;
#pragma unroll
				for (int q = 0; q < Q; ++q)
					y *= j;


				acc += x * y*(_I(i, j)[0] == 0);
			}
		I = _I;
		break;
	}
	return acc;
}

double calc_w3(int L, int S) {
	return L / (2*sqrt(M_PI * S)) - 1;
}

#include <cstdio>

void zad3(std::string fname, cv::Mat& image) {

	//greyscale_rgb(image);

	int S = calc_surface(image, 0);
	int L = calc_delim(image, 0);
	double W3 = calc_w3(L, S);
	long long m_00 = calc_m<0, 0>(image);
	long long m_20 = calc_m<2, 0>(image);
	long long m_02 = calc_m<0, 2>(image);
	long long m_10 = calc_m<1, 0>(image);
	long long m_01 = calc_m<0, 1>(image);
	long long m_11 = calc_m<1, 1>(image);
	double M20 = m_20 - static_cast<double>(m_10 * m_10) / m_00;
	double M02 = m_02 - static_cast<double>(m_01 * m_01) / m_00;
	double M11 = m_11 - static_cast<double>(m_10 * m_01) / m_00;
	double M1 = (M20 + M02) / (m_00*m_00);
	double M7 = (M20 * M02 - M11*M11) / (m_00*m_00*m_00*m_00);
	printf("%10s  S = %5d  L = %4d  W3 = %1.6lf  M1 = %1.6lf M7 = %1.6lf\n", fname.c_str(), S, L, W3, M1, M7);
	//std::cout << fname << " S = " << S << "  L = " << L << "  W3 = " << W3;
	//std::cout <<  "  M1 = " << M1 << "  M7 = " << M7 << std::endl;

	cv::imshow("Show", image);
}

// M1 = (M20 + M02 ) / m00^2
// M7 = (M20 * M02 - M11^2) / m00^4
// M20 = m20 - m10^2 / m00
// M02 = m02 - m01^2 / m00
// M11 = m11 - m10*m01 / m00

int main(int, char *[]) {
	
    std::cout << "Start ..." << std::endl;

	std::string figures[] = { "elipsa", "elipsa1", "kolo", "troj" }; // "strzalki_1", "strzalki_2" "prost"
	std::string figures2[] = { "strzalki_1", "strzalki_2" };
	std::string ext = ".dib";
	
	for (int i = 0; i < sizeof(figures) / sizeof(std::string); ++i) {
		cv::Mat image = cv::imread((figures[i] + ext).c_str());
		zad3(figures[i], image);
		cv::waitKey(500);
	}
	

	// cv::Mat image = cv::imread((figures2[1] + ext).c_str());
	// // cv::Mat t_image = tresh_by_channel(image, 2, 0, 45);
	// cv::imshow("Show", image);
	// for (int t = 0; t <= 180; t += 45) {
	// 	cv::Mat t_image = tresh_by_channel(image, 2, t, t+44);
	// 	cv::imshow("Show", t_image);
	// 	cv::waitKey(500);
	// }
	
	for (int i = 0; i < sizeof(figures2) / sizeof(std::string); ++i) {
		cv::Mat image = cv::imread((figures2[i] + ext).c_str());
		for (int t = 0; t <= 180; t += 45) {
			cv::Mat t_image = tresh_by_channel(image, 2, t, t+44);

			zad3(figures2[i], t_image);
			Orientation orient = get_orientation(t_image);
			printf("orientation: x: %-3.3f  y: %-3.3f  angle: %-4.2f\n", get<0>(orient), get<1>(orient), atan2(get<1>(orient), get<0>(orient))*180.0/M_PI);
			cv::waitKey(500);
		}
	}
	

	/* Lab2
	cv::Mat ranked = rankFilter(image, 1, 4);
	cv::Mat ranked2 = rankFilter(image, 4, 20);

	cv::imshow("Lena", image);
	cv::imshow("Median", ranked);
	cv::imshow("Ranked 9x9, 20", ranked2);
	*/

//	cv::Mat imqarts[2][2];
//	for(int i=0; i<2; ++i)
//		for(int j=0; j<2; ++j)
//			imqarts[i][j] = image(cv::Rect(i*image.rows/2, j*image.cols/2, image.rows/2, image.cols/2));
//
//	print_histogram(image, 8);
//    lighten_rgb(imqarts[0][0], -80);

//	greyscale_rgb(imqarts[1][0]);
//	contrast_rgb(imqarts[0][1], -20);
//    //std::cout << imqarts[0][0].isContinuous() << max.isContinuous() << std::endl;
//    //cv::imwrite("Max.png",max);
    // cv::waitKey(-1);
    return 0;
}

/*
Cztery cwiartki:
1) rozjasnic
2) przyciemnic
3) greyscale
4) original
histogram 8 poziomow jasnosci - wydruk do konsoli
debug: suma pikseli w programie
*/
