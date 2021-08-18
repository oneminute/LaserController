#include "Utils.h"
#include <QUuid>

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

void utils::limitToLayout(QVector3D & pos, QUADRANT quadrant, float width, float height)
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

    pos.setX(x);
    pos.setY(y);
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
