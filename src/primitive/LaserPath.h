#pragma once

#include "LaserShape.h"

class LaserPathPrivate;
class LaserPath : public LaserShape
{
    Q_OBJECT
public:
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
	LaserPrimitive * clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPath);
    Q_DISABLE_COPY(LaserPath);
};



