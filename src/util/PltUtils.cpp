#include "PltUtils.h"

#include <QtMath>

//void pltUtils::linePoints(const Eigen::Vector2d & p1, const Eigen::Vector2d & p2, QVector<Eigen::Vector2d> points, qreal factor)
//{
//    Eigen::Vector2d ptBegin = p1 * factor;
//    Eigen::Vector2d ptEnd = p2 * factor;
//    Eigen::Vector2d line = ptEnd - ptBegin;
//
//
//}

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

QByteArray pltUtils::points2Plt(const std::vector<cv::Point2f>& points)
{
    QByteArray buffer;
    if (points.empty())
        return buffer;

    cv::Point2f pt = points[0];
    buffer.append(QString("PU%1 %2;").arg(qFloor(pt.x)).arg(qFloor(pt.y)));
    for (int i = 0; i < points.size(); i++)
    {
        pt = points[i];
        buffer.append(QString("PD%1 %2;").arg(qFloor(pt.x)).arg(qFloor(pt.y)));
    }
    return buffer;
}

QByteArray pltUtils::image2Plt(const QImage & image)
{
    return QByteArray();
}
