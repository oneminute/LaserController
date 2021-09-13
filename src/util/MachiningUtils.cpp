#include "MachiningUtils.h"

#include <QtMath>
#include <QDebug>
#include <QQueue>
#include <QStack>
#include <QVector4D>

#include "common/common.h"
#include "common/Config.h"
#include "LaserApplication.h"
#include "ui/PreviewWindow.h"
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

int machiningUtils::path2Points(const QPainterPath & path, quint32 progressCode, 
    qreal progressQuota, LaserPointList& points, 
    QList<int>& startingIndices, QPointF& center, int closed, int startingIndiciesCount, 
    int diagonalThreshold)
{
    points.clear();
    startingIndices.clear();
    center = QPointF(0, 0);

    qreal length = path.length();
    QPointF firstPoint = path.pointAtPercent(0);
    QPointF lastPoint = path.pointAtPercent(1);
    QPointF point = firstPoint;
    qreal angle = path.angleAtPercent(0);
    qreal radians = qDegreesToRadians(angle);
    LaserPoint point4d(point, angle, angle - 360);

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

    bool isClosed;
    switch(closed)
    {
    case 0:
        isClosed = false;
        break;
    case 1:
        isClosed = true;
        break;
    case 2:
        isClosed = utils::fuzzyEquals(firstPoint, lastPoint);
        break;
    }
    ratios.enqueue(0);
    // If the curve is not closed, we dived it evenly into several segments, and store the 
    // end points of each segment.
    if (isClosed)
    {
        qreal avgRatio = 1.0 / startingIndiciesCount;
        for (int i = 1; i < startingIndiciesCount; i++)
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
    angle = 0;

    // 算法思路是，从第一个点开始建立锚点，并计算该锚点处的切线角度，然后后续每一个均计算其切角，
    // 当切角差值大于阈值时，建立新的锚点。
    // 同时，如果当前点距离上一个锚点距离大于阈值，也将建立锚点。
    // 计算锚点的同时，也会计算起刀点。设置最小外包框对角线阈值，小于该阈值，则不计算起刀点。非
    // 封闭曲线也不计算起刀点。其他的封闭曲线均按分割数计算起刀点。

    points.push_back(point4d);
    center += point;

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
        point4d.setAll(point, angle);
        //qLogD << i << ", angle: " << angle;

        radians = qDegreesToRadians(angle);

        qreal diff = qAbs(radians - anchorRadians);
        qreal dist = QLineF(point, anchor).length();

        diff = qRadiansToDegrees(diff);
        if (diff >= Config::Export::maxAnglesDiff() || radians * anchorRadians < 0)
        {
            points.push_back(point4d);
            center += point;
            anchor = point;
            anchorRadians = radians;
        }
        else if (diff != 0 && dist >= Config::Export::maxIntervalDistance())
        {
            points.push_back(point4d);
            center += point;
            anchor = point;
            anchorRadians = radians;
        }
        if (i % 100 == 0)
        {
            LaserApplication::previewWindow->addProgress(progressCode, 100.0 * progressQuota / length);
        }
    }
    LaserApplication::previewWindow->addProgress(progressCode, (qFloor(length) % 100) * progressQuota / length);

    angle = path.angleAtPercent(1);
    point4d.setAll(lastPoint, angle);
    points.push_back(point4d);
    center += point;
    center = center / points.size();
    if (!ratios.isEmpty())
    {
        startingIndices.append(points.length() - 1);
    }

    qLogD << "path with " << path.elementCount() << " elements convert to " << points.size() << " points.";
    return points.size();
}

QByteArray machiningUtils::points2Plt(const LaserPointList& points, QPointF& lastPoint)
{
    QByteArray buffer;
    if (points.empty())
        return buffer;

    QPointF pt = points.first().toPointF();
    QPointF diff = pt - lastPoint;
    lastPoint = pt;
    buffer.append(QString("PU%1 %2;").arg(qRound(diff.x())).arg(-qRound(diff.y())));
    for (size_t i = 0; i < points.size(); i++)
    {
        pt = points[i].toPointF();
        diff = pt - lastPoint;
        lastPoint = pt;
        buffer.append(QString("PD%1 %2;").arg(qRound(diff.x())).arg(-qRound(diff.y())));
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
