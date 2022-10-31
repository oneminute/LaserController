#pragma once

#include "LaserStampText.h"

class LaserHorizontalTextPrivate;
class LaserHorizontalText : public LaserStampText {
    Q_OBJECT
public:
    LaserHorizontalText(LaserDocument* doc, QString content,QSize size,
        QPointF center, bool bold = false, bool italic = false, bool uppercase = false,bool stampIntaglio = false, QString family = "Times New Roman",
        qreal space = 0,  QTransform transform = QTransform(), int layerIndex = 0, qreal weight = 0);
    virtual ~LaserHorizontalText();
    //void initTextPath();
    void computeTextPathProcess();
    void computeTextPath();
    
    void toCenter();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_HORIZONTALTEXT; }
    virtual QString typeName() { return tr("HorizontalText"); }
    virtual QJsonObject toJson();
    virtual QVector<QLine> edges();
    virtual void recompute();
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setSpace(qreal space);
    void setTextHeight(qreal height);
    void setTextWidth(qreal width);

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

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserHorizontalText)
    Q_DISABLE_COPY(LaserHorizontalText)
};