#include "LaserShape.h"

class LaserShapePrivate : public LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserShape)
public:
    LaserShapePrivate(LaserShape* ptr)
        : LaserPrimitivePrivate(ptr)
    {}
};

LaserShape::LaserShape(LaserShapePrivate* data, LaserDocument* doc, LaserPrimitiveType type, int layerIndex, QTransform saveTransform)
    : LaserPrimitive(data, doc, type,  saveTransform, layerIndex)
{
	Q_D(LaserShape);
    //m_type = LPT_SHAPE;
	d->layerIndex = layerIndex;
}

