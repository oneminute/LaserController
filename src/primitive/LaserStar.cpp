#include "LaserStar.h"
#include "LaserStampBasePrivate.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QtMath>

#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "widget/LaserViewer.h"

class LaserStarPrivate : public LaserStampBasePrivate
{
    Q_DECLARE_PUBLIC(LaserStar)
public:
    LaserStarPrivate(LaserStar* ptr)
        : LaserStampBasePrivate(ptr)
    {        
    }
    qreal radius;
    QPoint centerPoint;
    QPointF points[10];
};

LaserStar::LaserStar(LaserDocument * doc, QPoint centerPos, qreal radius, bool stampIntaglio, QTransform transform, int layerIndex)
    : LaserStampBase(new LaserStarPrivate(this), doc, LPT_STAR, stampIntaglio, transform, layerIndex)
{
    Q_D(LaserStar);
    //d->stampIntaglio = stampIntaglio;
    d->radius = radius;
    d->centerPoint = centerPos;
    computePath();
    //d->originalBoundingRect = d->boundingRect;
    setTransform(transform);
    setZValue(3);
}

LaserStar::~LaserStar()
{
}

void LaserStar::draw(QPainter * painter)
{
    Q_D(LaserStar);
    /*d->path.setFillRule(Qt::WindingFill);
    painter->setBrush(QBrush(this->layer()->color()));
    painter->drawPath(d->path);
    painter->setBrush(Qt::NoBrush);*/
    d->path.setFillRule(Qt::WindingFill);
    if (!d->stampIntaglio) {
        //painter->setBrush(QBrush(this->layer()->color()));

        //painter->setBrush(QBrush(d->doc->layers()[d->layerIndex]->color()));
        setStampBrush(painter, d->doc->layers()[d->layerIndex]->color(),QSize(d->boundingRect.width(), d->boundingRect.height()),  QTransform(), true);
        /*if (layer()) {
            painter->setBrush(QBrush(this->layer()->color()));
        }
        else {
            painter->setBrush(QBrush(Qt::red));
        }*/
        painter->drawPath(d->path);
        painter->setBrush(Qt::NoBrush);
    }
    else {
        //painter->setBrush(Qt::white);
        setStampBrush(painter, Qt::white, QSize(d->boundingRect.width(), d->boundingRect.height()), QTransform(), true);
        painter->drawPath(d->path);
        painter->setBrush(Qt::NoBrush);

    }
    
}

QJsonObject LaserStar::toJson()
{
    Q_D(const LaserStar);
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
    QJsonArray center = { d->centerPoint.x(), d->centerPoint.y() };
    object.insert("name", name());
    object.insert("className", this->metaObject()->className());
    object.insert("radius", d->radius);
    object.insert("layerIndex", layerIndex());
    object.insert("center", center);
    stampBaseToJson(object);
    return object;
}

LaserPrimitive * LaserStar::cloneImplement()
{
    Q_D(LaserStar);
    LaserStar* star = new LaserStar(document(),d->centerPoint, d->radius, 
        d->stampIntaglio, sceneTransform(), d->layerIndex);
    stampBaseClone(star);
    return star;
}

QVector<QLineF> LaserStar::edges()
{
    Q_D(LaserStar);
    return LaserPrimitive::edges(sceneTransform().map(d->path));
}

void LaserStar::computePath()
{
    Q_D(LaserStar);
    d->path = QPainterPath();
    //计算圆上5个点
    qreal cos18 = qCos(M_PI / 10);
    qreal sin18 = qSin(M_PI / 10);
    qreal cos54 = qCos(M_PI * 3 / 10);
    qreal sin54 = qSin(M_PI * 3 / 10);
    d->points[0] = QPoint(d->centerPoint.x(), d->centerPoint.y() - d->radius);
    d->points[1] = QPoint(d->centerPoint.x() + d->radius * cos18,
        d->centerPoint.y() - d->radius * sin18);
    d->points[2] = QPoint(d->centerPoint.x() + d->radius * cos54,
        d->centerPoint.y() + d->radius * sin54);
    d->points[3] = QPoint(d->centerPoint.x() - d->radius * cos54,
        d->centerPoint.y() + d->radius * sin54);
    d->points[4] = QPoint(d->centerPoint.x() - d->radius * cos18,
        d->centerPoint.y() - d->radius * sin18);
    //计算交点
    QLineF line0(d->points[0], d->points[2]);
    QLineF line1(d->points[0], d->points[3]);
    QLineF line2(d->points[1], d->points[3]);
    QLineF line3(d->points[1], d->points[4]);
    QLineF line4(d->points[4], d->points[2]);
    QPointF p;
    line0.intersect(line3, &(d->points[5]));
    line0.intersect(line2, &(d->points[6]));
    line4.intersect(line2, &(d->points[7]));
    line4.intersect(line1, &(d->points[8]));
    line3.intersect(line1, &(d->points[9]));
    //计算路径
    d->path.moveTo(d->points[0]);
    d->path.lineTo(d->points[5]);
    d->path.lineTo(d->points[1]);
    d->path.lineTo(d->points[6]);
    d->path.lineTo(d->points[2]);
    d->path.lineTo(d->points[7]);
    d->path.lineTo(d->points[3]);
    d->path.lineTo(d->points[8]);
    d->path.lineTo(d->points[4]);
    d->path.lineTo(d->points[9]);
    d->path.closeSubpath();
    d->boundingRect = d->path.boundingRect().toRect();
    d->originalPath = d->path;
}

qreal LaserStar::radius()
{
    Q_D(const LaserStar);
    return d->radius;
}

/*void LaserStar::updatePoints()
{
    Q_D(LaserStar);
    QPolygonF pol = (sceneTransform().map(d->path)).toFillPolygon();
    d->points[0] = pol.at(0).toPoint();
    d->points[1] = pol.at(1).toPoint();
    d->points[2] = pol.at(2).toPoint();
    d->points[3] = pol.at(3).toPoint();
    d->points[4] = pol.at(4).toPoint();
}
//按照（0，1）（1，2）...的顺序画就是星形，因为使用的星的path
QPoint * LaserStar::points()
{
    Q_D(LaserStar);
    updatePoints();
    return d->points;
}*/

void LaserStar::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mousePressEvent(event);
}

void LaserStar::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

void LaserStar::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    QList<QGraphicsView*> views = scene()->views();
    QGraphicsView* v = views[0];
    if (v->metaObject()->className() == "LaserViewer") {
        LaserViewer* viewer = qobject_cast<LaserViewer*> (v);
        viewer->onEndSelecting();
    }   
}

bool LaserStar::isClosed() const
{
    return true;
}

QPointF LaserStar::position() const
{
    Q_D(const LaserStar);
    return sceneTransform().map(d->path.pointAtPercent(0));
}

void LaserStar::setBoundingRectWidth(qreal width)
{
    Q_D(LaserStar);
    d->radius = width * 0.5;
    computePath();
}

void LaserStar::setBoundingRectHeight(qreal height)
{
    Q_D(LaserStar);
    d->radius = height * 0.5;
    computePath();
}

