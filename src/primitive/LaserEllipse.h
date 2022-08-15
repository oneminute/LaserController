#pragma once

#include "LaserPrimitive.h"
#include "LaserShape.h"

class LaserEllipsePrivate;
class LaserEllipse : public LaserShape
{
    Q_OBJECT
public:
    LaserEllipse(LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    LaserEllipse(const QRect bounds, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserEllipse() {}

    QRectF bounds() const;
    void setBounds(const QRect& bounds);

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

	//virtual QRect sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
    virtual bool isClosed() const;
    virtual QPointF position() const;

private:
	virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserEllipse)
    Q_DISABLE_COPY(LaserEllipse)
};

