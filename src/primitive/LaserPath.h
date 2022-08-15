#pragma once

#include "LaserShape.h"

class LaserPathPrivate;
class LaserPath : public LaserShape
{
    Q_OBJECT
public:
    LaserPath(LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    LaserPath(const QPainterPath& path, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserPath() {}

    QPainterPath path() const;
    void setPath(const QPainterPath& path);

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

    virtual QList<QPainterPath> subPaths() const;
	//virtual QRect sceneBoundingRect() const;

    virtual QJsonObject toJson();

	//virtual void reShape();
	QVector<QLineF> edges();
    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
	virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPath);
    Q_DISABLE_COPY(LaserPath);
};



