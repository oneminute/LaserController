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

    virtual void beginCreatingInternal(QUndoCommand* parentCmd,
        PrimitiveAddingCommand* addingCmd) override;
    virtual void endCreatingInterval(QUndoCommand* parentCmd,
        PrimitiveRemovingCommand* removingCmd) override;

private:
	virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPolygon)
    Q_DISABLE_COPY(LaserPolygon)
};

