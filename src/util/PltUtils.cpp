#include "PltUtils.h"

#include <QtMath>
#include <QDebug>

#include "common/common.h"
#include "common/Config.h"
#include "TypeUtils.h"

int pltUtils::linePoints(double x1, double y1, double x2, double y2, std::vector<cv::Point2f>& points, qreal factor, const Eigen::Matrix3d& transform)
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

int pltUtils::path2Points(const QPainterPath & path, std::vector<cv::Point2f>& points, cv::Mat& canvas)
{
    // ��ȡͼԪ���ܳ���
    qreal length = path.length();
    QPointF pt = path.pointAtPercent(0);
    qreal slope = path.slopeAtPercent(0);
    qreal radians = qIsInf(slope) ? M_PI_2 : qAtan(slope);

    // ���ڼ����𵶵�ı���
    QList<int> startingIndices;
    int startingPointsCount = 0;
    //int maxStartingPoints = qMax(qCeil(length / Config::OptimizePathMinStartingPointsInterval()), Config::OptimizePathMaxStartingPoints());
    //qreal startingPoiontsInterval = length / maxStartingPoints;

    QPointF startPt = pt;
    //qDebug() << "Slope:" << slope << ", Angle degrees: " << qRadiansToDegrees(radians);

    QPointF anchor = pt;
    qreal anchorSlope = slope;
    qreal anchorRadians = radians;

    // �����һ����
    points.push_back(typeUtils::qtPointF2CVPoint2f(pt));
    if (!canvas.empty())
        cv::circle(canvas, typeUtils::qtPointF2CVPoint2f(pt), 1, cv::Scalar(0, 0, 255));

    for (int i = 1; i < length; i++)
    {
        qreal percent = i / length;
        pt = path.pointAtPercent(percent);
        slope = path.slopeAtPercent(percent);
        radians = qIsInf(slope) ? M_PI_2 : qAtan(slope);

        qreal diff = qAbs(qAbs(radians) - qAbs(anchorRadians));
        qreal dist = QLineF(pt, anchor).length();

        diff = qRadiansToDegrees(diff);
        if (diff >= Config::Export::maxAnglesDiff() || radians * anchorRadians < 0)
        {
            if (!canvas.empty())
            {
                cv::line(canvas, typeUtils::qtPointF2CVPoint2f(anchor), typeUtils::qtPointF2CVPoint2f(pt), cv::Scalar(255, 0, 0));
                cv::circle(canvas, typeUtils::qtPointF2CVPoint2f(pt), 1, cv::Scalar(0, 0, 255));
            }
            points.push_back(typeUtils::qtPointF2CVPoint2f(pt));
            anchor = pt;
            anchorSlope = slope;
            anchorRadians = radians;
        }
        else if (diff != 0 && dist >= Config::Export::maxIntervalDistance())
        {
            if (!canvas.empty())
            {
                cv::line(canvas, typeUtils::qtPointF2CVPoint2f(anchor), typeUtils::qtPointF2CVPoint2f(pt), cv::Scalar(255, 0, 0));
                cv::circle(canvas, typeUtils::qtPointF2CVPoint2f(pt), 1, cv::Scalar(0, 0, 255));
            }
            points.push_back(typeUtils::qtPointF2CVPoint2f(pt));
            anchor = pt;
            anchorSlope = slope;
            anchorRadians = radians;
        }
    }

    pt = path.pointAtPercent(1.0);
    points.push_back(typeUtils::qtPointF2CVPoint2f(pt));
    if (!canvas.empty()) 
    {
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(anchor), typeUtils::qtPointF2CVPoint2f(pt), cv::Scalar(0));
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt), typeUtils::qtPointF2CVPoint2f(startPt), cv::Scalar(0));
    }

    qLogD << "path with " << path.elementCount() << " elements convert to " << points.size() << " points.";
    return points.size();
}

QByteArray pltUtils::points2Plt(const std::vector<cv::Point2f>& points)
{
    QByteArray buffer;
    if (points.empty())
        return buffer;

    //cv::Point2f pt = points[points.size() - 1];
    cv::Point2f pt = points[0];
    buffer.append(QString("PU%1 %2;").arg(qRound(pt.x)).arg(-qRound(pt.y)));
    for (int i = 0; i < points.size(); i++)
    {
        pt = points[i];
        buffer.append(QString("PD%1 %2;").arg(qRound(pt.x)).arg(-qRound(pt.y)));
    }
    return buffer;
}

QByteArray pltUtils::image2Plt(const QImage & image)
{
    return QByteArray();
}

bool pltUtils::pointsEql(const QPointF & pt1, const QPointF & pt2)
{
    return qRound(pt1.x()) == qRound(pt2.x()) && qRound(pt1.y()) == qRound(pt2.y());
}
