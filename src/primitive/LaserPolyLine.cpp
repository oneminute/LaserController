#include "LaserPolyLine.h"
#include "LaserShapePrivate.h"
#include "LaserPolygon.h"

#include <QGraphicsSceneEvent>
#include <QJsonArray>
#include <QPainter>

#include "common/Config.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "undo/PolylineAddPointCommand.h"
#include "undo/PrimitiveAddingCommand.h"
#include "undo/PrimitiveRemovingCommand.h"
#include "util/MachiningUtils.h"
#include "util/Utils.h"
#include "widget/LaserViewer.h"

class LaserPolylinePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserPolyline)
public:
    LaserPolylinePrivate(LaserPolyline* ptr)
        : LaserShapePrivate(ptr)
    {}
    QPoint editingPoint;
    QVector<QPoint> points;
};

LaserPolyline::LaserPolyline(const QPolygon & poly, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserPolylinePrivate(this), doc, LPT_POLYLINE, layerIndex, saveTransform)
{
    Q_D(LaserPolyline);
    setPolyline(poly);
	sceneTransformToItemTransform(saveTransform);
}

QPolygon LaserPolyline::polyline() const 
{
    Q_D(const LaserPolyline);
    QPolygon poly(d->points);
    return poly; 
}

void LaserPolyline::setPolyline(const QPolygon& poly) 
{
    Q_D(LaserPolyline);
    d->points.clear();
    d->path.clear();
    d->outline.clear();
    bool first = true;
    for (const QPoint& point : poly)
    {
        d->points.append(point);
        if (first)
        {
            d->editingPoint = point;
            d->path.moveTo(point);
            d->outline.moveTo(point);
            first = false;
        }
        else
        {
            d->path.lineTo(point);
            d->outline.lineTo(point);
        }
    }
    d->boundingRect = d->path.boundingRect().toRect();
}

void LaserPolyline::updatePath()
{
    Q_D(LaserPolyline);
    d->path.clear();
    d->outline.clear();
    bool first = true;
    for (const QPoint& point : d->points)
    {
        if (first)
        {
            d->path.moveTo(point);
            d->outline.moveTo(point);
            first = false;
        }
        else
        {
            d->path.lineTo(point);
            d->outline.lineTo(point);
        }
    }
    d->boundingRect = d->path.boundingRect().toRect();

}

int LaserPolyline::appendPoint(const QPoint& point)
{
    Q_D(LaserPolyline);
    d->editingPoint = point;
    d->points.append(point);
    updatePath();
    return d->points.size() - 1;
}

void LaserPolyline::removeLastPoint()
{
    Q_D(LaserPolyline);
    d->points.removeLast();
    updatePath();
}

void LaserPolyline::removePoint(int pointIndex)
{
    Q_D(LaserPolyline);
    d->points.remove(pointIndex);
    updatePath();
}

QPoint LaserPolyline::pointAt(int pointIndex)
{
    Q_D(LaserPolyline);
    return d->points[pointIndex];
}

void LaserPolyline::setEditingPoint(const QPoint& point)
{
    Q_D(LaserPolyline);
    d->editingPoint = point;
}

QPoint LaserPolyline::editingPoint() const
{
    Q_D(const LaserPolyline);
    return d->editingPoint;
}

LaserPointListList LaserPolyline::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserPolyline);
    ProgressItem* progress = new ProgressItem(tr("%1 update machining points").arg(name()), ProgressItem::PT_Simple, parentProgress);
    progress->setMaximum(d->points.size());
    d->machiningPointsList.clear();
    d->startingIndices.clear();
    bool isClosed = this->isClosed();
    LaserPointList points;
    QTransform t = sceneTransform();
    d->machiningCenter = QPoint(0, 0);
    for (int i = 0; i < d->points.size(); i++)
    {
        QPoint pt = t.map(d->points.at(i));

        QPoint cPt = pt;
        QPoint nPt = (i == d->points.size() - 1) ? pt + (pt - d->points.at(i - 1)) : d->points.at(i + 1);
        QPoint lPt = (i == 0) ? pt + (pt - d->points.at(1)) : d->points.at(i - 1);
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
    if (d->points.count() > 1)
	    painter->drawPath(d->path);

    if (isEditing())
    {
        if (d->points.count() >= 1)
		    painter->drawLine(d->points.last(), d->editingPoint);

        QPen oldPen = painter->pen();
        QPen newPen(Qt::red, 1);
        newPen.setCosmetic(true);
        painter->setPen(newPen);
        for (const QPoint& point : d->points)
        {
            painter->drawLine(point + QPoint(-1000, -1000), point + QPoint(1000, 1000));
            painter->drawLine(point + QPoint(1000, -1000), point + QPoint(-1000, 1000));
        }
        painter->setPen(oldPen);
    }
}

QJsonObject LaserPolyline::toJson()
{
	Q_D(const LaserPolyline);
	QJsonObject object;
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
	for (int pIndex = 0; pIndex < d->points.size(); pIndex++) {
		QPointF point = d->points[pIndex];
		QJsonArray pointArray = {point.x(), point.y()};
		poly.append(pointArray);
	}
	object.insert("name", name());
	object.insert("className", this->metaObject()->className());
	object.insert("matrix", matrix);
	object.insert("poly", poly);
	object.insert("layerIndex", layerIndex());
	return object;
}

QVector<QLineF> LaserPolyline::edges()
{
	Q_D(const LaserPolyline);
	QPainterPath path;
	path.addPolygon(sceneTransform().map(d->points));
	return LaserPrimitive::edges(path, true);
}

void LaserPolyline::sceneMousePressEvent(
    LaserViewer* viewer,
    LaserScene* scene,
    const QPoint& point,
    QMouseEvent* event)
{
}

void LaserPolyline::sceneMouseMoveEvent(
    LaserViewer* viewer,
    LaserScene* scene,
    const QPoint& point,
    QMouseEvent* event,
    bool isPressed)
{
    Q_D(LaserPolyline);
    if (isEditing())
    {
        setEditingPoint(point);
    }
}

void LaserPolyline::sceneMouseReleaseEvent(
    LaserViewer* viewer,
    LaserScene* scene,
    const QPoint& point,
    QMouseEvent* event,
    bool isPressed)
{
    Q_D(LaserPolyline);
    if (event->button() == Qt::LeftButton)
    {
        if (d->editingPoint == d->points.last())
            return;

        if (d->editingPoint == d->points.first()) {
            LaserLayer* layer = document()->getCurrentOrCapableLayer(LPT_POLYGON);
            if (layer)
            {
                QUndoCommand* cmd = new QUndoCommand(tr("Add Polygon"));

                PrimitiveRemovingCommand* cmdRemoving = new PrimitiveRemovingCommand(
                    tr("Remove Polyline"), viewer, scene, document(), 
                    this->id(), this->layer()->id(), this, cmd
                );
                cmdRemoving->setUndoCallback([=]()
                    {
                        emit viewer->creatingPolygon();
                    }
                );

                LaserPolygon* polygon = new LaserPolygon(QPolygon(d->points), document(),
                    QTransform(), layer->index());
                PrimitiveAddingCommand* cmdAdding = new PrimitiveAddingCommand(
                    tr("Convert to Polygon"), viewer, scene, document(), 
                    polygon->id(), layer->id(), polygon, cmd
                );

                viewer->addUndoCommand(cmd);
                setCursor(Qt::ArrowCursor);
                setEditing(false);
                viewer->endEditing();
                emit viewer->readyPolygon();
            }
        }
        else
        {
            PolylineAddPointCommand* cmd = new PolylineAddPointCommand(
                tr("Add Point to Polyline"), viewer, scene, document(), this->id(), d->editingPoint, d->points.size()
            );
            viewer->addUndoCommand(cmd);
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        viewer->endEditing();
        setEditing(false);
        emit viewer->readyPolygon();
    }
}

void LaserPolyline::sceneKeyPressEvent(
    LaserViewer* viewer,
    QKeyEvent* event)
{
}

void LaserPolyline::sceneKeyReleaseEvent(
    LaserViewer* viewer,
    QKeyEvent* event)
{
}

LaserPrimitive * LaserPolyline::cloneImplement()
{
	Q_D(LaserPolyline);
	LaserPolyline* polyline = new LaserPolyline(d->points, document(), 
        sceneTransform(), d->layerIndex);
	return polyline;
}

bool LaserPolyline::isClosed() const
{
    Q_D(const LaserPolyline);
    return utils::fuzzyEquals(d->points.first(), d->points.last());
}

QPointF LaserPolyline::position() const
{
    Q_D(const LaserPolyline);
    return sceneTransform().map(d->points.first());
}

