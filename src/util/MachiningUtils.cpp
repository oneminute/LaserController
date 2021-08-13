#include "MachiningUtils.h"

#include <QtMath>
#include <QDebug>

#include "common/common.h"
#include "common/Config.h"
#include "TypeUtils.h"

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

int machiningUtils::path2Points(const QPainterPath & path, QVector<QPointF>& points, cv::Mat& canvas)
{
    qreal length = path.length();
    QPointF point = path.pointAtPercent(0);
    qreal slope = path.slopeAtPercent(0);
    //qreal radians = qIsInf(slope) ? M_PI_2 : qAtan(slope);
    qreal radians = qDegreesToRadians(path.angleAtPercent(0));

    QList<int> startingIndices;
    int startingPointsCount = 0;


    QPointF anchor = point;
    qreal anchorSlope = slope;
    qreal anchorRadians = radians;
    qreal angle = 0;

    points.push_back(point);
    if (!canvas.empty())
    {
        cv::circle(canvas, typeUtils::qtPointF2CVPoint2f(point), 1, cv::Scalar(0, 0, 255));
    }

    for (int i = 1; i < length; i++)
    {
        qreal percent = i / length;
        point = path.pointAtPercent(percent);
        slope = path.slopeAtPercent(percent);
        angle = path.angleAtPercent(percent);

        //qreal radians2 = qDegreesToRadians(angle);
        //qLogD << i << ": " << angle << ", " << radians;
        //radians = qIsInf(slope) ? M_PI_2 : qAtan(slope);
        radians = qDegreesToRadians(angle);

        //qreal diff = qAbs(qAbs(radians) - qAbs(anchorRadians));
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
            anchorSlope = slope;
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
            anchorSlope = slope;
            anchorRadians = radians;
        }
    }

    point = path.pointAtPercent(1.0);
    points.push_back(point);
    if (!canvas.empty()) 
    {
        QPointF startPt = path.pointAtPercent(0);
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(anchor), typeUtils::qtPointF2CVPoint2f(point), cv::Scalar(0));
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(point), typeUtils::qtPointF2CVPoint2f(startPt), cv::Scalar(0));
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
