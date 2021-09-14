#ifndef LASERPOINT_H
#define LASERPOINT_H

#include <QtGlobal>
#include <QPointF>
#include <QVector4D>

class LaserPrimitive;

class LaserPoint
{
public:
    enum PointType
    {
        PT_MoveTo,
        PT_LineTo
    };

    explicit LaserPoint(PointType type = PT_LineTo);
    explicit LaserPoint(qreal x_, qreal y_, qreal angle1_, qreal angle2_, PointType type = PT_LineTo);
    explicit LaserPoint(const QPointF& point, qreal angle1_ = 0, qreal angle2_ = 360, PointType type = PT_LineTo);

    static int vectorSize();
    qreal* vector();

    qreal x() const { return m_x; }
    qreal y() const { return m_y; }
    qreal angle1() const { return m_angle1; }
    qreal angle2() const { return m_angle2; }

    void setX(qreal x) { m_x = x; }
    void setY(qreal y) { m_y = y; }
    void setAngle1(qreal angle1) { m_angle1 = angle1; }
    void setAngle2(qreal angle2) { m_angle2 = angle2; }
    void setAll(qreal x, qreal y, qreal angle1, qreal angle2)
    {
        m_x = x;
        m_y = y;
        m_angle1 = angle1 >= 0 ? angle1 : angle1 + 360;
        m_angle2 = angle2 >= 0 ? angle2 : angle2 + 360;
    }

    void setAll(const QPointF& point, qreal angle)
    {
        m_x = point.x();
        m_y = point.y();
        if (angle >= 0)
        {
            m_angle1 = angle;
            m_angle2 = 360 - angle;
        }
        else
        {
            m_angle1 = 360 + angle;
            m_angle2 = 360 - m_angle1;
        }
    }

    void setAll(const QPointF& point, qreal angle1, qreal angle2)
    {
        m_x = point.x();
        m_y = point.y();
        m_angle1 = angle1;
        m_angle2 = angle2;
    }

    LaserPoint& operator+=(const LaserPoint& laserPoint);
    LaserPoint& operator-=(const LaserPoint& laserPoint);
    LaserPoint& operator*=(float factor);
    LaserPoint& operator*=(const LaserPoint& laserPoint);
    LaserPoint& operator/=(float factor);

    friend inline bool operator==(const LaserPoint& l1, const LaserPoint& l2);
    friend inline bool operator!=(const LaserPoint& l1, const LaserPoint& l2);

    friend inline const LaserPoint operator+(const LaserPoint& l1, const LaserPoint& l2);
    friend inline const LaserPoint operator-(const LaserPoint& l1, const LaserPoint& l2);
    friend inline const LaserPoint operator-(const LaserPoint& l1);
    friend inline const LaserPoint operator*(float factor, const LaserPoint& l);
    friend inline const LaserPoint operator*(const LaserPoint& l, float factor);
    friend inline const LaserPoint operator*(const LaserPoint& l1, const LaserPoint& l2);
    friend inline const LaserPoint operator/(const LaserPoint& l1, float divisor);
    friend inline const LaserPoint operator/(const LaserPoint& l1, const LaserPoint& l2);

    friend inline const bool qFuzzyCompare(const LaserPoint& l1, const LaserPoint& l2);

    inline QPointF toPointF() const;
    inline QVector4D toVector4D() const;

    qreal length() const;

    inline PointType pointType() const;
    inline void setPointType(PointType type);

private:
    union 
    {
        struct 
        {
            qreal m_x;
            qreal m_y;
            qreal m_angle1;
            qreal m_angle2;
        };
        qreal m_vec[4];
    };

    PointType m_type;
};

inline bool operator==(const LaserPoint& l1, const LaserPoint& l2)
{
    return l1.m_x == l2.m_x &&
        l1.m_y == l2.m_y &&
        l1.m_angle1 == l2.m_angle1 &&
        l1.m_angle2 == l2.m_angle2;
}

inline bool operator!=(const LaserPoint& l1, const LaserPoint& l2)
{
    return l1.m_x != l2.m_x ||
        l1.m_y != l2.m_y ||
        l1.m_angle1 != l2.m_angle1 ||
        l1.m_angle2 != l2.m_angle2;
}

inline const LaserPoint operator+(const LaserPoint& l1, const LaserPoint& l2)
{
    return LaserPoint(
        l1.m_x + l2.m_x,
        l1.m_y + l2.m_y,
        l1.m_angle1 + l2.m_angle1,
        l1.m_angle2 + l2.m_angle2
    );
}

inline const LaserPoint operator-(const LaserPoint& l1, const LaserPoint& l2)
{
    return LaserPoint(
        l1.m_x - l2.m_x,
        l1.m_y - l2.m_y,
        l1.m_angle1 - l2.m_angle1,
        l1.m_angle2 - l2.m_angle2
    );
}

inline const LaserPoint operator-(const LaserPoint& l1)
{
    return LaserPoint(
        -l1.m_x, -l1.m_y, -l1.m_angle1, -l1.m_angle2
    );
}

inline const LaserPoint operator*(float factor, const LaserPoint& l)
{
    return LaserPoint(
        l.m_x * factor, l.m_y * factor, l.m_angle1 * factor, l.m_angle2 * factor
    );
}

inline const LaserPoint operator*(const LaserPoint& l, float factor)
{
    return LaserPoint(
        l.m_x * factor, l.m_y * factor, l.m_angle1 * factor, l.m_angle2 * factor
    );
}

inline const LaserPoint operator*(const LaserPoint& l1, const LaserPoint& l2)
{
    return LaserPoint(
        l1.m_x * l2.m_x,
        l1.m_y * l2.m_y,
        l1.m_angle1 * l2.m_angle1,
        l1.m_angle2 * l2.m_angle2
    );
}

inline const LaserPoint operator/(const LaserPoint& l, float divisor)
{
    return LaserPoint(
        l.m_x / divisor, l.m_y / divisor, l.m_angle1 / divisor, l.m_angle2 / divisor
    );
}

inline const LaserPoint operator/(const LaserPoint& l1, const LaserPoint& l2)
{
    return LaserPoint(
        l1.m_x / l2.m_x,
        l1.m_y / l2.m_y,
        l1.m_angle1 / l2.m_angle1,
        l1.m_angle2 / l2.m_angle2
    );
}

inline const bool qFuzzyCompare(const LaserPoint& l1, const LaserPoint& l2)
{
    return qFuzzyCompare(l1.m_x, l2.m_x) &&
        qFuzzyCompare(l1.m_y, l2.m_y) &&
        qFuzzyCompare(l1.m_angle1, l2.m_angle1) &&
        qFuzzyCompare(l1.m_angle2, l2.m_angle2);
}

inline QPointF LaserPoint::toPointF() const
{
    return QPointF(m_x, m_y);
}

inline QVector4D LaserPoint::toVector4D() const
{
    return QVector4D(m_x, m_y, m_angle1, m_angle2);
}

inline LaserPoint::PointType LaserPoint::pointType() const
{
    return m_type;
}

inline void LaserPoint::setPointType(PointType type)
{
    m_type = type;
}

#endif // LASERPOINT_H
