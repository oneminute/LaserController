#pragma once

#include "LaserShape.h"

class LaserPolygonPrivate;
class LaserPolygon : public LaserShape
{
    Q_OBJECT
public:
    LaserPolygon(LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    LaserPolygon(const QPolygon& poly, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserPolygon() {}

    QPolygon polyline() const;
    void setPolyline(const QPolygon& poly);

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

	//virtual QRect sceneBoundingRect() const;

	virtual QJsonObject toJson();
	virtual QVector<QLine> edges();
    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
	virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPolygon)
    Q_DISABLE_COPY(LaserPolygon)
};

