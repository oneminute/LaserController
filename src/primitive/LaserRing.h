#pragma once

#include "LaserStampBase.h"

class LaserRingPrivate;
class LaserRing : public LaserStampBase {
    Q_OBJECT
public:
    LaserRing(LaserDocument* doc, QRectF outerRect, qreal width,bool stampIntaglio = false, QTransform transform = QTransform(),
        int layerIndex = 0);
    virtual ~LaserRing();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_RING; }
    virtual QString typeName() { return tr("Ring"); }
    virtual QJsonObject toJson();
    LaserPrimitive * clone();
    QVector<QLineF> edges();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    QRectF innerRect();
    QPainterPath outerPath();
    QPainterPath innerPath();
    
    void setInner(bool bl);
    bool isInner();
    void setBorderWidth(qreal w);
    qreal borderWidth();
    void computePath();
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserRing)
    Q_DISABLE_COPY(LaserRing)
};