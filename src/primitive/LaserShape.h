#pragma once

#include "LaserPrimitive.h"

class LaserShapePrivate;
class LaserShape : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserShape(LaserShapePrivate* data, LaserDocument* doc, LaserPrimitiveType type, int layerIndex = 1, QTransform transform = QTransform());
    virtual ~LaserShape() { } 
    virtual QByteArray filling(ProgressItem* progress, QPoint& lastPoint) override;
	int layerIndex();
private:
    Q_DISABLE_COPY(LaserShape);
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserShape);
};
