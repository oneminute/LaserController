#ifndef PAGESIZE_H
#define PAGESIZE_H

#include "common/common.h"

#include <QObject>
#include <QMap>

class PageSize
{
public:
    explicit PageSize();
    explicit PageSize(const QString& _name, qreal _width, qreal _height);

    QString toString();
    static QMap<int, PageSize>& presets();

    QString name() { return m_name; }
    qreal width() { return m_width; }
    qreal height() { return m_height; }

private:
    QString m_name;
    qreal m_width;
    qreal m_height;
};

#endif // PAGESIZE_H