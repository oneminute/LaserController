#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QFrame>

namespace utils
{
    QString createUUID(const QString& prefix = "");

    int parsePortName(const QString& name);

    QFrame* createSeparator(int width = 0, int height = 0, QFrame::Shape shape = QFrame::VLine, QFrame::Shadow shadow = QFrame::Sunken);
}

#endif // UTILS_H
