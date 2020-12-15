#include "Utils.h"
#include <QUuid>

QString utils::createUUID(const QString& prefix)
{
    QString ret = prefix;
    ret.append(QUuid::createUuid().toString(QUuid::Id128));
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
    case QUADRANT1:
    {
        if (x < 0) x = -x;
        if (y < 0) y = -y;
    }
        break;
    case QUADRANT2:
    {
        if (x > 0) x = -x;
        if (y < 0) y = -y;
    }
        break;
    case QUADRANT3:
    {
        if (x > 0) x = -x;
        if (y > 0) y = -y;
    }
        break;
    case QUADRANT4:
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
    case QUADRANT1:
    {
        x = qBound(0.f, x, width);
        y = qBound(0.f, y, width);
    }
        break;
    case QUADRANT2:
    {
        x = qBound(-width, x, 0.f);
        y = qBound(0.f, y, width);
    }
        break;
    case QUADRANT3:
    {
        x = qBound(-width, x, 0.f);
        y = qBound(-height, y, 0.f);
    }
        break;
    case QUADRANT4:
    {
        x = qBound(0.f, x, width);
        y = qBound(-height, y, 0.f);
    }
        break;
    }

    pos.setX(x);
    pos.setY(y);
}
