#pragma once

#include "LaserStampBase.h"

class LaserPartyEmblemPrivate;
class LaserPartyEmblem : public LaserStampBase {
    Q_OBJECT
public:
    LaserPartyEmblem(LaserDocument* doc, QPoint centerPos, qreal radius, bool stampIntaglio = false, QTransform transform = QTransform(),
        int layerIndex = 0);
    virtual ~LaserPartyEmblem();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_PARTYEMBLEM; }
    virtual QString typeName() { return tr("PartyEmblem"); }
    virtual QJsonObject toJson();
    virtual QVector<QLine> edges();
    void computePath();
    qreal radius();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);

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
        bool isPressed,
        QUndoCommand* parentCmd) override;
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
    virtual LaserPrimitive* cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPartyEmblem)
    Q_DISABLE_COPY(LaserPartyEmblem)
};
