#pragma once

#include "LaserStampBase.h"

class LaserStarPrivate;
class LaserStar : public LaserStampBase {
    Q_OBJECT
public:
    LaserStar(LaserDocument* doc, QPoint centerPos, qreal radius, bool stampIntaglio = false, QTransform transform = QTransform(),
        int layerIndex = 0);
    virtual ~LaserStar();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_STAR; }
    virtual QString typeName() { return tr("Star"); }
    virtual QJsonObject toJson();
    LaserPrimitive * clone();
    QVector<QLineF> edges();
    void computePath();
    qreal radius();
    //void updatePoints();
    //QPoint* points();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStar)
    Q_DISABLE_COPY(LaserStar)
};
