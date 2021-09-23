#include "LaserPoint.h"

#include <QtMath>
#include <QDebug>

#include "common/Config.h"

LaserPoint::LaserPoint()
    : m_x(0)
    , m_y(0)
    , m_angle1(0)
    , m_angle2(0)
{
}

LaserPoint::LaserPoint(qreal x_, qreal y_, qreal angle1_, qreal angle2_)
    : m_x(x_)
    , m_y(y_)
    , m_angle1(angle1_)
    , m_angle2(angle2_)
{
}

LaserPoint::LaserPoint(const QPointF& point, qreal angle1_, qreal angle2_)
    : m_x(point.x())
    , m_y(point.y())
    , m_angle1(angle1_)
    , m_angle2(angle2_)
{

}

int LaserPoint::vectorSize()
{
    return sizeof(m_vec) / sizeof(qreal);
}

qreal* LaserPoint::vector()
{
    return m_vec;
}

int LaserPoint::laneIndex() const
{
    qreal coord = Config::PathOptimization::groupingOrientation() == Qt::Horizontal ?
        m_y : m_x;
    int index = qRound(coord / (Config::PathOptimization::groupingGridInterval() * 1000));
    return index;
}

LaserPoint& LaserPoint::operator+=(const LaserPoint& laserPoint)
{
    m_x += laserPoint.m_x;
    m_y += laserPoint.m_y;
    m_angle1 += laserPoint.m_angle1;
    m_angle2 += laserPoint.m_angle2;
    return *this;
}

LaserPoint& LaserPoint::operator-=(const LaserPoint& laserPoint)
{
    m_x -= laserPoint.m_x;
    m_y -= laserPoint.m_y;
    m_angle1 -= laserPoint.m_angle1;
    m_angle2 -= laserPoint.m_angle2;
    return *this;
}

LaserPoint& LaserPoint::operator*=(float factor)
{
    m_x *= factor;
    m_y *= factor;
    m_angle1 *= factor;
    m_angle2 *= factor;
    return *this;
}

LaserPoint& LaserPoint::operator*=(const LaserPoint& laserPoint)
{
    m_x *= laserPoint.m_x;
    m_y *= laserPoint.m_y;
    m_angle1 *= laserPoint.m_angle1;
    m_angle2 *= laserPoint.m_angle2;
    return *this;
}

LaserPoint& LaserPoint::operator/=(float factor)
{
    m_x /= factor;
    m_y /= factor;
    m_angle1 /= factor;
    m_angle2 /= factor;
    return *this;
}

qreal LaserPoint::length() const
{
    return qSqrt(m_x * m_x + m_y * m_y);
}

QDebug operator<<(QDebug debug, const LaserPoint& point)
{
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << "[" << point.x() << ", " <<
        point.y() << ", " << point.angle1() << ", " <<
        point.angle2() << "]";
    return debug;
}

