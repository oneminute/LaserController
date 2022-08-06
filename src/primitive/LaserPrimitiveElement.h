#pragma once

#include <QObject>
#include <QPainterPath>

class LaserPrimitive;
class LaserPrimitiveElementPrivate;

class LaserPrimitiveElement
{
public:
    LaserPrimitiveElement(LaserPrimitive* parent);
    ~LaserPrimitiveElement();

    QPainterPath painterPath() const;
    QList<LaserPrimitiveElementPrivate>& inners();
    LaserPrimitiveElementPrivate* outer();

private:
    QScopedPointer<LaserPrimitiveElementPrivate> d_ptr;
    QPainterPath m_path;

    Q_DECLARE_PRIVATE(LaserPrimitiveElement)
};
