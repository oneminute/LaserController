#pragma once

#include "LaserStampBase.h"

class LaserStampBitmapPrivate;
class LaserStampBitmap : public LaserStampBase {
    Q_OBJECT
public:
    LaserStampBitmap(const QImage& image, const QRect& bounds, bool stampIntaglio, LaserDocument* doc, QTransform transform = QTransform(),
        int layerIndex = 0);
    ~LaserStampBitmap();
    void computeImage(bool generateStamp = false);
    virtual void setStampIntaglio(bool bl);
    virtual bool isClosed() const { return true; };
    virtual QJsonObject toJson();
    virtual void draw(QPainter* painter);
    void setOriginalImage(QImage image);
    void setFingerprint();
    void computeMask();
    void setBounds(QRect bounds);
    QImage generateStampImage();

    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
    virtual void setAntiFakePath(QPainterPath path);
    virtual void setAntiFakeImage(QImage image);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

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
    virtual LaserPrimitive* cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStampBitmap)
    Q_DISABLE_COPY(LaserStampBitmap)
};