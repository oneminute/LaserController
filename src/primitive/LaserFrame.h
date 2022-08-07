#pragma once

#include "LaserStampBase.h"

class LaserFramePrivate;
class LaserFrame : public LaserStampBase {
    Q_OBJECT
public:
    //cornnerRadilus 为正是圆角，为副是内角
    //内角分用圆形且和用正方形切
    LaserFrame(LaserDocument* doc, QRect outerRect, qreal width, qreal cornerRadilus, bool stampIntaglio = false,
        QTransform transform = QTransform(),int layerIndex = 0, int cornerType = CRT_Round);
    virtual~LaserFrame();

    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_FRAME; }
    virtual QString typeName() { return tr("Frame"); }
    virtual QJsonObject toJson();
    LaserPrimitive * clone(QTransform t);
    QVector<QLineF> edges();
    void setInner(bool bl);
    bool isInner();
    QPainterPath outerPath();
    QPainterPath innerPath();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    void setCornerRadius(qreal cornerRadius, int type);
    qreal cornerRadius();
    int cornerRadiusType();
    QRectF innerRect();
    void setBorderWidth(qreal w);
    qreal borderWidth();
    void computePath();
    bool needAuxiliaryLine();
    void setNeedAuxiliaryLine(bool bl);
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);

private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserFrame)
    Q_DISABLE_COPY(LaserFrame)
};