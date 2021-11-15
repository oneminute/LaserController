#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QImage>
#include <opencv2/opencv.hpp>
#include <QMap>
#include <Eigen/Core>
#include <Eigen/Dense>

class ProgressItem;

#define MM_TO_INCH 0.03937

namespace imageUtils
{
    cv::Mat halftone(cv::Mat src, float mmWidth, float mmHeight, float lpi = 100, float dpi = 600, float degrees = 45.0);
    cv::Mat halftone2(cv::Mat src, float lpi = 100, float dpi = 600, float degrees = 45.0, float nonlinearCoefficient = 1.5f);
    cv::Mat halftone3(cv::Mat src, float lpi = 100, float dpi = 600, float degrees = 45.0);
    cv::Mat halftone4(ProgressItem* progress, cv::Mat src,float degrees = 45.0, int gridSize = 12);
    cv::Mat halftone5(cv::Mat src,float degrees = 45.0, int gridSize = 12);
    cv::Mat halftone6(ProgressItem* progress, cv::Mat src,float degrees = 45.0, int gridSize = 12);

    int sumMat(const cv::Mat& mat, QPoint& point);
    void generateGroupingDitcher(cv::Mat& srcRoi, cv::Mat& dstRoi);
    void generatePattern(cv::Mat& dstRoi, int sum, QPoint& center, int initAngle = 90, int rotationAngle = -90, int stepAngle = 45);

    // Floyd-Steinberg
    cv::Mat floydSteinberg(cv::Mat src, float mmWidth, float mmHeight, float lpi = 50, float dpi = 600);

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

    QByteArray image2EngravingData(ProgressItem* parentProgress, cv::Mat mat, 
        const QRectF& boundingRect, qreal rowInterval, QPointF& lastPoint, qreal accLength);

    QPointF closestPointTo(const QPointF &target, const QPainterPath &sourcePath);
    bool hit(const QLineF& ray, const QPainterPath& target, QPointF& hitPos);

}

#endif // 