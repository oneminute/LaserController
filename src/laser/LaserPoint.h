#ifndef LASERPOINT_H
#define LASERPOINT_H

#include <QtGlobal>
#include <QPoint>

class LaserPrimitive;

class LaserPoint
{
public:
    explicit LaserPoint();
    explicit LaserPoint(int x_, int y_/*, int angle1_ = 0, int angle2_ = 360*/);
    explicit LaserPoint(const QPoint& point/*, int angle1_ = 0, int angle2_ = 360*/);

    static int vectorSize();
    int* vector();

    int x() const { return m_x; }
    int y() const { return m_y; }
    //int angle1() const { return m_angle1; }
    //int angle2() const { return m_angle2; }

    void setX(int x) { m_x = x; }
    void setY(int y) { m_y = y; }
    //void setAngle1(int angle1) { m_angle1 = angle1; }
    //void setAngle2(int angle2) { m_angle2 = angle2; }
    void setAll(int x, int y/*, int angle1, int angle2*/)
    {
        m_x = x;
        m_y = y;
        //m_angle1 = angle1 >= 0 ? angle1 : angle1 + 360;
        //m_angle2 = angle2 >= 0 ? angle2 : angle2 + 360;
    }

    //void setAll(const QPoint& point/*, int angle*/)
    //{
    //    m_x = point.x();
    //    m_y = point.y();
    //    /*if (angle >= 0)
    //    {
    //        m_angle1 = angle;
    //        m_angle2 = 360 - angle;
    //    }
    //    else
    //    {
    //        m_angle1 = 360 + angle;
    //        m_angle2 = 360 - m_angle1;
    //    }*/
    //}

    //void setAll(const QPoint& point/*, int angle1, int angle2*/)
    //{
    //    m_x = point.x();
    //    m_y = point.y();
    //    //m_angle1 = angle1;
    //    //m_angle2 = angle2;
    //}

    int laneIndex() const;

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

    //friend inline const bool qFuzzyCompare(const LaserPoint& l1, const LaserPoint& l2);

    inline QPoint toPoint() const;

    int length() const;

private:
    union 
    {
        struct 
        {
            int m_x;
            int m_y;
            //int m_angle1;
            //int m_angle2;
        };
        int m_vec[2];
    };
};

QDebug operator<<(QDebug debug, const LaserPoint& point);

inline bool operator==(const LaserPoint& l1, const LaserPoint& l2)
{
    return l1.m_x == l2.m_x &&
        l1.m_y == l2.m_y/* &&
        l1.m_angle1 == l2.m_angle1 &&
        l1.m_angle2 == l2.m_angle2*/;
}

inline bool operator!=(const LaserPoint& l1, const LaserPoint& l2)
{
    return l1.m_x != l2.m_x ||
        l1.m_y != l2.m_y /*||
        l1.m_angle1 != l2.m_angle1 ||
        l1.m_angle2 != l2.m_angle2*/;
}

inline const LaserPoint operator+(const LaserPoint& l1, const LaserPoint& l2)
{
    return LaserPoint(
        l1.m_x + l2.m_x,
        l1.m_y + l2.m_y/*,
        l1.m_angle1 + l2.m_angle1,
        l1.m_angle2 + l2.m_angle2*/
    );
}

inline const LaserPoint operator-(const LaserPoint& l1, const LaserPoint& l2)
{
    return LaserPoint(
        l1.m_x - l2.m_x,
        l1.m_y - l2.m_y/*,
        l1.m_angle1 - l2.m_angle1,
        l1.m_angle2 - l2.m_angle2*/
    );
}

inline const LaserPoint operator-(const LaserPoint& l1)
{
    return LaserPoint(
        -l1.m_x, -l1.m_y/*, -l1.m_angle1, -l1.m_angle2*/
    );
}

inline const LaserPoint operator*(float factor, const LaserPoint& l)
{
    return LaserPoint(
        l.m_x * factor, l.m_y * factor/*, l.m_angle1 * factor, l.m_angle2 * factor*/
    );
}

inline const LaserPoint operator*(const LaserPoint& l, float factor)
{
    return LaserPoint(
        l.m_x * factor, l.m_y * factor/*, l.m_angle1 * factor, l.m_angle2 * factor*/
    );
}

inline const LaserPoint operator*(const LaserPoint& l1, const LaserPoint& l2)
{
    return LaserPoint(
        l1.m_x * l2.m_x,
        l1.m_y * l2.m_y/*,
        l1.m_angle1 * l2.m_angle1,
        l1.m_angle2 * l2.m_angle2*/
    );
}

inline const LaserPoint operator/(const LaserPoint& l, float divisor)
{
    return LaserPoint(
        l.m_x / divisor, l.m_y / divisor/*, l.m_angle1 / divisor, l.m_angle2 / divisor*/
    );
}

inline const LaserPoint operator/(const LaserPoint& l1, const LaserPoint& l2)
{
    return LaserPoint(
        l1.m_x / l2.m_x,
        l1.m_y / l2.m_y/*,
        l1.m_angle1 / l2.m_angle1,
        l1.m_angle2 / l2.m_angle2*/
    );
}

//inline const bool qFuzzyCompare(const LaserPoint& l1, const LaserPoint& l2)
//{
//    return qFuzzyCompare(l1.m_x, l2.m_x) &&
//        qFuzzyCompare(l1.m_y, l2.m_y) &&
//        qFuzzyCompare(l1.m_angle1, l2.m_angle1) &&
//        qFuzzyCompare(l1.m_angle2, l2.m_angle2);
//}

inline QPoint LaserPoint::toPoint() const
{
    return QPoint(m_x, m_y);
}

#endif // LASERPOINT_H
