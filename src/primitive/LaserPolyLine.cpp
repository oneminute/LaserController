#include "LaserPolyLine.h"
#include "LaserShapePrivate.h"

#include <QJsonArray>
#include <QPainter>

#include "common/Config.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "util/MachiningUtils.h"
#include "util/Utils.h"

class LaserPolylinePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserPolyline)
public:
    LaserPolylinePrivate(LaserPolyline* ptr)
        : LaserShapePrivate(ptr)
    {}
    QPolygon poly;
};

LaserPolyline::LaserPolyline(const QPolygon & poly, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserPolylinePrivate(this), doc, LPT_POLYLINE, layerIndex, saveTransform)
{
    Q_D(LaserPolyline);
    d->poly = poly;
	d->path.addPolygon(d->poly);
	sceneTransformToItemTransform(saveTransform);
    d->boundingRect = d->poly.boundingRect();
	//d->originalBoundingRect = d->boundingRect;
    d->outline.moveTo(*d->poly.begin());
    for (int i = 1; i < d->poly.count(); i++)
    {
        d->outline.lineTo(d->poly[i]);
    }
}

QPolygon LaserPolyline::polyline() const 
{
    Q_D(const LaserPolyline);
    return d->poly; 
}

void LaserPolyline::setPolyline(const QPolygon& poly) 
{
    Q_D(LaserPolyline);
    d->poly = poly; 
    d->path = QPainterPath();
	d->path.addPolygon(d->poly);
    d->boundingRect = d->poly.boundingRect();
	//d->originalBoundingRect = d->boundingRect;
    d->outline = QPainterPath();
    d->outline.moveTo(*d->poly.begin());
    for (int i = 1; i < d->poly.count(); i++)
    {
        d->outline.lineTo(d->poly[i]);
    }
}

int LaserPolyline::appendPoint(const QPoint& point)
{
    Q_D(LaserPolyline);
    d->poly.append(point);
    return d->poly.size() - 1;
}

void LaserPolyline::removeLastPoint()
{
    Q_D(LaserPolyline);
    d->poly.removeLast();
}

void LaserPolyline::removePoint(int pointIndex)
{
    Q_D(LaserPolyline);
    d->poly.remove(pointIndex);
}

QPoint LaserPolyline::pointAt(int pointIndex)
{
    Q_D(LaserPolyline);
    return d->poly[pointIndex];
}

LaserPointListList LaserPolyline::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserPolyline);
    ProgressItem* progress = new ProgressItem(tr("%1 update machining points").arg(name()), ProgressItem::PT_Simple, parentProgress);
    progress->setMaximum(d->poly.size());
    d->machiningPointsList.clear();
    d->startingIndices.clear();
    bool isClosed = this->isClosed();
    LaserPointList points;
    QTransform t = sceneTransform();
    d->machiningCenter = QPoint(0, 0);
    for (int i = 0; i < d->poly.size(); i++)
    {
        QPoint pt = t.map(d->poly.at(i));

        QPoint cPt = pt;
        QPoint nPt = (i == d->poly.size() - 1) ? pt + (pt - d->poly.at(i - 1)) : d->poly.at(i + 1);
        QPoint lPt = (i == 0) ? pt + (pt - d->poly.at(1)) : d->poly.at(i - 1);
        QLineF line1(cPt, nPt);
        QLineF line2(cPt, lPt);
        qreal angle1 = line1.angle();
        qreal angle2 = line2.angle();
        points.append(LaserPoint(pt.x(), pt.y()/*, qRound(angle1), qRound(angle2)*/));
        if (isClosed)
        {
            d->startingIndices.append(i);
        }
        d->machiningCenter += pt;
        progress->increaseProgress();
    }
    if (!isClosed)
    {
        d->startingIndices.append(0);
        d->startingIndices.append(points.size() - 1);
    }
        
    d->machiningCenter /= points.size();
    d->machiningPointsList.append(points);
    progress->finish();
    return d->machiningPointsList;
}

void LaserPolyline::draw(QPainter * painter)
{
    Q_D(LaserPolyline);
	painter->drawPath(d->path);
    //painter->drawRect(boundingRect());
}

QJsonObject LaserPolyline::toJson()
{
	Q_D(const LaserPolyline);
	QJsonObject object;
	//QJsonArray position = { pos().x(), pos().y() };
	QTransform t = QTransform();
	
	QJsonArray matrix = {
		t.m11(), t.m12(), t.m13(),
		t.m21(), t.m22(), t.m23(),
		t.m31(), t.m32(), t.m33()
	};
	QTransform parentTransform = this->sceneTransform();
	QJsonArray parentMatrix = { parentTransform.m11(), parentTransform.m12(), parentTransform.m13(), parentTransform.m21(), parentTransform.m22(), parentTransform.m23(), parentTransform.m31(), parentTransform.m32(), parentTransform.m33() };
	object.insert("parentMatrix", parentMatrix);
	qDebug() << parentTransform;
	QJsonArray poly;
	for (int pIndex = 0; pIndex < d->poly.size(); pIndex++) {
		QPointF point = d->poly[pIndex];
		QJsonArray pointArray = {point.x(), point.y()};
		poly.append(pointArray);
	}
	object.insert("name", name());
	object.insert("className", this->metaObject()->className());
	//object.insert("position", position);
	object.insert("matrix", matrix);
	object.insert("poly", poly);
	object.insert("layerIndex", layerIndex());
	return object;
}

QVector<QLineF> LaserPolyline::edges()
{
	Q_D(const LaserPolyline);
	QPainterPath path;
	path.addPolygon(sceneTransform().map(d->poly));
	return LaserPrimitive::edges(path, true);
}

LaserPrimitive * LaserPolyline::clone(QTransform t)
{
	Q_D(LaserPolyline);
	LaserPolyline* polyline = new LaserPolyline(d->poly, document(), t, d->layerIndex);
	return polyline;
}

bool LaserPolyline::isClosed() const
{
    Q_D(const LaserPolyline);
    return utils::fuzzyEquals(d->poly.first(), d->poly.last());
}

QPointF LaserPolyline::position() const
{
    Q_D(const LaserPolyline);
    return sceneTransform().map(d->poly.first());
}

