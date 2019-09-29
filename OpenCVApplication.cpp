// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include "stdlib.h"

#define IM0 "cones/left.png"
#define IM1 "cones/right.png"
#define DISP 64
#define P1 3
#define P2 20
#define DIRECTIONS 16
#define LOW 20
#define HIGH 235

#define IM0_GRAY "results/left.png"
#define IM1_GRAY "results/right.png"

/* result images' names on disk */
#define DIR_NAME "results"
#define IMRES "results/res.png"
#define IMRES_NORM "results/res_normalized.png"
#define IMRES_MED_FILT "results/res_median_filter.png"

/* put this to 1 if you want to save result images */
#define SAVE_TO_DISK 1

int dx[16] = { 0,-1,-1,-1,0,1,1,1,-1,-2,-2,-1,1,2,2,1 };
int dy[16] = { 1,1,0,-1,-1,-1,0,1,2,1,-1,-2,-2,-1,1,2 };

void make_directory(const char* name) 
{
#ifdef __WIN32__
	_mkdir(name);
#elif __linux__
	mkdir(name, ACCESSPERMS); 
#else
// other platform
#endif
}

void census(Mat_<uchar> img, uint64_t **cens) {

	int c = 9 / 2;
	int r = 7 / 2;

	for (int i = r; i < img.rows - r; i++) {
		for (int j = c; j < img.cols - c; j++) {
			cens[i][j] = 0;
		}
	}

	for (int i = r; i < img.rows - r; i++) {
		for (int j = c; j < img.cols - c; j++) {
			uint64_t ce = 0;
			for (int x = -r; x <= r; x++) {
				for (int y = -c; y <= c; y++) {
					if (x || y) {
						ce = ce << 1;
						if (img(i, j) > img(i + x, j + y)) {
							ce |= 1;
						}
					}
				}
			}
			cens[i][j] = ce;
		}
	}
}

short hamming(uint64_t h1, uint64_t h2) {
	uint64_t val = h1 ^ h2;
	short dist = 0;
	while (val) {
		++dist;
		val &= val - 1;
	}

	return dist;
}

void computeCost(uint64_t **cens0, uint64_t **cens1, int w, int h, int disp, short ***cost) {
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			for (int d = 0; d < disp; d++) {
				if (j + d < h) {
					cost[i][j][d] = hamming(cens0[i][j + d], cens1[i][j]);
				}
				else {
					cost[i][j][d] = 64;
				}
			}
		}
	}
}

void computePaths(short**** paths, short*** cost, int rows, int cols, int disparityRange) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int d = 0; d < disparityRange; d++) {
				for (int k = 0; k < DIRECTIONS; k++) {
					paths[i][j][d][k] = cost[i][j][d];
					if (i + dx[k] < 0 || i + dx[k] >= rows || j + dy[k] < 0 || j + dy[k] >= cols) {
						continue;
					}
					short mn = SHRT_MAX;
					for (int disp = 0; disp < disparityRange; disp++) {
						if (mn > cost[i + dx[k]][j + dy[k]][disp]) {
							mn = cost[i + dx[k]][j + dy[k]][disp];
						}
					}

					short c1 = cost[i + dx[k]][j + dy[k]][d];
					short c2 = cost[i + dx[k]][j + dy[k]][d - 1] + P1;
					short c3 = cost[i + dx[k]][j + dy[k]][d + 1] + P1;

#ifdef __WIN32__
					paths[i][j][d][k] += min(c1, min(c2, min(c3, mn + P2))) - mn;
#elif __linux__
					paths[i][j][d][k] += MIN(c1, MIN(c2, MIN(c3, mn + P2))) - mn;
#else
// other platform
#endif
				}
			}
		}
	}
}

void aggregatecosts(short ****paths, short ***agcost, int rows, int cols, int disparity) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int d = 0; d < disparity; d++) {
				for (int k = 0; k < DIRECTIONS; k++) {
					agcost[i][j][d] += paths[i][j][d][k];
				}
			}
		}
	}
}

Mat_<uchar> createDisparityMap(short ***agcost, int rows, int cols, int disparity) {
	Mat_<uchar> res(rows, cols);
	res.setTo(0);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			short mn = SHRT_MAX;
			short disp = 0;
			for (int d = 0; d < disparity; d++) {
				if (agcost[i][j][d] > 0) {
					if (agcost[i][j][d] < mn) {
						mn = agcost[i][j][d];
						disp = d;
					}
				}
			}
			res(i, j) = disp;
		}
	}

	return res;
}

Mat_<uchar> normalize(Mat_<uchar> img) {
	Mat_<uchar> dst(img.rows, img.cols);
	uchar mn = 255, mx = 0;
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			if (img(i, j) < mn) mn = img(i, j);
			if (img(i, j) > mx) mx = img(i, j);
		}
	}
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			dst(i, j) = (img(i, j) / (float)mx) * 255;
		}
	}

	return dst;
}

int isInside(Mat img, int i, int j) {
	if (i >= 0 && i < img.rows && j >= 0 && j < img.cols) {
		return 1;
	}
	return 0;
}

Mat_<uchar> medianFilter(Mat_<uchar> img, int size) {

	Mat_<uchar> res(img.rows, img.cols);
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			std::vector<int> v;
			for (int u = -size / 2; u <= size / 2; u++) {
				for (int q = -size / 2; q <= size / 2; q++) {
					if (isInside(img, i + u, j + q)) {
						v.push_back(img(i + u, j + q));
					}
				}
			}
			std::sort(v.begin(), v.end());
			res(i, j) = v.at(v.size() / 2);
		}
	}

	return res;
}

int main()
{
	Mat_<uchar> im0 = imread(IM0, CV_LOAD_IMAGE_GRAYSCALE);
	Mat_<uchar> im1 = imread(IM1, CV_LOAD_IMAGE_GRAYSCALE);
	int disparityRange = DISP;

#if SAVE_TO_DISK
	make_directory(DIR_NAME);
	imwrite(IM0_GRAY, im0);
	imwrite(IM1_GRAY, im1);
#else
	imshow("Im0", im0);
	imshow("Im1", im1);
#endif

	uint64_t **cens0 = new uint64_t*[im0.rows];
	for (int i = 0; i < im0.rows; i++) {
		cens0[i] = new uint64_t[im0.cols];
		for (int j = 0; j < im0.cols; j++) {
			cens0[i][j] = 0;
		}
	}

	uint64_t **cens1 = new uint64_t*[im1.rows];
	for (int i = 0; i < im1.rows; i++) {
		cens1[i] = new uint64_t[im1.cols];
		for (int j = 0; j < im1.cols; j++) {
			cens1[i][j] = 0;
		}
	}

	census(im0, cens0);
	census(im1, cens1);

	short ***cost = new short**[im0.rows];
	for (int i = 0; i < im0.rows; i++) {
		cost[i] = new short*[im0.cols];
		for (int j = 0; j < im0.cols; j++) {
			cost[i][j] = new short[disparityRange];
			for (int d = 0; d < disparityRange; d++) {
				cost[i][j][d] = 0;
			}
		}
	}

	computeCost(cens0, cens1, im0.rows, im0.cols, disparityRange, cost);

	short ****paths = new short ***[im0.rows];
	for (int i = 0; i < im0.rows; i++) {
		paths[i] = new short **[im0.cols];
		for (int j = 0; j < im0.cols; j++) {
			paths[i][j] = new short*[disparityRange];
			for (int d = 0; d < disparityRange; d++) {
				paths[i][j][d] = new short[DIRECTIONS];
				for (int k = 0; k < DIRECTIONS; ++k) {
					paths[i][j][d][k] = 0;
				}
			}
		}
	}

	computePaths(paths, cost, im0.rows, im0.cols, disparityRange);

	short ***agcost = new short**[im0.rows];
	for (int i = 0; i < im0.rows; i++) {
		agcost[i] = new short*[im0.cols];
		for (int j = 0; j < im0.cols; j++) {
			agcost[i][j] = new short[disparityRange];
			for (int d = 0; d < disparityRange; d++) {
				agcost[i][j][d] = 0;
			}
		}
	}

	aggregatecosts(paths, agcost, im0.rows, im0.cols, disparityRange);

	Mat_<uchar> res = createDisparityMap(agcost, im0.rows, im0.cols, disparityRange);

#if SAVE_TO_DISK
	imwrite(IMRES, res);
#else
	imshow("Res", res);
#endif

	res = normalize(res);

#if SAVE_TO_DISK
	imwrite(IMRES_NORM, res);
#else
	imshow("Res normalized", res);
#endif

	res = medianFilter(res, 11);

#if SAVE_TO_DISK
	imwrite(IMRES_MED_FILT, res);
#else
	imshow("Res median filter", res);

	waitKey(0);
#endif
	
	return 0;
}
