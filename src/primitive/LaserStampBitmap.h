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
    virtual LaserPrimitive* clone();
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

private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStampBitmap)
        Q_DISABLE_COPY(LaserStampBitmap)
};