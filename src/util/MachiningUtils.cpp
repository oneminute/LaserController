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

void machiningUtils::path2Points(
    ProgressItem* parentProgress,
    const QPainterPath& path, LaserPointListList& pointsList, 
    QList<int>& startingIndices, QPoint& center, const QTransform& transform)
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
    QList<QPolygon> polygons = path2SubpathPolygons(parentProgress, path, transform, Config::Export::curveFlatteningThreshold());
    LaserPointList allPoints;
    for (QPolygon& polygon : polygons)
    {
        LaserPointList points;
        QList<int> indices;
        polygon2Points(nullptr, polygon, points, indices, QPoint());

        for (int index : indices)
        {
            startingIndices.append(index + allPoints.count());
        }
        allPoints.append(points);
        pointsList.append(points);
        if (progress)
            progress->increaseProgress();
    }
    center = utils::center(allPoints).toPoint();
    if (progress)
        progress->finish();
}

QList<QPolygon> machiningUtils::path2SubpathPolygons(
    ProgressItem* parentProgress,
    const QPainterPath& path, const QTransform& matrix, 
    qreal bezier_flattening_threshold)
{
    QList<QPolygon> flatCurves;
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
    QPolygon current;
    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element& e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            if (current.size() > 1)
                flatCurves += current;
            current.clear();
            current.reserve(16);
            current += QPoint(e.x, e.y) * matrix;
            break;
        case QPainterPath::LineToElement:
            current += QPoint(e.x, e.y) * matrix;
            break;
        case QPainterPath::CurveToElement: {
            Q_ASSERT(path.elementAt(i + 1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(path.elementAt(i + 2).type == QPainterPath::CurveToDataElement);
            QBezier bezier = QBezier::fromPoints(QPoint(path.elementAt(i - 1).x, path.elementAt(i - 1).y) * matrix,
                QPoint(e.x, e.y) * matrix,
                QPoint(path.elementAt(i + 1).x, path.elementAt(i + 1).y) * matrix,
                QPoint(path.elementAt(i + 2).x, path.elementAt(i + 2).y) * matrix);
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
    const QPolygon& polygon, LaserPointList& points, 
    QList<int>& startingIndices, QPoint& center)
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
    center = QPoint(0, 0);
    for (int i = 0; i < polygon.size(); i++)
    {
        QPoint pt = polygon[i];
        QPoint cPt = pt;
        QPoint nPt = (i == polygon.size() - 1) ? polygon.first() : polygon.at(i + 1);
        QPoint lPt = (i == 0) ? polygon.last() : polygon.at(i - 1);
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

QByteArray machiningUtils::pointListList2Plt(ProgressItem* progress, 
    const LaserPointListList& pointList, QPoint& lastPoint, const QTransform& t)
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

QByteArray machiningUtils::pointList2Plt(ProgressItem* progress, const LaserPointList& points, QPoint& lastPoint, const QTransform& t)
{
    QByteArray buffer;
    if (points.empty())
        return buffer;

    QPoint pt = t.map(points.first().toPoint());
    QPoint diff = pt - lastPoint;
    if (Config::Device::fullRelative() || Config::Device::startFrom() != SFT_AbsoluteCoords)
    {
        //buffer.append(QString("pu%1 %2;").arg(rel.x()).arg(rel.y()));
        buffer.append(QString("pu%1 %2;").arg(pt.x() - lastPoint.x()).arg(pt.y() - lastPoint.y()));
    }
    else
    {
        buffer.append(QString("PU%1 %2;").arg(pt.x()).arg(pt.y()));
    }
    lastPoint = pt;
    for (size_t i = 1; i < points.size(); i++)
    {
        LaserPoint lPt = points.at(i);
        QPoint pt = t.map(lPt.toPoint());
        QPoint diff = pt - lastPoint;
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

        if (Config::Device::fullRelative() || Config::Device::startFrom() != SFT_AbsoluteCoords)
        {
            command = command.toLower();
            buffer.append(command.arg(pt.x() - lastPoint.x()).arg(pt.y() - lastPoint.y()));
        }
        else
        {
            buffer.append(command.arg(pt.x()).arg(pt.y()));
        }
        lastPoint = pt;
        if (progress)
            progress->increaseProgress();
    }
    return buffer;
}

QPoint machiningUtils::calculateResidual(const QPointF& diff, QPointF& residual)
{
    int x = qRound(diff.x());
    int y = qRound(diff.y());
    residual += QPointF(diff.x() - x, diff.y() - y);
    if (residual.x() >= 1)
    {
        x += 1;
        residual.setX(residual.x() - 1);
    }
    else if (residual.x() <= -1)
    {
        x -= 1;
        residual.setX(residual.x() + 1);
    }
    if (residual.y() >= 1)
    {
        y += 1;
        residual.setY(residual.y() - 1);
    }
    else if (residual.y() <= -1)
    {
        y -= 1;
        residual.setY(residual.y() + 1);
    }
    return QPoint(x, y);
}

QByteArray machiningUtils::lineList2Plt(ProgressItem* progress, 
    const LaserLineListList& lineList, QPoint& lastPoint)
{
    QByteArray buffer;
    
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
            QLine line = lines.at(j);

            QPoint pt1 = line.p1();
            QPoint pt2 = line.p2();
            if (!forward)
            {
                pt1 = line.p2();
                pt2 = line.p1();
            }

            QPoint diff1 = pt1 - lastPoint;
            QPoint diff2 = pt2 - pt1;
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
            if (Config::Device::fullRelative() || Config::Device::startFrom() != SFT_AbsoluteCoords)
            {
                command1 = command1.toLower();
                command2 = command2.toLower();
                //QPoint rel = calculateResidual(diff1, residual);
                //buffer.append(command1.arg(rel.x()).arg(rel.y()));
                //rel = calculateResidual(diff2, residual);
                //buffer.append(command2.arg(rel.x()).arg(rel.y()));

                buffer.append(command1.arg(pt1.x() - lastPoint.x()).arg(pt1.y() - lastPoint.y()));
                lastPoint = pt1;
                buffer.append(command2.arg(pt2.x() - lastPoint.x()).arg(pt2.y() - lastPoint.y()));
                lastPoint = pt2;
            }
            else
            {
                buffer.append(command1.arg(pt1.x()).arg(pt1.y()));
                buffer.append(command2.arg(pt2.x()).arg(pt2.y()));
                lastPoint = pt2;
            }
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
