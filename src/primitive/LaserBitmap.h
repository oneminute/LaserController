#pragma once

#include "LaserShape.h"

class LaserBitmapPrivate;
class LaserBitmap : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserBitmap(LaserDocument* doc, QTransform transform = QTransform(),
        int layerIndex = 0);
    LaserBitmap(const QImage& image, const QRect& bounds, LaserDocument* doc, 
        QTransform transform = QTransform(), 
		int layerIndex = 0);
    virtual ~LaserBitmap() {}

    QImage image() const;
    void setImage(const QImage& image);

    QRectF bounds() const;
    void setRect(QRect rect);

    virtual QByteArray engravingImage(ProgressItem* parentProgress, QPoint& lastPoint);
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_BITMAP; }
    virtual QString typeName() { return tr("Bitmap"); }
	virtual QJsonObject toJson();

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
	//virtual QRect sceneBoundingRect() const;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);

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
	virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserBitmap)
    Q_DISABLE_COPY(LaserBitmap)

};

