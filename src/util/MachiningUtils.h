#ifndef MACHININGUTILS_H
#define MACHININGUTILS_H

#include <Eigen/Core>
#include <QImage>
#include <QPoint>
#include <QVector>
#include <QPainterPath>
#include <opencv2/opencv.hpp>

#include "laser/LaserPointList.h"
#include "laser/LaserLineList.h"

class ProgressItem;

namespace machiningUtils
{
    int linePoints(double x1, double y1, double x2, double y2, std::vector<cv::Point2f>& points, qreal factor, const Eigen::Matrix3d& transform);

    void path2Points(
        ProgressItem* parentProgress,
        const QPainterPath& path,
        LaserPointListList& pointsList, 
        QList<int>& startingIndices,
        QPoint& center = QPoint(),
        const QTransform& transform = QTransform());

    QList<QPolygon> path2SubpathPolygons(
        ProgressItem* parentProgress,
        const QPainterPath& path, const QTransform& matrix,
        qreal bezier_flattening_threshold = 0.5);

    void polygon2Points(
        ProgressItem* parentProgress,
        const QPolygon& polygon,
        LaserPointList& points,
        QList<int>& startingIndices,
        QPoint& center);

    QByteArray pointListList2Plt(ProgressItem* progress, const LaserPointListList& pointList, QPoint& lastPoint, const QTransform& t = QTransform());

    QByteArray pointList2Plt(ProgressItem* progress, const LaserPointList& points, QPoint& lastPoint, const QTransform& t = QTransform());

    QPoint calculateResidual(const QPointF& diff, QPointF& residual);

    QByteArray lineList2Plt(ProgressItem* progress, const LaserLineListList& lineList, QPoint& lastPoint);

    QByteArray image2Plt(const QImage& image);

    bool pointsEql(const QPointF& pt1, const QPointF& pt2);

    QList<QPoint> boundingPoints(int originIndex, const QRect& bounding);

    QList<QPoint> boundingPoints(int jobIndex, const QRect& bounding, const QPoint& startPos);
}

#endif // MACHININGUTILS_H
