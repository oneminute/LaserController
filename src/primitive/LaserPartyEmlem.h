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
    QVector<QLineF> edges();
    void computePath();
    qreal radius();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
private:
    virtual LaserPrimitive* cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPartyEmblem)
    Q_DISABLE_COPY(LaserPartyEmblem)
};
