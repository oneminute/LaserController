#include "LaserEllipse.h"
#include "LaserShapePrivate.h"

#include <QJsonArray>
#include <QPainter>

#include "common/Config.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "undo/PrimitiveAddingCommand.h"
#include "util/MachiningUtils.h"
#include "util/Utils.h"
#include "widget/LaserViewer.h"

class LaserEllipsePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserEllipse)
public:
    LaserEllipsePrivate(LaserEllipse* ptr)
        : LaserShapePrivate(ptr)
    {}
    QPoint point1;
    QPoint point2;
};

LaserEllipse::LaserEllipse(LaserDocument* doc, QTransform transform, int layerIndex)
    : LaserEllipse(QRect(), doc, transform, layerIndex)
{
}

LaserEllipse::LaserEllipse(const QRect bounds, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserEllipsePrivate(this), doc, LPT_ELLIPSE, layerIndex, saveTransform)
{
	
    Q_D(LaserEllipse);
    d->boundingRect = bounds;
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
    if (isEditing())
    {
        QRect rect(d->point1, d->point2);
        qLogD << rect;
        painter->drawEllipse(rect);
         QPen oldPen = painter->pen();
        QPen newPen(Qt::red, 1);
        newPen.setCosmetic(true);
        painter->setPen(newPen);
        utils::drawCrossingLines(rect.topLeft(), 1000, painter);
        utils::drawCrossingLines(rect.topRight(), 1000, painter);
        utils::drawCrossingLines(rect.bottomLeft(), 1000, painter);
        utils::drawCrossingLines(rect.bottomRight(), 1000, painter);
        painter->setPen(oldPen);
    }
    else
    {
        painter->drawPath(d->path);
    }
}

QJsonObject LaserEllipse::toJson()
{
	Q_D(const LaserEllipse);
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
	//bounds
	QJsonArray bounds = { d->boundingRect.x(), d->boundingRect.y(),d->boundingRect.width(), d->boundingRect.height() };
	object.insert("name", name());
	object.insert("className", this->metaObject()->className());
	//object.insert("position", position);
	object.insert("matrix", matrix);
	object.insert("bounds", bounds);
	object.insert("layerIndex", layerIndex());
	return object;
}

//scene
QVector<QLine> LaserEllipse::edges()
{
	Q_D(const LaserEllipse);
	return LaserPrimitive::edges(sceneTransform().map(d->path));
}

void LaserEllipse::sceneMousePressEvent(LaserViewer* viewer, LaserScene* scene, const QPoint& point, QMouseEvent* event)
{
    Q_D(LaserEllipse);
    if (isEditing())
    {
        d->point1 = point;
        d->point2 = point;

        document()->addPrimitive(this, true, true);
    }
}

void LaserEllipse::sceneMouseMoveEvent(LaserViewer* viewer, LaserScene* scene, const QPoint& point, QMouseEvent* event, bool isPressed)
{
    Q_D(LaserEllipse);
    if (isEditing())
    {
        d->point2 = point;
    }
}

void LaserEllipse::sceneMouseReleaseEvent(LaserViewer* viewer, LaserScene* scene, const QPoint& point, QMouseEvent* event, bool isPressed)
{
    Q_D(LaserEllipse);
    if (isEditing())
    {
        d->point2 = point;
        //防止宽高为负值
        int left = qMin(d->point1.x(), d->point2.x());
        int top = qMin(d->point1.y(), d->point2.y());
        int right = qMax(d->point1.x(), d->point2.x());
        int bottom = qMax(d->point1.y(), d->point2.y());
        int width = right - left;
        int height = bottom - top;
        QRect rect = QRect(QPoint(left, top), QSize(width, height));
        //qLogD << d->point1 << ", " << d->point2 << ", " << rect;
        if (rect.isNull() || rect.isEmpty() || !rect.isValid())
        {
            document()->removePrimitive(this, true, true, true);
        }
        else
        {
            setEditing(false);
            setBounds(rect);
        }

        emit viewer->endEditing();
    }
}

void LaserEllipse::sceneKeyPressEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

void LaserEllipse::sceneKeyReleaseEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

void LaserEllipse::beginCreatingInternal(QUndoCommand* parentCmd,
    PrimitiveAddingCommand* addingCmd)
{
}

void LaserEllipse::endCreatingInterval(QUndoCommand* parentCmd,
    PrimitiveRemovingCommand* removingCmd)
{
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

