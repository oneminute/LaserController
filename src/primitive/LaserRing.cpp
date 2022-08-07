#include "LaserRing.h"
#include "LaserStampBasePrivate.h"

#include <QJsonArray>
#include <QPainter>
#include <QtMath>

#include "LaserApplication.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "ui/LaserControllerWindow.h"

class LaserRingPrivate : public LaserStampBasePrivate
{
    Q_DECLARE_PUBLIC(LaserRing)
public:
    LaserRingPrivate(LaserRing* ptr)
        : LaserStampBasePrivate(ptr)
    {
    }
    qreal outerRadius;
    QPoint centerPoint;
    qreal width;
    QRectF outerBounds;
    QRectF innerBounds;
    QPainterPath outerPath;
    QPainterPath innerPath;
    QPixmap mask;
    bool isInner;//是否是内圈
};
LaserRing::LaserRing(LaserDocument* doc, QRectF outerRect, qreal width, bool stampIntaglio,QTransform transform, int layerIndex)
    : LaserStampBase(new LaserRingPrivate(this), doc, LPT_RING, stampIntaglio, transform, layerIndex)
{
    Q_D(LaserRing);
    if (width <= 0) {
        width = 1;
    }
    d->isInner = false;
    //d->stampIntaglio = stampIntaglio;
    d->width = qAbs(width);
    d->outerBounds = outerRect;
    d->innerBounds = QRectF(QPointF(outerRect.topLeft().x() + d->width, outerRect.topLeft().y() + d->width), 
        QPointF(outerRect.bottomRight().x() - d->width, outerRect.bottomRight().y() - d->width));
    d->outerPath.addEllipse(d->outerBounds);
    d->innerPath.addEllipse(d->innerBounds);
    computePath();
    setTransform(transform);
    setZValue(1);
    //mask
    /*qreal w = d->boundingRect.width() * 0.28;
    qreal h = d->boundingRect.height() * 0.28;
    d->mask  = QPixmap (":/ui/icons/images/fingerprint.png");
    QTransform t;
    t.scale(w / d->mask.width(), h / d->mask.height());
    d->mask = d->mask.transformed(t);
    d->mask = d->mask.createMaskFromColor(Qt::transparent);*/
}
LaserRing::~LaserRing()
{
}
void LaserRing::draw(QPainter * painter)
{
    Q_D(LaserRing);
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
            painter->drawPath(d->outerPath - d->antiFakePath);
        }
    }
    else {
        if (!d->stampIntaglio) {
            painter->setBrush(Qt::white);
            painter->drawPath(d->outerPath);
            //painter->setBrush(QBrush(color));
            setStampBrush(painter, color, QSize(d->boundingRect.width(), d->boundingRect.height()));
            painter->drawPath(d->path);
        }
        else {
            painter->setBrush(QBrush(color));
            painter->drawPath(d->outerPath);
            //painter->setBrush(Qt::white);
            setStampBrush(painter, Qt::white, QSize(d->boundingRect.width(), d->boundingRect.height()), QTransform(), true);
            painter->drawPath(d->path);
        }
    }
    
    painter->setBrush(Qt::NoBrush);
    
    /*int len = image.width() * image.height();
    for (int i = 0; i < image.width(); i++) {
        for (int j = 0; j < image.height(); j++) {
            QRgb rgb = image.pixel(i, j);
            if (qGray(rgb) == 255) {
                image.setPixel(i, j, QColor(0, 0, 0, 0).rgba());
            }
        }
    }*/   
}
QJsonObject LaserRing::toJson()
{
    Q_D(const LaserRing);
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
    QJsonArray bounds = { d->outerBounds.x(), d->outerBounds.y(), d->outerBounds.width(), d->outerBounds.height() };
    object.insert("name", name());
    object.insert("className", this->metaObject()->className());
    object.insert("width", d->width);
    object.insert("layerIndex", layerIndex());
    object.insert("bounds", bounds);
    stampBaseToJson(object);
    return object;
}
LaserPrimitive * LaserRing::clone(QTransform t)
{
    Q_D(LaserRing);
    LaserRing* ring = new LaserRing(document(), d->outerBounds, d->width,d->stampIntaglio, t, d->layerIndex);
    stampBaseClone(ring);
    return ring;
}
QVector<QLineF> LaserRing::edges()
{
    Q_D(LaserRing);
    //qDebug()<<LaserPrimitive::edges(sceneTransform().map(d->path)).size();
    return LaserPrimitive::edges(sceneTransform().map(d->path));
}
void LaserRing::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mousePressEvent(event);
}
void LaserRing::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mouseMoveEvent(event);
}
void LaserRing::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    QList<QGraphicsView*> views = scene()->views();
    QGraphicsView* v = views[0];
    if (v->metaObject()->className() == "LaserViewer") {
        LaserViewer* viewer = qobject_cast<LaserViewer*> (v);
        viewer->onEndSelecting();
    }
    
}
bool LaserRing::isClosed() const
{
    return true;
}
QPointF LaserRing::position() const
{
    Q_D(const LaserRing);
    return sceneTransform().map(d->path.pointAtPercent(0));
}

QRectF LaserRing::innerRect()
{
    Q_D(const LaserRing);
    return d->innerBounds;
}

QPainterPath LaserRing::outerPath()
{
    Q_D(const LaserRing);
    return d->outerPath;
}

QPainterPath LaserRing::innerPath()
{
    Q_D(const LaserRing);
    return d->innerPath;
}

void LaserRing::setInner(bool bl)
{
    Q_D(LaserRing);
    d->isInner = bl;
    if (bl) {
        setZValue(2);
    }
    else {
        setZValue(1);
    }
}

bool LaserRing::isInner()
{
    Q_D(LaserRing);
    return d->isInner;
}

void LaserRing::setBorderWidth(qreal w)
{
    Q_D(LaserRing);
    d->width = w;
    d->innerBounds = QRectF(QPointF(d->outerBounds.topLeft().x() + w, d->outerBounds.topLeft().y() + w),
        QPointF(d->outerBounds.bottomRight().x() - w, d->outerBounds.bottomRight().y() - w));
    d->innerPath = QPainterPath();
    d->innerPath.addEllipse(d->innerBounds);
    computePath();

}

qreal LaserRing::borderWidth()
{
    Q_D(const LaserRing);
    return d->width;
}

void LaserRing::computePath()
{
    Q_D(LaserRing);
    d->path = d->outerPath - d->innerPath;
    d->boundingRect = d->path.boundingRect().toRect();
    d->originalPath = d->path;
}

void LaserRing::setBoundingRectWidth(qreal width)
{
    Q_D(LaserRing);
    qreal diff = d->outerBounds.width() - width;
    d->outerBounds = QRect(d->outerBounds.left() + diff * 0.5, d->outerBounds.top(), width, d->outerBounds.height());
    d->outerPath = QPainterPath();
    d->outerPath.addEllipse(d->outerBounds);
    d->innerBounds = QRect(d->innerBounds.left() + diff * 0.5, d->innerBounds.top(), d->innerBounds.width() - diff, d->innerBounds.height());
    d->innerPath = QPainterPath();
    d->innerPath.addEllipse(d->innerBounds);
    computePath();
}

void LaserRing::setBoundingRectHeight(qreal height)
{
    Q_D(LaserRing);
    qreal diff = d->outerBounds.height() - height;
    d->outerBounds = QRect(d->outerBounds.left(), d->outerBounds.top()+diff*0.5, d->outerBounds.width(), height);
    d->outerPath = QPainterPath();
    d->outerPath.addEllipse(d->outerBounds);
    d->innerBounds = QRect(d->innerBounds.left(), d->innerBounds.top() + diff * 0.5, d->innerBounds.width(), d->innerBounds.height() - diff);
    d->innerPath = QPainterPath();
    d->innerPath.addEllipse(d->innerBounds);
    computePath();
}

