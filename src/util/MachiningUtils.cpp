#include "MachiningUtils.h"

#include <QtMath>
#include <QDebug>
#include <QQueue>
#include <QStack>

#include "common/common.h"
#include "common/Config.h"
#include "TypeUtils.h"
#include "Utils.h"

int machiningUtils::linePoints(double x1, double y1, double x2, double y2, std::vector<cv::Point2f>& points, qreal factor, const Eigen::Matrix3d& transform)
{
    Eigen::Vector2d ptStart(x1, y1);
    Eigen::Vector2d ptEnd(x2, y2);
    Eigen::Matrix2d rot = transform.topLeftCorner(2, 2);
    Eigen::Vector2d trans = transform.topRightCorner(2, 1);
    ptStart = rot * ptStart + trans;
    ptEnd = rot * ptEnd + trans;
    ptStart *= factor;
    ptEnd *= factor;
    Eigen::Vector2d line = ptEnd - ptStart;
    Eigen::Vector2d lineDir = line.normalized();

    double steps = line.norm();
    int count = 0;

    for (int i = 0; i < steps; i++)
    {
        Eigen::Vector2d pt = ptStart + lineDir * i;

        if (i > steps - 5)
        {
            if ((ptEnd - pt).dot(lineDir) < 0)
            {
                break;
            }
        }

        points.push_back(cv::Point2f(pt[0], pt[1]));
        count++;
    }
    return count;
}

int machiningUtils::path2Points(const QPainterPath & path, QVector<QPointF>& points, 
    QList<int>& startingIndices, int startingIndiciesCount, int diagonalThreshold, cv::Mat& canvas)
{
    qreal length = path.length();
    QPointF firstPoint = path.pointAtPercent(0);
    QPointF lastPoint = path.pointAtPercent(1);
    QPointF point = firstPoint;
    qreal radians = qDegreesToRadians(path.angleAtPercent(0));

    // Calculate bounding rect of current path. If the diagonal is less than diagonalThreshold,
    // we just choose the first point as the only starting point for closed curves,
    // and the first and the last points as starting points for opened curves.
    QRectF boundingRect = path.boundingRect();
    qreal diagonal = QVector2D(boundingRect.topLeft() - boundingRect.bottomRight()).length();
    if (diagonal < diagonalThreshold)
    {
        startingIndiciesCount = 1;
    }

    // Calculate starting points. We use ratio of length of curve to find proper starting point.
    QQueue<qreal> ratios;
    int startingPointsCount = 1;

    bool isClosed = utils::fuzzyEquals(firstPoint, lastPoint);
    ratios.enqueue(0);
    // If the curve is not closed, we dived it evenly into several segments, and store the 
    // end points of each segment.
    if (isClosed)
    {
        qreal avgRatio = 1.0 / startingIndiciesCount;
        for (int i = 0; i < startingIndiciesCount; i++)
        {
            ratios.append(avgRatio * i);
        }
    }
    else
    {
        ratios.append(1);
    }

    QPointF anchor = point;
    qreal anchorRadians = radians;
    qreal angle = 0;

    // 算法思路是，从第一个点开始建立锚点，并计算该锚点处的切线角度，然后后续每一个均计算其切角，
    // 当切角差值大于阈值时，建立新的锚点。
    // 同时，如果当前点距离上一个锚点距离大于阈值，也将建立锚点。
    // 计算锚点的同时，也会计算起刀点。设置最小外包框对角线阈值，小于该阈值，则不计算起刀点。非
    // 封闭曲线也不计算起刀点。其他的封闭曲线均按分割数计算起刀点。

    points.push_back(point);
    if (!canvas.empty())
    {
        cv::circle(canvas, typeUtils::qtPointF2CVPoint2f(point), 1, cv::Scalar(0, 0, 255));
    }

    for (int i = 1; i < length; i++)
    {
        qreal percent = i / length;
        if (!ratios.isEmpty() && percent > ratios.head())
        {
            ratios.dequeue();
            startingIndices.append(points.size() - 1);
        }

        point = path.pointAtPercent(percent);
        angle = path.angleAtPercent(percent);

        radians = qDegreesToRadians(angle);

        qreal diff = qAbs(radians - anchorRadians);
        qreal dist = QLineF(point, anchor).length();

        diff = qRadiansToDegrees(diff);
        if (diff >= Config::Export::maxAnglesDiff() || radians * anchorRadians < 0)
        {
            if (!canvas.empty())
            {
                cv::line(canvas, typeUtils::qtPointF2CVPoint2f(anchor), typeUtils::qtPointF2CVPoint2f(point), cv::Scalar(255, 0, 0));
                cv::circle(canvas, typeUtils::qtPointF2CVPoint2f(point), 1, cv::Scalar(0, 0, 255));
            }
            points.push_back(point);
            anchor = point;
            anchorRadians = radians;
        }
        else if (diff != 0 && dist >= Config::Export::maxIntervalDistance())
        {
            if (!canvas.empty())
            {
                cv::line(canvas, typeUtils::qtPointF2CVPoint2f(anchor), typeUtils::qtPointF2CVPoint2f(point), cv::Scalar(255, 0, 0));
                cv::circle(canvas, typeUtils::qtPointF2CVPoint2f(point), 1, cv::Scalar(0, 0, 255));
            }
            points.push_back(point);
            anchor = point;
            anchorRadians = radians;
        }
    }

    points.push_back(lastPoint);
    if (!ratios.isEmpty())
    {
        startingIndices.append(points.length() - 1);
    }
    if (!canvas.empty()) 
    {
        QPointF startPt = path.pointAtPercent(0);
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(anchor), typeUtils::qtPointF2CVPoint2f(point), cv::Scalar(0));
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(point), typeUtils::qtPointF2CVPoint2f(startPt), cv::Scalar(0));

        for (int i : startingIndices)
        {
            QPointF pt = points[i];
            cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt + QPointF(-40, -40)), typeUtils::qtPointF2CVPoint2f(pt + QPointF(40, 40)), cv::Scalar(0, 0, 255));
            cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt + QPointF(-40, 40)), typeUtils::qtPointF2CVPoint2f(pt + QPointF(40, -40)), cv::Scalar(0, 0, 255));
        }
    }

    qLogD << "path with " << path.elementCount() << " elements convert to " << points.size() << " points.";
    return points.size();
}

QByteArray machiningUtils::points2Plt(const QVector<QPointF>& points)
{
    QByteArray buffer;
    if (points.empty())
        return buffer;

    QPointF pt = points[0];
    buffer.append(QString("PU%1 %2;").arg(qRound(pt.x())).arg(-qRound(pt.y())));
    for (size_t i = 0; i < points.size(); i++)
    {
        pt = points[i];
        buffer.append(QString("PD%1 %2;").arg(qRound(pt.x())).arg(-qRound(pt.y())));
    }
    return buffer;
}

QByteArray machiningUtils::image2Plt(const QImage & image)
{
    return QByteArray();
}

bool machiningUtils::pointsEql(const QPointF & pt1, const QPointF & pt2)
{
    return qRound(pt1.x()) == qRound(pt2.x()) && qRound(pt1.y()) == qRound(pt2.y());
}
