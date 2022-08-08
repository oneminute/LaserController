#pragma once

#include "LaserShape.h"

class LaserNurbsPrivate;
class LaserNurbs : public LaserShape
{
    Q_OBJECT
public:
    enum BasisType
    {
        BT_BEZIER,
        BT_BSPLINE
    };

    LaserNurbs(const QList<QPointF> controlPoints, const QList<qreal> knots, const QList<qreal> weights, BasisType basisType, LaserDocument* doc, 
		QTransform transform = QTransform(), int layerIndex = 1);
    ~LaserNurbs() {}

    virtual void draw(QPainter* painter);

	//virtual QRect sceneBoundingRect() const;

    void updateCurve();

	LaserPrimitive * clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserNurbs)
    Q_DISABLE_COPY(LaserNurbs)
};
