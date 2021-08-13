#ifndef MACHININGUTILS_H
#define MACHININGUTILS_H

#include <Eigen/Core>
#include <QImage>
#include <QPoint>
#include <QVector>
#include <QPainterPath>

#include <opencv2/opencv.hpp>

namespace machiningUtils
{
    //void linePoints(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, QVector<Eigen::Vector2d> points, qreal factor);
    int linePoints(double x1, double y1, double x2, double y2, std::vector<cv::Point2f>& points, qreal factor, const Eigen::Matrix3d& transform);

    int path2Points(const QPainterPath& path, QVector<QPointF>& points, cv::Mat& canvas = cv::Mat());

    QByteArray points2Plt(const QVector<QPointF>& points);

    QByteArray image2Plt(const QImage& image);

    bool pointsEql(const QPointF& pt1, const QPointF& pt2);
}

#endif // MACHININGUTILS_H
