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
#include "algorithm/QBezier.h"
#include "laser/LaserDevice.h"
#include "task/ProgressItem.h"
#include "task/ProgressModel.h"

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

int machiningUtils::path2Points(
    ProgressItem* parentProgress,
    const QPainterPath & path, LaserPointList& points, 
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
        
    }

    angle = path.angleAtPercent(1);
    point4d.setAll(lastPoint, angle);
    points.push_back(point4d);
    center += point;
    center = center / points.size();
    if (!ratios.isEmpty())
    {
        startingIndices.append(points.length() - 1);
    }

    //qLogD << "path with " << path.elementCount() << " elements convert to " << points.size() << " points.";
    return points.size();
}

void machiningUtils::path2Points(
    ProgressItem* parentProgress,
    const QPainterPath& path, LaserPointListList& pointsList, 
    QList<int>& startingIndices, QPointF& center, const QTransform& transform)
{
    pointsList.clear();
    startingIndices.clear();
    ProgressItem* progress = nullptr;
    if (parentProgress)
    {
        if (parentProgress->progressType() == ProgressItem::PT_Simple)
        {
            progress = parentProgress;
        }
        else if (parentProgress->progressType() == ProgressItem::PT_Complex)
        {
            progress = LaserApplication::progressModel->createSimpleItem(QObject::tr("Path to points"), parentProgress);
            progress->setMaximum(path.elementCount());
        }
    }
    QList<QPolygonF> polygons = path2SubpathPolygons(parentProgress, path, transform, Config::Export::curveFlatteningThreshold());
    LaserPointList allPoints;
    for (QPolygonF& polygon : polygons)
    {
        LaserPointList points;
        QList<int> indices;
        polygon2Points(nullptr, polygon, points, indices, QPointF());

        for (int index : indices)
        {
            startingIndices.append(index + allPoints.count());
        }
        allPoints.append(points);
        pointsList.append(points);
        if (progress)
            progress->increaseProgress();
    }
    center = utils::center(allPoints).toPointF();
    if (progress)
        progress->finish();
}

QList<QPolygonF> machiningUtils::path2SubpathPolygons(
    ProgressItem* parentProgress,
    const QPainterPath& path, const QTransform& matrix, 
    qreal bezier_flattening_threshold)
{
    QList<QPolygonF> flatCurves;
    if (path.isEmpty())
        return flatCurves;

    ProgressItem* progress = nullptr;
    if (parentProgress)
    {
        if (parentProgress->progressType() == ProgressItem::PT_Simple)
        {
            progress = parentProgress;
            progress->setMaximum(path.elementCount());
        }
        else if (parentProgress->progressType() == ProgressItem::PT_Complex)
        {
            progress = LaserApplication::progressModel->createSimpleItem(QObject::tr("Bazier to polygons"), parentProgress);
            progress->setMaximum(path.elementCount());
        }
    }
    QPolygonF current;
    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element& e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            if (current.size() > 1)
                flatCurves += current;
            current.clear();
            current.reserve(16);
            current += QPointF(e.x, e.y) * matrix;
            break;
        case QPainterPath::LineToElement:
            current += QPointF(e.x, e.y) * matrix;
            break;
        case QPainterPath::CurveToElement: {
            Q_ASSERT(path.elementAt(i + 1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(path.elementAt(i + 2).type == QPainterPath::CurveToDataElement);
            QBezier bezier = QBezier::fromPoints(QPointF(path.elementAt(i - 1).x, path.elementAt(i - 1).y) * matrix,
                QPointF(e.x, e.y) * matrix,
                QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y) * matrix,
                QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y) * matrix);
            bezier.addToPolygon(&current, bezier_flattening_threshold);
            i += 2;
            break;
        }
        case QPainterPath::CurveToDataElement:
            Q_ASSERT(!"QPainterPath::toSubpathPolygons(), bad element type");
            break;
        }
        if (progress)
            progress->increaseProgress();
    }
    if (current.size() > 1)
        flatCurves += current;
    if (progress)
        progress->finish();
    return flatCurves;
}

void machiningUtils::polygon2Points(
    ProgressItem* parentProgress,
    const QPolygonF& polygon, LaserPointList& points, 
    QList<int>& startingIndices, QPointF& center)
{
    ProgressItem* progress = nullptr;
    if (parentProgress)
    {
        if (parentProgress->progressType() == ProgressItem::PT_Simple)
        {
            progress = parentProgress;
        }
        else if (parentProgress->progressType() == ProgressItem::PT_Complex)
        {
            progress = LaserApplication::progressModel->createSimpleItem(QObject::tr("Polygon to Points"), parentProgress);
            progress->setMaximum(polygon.size());
        }
    }
    points.clear();
    startingIndices.clear();
    bool isClosed = utils::fuzzyCompare(polygon.first(), polygon.last());
    center = QPointF(0, 0);
    for (int i = 0; i < polygon.size(); i++)
    {
        QPointF pt = polygon[i];
        QPointF cPt = pt;
        QPointF nPt = (i == polygon.size() - 1) ? polygon.first() : polygon.at(i + 1);
        QPointF lPt = (i == 0) ? polygon.last() : polygon.at(i - 1);
        QLineF line1(cPt, nPt);
        QLineF line2(cPt, lPt);
        qreal angle1 = line1.angle();
        qreal angle2 = line2.angle();
        points.append(LaserPoint(pt.x(), pt.y(), angle1, angle2));
        center += pt;
        if (isClosed)
        {
            startingIndices.append(i);
        }
        if (progress)
            progress->increaseProgress();
    }
    if (!isClosed)
    {
        startingIndices.append(0);
        startingIndices.append(points.length() - 1);
    }
        
    center /= points.size();
    if (progress)
        progress->finish();
}

QByteArray machiningUtils::pointListList2Plt(ProgressItem* progress, const LaserPointListList& pointList, QPointF& lastPoint, const QTransform& t)
{
    QByteArray buffer;
    int total = 0;
    for (const LaserPointList& points : pointList)
    {
        total += points.length();
    }
    if (progress)
        progress->setMaximum(total);
    for (const LaserPointList& points : pointList)
    {
        buffer.append(pointList2Plt(progress, points, lastPoint, t));
    }
    if (progress)
        progress->finish();
    return buffer;
}

QByteArray machiningUtils::pointList2Plt(ProgressItem* progress, const LaserPointList& points, QPointF& lastPoint, const QTransform& t)
{
    QByteArray buffer;
    if (points.empty())
        return buffer;

    QPointF pt = t.map(points.first().toPointF());
    QPointF diff = pt - lastPoint;
    lastPoint = pt;
    if (Config::Device::startFrom() != SFT_AbsoluteCoords)
        buffer.append(QString("pu%1 %2;").arg(qRound(diff.x())).arg(qRound(diff.y())));
    else
    {
        //int x = LaserApplication::device->deviceTranslateXMachining(qRound(pt.x()));
        //int y = LaserApplication::device->deviceTranslateYMachining(qRound(pt.y()));
        buffer.append(QString("PU%1 %2;").arg(pt.x()).arg(pt.y()));
    }
    for (size_t i = 1; i < points.size(); i++)
    {
        LaserPoint lPt = points.at(i);
        QPointF pt = t.map(lPt.toPointF());
        QPointF diff = pt - lastPoint;
        QString command = "PD%1 %2;";

        if (points.size() == 2)
        {
            command = "CO%1 %2;";
        }
        else if (i == 1)
        {
            command = "CS%1 %2;";
        }
        else if (i == points.length() - 1)
        {
            command = "CE%1 %2;";
        }
        else
        {
            command = "CM%1 %2;";
        }

        if (Config::Device::startFrom() != SFT_AbsoluteCoords)
        {
            command = command.toLower();
            buffer.append(command.arg(qRound(diff.x())).arg(qRound(diff.y())));
        }
        else
        {
            //int x = LaserApplication::device->deviceTranslateXMachining(qRound(pt.x()));
            //int y = LaserApplication::device->deviceTranslateYMachining(qRound(pt.y()));
            buffer.append(command.arg(pt.x()).arg(pt.y()));
        }
        lastPoint = pt;
        if (progress)
            progress->increaseProgress();
    }
    return buffer;
}

QByteArray machiningUtils::lineList2Plt(ProgressItem* progress, const LaserLineListList& lineList, QPointF& lastPoint)
{
    QByteArray buffer;
    // 建立所有线点的kdtree
    //lines.buildKdtree();
    // 从起刀点位置开始依次寻找最优点
    //QPointF point = path.boundingRect().topLeft();
    //QLineF lastLine = lines.first();
    progress->setMaximum(lineList.count());
    bool forward = true;
    for (int i = 0; i < lineList.count(); i++)
    {
        //QLineF line = lines.nearestSearch(point);
        LaserLineList lines = lineList.at(i);
        
        int step = 1;
        int j = 0;
        int end = lines.length();
        if (!forward)
        {
            step = -1;
            j = lines.length() - 1;
            end = -1;
        }

        int count = 0;
        for (; j != end; j += step)
        {
            QLineF line = lines.at(j);

            QPointF pt1 = line.p1();
            QPointF pt2 = line.p2();
            if (!forward)
            {
                pt1 = line.p2();
                pt2 = line.p1();
            }

            QPointF diff1 = pt1 - lastPoint;
            QPointF diff2 = pt2 - pt1;
            QString command1 = "PU%1 %2;";
            QString command2 = "CM%1 %2;";

            if (lines.length() == 1)
            {
                command2 = "CO%1 %2;";
            }
            else
            {
                if (count == 0)
                {
                    command2 = "CS%1 %2;";
                }
                else if (count == lines.length() - 1)
                {
                    command1 = "CU%1 %2;";
                    command2 = "CE%1 %2;";
                }
                else
                {
                    command1 = "CU%1 %2;";
                }
                count++;
            }
            if (Config::Device::startFrom() != SFT_AbsoluteCoords)
            {
                command1 = command1.toLower();
                command2 = command2.toLower();
                buffer.append(command1.arg(qRound(diff1.x())).arg(qRound(diff1.y())));
                buffer.append(command2.arg(qRound(diff2.x())).arg(qRound(diff2.y())));
            }
            else
            {
                //int x = LaserApplication::device->deviceTranslateXMachining(qRound(pt1.x()));
                //int y = LaserApplication::device->deviceTranslateYMachining(qRound(pt1.y()));
                buffer.append(command1.arg(pt1.x()).arg(pt1.y()));
                //x = LaserApplication::device->deviceTranslateXMachining(qRound(pt2.x()));
                //y = LaserApplication::device->deviceTranslateYMachining(qRound(pt2.y()));
                buffer.append(command2.arg(pt2.x()).arg(pt2.y()));
            }
            //point = line.p2();
            lastPoint = pt2;
        }
        forward = !forward;
        progress->increaseProgress();
    }
    progress->finish();
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
