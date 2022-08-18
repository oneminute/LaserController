#include "LaserLine.h"
#include "LaserShapePrivate.h"

#include <QJsonArray>
#include <QPainter>

#include "common/Config.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "undo/LineAddPointCommand.h"
#include "undo/PrimitiveAddingCommand.h"
#include "undo/PrimitiveRemovingCommand.h"
#include "util/MachiningUtils.h"
#include "widget/LaserViewer.h"

class LaserLinePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserLine)
public:
    LaserLinePrivate(LaserLine* ptr)
        : LaserShapePrivate(ptr)
    {}

    QLine line;
    QVector<QPoint> points;
    QPoint editingPoint;
};

LaserLine::LaserLine(LaserDocument* doc, QTransform transform, int layerIndex)
    : LaserLine(QLine(), doc, transform, layerIndex)
{
}

LaserLine::LaserLine(const QLine & line, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserLinePrivate(this), doc, LPT_LINE, layerIndex, saveTransform)
{
    Q_D(LaserLine);
    d->line = line;
	sceneTransformToItemTransform(saveTransform);
    d->path.moveTo(d->line.p1());
    d->path.lineTo(d->line.p2());
    d->boundingRect = d->path.boundingRect().toRect();
    d->outline.moveTo(d->line.p1());
    d->outline.lineTo(d->line.p2());
}

QLine LaserLine::line() const 
{
    Q_D(const LaserLine);
	
    return d->line; 
}

void LaserLine::setLine(const QLine& line) 
{
    Q_D(LaserLine);
    d->line = line; 
    d->path = QPainterPath();
    d->path.moveTo(d->line.p1());
    d->path.lineTo(d->line.p2());
    d->boundingRect = d->path.boundingRect().toRect();
    d->outline = QPainterPath();
    d->outline.moveTo(d->line.p1());
    d->outline.lineTo(d->line.p2());
}

void LaserLine::setEditingPoint(const QPoint& point)
{
    Q_D(LaserLine);
}

QPoint LaserLine::editingPoint() const
{
    return QPoint();
}

int LaserLine::appendPoint(const QPoint& point)
{
    Q_D(LaserLine);
    d->editingPoint = point;
    d->points.append(point);
    if (d->points.size() == 2)
    {
        setLine(QLine(d->points.first(), d->points.last()));
    }
    return d->points.size() - 1;
}

void LaserLine::removeLastPoint()
{
    Q_D(LaserLine);
    d->points.removeLast();
}

void LaserLine::removePoint(int pointIndex)
{
    Q_D(LaserLine);
    d->points.remove(pointIndex);
}

QJsonObject LaserLine::toJson()
{
	Q_D(const LaserLine);
	QJsonObject object;
	QTransform transform = QTransform();
	QJsonArray matrix = {
		transform.m11(), transform.m12(), transform.m13(),
		transform.m21(), transform.m22(), transform.m23(),
		transform.m31(), transform.m32(), transform.m33()
	};
	QTransform parentTransform = this->sceneTransform();
	QJsonArray parentMatrix = { parentTransform.m11(), parentTransform.m12(), parentTransform.m13(), parentTransform.m21(), parentTransform.m22(), parentTransform.m23(), parentTransform.m31(), parentTransform.m32(), parentTransform.m33() };
	object.insert("parentMatrix", parentMatrix);
	//line
	QJsonArray line = { d->line.p1().x(), d->line.p1().y(),d->line.p2().x(), d->line.p2().y() };
	object.insert("name", name());
	object.insert("className", this->metaObject()->className());
	//object.insert("position", position);
	object.insert("matrix", matrix);
	object.insert("line", line);
	object.insert("layerIndex", layerIndex());
	return object;
}

QVector<QLine> LaserLine::edges()
{
	Q_D(const LaserLine);
	QVector<QLine>list;
	QLine line = sceneTransform().map(d->line);
	list.append(line);
	return list;
}

void LaserLine::sceneMousePressEvent(LaserViewer* viewer, LaserScene* scene, 
    const QPoint& point, QMouseEvent* event)
{
}

void LaserLine::sceneMouseMoveEvent(LaserViewer* viewer, LaserScene* scene, 
    const QPoint& point, QMouseEvent* event, bool isPressed)
{
    Q_D(LaserLine);
    if (isEditing())
    {
        d->editingPoint = point;
    }
}

void LaserLine::sceneMouseReleaseEvent(LaserViewer* viewer, LaserScene* scene, 
    const QPoint& point, QMouseEvent* event, bool isPressed)
{
    Q_D(LaserLine);
    LaserLayer* layer = this->layer();
    if (event->button() == Qt::LeftButton)
    {
        // if there's no points in current line
        if (d->points.isEmpty())
        {
            setEditing(true);

            // the current operation includs two sub commands:
            //   1. add the LaserLine primitive object;
            //   2. add a point to it;
            // so we create a parent QUndoCommand to comibine the two commands
            // togather as one step.
            QUndoCommand* cmd = new QUndoCommand(tr("Add Line"));

            PrimitiveAddingCommand* cmdAdding = new PrimitiveAddingCommand(
                tr("Add Line"), viewer, scene, this->document(), this->id(), 
                layer->id(), this, cmd);

            // we must ensure that when we undo the adding operation we should 
            // end the editing state in LaserViewer
            cmdAdding->setUndoCallback([=]()
                {
                    emit viewer->endEditing();
                }
            );
            // as we adding and editing the line, we must ensure that the
            // LaserViewer know it's in editing state
            cmdAdding->setRedoCallback([=]()
                {
                    viewer->setEditingPrimitiveId(id());
                    emit viewer->beginEditing();
                }
            );

            LineAddPointCommand* cmdAddPoint = new LineAddPointCommand(
                tr("Add Point to Line"), viewer, scene, document(), 
                id(), point, d->points.size(), cmd);
            viewer->addUndoCommand(cmd);
        }
        // there're more than one point in the current editing polygon
        else
        {
            if (d->editingPoint == d->points.last())
                return;

            LineAddPointCommand* cmd = new LineAddPointCommand(
                tr("Add Point to Line"), viewer, scene, document(),
                id(), d->editingPoint, d->points.size());
            viewer->addUndoCommand(cmd);

            d->points.clear();
            setEditing(false);
            emit viewer->endEditing();
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        if (d->points.size() <= 1)
        {
            PrimitiveRemovingCommand* cmdRemoving = new PrimitiveRemovingCommand(
                tr("Remove Line"), viewer, scene, document(), id(), layer->id(), this);
            cmdRemoving->setUndoCallback([=]()
                {
                    emit viewer->beginEditing();
                }
            );
            viewer->addUndoCommand(cmdRemoving);
        }
        else
        {
            d->points.clear();
            setEditing(false);
            emit viewer->endEditing();
        }
    }
}

void LaserLine::sceneKeyPressEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

void LaserLine::sceneKeyReleaseEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

LaserPrimitive * LaserLine::cloneImplement()
{
	Q_D(const LaserLine);
	LaserLine* line = new LaserLine(this->line(), document(), sceneTransform(),
        d->layerIndex);
	return line;
}

bool LaserLine::isClosed() const
{
    return false;
}

QPointF LaserLine::position() const
{
    Q_D(const LaserLine);
    return sceneTransform().map(d->line.p1());
}

LaserPointListList LaserLine::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserLine);
    d->machiningPointsList.clear();
    d->startingIndices.clear();

    ProgressItem* progress = new ProgressItem(tr("%1 update machining points").arg(name()), ProgressItem::PT_Simple, parentProgress);
	QTransform t = sceneTransform();
    QPointF pt1 = t.map(d->line.p1());
    QPointF pt2 = t.map(d->line.p2());
    QLineF line1(pt1, pt2);
    QLineF line2(pt2, pt1);
    qreal angle1 = line1.angle();
    qreal angle2 = line2.angle();
    LaserPointList points;
    points.append(LaserPoint(pt1.x(), pt1.y()/*, angle1, angle2*/));
    points.append(LaserPoint(pt2.x(), pt2.y()/*, angle2, angle1*/));
    d->startingIndices.append(0);
    d->startingIndices.append(1);
    d->machiningPointsList.append(points);
    progress->finish();
    
    return d->machiningPointsList;
}

void LaserLine::draw(QPainter * painter)
{
    Q_D(LaserLine);
    if (isEditing())
    {
        painter->drawLine(QLine(d->points.first(), d->editingPoint));
        QPen oldPen = painter->pen();
        QPen newPen(Qt::red, 1);
        newPen.setCosmetic(true);
        painter->setPen(newPen);
        painter->drawLine(d->points.first() + QPoint(-1000, -1000), d->points.first() + QPoint(1000, 1000));
        painter->drawLine(d->points.first() + QPoint(1000, -1000), d->points.first() + QPoint(-1000, 1000));
        painter->setPen(oldPen);
    }
    else
    {
        painter->drawLine(d->line);
    }
}


