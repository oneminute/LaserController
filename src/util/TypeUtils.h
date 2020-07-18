#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <QObject>
#include <QPoint>

#include <opencv2/opencv.hpp>

namespace typeUtils
{
    cv::Point2f qtPointF2CVPoint2f(const QPointF& pt);
}

#endif // TYPEUTILS_H
