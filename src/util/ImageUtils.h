#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QImage>
#include <opencv2/opencv.hpp>
#include <QMap>

class ProgressItem;

#define MM_TO_INCH 0.03937

namespace imageUtils
{
    cv::Mat halftone6(ProgressItem* progress, cv::Mat src,float degrees = 45.0, int gridSize = 12);

    int sumMat(const cv::Mat& mat, QPoint& point);
    void generatePattern(cv::Mat& dstRoi, int sum, QPoint& center, int initAngle = 90, int rotationAngle = -90, int stepAngle = 45);

    QByteArray image2EngravingData(ProgressItem* parentProgress, cv::Mat mat, 
        const QRect& boundingRect, int rowInterval, QPoint& lastPoint,
        const QTransform& t = QTransform());

    QPointF closestPointTo(const QPointF &target, const QPainterPath &sourcePath);
    bool hit(const QLineF& ray, const QPainterPath& target, QPointF& hitPos);

    QImage parseImageData(const QByteArray& data, int rowInterval);
}

#endif // 