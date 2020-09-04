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
