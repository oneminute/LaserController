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
    /// <param name="isClosed">0表示非封闭，1表示封闭，2表示未知，需要由函数本身判断</param>
    /// <returns></returns>
    int path2Points(const QPainterPath& path,
        quint32 progressCode,
        qreal progressQuota,
        LaserPointList& points,
        QList<int>& startingIndices,
        QPointF& center,
        int closed = 2,
        int startingIndiciesCount = 8, 
        int diagonalThreshold = 2 * 40);

    void path2Points(const QPainterPath& path,
        LaserPointListList& pointsList,
        QList<int>& startingIndices,
        QPointF& center = QPointF(),
        const QTransform& transform = QTransform());

    QList<QPolygonF> path2SubpathPolygons(const QPainterPath& path, const QTransform& matrix, qreal bezier_flattening_threshold = 0.5);

    void polygon2Points(const QPolygonF& polygon,
        LaserPointList& points,
        QList<int>& startingIndices,
        QPointF& center);

    QByteArray pointListList2Plt(const LaserPointListList& pointList, QPointF& lastPoint);

    QByteArray pointList2Plt(const LaserPointList& points, QPointF& lastPoint);

    QByteArray lineList2Plt(const LaserLineListList& lineList, QPointF& lastPoint);

    QByteArray image2Plt(const QImage& image);

    bool pointsEql(const QPointF& pt1, const QPointF& pt2);
}

#endif // MACHININGUTILS_H
