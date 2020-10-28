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
    cv::Mat halftone2(cv::Mat src, float lpi = 100, float dpi = 600, float degrees = 45.0, float nonlinearCoefficient = 1.5f);
    cv::Mat halftone3(cv::Mat src, float lpi = 100, float dpi = 600, float degrees = 45.0, float nonlinearCoefficient = 1.5f);

    // Floyd-Steinberg
    cv::Mat floydSteinberg(cv::Mat src, float mmWidth, float mmHeight, float lpi = 100, float dpi = 600);

    cv::Mat generateSpiralPattern(int gridSize, int& grades, float degrees = 45);
    cv::Mat generateSpiralDitchMatRec(int circleNum);

    cv::Mat generateBayerPattern(int gridSize, int& grades, float degrees = 45);
    cv::Mat generateBayerDitchMatRec(int circleNum);

    cv::Mat generateCircleMat(int gridSize, int& grades, float degrees = 45);

    cv::Mat generateRotatedPattern(cv::Mat src, int gridSize, float degrees = 45);
    cv::Mat generateRotatedPattern2(cv::Mat src, int gridSize, float degrees = 45);
    cv::Mat generateRotatedPattern45(cv::Mat src);

    cv::Mat generateRoundSpiralPattern(int gridSize, int& grades, float degrees = 45);
    cv::Mat generateRoundSpiralMat(int gridSize);

    cv::Mat rotateMat(cv::Mat src, float degrees);

    QByteArray image2EngravingData(cv::Mat mat, qreal x, qreal y, qreal rowInterval, qreal width);

    QPointF closestPointTo(const QPointF &target, const QPainterPath &sourcePath);
    bool hit(const QLineF &line, const QPainterPath& path, QPointF &hitPos);

}

#endif // 