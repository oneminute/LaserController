#include "LaserEllipse.h"
#include "LaserShapePrivate.h"

#include <QJsonArray>
#include <QPainter>

#include "common/Config.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "util/MachiningUtils.h"

class LaserEllipsePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserEllipse)
public:
    LaserEllipsePrivate(LaserEllipse* ptr)
        : LaserShapePrivate(ptr)
    {}
};

LaserEllipse::LaserEllipse(const QRect bounds, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserEllipsePrivate(this), doc, LPT_ELLIPSE, layerIndex, saveTransform)
{
	
    Q_D(LaserEllipse);
    d->boundingRect = bounds;
	//d->originalBoundingRect = bounds;
	sceneTransformToItemTransform(saveTransform);
	d->path.addEllipse(bounds);
	d->boundingRect = d->path.boundingRect().toRect();
    d->outline.addEllipse(bounds);
}

QRectF LaserEllipse::bounds() const 
{
    Q_D(const LaserEllipse);
    return d->boundingRect; 
} 

void LaserEllipse::setBounds(const QRect& bounds) 
{
    Q_D(LaserEllipse);
    d->boundingRect = bounds; 
	//d->originalBoundingRect = bounds;
    d->path = QPainterPath();
	d->path.addEllipse(d->boundingRect);
    d->outline = QPainterPath();
    d->outline.addEllipse(bounds);
}

LaserPointListList LaserEllipse::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserEllipse);
    Q_ASSERT(!d->path.isEmpty());
    QPainterPath path = sceneTransform().map(d->path);

    QList<int> indices;
    ProgressItem* progress = nullptr;
    if (parentProgress) 
        progress = new ProgressItem(tr("%1 update machining points").arg(name()), ProgressItem::PT_Simple, parentProgress);
    machiningUtils::path2Points(progress, path, d->machiningPointsList, indices, d->machiningCenter);
    /*QImage image(8192, 8192, QImage::Format_RGB888);
    QPainter painter(&image);
    QTransform t = LaserApplication::device->to1stQuad();
    t = t * QTransform::fromScale(8192 * 1.0 / Config::SystemRegister::xMaxLength(), 8192 * 1.0 / Config::SystemRegister::xMaxLength());
    painter.drawPath(t.map(path));
    QPen pen(Qt::red);
    painter.setPen(pen);
    for (const LaserPoint& lpt : d->machiningPointsList.first())
    {
        QPoint pt = lpt.toPoint();
        pt = t.map(pt);
        painter.drawPoint(pt);
    }
    image.save("tmp/circle.tiff", "TIFF");*/

    if (indices.length() <= Config::PathOptimization::maxStartingPoints())
    {
        d->startingIndices = indices;
    }
    else
    {
        for (int i = 0; i < Config::PathOptimization::maxStartingPoints(); i++)
        {
            int index = i * indices.length() / Config::PathOptimization::maxStartingPoints();
            d->startingIndices.append(indices.at(index));
        }
    }

    return d->machiningPointsList;
}

void LaserEllipse::draw(QPainter* painter)
{
    Q_D(LaserEllipse);
	painter->drawPath(d->path);
	painter->setPen(QPen(Qt::black, 1));
}

//QRect LaserEllipse::sceneBoundingRect() const
//{
//	Q_D(const LaserEllipse);
//	return sceneTransform().mapRect(d->boundingRect);
//}

/*void LaserEllipse::reShape()
{
	Q_D(LaserEllipse);
	d->path = transform().map(d->path);
	d->boundingRect = d->path.boundingRect();
	d->allTransform = d->allTransform * transform();
	setTransform(QTransform());
}*/

QJsonObject LaserEllipse::toJson()
{
	Q_D(const LaserEllipse);
	QJsonObject object;
	//QJsonArray position = { pos() .x(), pos() .y()};
	//QTransform transform = d->allTransform;
	QTransform transform = QTransform();
	QJsonArray matrix = { 
		transform.m11(), transform.m12(), transform.m13(), 
		transform.m21(), transform.m22(), transform.m23(), 
		transform.m31(), transform.m32(), transform.m33()
	};
	QTransform parentTransform = this->sceneTransform();
	QJsonArray parentMatrix = { parentTransform.m11(), parentTransform.m12(), parentTransform.m13(), parentTransform.m21(), parentTransform.m22(), parentTransform.m23(), parentTransform.m31(), parentTransform.m32(), parentTransform.m33() };
	object.insert("parentMatrix", parentMatrix);
	//bounds
	QJsonArray bounds = { d->boundingRect.x(), d->boundingRect.y(),d->boundingRect.width(), d->boundingRect.height() };
	QJsonArray();
	object.insert("name", name());
	object.insert("className", this->metaObject()->className());
	//object.insert("position", position);
	object.insert("matrix", matrix);
	object.insert("bounds", bounds);
	object.insert("layerIndex", layerIndex());
	return object;
}
//scene
QVector<QLineF> LaserEllipse::edges()
{
	Q_D(const LaserEllipse);
	return LaserPrimitive::edges(sceneTransform().map(d->path));
}

LaserPrimitive * LaserEllipse::cloneImplement()
{
	Q_D(LaserEllipse);
	LaserEllipse* ellipse = new LaserEllipse(d->boundingRect, document(), 
        sceneTransform(), d->layerIndex);
	return ellipse;
}

void LaserEllipse::setBoundingRectWidth(qreal width)
{
    Q_D(LaserEllipse);
    setBounds(QRect(d->boundingRect.left(), d->boundingRect.top(), width, 
        d->boundingRect.height()));
}

void LaserEllipse::setBoundingRectHeight(qreal height)
{
    Q_D(LaserEllipse);
    setBounds(QRect(d->boundingRect.left(), d->boundingRect.top(), d->boundingRect.width(), height));
}

bool LaserEllipse::isClosed() const
{
    return true;
}

QPointF LaserEllipse::position() const
{
    Q_D(const LaserEllipse);
    return sceneTransform().map(d->path.pointAtPercent(0));
}

