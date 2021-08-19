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
    int linePoints(double x1, double y1, double x2, double y2, std::vector<cv::Point2f>& points, qreal factor, const Eigen::Matrix3d& transform);

    /// <summary>
    /// 通过QPainterPath对象创建加工用的多边形。每一个点的单位均为1/40毫米。
    /// </summary>
    /// <param name="path"></param>
    /// <param name="points"></param>
    /// <param name="startingIndices"></param>
    /// <param name="startingIndiciesCount"></param>
    /// <param name="diagonalThreshold"></param>
    /// <param name="canvas"></param>
    /// <returns></returns>
    int path2Points(const QPainterPath& path, 
        QVector<QPointF>& points, 
        QList<int>& startingIndices,
        QPointF& center,
        int startingIndiciesCount = 8, 
        int diagonalThreshold = 2 * 40,
        cv::Mat& canvas = cv::Mat());

    QByteArray points2Plt(const QVector<QPointF>& points);

    QByteArray image2Plt(const QImage& image);

    bool pointsEql(const QPointF& pt1, const QPointF& pt2);
}

#endif // MACHININGUTILS_H
