#include "Ostromoukhov.h"
#include <vector>
#include <algorithm>
#include "utils.h"

using namespace std;

double table[128][3] = {
    {13, 0, 5},
    {13, 0, 5},
    {21, 0, 10},
    {7, 0, 4},
    {8, 0, 5},
    {47, 3, 28},
    {23, 3, 13},
    {15, 3, 8},
    {22, 6, 11},
    {43, 15, 20},
    {7, 3, 3},
    {501, 224, 211},
    {249, 116, 103},
    {165, 80, 67},
    {123, 62, 49},
    {489, 256, 191},
    {81, 44, 31},
    {483, 272, 181},
    {60, 35, 22},
    {53, 32, 19},
    {237, 148, 83},
    {471, 304, 161},
    {3, 2, 1},
    {481, 314, 185},
    {354, 226, 155},
    {1389,866, 685},
    {227, 138, 125},
    {267, 158, 163},
    {327, 188, 220},
    {61, 34, 45},
    {627, 338, 505},
    {1227,638, 1075},
    {20, 10, 19},
    {1937,1000,1767},
    {977, 520, 855},
    {657, 360, 551},
    {71, 40, 57},
    {2005,1160,1539},
    {337, 200, 247},
    {2039,1240,1425},
    {257, 160, 171},
    {691, 440, 437},
    {1045,680, 627},
    {301, 200, 171},
    {177, 120, 95},
    {2141,1480,1083},
    {1079,760, 513},
    {725, 520, 323},
    {137, 100, 57},
    {2209,1640,855},
    {53, 40, 19},
    {2243,1720,741},
    {565, 440, 171},
    {759, 600, 209},
    {1147,920, 285},
    {2311,1880,513},
    {97, 80, 19},
    {335, 280, 57},
    {1181,1000,171},
    {793, 680, 95},
    {599, 520, 57},
    {2413,2120,171},
    {405, 360, 19},
    {2447,2200,57},
    {11, 10, 0},
    {158, 151, 3},
    {178, 179, 7},
    {1030,1091,63},
    {248, 277, 21},
    {318, 375, 35},
    {458, 571, 63},
    {878, 1159,147},
    {5, 7, 1},
    {172, 181, 37},
    {97, 76, 22},
    {72, 41, 17},
    {119, 47, 29},
    {4, 1, 1},
    {4, 1, 1},
    {4, 1, 1},
    {4, 1, 1},
    {4, 1, 1},
    {4, 1, 1},
    {4, 1, 1},
    {4, 1, 1},
    {4, 1, 1},
    {65, 18, 17},
    {95, 29, 26},
    {185, 62, 53},
    {30, 11, 9},
    {35, 14, 11},
    {85, 37, 28},
    {55, 26, 19},
    {80, 41, 29},
    {155, 86, 59},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {5, 3, 2},
    {305, 176, 119},
    {155, 86, 59},
    {105, 56, 39},
    {80, 41, 29},
    {65, 32, 23},
    {55, 26, 19},
    {335, 152, 113},
    {85, 37, 28},
    {115, 48, 37},
    {35, 14, 11},
    {355, 136, 109},
    {30, 11, 9},
    {365, 128, 107},
    {185, 62, 53},
    {25, 8, 7},
    {95, 29, 26},
    {385, 112, 103},
    {65, 18, 17},
    {395, 104, 101},
    {4, 1, 1}
};

double d(int i, int j) {
    double res;
    if (i > 127) i = 255 - i;
    res = table[i][j] / (table[i][0] + table[i][1] + table[i][2]);
    return res;
}

double d10(int i) {
    return d(i,0);
}

double d_11(int i) {
    return d(i,1);
}

double d01(int i) {
    return d(i,2);
}

inline double round(double d)
{
    return floor(d + 0.5);
}

IplImage *OstromoukhovHalftone(IplImage *I) {
    vector<float> vd; vd.reserve(I->width * I->height);
    for (int i = 0; i < I->width * I->height; i++) {
        vd.push_back(cvGet1D(I, i).val[0]);
    }
    sort(vd.begin(), vd.end());
    reverse(vd.begin(), vd.end());
    float avgLum = cvAvg(I).val[0];
    int idx = round(avgLum * I->width * I->height);
    float thres = vd[idx];
    //printf("%lf %lf\n", thres, avgLum);
    IplImage *temp = cvCloneImage(I);
    IplImage *res = cvCreateImage(cvSize(I->width, I->height), 8, 1);
    cvZero(res);
    for (int i = 0; i < I->height; i++) {
        for (int j = 0; j < I->width; j++) {
            double error = 0.;
            double val = cvGet2D(temp,i,j).val[0];
            //val = clamp(val, 0., 1.);
            if (val > avgLum) {
                cvSet2D(res, i, j, cvScalar(1));
                error = val - 1.;
            }
            else {
                cvSet2D(res, i, j, cvScalar(0));
                error = val;
            }
            int level = round(val*255.0);
            level = clamp(level, 0, 255);
            if (j + 1 < I->width)
                (CV_IMAGE_ELEM(temp, float, i, j + 1)) += error *d10(level);
            if (i + 1 < I->height && j - 1 >= 0)
                (CV_IMAGE_ELEM(temp, float, i + 1, j - 1)) += error *d_11(level);
            if (i + 1 < I->height)
                (CV_IMAGE_ELEM(temp, float, i + 1, j)) += error *d01(level);
        }
    }
    //printf("%lf", cvAvg(res).val[0]);
    cvReleaseImage(&temp);
    return res;
}

cv::Mat OstromoukhovHalftone(cv::Mat I) {
    CV_Assert(I.type() == CV_32FC1);
    int nPix = I.rows * I .cols;
    vector<float> vd; vd.reserve(nPix);
    for (int i = 0; i < I.rows; i++) {
        for (int j = 0; j < I.cols; j++) {
            vd.push_back(I.at<float>(i,j));
        }
    }
    sort(vd.begin(), vd.end());
    reverse(vd.begin(), vd.end());
    float avgLum = cv::mean(I)[0];
    int idx = round(avgLum * nPix);
    float thres = vd[idx];
    //printf("%lf %lf\n", thres, avgLum);
    cv::Mat temp = I.clone();
    cv::Mat res(I.rows, I.cols, CV_8UC1);
    res.setTo(0);
    for (int i = 0; i < I.rows; i++) {
        for (int j = 0; j < I.cols; j++) {
            float error = 0.;
            float val = temp.at<float>(i,j);
            //val = clamp(val, 0., 1.);
            if (val > avgLum) {
                res.at<uchar>(i,j) = 1;
                error = val - 1.;
            }
            else {
                res.at<uchar>(i,j) = 0;
                error = val;
            }
            int level = round(val*255.0);
            level = clamp(level, 0, 255);
            if (j + 1 < I.cols)
                temp.at<float>(i,j + 1) += error *d10(level);
            if (i + 1 < I.rows && j - 1 >= 0)
                temp.at<float>(i + 1,j - 1) +=  error *d_11(level);
            if (i + 1 < I.rows)
                temp.at<float>(i + 1, j) += error *d01(level);
        }
    }
    //printf("%lf", cvAvg(res).val[0]);
    return res;
}

