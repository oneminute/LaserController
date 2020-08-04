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
