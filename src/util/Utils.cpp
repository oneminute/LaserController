#include "Utils.h"
#include <QUuid>

QString utils::createUUID(const QString& prefix)
{
    QString ret = prefix;
    ret.append(QUuid::createUuid().toString(QUuid::Id128));
    return ret;
}
