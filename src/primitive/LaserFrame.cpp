#include "LaserFrame.h"
#include "LaserStampBasePrivate.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>

#include "common/Config.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "widget/LaserViewer.h"

class LaserFramePrivate : public LaserStampBasePrivate
{
    Q_DECLARE_PUBLIC(LaserFrame)
public:
    LaserFramePrivate(LaserFrame* ptr)
        : LaserStampBasePrivate(ptr)
    {
    }
    QRect outerRect;
    QRect innerRect;
    qreal width;
    qreal cornerRadius;
    QPainterPath outerPath;
    QPainterPath innerPath;
    int cornerType;
    bool needAuxiliaryLine;
    bool isInner;//内框
};

LaserFrame::LaserFrame(LaserDocument* doc, QRect outerRect, qreal width, qreal cornnerRadilus, bool stampIntaglio,
    QTransform transform, int layerIndex, int cornerType)
    : LaserStampBase(new LaserFramePrivate(this), doc, LPT_FRAME, stampIntaglio, transform, layerIndex)
{
    Q_D(LaserFrame);
    if (width <= 0) {
        width = 1;
    }
    d->isInner = false;
    //d->stampIntaglio = stampIntaglio;
    d->needAuxiliaryLine = true;
    d->outerRect = outerRect;
    d->width = width;
    d->cornerRadius = qAbs(cornnerRadilus);
    d->innerRect = QRect(QPoint(outerRect.topLeft().x() + width, outerRect.topLeft().y() + width), 
        QPoint(outerRect.bottomRight().x() - width, outerRect.bottomRight().y() - width));
    d->cornerType = cornerType;
    setCornerRadius(d->cornerRadius, cornerType);
    
    //d->originalBoundingRect = d->boundingRect;
    setTransform(transform);
    setZValue(1);
}

LaserFrame::~LaserFrame()
{
}
void LaserFrame::draw(QPainter * painter)
{
    Q_D(LaserFrame);
    QColor color = d->doc->layers()[d->layerIndex]->color();
    /*QColor color = Qt::red;
    if (layer()) {
        color = this->layer()->color();
    }*/
    
    if (!d->isInner) {
        if (!d->stampIntaglio) {
            painter->setBrush(Qt::white);
            painter->drawPath(d->outerPath);
            //painter->setBrush(QBrush(color));
            setStampBrush(painter, color, QSize(d->boundingRect.width(), d->boundingRect.height()));
            painter->drawPath(d->path);
        }
        else {
            //painter->setBrush(QBrush(color));
            setStampBrush(painter, color, QSize(d->boundingRect.width(), d->boundingRect.height()));
            painter->drawPath(d->outerPath- d->antiFakePath);
        }
    }
    else {
        painter->setPen(Qt::NoPen);
        if (!d->stampIntaglio) {
            painter->setBrush(Qt::white);
            painter->drawPath(d->outerPath);
            //painter->setBrush(QBrush(color));
            setStampBrush(painter, color, QSize(d->boundingRect.width(), d->boundingRect.height()));
        }
        else {
            painter->setBrush(QBrush(color));
            painter->drawPath(d->innerPath);
            //painter->setBrush(Qt::white);
            setStampBrush(painter, Qt::white, QSize(d->boundingRect.width(), d->boundingRect.height()), QTransform(), true);
        }
        
        painter->drawPath(d->path);
    }
    
    painter->setBrush(Qt::NoBrush);
    //绘制辅助线
    if (d->needAuxiliaryLine) {
        painter->setPen(QPen(Qt::black, 280, Qt::DotLine));
        qreal halfW = d->innerRect.width() * 0.5;
        qreal halfH = d->innerRect.height() * 0.5;
        QLine hLine(QPoint(d->innerRect.center().x() - halfW, d->innerRect.center().y()),
            QPoint(d->innerRect.center().x() + halfW, d->innerRect.center().y()));
        QLine vLine(QPoint(d->innerRect.center().x(), d->innerRect.center().y() - halfH),
            QPoint(d->innerRect.center().x(), d->innerRect.center().y() + halfH));
        painter->drawLine(hLine);
        painter->drawLine(vLine);
    }
    //painter->setBrush(QBrush(Qt::red));
    //painter->setPen(QPen(Qt::red));
    //painter->drawPath(d->antiFakePath);
}
QJsonObject LaserFrame::toJson()
{
    Q_D(const LaserFrame);
    QJsonObject object;
    QTransform pt = QTransform();
    QJsonArray pm = {
        pt.m11(), pt.m12(), pt.m13(),
        pt.m21(), pt.m22(), pt.m23(),
        pt.m31(), pt.m32(), pt.m33()
    };
    object.insert("parentMatrix", pm);
    QTransform t = this->sceneTransform();
    QJsonArray matrix = { t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31(), t.m32(), t.m33() };
    object.insert("matrix", matrix);
    QJsonArray bounds = { d->outerRect.x(), d->outerRect.y(), d->outerRect.width(), d->outerRect.height() };
    object.insert("name", name());
    object.insert("className", this->metaObject()->className());
    object.insert("width", d->width);
    object.insert("layerIndex", layerIndex());
    object.insert("bounds", bounds);
    object.insert("cornerType", d->cornerType);
    object.insert("cornerRadius", d->cornerRadius);
    stampBaseToJson(object);
    return object;
}
LaserPrimitive * LaserFrame::cloneImplement()
{
    Q_D(LaserFrame);
    LaserFrame* frame = new LaserFrame(document(), d->outerRect, d->width, 
        d->cornerRadius, d->stampIntaglio, sceneTransform(), d->layerIndex);
    stampBaseClone(frame);
    return frame;
}
QVector<QLine> LaserFrame::edges()
{
    Q_D(LaserFrame);
    return LaserPrimitive::edges(sceneTransform().map(d->path));
}
void LaserFrame::setInner(bool bl)
{
    Q_D(LaserFrame);
    d->isInner = bl;
    if (bl) {
        setZValue(2);
    }
    else {
        setZValue(1);
    }
}
bool LaserFrame::isInner()
{
    Q_D(LaserFrame);
    return d->isInner;
}
QPainterPath LaserFrame::outerPath()
{
    Q_D(LaserFrame);
    return d->outerPath;
}
QPainterPath LaserFrame::innerPath()
{
    Q_D(LaserFrame);
    return d->innerPath;
}
void LaserFrame::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mousePressEvent(event);
}
void LaserFrame::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mouseMoveEvent(event);
}
void LaserFrame::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    QList<QGraphicsView*> views = scene()->views();
    QGraphicsView* v = views[0];
    if (v->metaObject()->className() == "LaserViewer") {
        LaserViewer* viewer = qobject_cast<LaserViewer*> (views[0]);
        viewer->onEndSelecting();
    }
    
}
bool LaserFrame::isClosed() const
{
    return true;
}
QPointF LaserFrame::position() const
{
    Q_D(const LaserFrame);
    return sceneTransform().map(d->path.pointAtPercent(0));
}
void LaserFrame::setCornerRadius(qreal cornerRadius, int type)
{
    Q_D(LaserFrame);
    d->cornerRadius = qAbs(cornerRadius);
    d->cornerType = type;
    d->outerPath = computeCornerRadius(d->outerRect, d->cornerRadius, type);
    qreal shorter = d->outerRect.width();
    if (shorter > d->outerRect.height()) {
        shorter = d->outerRect.height();
    }
    shorter *= 0.5;
    qreal innerR;
    innerR = d->cornerRadius - d->width*0.5;
    if (innerR < 100) {
        innerR = 100;
    }
    d->innerPath = computeCornerRadius(d->innerRect, innerR, type);
    computePath();
}
qreal LaserFrame::cornerRadius()
{
    Q_D(LaserFrame);
    return d->cornerRadius;
}
int LaserFrame::cornerRadiusType()
{
    Q_D(LaserFrame);
    return d->cornerType;
}
QRectF LaserFrame::innerRect()
{
    Q_D(LaserFrame);
    return d->innerRect;
}
void LaserFrame::setBorderWidth(qreal w)
{
    Q_D(LaserFrame);
    if (w <= 0) {
        w = 1;
    }
    d->width = w;
    d->innerRect = QRect(QPoint(d->outerRect.topLeft().x() + w, d->outerRect.topLeft().y() + w),
        QPoint(d->outerRect.bottomRight().x() - w, d->outerRect.bottomRight().y() - w));
    d->innerPath = computeCornerRadius(d->innerRect, d->cornerRadius, d->cornerType);
    computePath();
    
}
qreal LaserFrame::borderWidth()
{
    Q_D(LaserFrame);
    return d->width;
}
void LaserFrame::computePath()
{
    Q_D(LaserFrame);
    
    d->path = d->outerPath - d->innerPath;
    d->boundingRect = d->path.boundingRect().toRect();
    d->originalPath = d->path;
}
bool LaserFrame::needAuxiliaryLine()
{
    Q_D(LaserFrame);
    return d->needAuxiliaryLine;
}
void LaserFrame::setNeedAuxiliaryLine(bool bl)
{
    Q_D(LaserFrame);
    d->needAuxiliaryLine = bl;
}
void LaserFrame::setBoundingRectWidth(qreal width)
{
    Q_D(LaserFrame);
    qreal diff = d->outerRect.width() - width;
    d->outerRect = QRect(d->outerRect.left() + diff * 0.5, d->outerRect.top(), width, d->outerRect.height());
    d->outerPath = computeCornerRadius(d->outerRect, d->cornerRadius, d->cornerType);
    d->innerRect = QRect(d->innerRect.left() + diff * 0.5, d->innerRect.top(), d->innerRect.width() - diff, d->innerRect.height());
    d->innerPath = computeCornerRadius(d->innerRect, d->cornerRadius, d->cornerType);
    computePath();
}
void LaserFrame::setBoundingRectHeight(qreal height)
{
    Q_D(LaserFrame);
    qreal diff = d->outerRect.height() - height;
    d->outerRect = QRect(d->outerRect.left(), d->outerRect.top() + diff * 0.5, d->outerRect.width(), height);
    d->outerPath = computeCornerRadius(d->outerRect, d->cornerRadius, d->cornerType);
    d->innerRect = QRect(d->innerRect.left(), d->innerRect.top() + diff * 0.5, d->innerRect.width(), d->innerRect.height() - diff);
    d->innerPath = computeCornerRadius(d->innerRect, d->cornerRadius, d->cornerType);
    computePath();
}

void LaserFrame::sceneMousePressEvent(LaserViewer* viewer, LaserScene* scene, const QPoint& point, QMouseEvent* event)
{
}

void LaserFrame::sceneMouseMoveEvent(LaserViewer* viewer, LaserScene* scene, const QPoint& point, QMouseEvent* event, bool isPressed)
{
}

void LaserFrame::sceneMouseReleaseEvent(LaserViewer* viewer, LaserScene* scene, const QPoint& point, QMouseEvent* event, bool isPressed)
{
}

void LaserFrame::sceneKeyPressEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

void LaserFrame::sceneKeyReleaseEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

void LaserFrame::beginCreatingInternal(QUndoCommand* parentCmd, PrimitiveAddingCommand* addingCmd)
{
}

void LaserFrame::endCreatingInterval(QUndoCommand* parentCmd, PrimitiveRemovingCommand* removingCmd)
{
}

