#pragma once

#include "LaserShape.h"

class LaserRectPrivate;
class LaserRect : public LaserShape
{
    Q_OBJECT
public:
    LaserRect(const QRect rect, int cornerRadius, LaserDocument* doc, 
        QTransform transform = QTransform(), int layerIndex = 1, int cornerRadiusType = CRT_Round);
    virtual ~LaserRect() {}

    QRect rect() const;
    void setRect(const QRect& rect);

    int cornerRadius() const;
    int cornerType() const;
    void setCornerRadius(int cornerRadius, int type);
    bool isRoundedRect() const;

    virtual void draw(QPainter* painter);
    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);

	//virtual QRect sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
	virtual virtual LaserPrimitive* cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserRect)
    Q_DISABLE_COPY(LaserRect)
};

