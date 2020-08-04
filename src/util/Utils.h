#ifndef UTILS_H
#define UTILS_H

#include <QObject>

namespace utils
{
    QString createUUID(const QString& prefix = "");

    int parsePortName(const QString& name);
}

#endif // UTILS_H
