#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QImage>
#include <opencv2/opencv.hpp>
#include <QMap>
#include <Eigen/Core>
#include <Eigen/Dense>

#define MM_TO_INCH 0.03937

namespace imageUtils
{
    cv::Mat halftone(cv::Mat src, float mmWidth, float mmHeight, float lpi = 100, float dpi = 600, float degrees = 45.0);

    // Floyd-Steinberg
    cv::Mat floydSteinberg(cv::Mat src, float mmWidth, float mmHeight, float lpi = 100, float dpi = 600);

    cv::Mat generateSpiralDitchMat(int circleNum, float degrees = 45);
    cv::Mat generateSpiralDitchMatRec(int circleNum);

    cv::Mat generateBayerDitchMat(int circleNum, float degrees = 45);
    cv::Mat generateBayerDitchMatRec(int circleNum);

    cv::Mat generateCircleMat(int size, float degrees = 45);

    cv::Mat generateRotatedPattern(cv::Mat src, float degrees = 45);
}

#endif // 