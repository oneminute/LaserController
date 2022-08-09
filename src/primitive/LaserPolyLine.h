#pragma once

#include "LaserShape.h"

class LaserPolylinePrivate;
class LaserPolyline : public LaserShape
{
    Q_OBJECT
public:
    LaserPolyline(const QPolygon& poly, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserPolyline() {}

    QPolygon polyline() const;
    void setPolyline(const QPolygon& poly);
    int appendPoint(const QPoint& point);
    void removeLastPoint();
    void removePoint(int pointIndex);
    QPoint pointAt(int pointIndex);

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

	virtual QJsonObject toJson();
	QVector<QLineF> edges();
    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
	virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPolyline)
    Q_DISABLE_COPY(LaserPolyline)
};

