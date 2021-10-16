#include "Utils.h"
#include <QUuid>
#include <QtMath>
#include "scene/LaserPrimitive.h"
#include "Eigen/Core"
#include "Eigen/Dense"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

QString utils::createUUID(const QString& prefix)
{
    QString ret = prefix;
    ret.append(QUuid::createUuid().toString());
    return ret;
}

int utils::parsePortName(const QString & name)
{
    QRegExp re(".*COM(\\d+)");
    re.indexIn(name);
    QString portName = re.cap(1);
    bool ok = false;
    int port = portName.toInt(&ok);
    return port;
}

QFrame * utils::createSeparator(int width, int height, QFrame::Shape shape, QFrame::Shadow shadow)
{
    QFrame* separator = new QFrame;
    separator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    if (width)
    {
        separator->setFixedWidth(width);
    }
    if (height)
    {
        separator->setFixedHeight(height);
    }
    separator->setFrameShape(shape);
    separator->setFrameShadow(shadow);
    return separator;
}

QVector3D utils::putToQuadrant(const QVector3D & pos, QUADRANT quadrant)
{
    float x = pos.x();
    float y = pos.y();
    float z = pos.z();
    switch (quadrant)
    {
    case QUADRANT_1:
    {
        if (x < 0) x = -x;
        if (y < 0) y = -y;
    }
        break;
    case QUADRANT_2:
    {
        if (x > 0) x = -x;
        if (y < 0) y = -y;
    }
        break;
    case QUADRANT_3:
    {
        if (x > 0) x = -x;
        if (y > 0) y = -y;
    }
        break;
    case QUADRANT_4:
    {
        if (x < 0) x = -x;
        if (y > 0) y = -y;
    }
        break;
    }

    return QVector3D(x, y, z);
}

QVector3D utils::limitToLayout(const QVector3D & pos, QUADRANT quadrant, float width, float height)
{
    float x = pos.x();
    float y = pos.y();
    switch (quadrant)
    {
    case QUADRANT_1:
    {
        x = qBound(0.f, x, width);
        y = qBound(0.f, y, width);
    }
        break;
    case QUADRANT_2:
    {
        x = qBound(-width, x, 0.f);
        y = qBound(0.f, y, width);
    }
        break;
    case QUADRANT_3:
    {
        x = qBound(-width, x, 0.f);
        y = qBound(-height, y, 0.f);
    }
        break;
    case QUADRANT_4:
    {
        x = qBound(0.f, x, width);
        y = qBound(-height, y, 0.f);
    }
        break;
    }

    return QVector3D(x, y, pos.z());
}

bool utils::checkTwoPointEqueal(const QPointF & point1, const QPointF & point2, float scop)
{
	qreal distance = QVector2D(point2 - point1).length();
	if (distance <= scop)
	{
		return true;
	}
	return false;
}

bool utils::fuzzyEquals(const QPointF& pt1, const QPointF& pt2)
{
    qreal length = QVector2D(pt1 - pt2).length();
    return qFuzzyIsNull(length);
}

bool utils::fuzzyEquals(qreal a, qreal b, qreal limit)
{
    return qAbs(a - b) < limit;
}

bool utils::fuzzyCompare(const QPointF& p1, const QPointF& p2)
{
    return qFuzzyCompare(p1.x(), p2.x()) &&
        qFuzzyCompare(p1.y(), p2.y());
}

void utils::sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem * item)
{
	item->setTransform(sceneTransform);
	item->setPos(0, 0);
}
quint32 utils::factorial(quint32 n)
{
    if (n == 0)
        return 1;
    return n * factorial(n - 1);
}

QPointF utils::center(const QVector<QPointF>& points)
{
    QPointF center(0, 0);
    if (points.isEmpty())
        return center;

    for (const QPointF& p : points)
    {
        center += p;
    }
    center /= points.size();
    return center;
}

LaserPoint utils::center(const LaserPointList& points)
{
    LaserPoint center(0, 0, 0, 0);
    if (points.isEmpty())
        return center;

    for (const LaserPoint& p : points)
    {
        center += p;
    }
    center /= points.size();
    return center;
}

QRectF utils::boundingRect(const QList<LaserPrimitive*>& primitives)
{
    QRectF bounding(0, 0, 0, 0);
    int count = 0;
    for (LaserPrimitive* primitive: primitives)
    {
        QRectF rect = primitive->sceneBoundingRect();
        if (count++ == 0)
        {
            bounding = rect;
            continue;
        }
        if (rect.left() < bounding.left())
            bounding.setLeft(rect.left());
        if (rect.top() < bounding.top())
            bounding.setTop(rect.top());
        if (rect.right() > bounding.right())
            bounding.setRight(rect.right());
        if (rect.bottom() > bounding.bottom())
            bounding.setBottom(rect.bottom());
    }
    return bounding;
}

LaserLineListList utils::interLines(const QPainterPath& path, qreal rowInterval)
{
    LaserLineListList lineList;
    QRectF boundingRect = path.boundingRect();

    qreal y = boundingRect.top();
    for (; y <= boundingRect.bottom(); y += rowInterval)
    {
        QRectF intersectRect(boundingRect.left() - 10, y, boundingRect.width() + 10, 1);
        QPainterPath intersectPath;
        intersectPath.addRect(intersectRect);

        QPainterPath intersected = intersectPath.intersected(path);
        QMap<qreal, qreal> linePoints;
        for (int i = 0; i < intersected.elementCount(); i++)
        {
            QPainterPath::Element e = intersected.elementAt(i);
            //qDebug() << i << e.x << e.y << e.type;
            if (utils::fuzzyEquals(e.y, y, 0.1))
            {
                linePoints.insert(e.x, e.x);
            }
        }
        qreal last;
        int i = 0;
        LaserLineList lines;
        for (qreal curr : linePoints)
        {
            //qDebug() << curr;
            if (i++ == 0)
            {
                last = curr;
                continue;
            }
            qreal mean = (last + curr) / 2;

            QPointF pt(mean, y);
            if (path.contains(pt))
            {
                qreal x1 = last;
                qreal x2 = curr;
                if (x1 > x2) qSwap(x1, x2);
                lines.append(QLineF(QPointF(x1, y), QPointF(x2, y)));
            }
            last = curr;
        }
        if (!lines.empty())
            lineList.append(lines);
    }
    qLogD << lineList.count() << " lines generated";
    qLogD << boundingRect.height() / rowInterval << " lines expected";

    return lineList;
}

RELATION utils::determineRelationship(const QPainterPath& a, const QPainterPath& b)
{
    RELATION rel = determineRelationship(a.boundingRect(), b.boundingRect());
    if (rel == RELATION::A_CONTAINS_B)
    {
        if (a.contains(b))
            return rel;
        else
            rel = RELATION::NONE;
    }
    else if (rel == RELATION::B_CONTAINS_A)
    {
        if (b.contains(a))
            return rel;
        else
            rel = RELATION::NONE;
    }
    else
    {
        rel = RELATION::NONE;
    }
    //if (a.contains(b))
    //{
    //    // candidate primitive contains tree node primitive
    //    rel = A_CONTAINS_B;
    //}
    //else if (b.contains(a))
    //{
    //    // tree node primitive contains candidate primitive
    //    rel = B_CONTAINS_A;
    //}
    //else if (a.intersects(b))
    //{
    //    // a intersects with b
    //    rel = INTERSECTION;
    //}
    //else
    //{
    //    // no relationship between candidate primitive and tree node primitive
    //    rel = RELATION::NONE;
    //}
    return rel;
}

RELATION utils::determineRelationship(const QRectF& a, const QRectF& b)
{
    RELATION hRel = RELATION::NONE;
    RELATION vRel = RELATION::NONE;
    qreal left = qMin(a.left(), b.left());
    qreal right = qMax(a.right(), b.right());
    qreal top = qMin(a.top(), b.top());
    qreal bottom = qMax(a.bottom(), b.bottom());
    qreal interWidth = right - left;
    qreal interHeight = bottom - top;
    qreal hSum = a.width() + b.width();
    qreal vSum = a.height() + b.height();

    if (interWidth < hSum)
    {
        hRel = RELATION::INTERSECTION;
        if (interWidth == a.width())
        {
            hRel = RELATION::A_CONTAINS_B;
        }
        else if (interWidth == b.width())
        {
            hRel = RELATION::B_CONTAINS_A;
        }
    }

    if (interHeight < vSum)
    {
        vRel = RELATION::INTERSECTION;
        if (interHeight == a.height())
        {
            vRel = RELATION::A_CONTAINS_B;
        }
        else if (interHeight == b.height())
        {
            vRel = RELATION::B_CONTAINS_A;
        }
    }

    RELATION rel = RELATION::NONE;
    if (vRel == RELATION::INTERSECTION && hRel == RELATION::INTERSECTION)
    {
        rel = RELATION::INTERSECTION;
    }
    if (vRel == A_CONTAINS_B && hRel == A_CONTAINS_B)
    {
        rel = A_CONTAINS_B;
    }
    if (vRel == B_CONTAINS_A && hRel == B_CONTAINS_A)
    {
        rel = B_CONTAINS_A;
    }

    return rel;
}

QTransform utils::fromPointPairs(const PointPairList& pointPairs)
{
    // solve Ax=b
    // 
    // A = | sigma(x1^2 + y1^2)                   0  sigma(x1)  sigma(y1) |
    //     |                  0  sigma(x1^2 - y1^2) -sigma(y1)  sigma(x1) |
    //     |          sigma(x1)          -sigma(y1)          1          0 |
    //     |          sigma(y1)           sigma(x1)          0          1 |
    // 
    // b = | sigma(x1 * x2 + y1 * y2) |
    //     | sigma(x1 * y2 - x2 * y1) |
    //     |                sigma(x2) |
    //     |                sigma(y2) |
    //
    // x = | a |
    //     | b |
    //     | c |
    //     | d |
    //
    // a = cos(angle), b = sin(angle), c = dx, d = dy

    qreal sumX1 = 0;
    qreal sumX2 = 0;
    qreal sumY1 = 0;
    qreal sumY2 = 0;
    qreal sumX12AY12 = 0;
    qreal sumX12SY12 = 0;
    qreal sumX1X2AY1Y2 = 0;
    qreal sumX1Y2SX2Y1 = 0;

    for (const PointPair& pair : pointPairs)
    {
        qreal x1 = pair.second.x();
        qreal y1 = pair.second.y();
        qreal x2 = pair.first.x();
        qreal y2 = pair.first.y();

        qreal x12 = x1 * x1;
        qreal y12 = y1 * y1;
        qreal x1x2 = x1 * x2;
        qreal y1y2 = y1 * y2;
        qreal x1y2 = x1 * y2;
        qreal x2y1 = x2 * y1;

        sumX1 += x1;
        sumX2 += x2;
        sumY1 += y1;
        sumY2 += y2;
        sumX12AY12 += x12 + y12;
        sumX12SY12 += x12 - y12;
        sumX1X2AY1Y2 += x1x2 + y1y2;
        sumX1Y2SX2Y1 += x1y2 - x2y1;
    }

    Eigen::MatrixXd A(4, 4);
    A << sumX12AY12,          0,  sumX1, sumY1,
                  0, sumX12SY12, -sumY1, sumX1,
              sumX1,      -sumY1,    1,      0,
              sumY1,       sumX1,    0,      1;
    Eigen::Vector4d b;
    b << sumX1X2AY1Y2, 
         sumX1Y2SX2Y1, 
                sumX2, 
                sumY2;
    std::cout << A << std::endl << std::endl;
    std::cout << b << std::endl << std::endl;

    Eigen::MatrixXd x = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
    std::cout << x << std::endl << std::endl;

    qreal angle1 = qRadiansToDegrees(qAcos(x(0)));
    qreal angle2 = qRadiansToDegrees(qAsin(x(1)));
    qLogD << "angle1: " << angle1;
    qLogD << "angle2: " << angle2;

    QTransform transform(x(0), x(1), -x(1), x(0), x(2), x(3));
    qLogD << transform;
    return transform;
}


