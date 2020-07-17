#ifndef PLTUTILS_H
#define PLTUTILS_H

#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include <QImage>

#include <QPoint>
#include <QVector>

namespace pltUtils
{
    //void linePoints(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, QVector<Eigen::Vector2d> points, qreal factor);
    int linePoints(double x1, double y1, double x2, double y2, std::vector<cv::Point2f>& points, qreal factor, const Eigen::Matrix3d& transform);

    QByteArray points2Plt(const std::vector<cv::Point2f>& points);

    QByteArray image2Plt(const QImage& image);
}

#endif // PLTUTILS_H
