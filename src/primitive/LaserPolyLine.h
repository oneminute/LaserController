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
    void updatePath();
    int appendPoint(const QPoint& point);
    void removeLastPoint();
    void removePoint(int pointIndex);
    QPoint pointAt(int pointIndex);
    void setEditingPoint(const QPoint& point);
    QPoint editingPoint() const;

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

	virtual QJsonObject toJson();
	QVector<QLineF> edges();
    virtual bool isClosed() const;
    virtual QPointF position() const;

protected:
    // the following functions only used in editing mode.
    virtual void sceneMousePressEvent(
        LaserViewer* viewer,
        LaserScene* scene, 
        const QPoint& point,
        QMouseEvent* event) override;
    virtual void sceneMouseMoveEvent(
        LaserViewer* viewer,
        LaserScene* scene,
        const QPoint& point,
        QMouseEvent* event,
        bool isPressed) override;
    virtual void sceneMouseReleaseEvent(
        LaserViewer* viewer,
        LaserScene* scene,
        const QPoint& point,
        QMouseEvent* event,
        bool isPressed) override;
	virtual void sceneKeyPressEvent(
        LaserViewer* viewer,
        QKeyEvent *event);
	virtual void sceneKeyReleaseEvent(
        LaserViewer* viewer,
        QKeyEvent *event);

private:
	virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPolyline)
    Q_DISABLE_COPY(LaserPolyline) 

    friend class LaserViewer;
};

