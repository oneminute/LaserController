#include "LaserPrimitiveElement.h"

#include "LaserPrimitive.h"

class LaserPrimitiveElementPrivate
{
    Q_DECLARE_PUBLIC(LaserPrimitiveElement)
public:
    LaserPrimitiveElementPrivate(LaserPrimitiveElement* ptr)
        : q_ptr(ptr)
    {}

    LaserPrimitiveElement* q_ptr;

    QPainterPath path;
    QList<LaserPrimitiveElementPrivate*> inners;
    LaserPrimitiveElementPrivate* outer;
};

LaserPrimitiveElement::LaserPrimitiveElement(LaserPrimitive* parent)
{
}

LaserPrimitiveElement::~LaserPrimitiveElement()
{
}
