#include "Utils.h"
#include <QUuid>
#include "scene/LaserPrimitive.h"

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
            if (utils::fuzzyEquals(e.y, y, 1))
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
