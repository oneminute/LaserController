#include "LaserPartyEmlem.h"
#include "LaserStampBasePrivate.h"

#include <QJsonArray>
#include <QPainter>
#include <QtMath>

#include "common/Config.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "util/Utils.h"

class LaserPartyEmblemPrivate : public LaserStampBasePrivate
{
    Q_DECLARE_PUBLIC(LaserPartyEmblem)
public:
    LaserPartyEmblemPrivate(LaserPartyEmblem* ptr)
        : LaserStampBasePrivate(ptr)
    {
    }
    qreal radius;
    QPoint centerPoint;
};
LaserPartyEmblem::LaserPartyEmblem(LaserDocument* doc, QPoint centerPos, qreal radius, bool stampIntaglio, QTransform transform,
    int layerIndex) 
    :LaserStampBase(new LaserPartyEmblemPrivate(this), doc, LPT_PARTYEMBLEM, stampIntaglio, transform, layerIndex)
{
    Q_D(LaserPartyEmblem);
    //d->stampIntaglio = stampIntaglio;
    d->centerPoint = centerPos;
    d->radius = radius;
    setTransform(transform);
    computePath();
    setZValue(3);
}
LaserPartyEmblem::~LaserPartyEmblem()
{
}

void LaserPartyEmblem::draw(QPainter* painter)
{
    Q_D(const LaserPartyEmblem);
    if (!d->stampIntaglio) {
        //painter->setBrush(QBrush(this->layer()->color()));
        //painter->setBrush(QBrush(d->doc->layers()[d->layerIndex]->color()));
        setStampBrush(painter, d->doc->layers()[d->layerIndex]->color(), QSize(d->boundingRect.width(), d->boundingRect.height()));
    }
    else {
        //painter->setBrush(QBrush(Qt::white));
        setStampBrush(painter, Qt::white, QSize(d->boundingRect.width(), d->boundingRect.height()), QTransform(), true);
    }
    painter->drawPath(d->path);
    painter->setBrush(Qt::NoBrush);
}

QJsonObject LaserPartyEmblem::toJson()
{
    Q_D(const LaserPartyEmblem);
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

LaserPrimitive* LaserPartyEmblem::cloneImplement()
{
    Q_D(LaserPartyEmblem);
    LaserPartyEmblem* p = new LaserPartyEmblem(d->doc, d->centerPoint, d->radius, 
        d->stampIntaglio, sceneTransform(), d->layerIndex);
    stampBaseClone(p);
    return p;
}

QVector<QLine> LaserPartyEmblem::edges()
{
    Q_D(LaserPartyEmblem);
    return LaserPrimitive::edges(sceneTransform().map(d->path));
}

void LaserPartyEmblem::computePath()
{
    Q_D(LaserPartyEmblem);
    QPoint center(0, 0);
    QTransform rotateT;
    rotateT.rotate(-45);
    //moon
    QRect moonRect(center.x() - d->radius, center.y() - d->radius, d->radius * 2, d->radius * 2);
    QVector2D moonDiagonalVec1(moonRect.topRight() - moonRect.bottomLeft());
    QVector2D moonDiagonalVec2(moonRect.topLeft() - moonRect.bottomRight());
    QPainterPath moonPath;
    QPainterPath moonCirclePath;
    moonCirclePath.addEllipse(moonRect);   
    QRect offsetMoonRect(QPoint(moonRect.left()- d->radius*0.00, moonRect.top() - d->radius * 0.00), QPoint(moonRect.left() + d->radius * 1.65, moonRect.top() + d->radius * 1.65));
    QPainterPath offsetMoonPath;
    offsetMoonPath.addEllipse(offsetMoonRect);
    moonPath = moonCirclePath - offsetMoonPath;
    QTransform rotateT1;
    rotateT1.rotate(7);
    moonPath = rotateT1.map(moonPath);
    //hammer head
    qreal hammerHeadW = d->radius * 2 * (0.51);
    qreal hammerHeadH = d->radius * 2 * (0.19);
    QRect hammerHeadRect(center.x() - hammerHeadW * 0.5, center.y() - hammerHeadH * 0.5, hammerHeadW, hammerHeadH);
    QPainterPath hammerPath;
    hammerPath.addRect(hammerHeadRect);
    //hammer gap
    QPainterPath hammerGap;
    QPointF gapCenter = hammerHeadRect.topRight();
    QPointF hammerGapTL(gapCenter.x() - hammerHeadH * 0.5, gapCenter.y() - hammerHeadH * 0.5);
    hammerGap.addEllipse(QRectF(hammerGapTL.x(), hammerGapTL.y(),hammerHeadH, hammerHeadH));
    hammerPath -= hammerGap;
    hammerPath = rotateT.map(hammerPath);
    QTransform translateT;
    translateT.translate(-d->radius * 0.34, -d->radius * 0.34);
    hammerPath = translateT.map(hammerPath);
    //hammer rod
    QPainterPath hammerRod;
    qreal hammerRodDiff = hammerHeadH * qSin(qDegreesToRadians(45.0));

    QPointF leftPoint(moonRect.left(), moonRect.top() + hammerRodDiff);
    QPointF topPoint(moonRect.left() + hammerRodDiff, moonRect.top());
    QPointF bottomPoint(moonRect.right() - hammerRodDiff, moonRect.bottom());
    QPointF rightPoint(moonRect.right(), moonRect.bottom() - hammerRodDiff);
    QLineF hammerRodLine1(topPoint , rightPoint);
    QLineF hammerRodLine2(leftPoint, bottomPoint);
    
    QPointF hammerHeadRectCenter = rotateT.map(hammerHeadRect.center());
    hammerHeadRectCenter = translateT.map(hammerHeadRectCenter);
    QLineF hammerHeaderHCenterLine(hammerHeadRectCenter,
        QPointF(hammerHeadRectCenter.x() + moonDiagonalVec1.x(), hammerHeadRectCenter.y() + moonDiagonalVec1.y()));
    QPointF topPointIntersect;
    QPointF leftPointIntersect;
    hammerRodLine1.intersect(hammerHeaderHCenterLine, &topPointIntersect);  
    hammerRodLine2.intersect(hammerHeaderHCenterLine, &leftPointIntersect);
    QPolygonF polygon;
    polygon.append(topPointIntersect);
    polygon.append(rightPoint);
    polygon.append(bottomPoint);
    polygon.append(leftPointIntersect);
    QPainterPath hammerRodPath;
    hammerRodPath.addPolygon(polygon);
    hammerPath += hammerRodPath;
    //moon offset
    QPainterPath offsetMoonRect_1Path;
    offsetMoonRect_1Path.addRect(hammerHeadRect);
    QPointF offset1(rotateT.map(hammerHeadRect.topLeft()));
    offset1 = translateT.map(offset1);
    offset1 = QPointF(offset1.x() + moonDiagonalVec2.x(), offset1.y() + moonDiagonalVec2.y());
    QPointF offset2(rotateT.map(hammerHeadRect.bottomLeft()));
    offset2 = translateT.map(offset2);
    QPointF offset3(offset1.x() - moonDiagonalVec1.x(), offset1.y() - moonDiagonalVec1.y());
    QPointF offset4(offset2.x() - moonDiagonalVec1.x(), offset2.y() - moonDiagonalVec1.y());
    QPolygonF offsetPoly;
    offsetPoly.append(offset1);
    offsetPoly.append(offset2);
    offsetPoly.append(offset4);
    offsetPoly.append(offset3);
    offsetMoonRect_1Path.addPolygon(offsetPoly);
    moonPath = moonPath - offsetMoonRect_1Path;
    //left bottom small circle
    QPointF smallCircleBottomLeft = moonRect.bottomLeft();
    QPainterPath smallCirclePath;
    qreal smallCircleRadius = d->radius * 0.355;
    smallCirclePath.addEllipse(QRectF(QPointF(smallCircleBottomLeft.x(), smallCircleBottomLeft.y() - smallCircleRadius),
        QPointF(smallCircleBottomLeft.x()+ smallCircleRadius, smallCircleBottomLeft.y())));
    d->path = moonPath+ hammerPath + smallCirclePath;
    //move
    QTransform translateEnd;
    translateEnd.translate(d->centerPoint.x(), d->centerPoint.y());
    d->path = translateEnd.map(d->path);
    d->boundingRect = d->path.boundingRect().toRect();
    d->originalPath = d->path;
}

qreal LaserPartyEmblem::radius()
{
    Q_D(const LaserPartyEmblem);
    return d->radius;
}

void LaserPartyEmblem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
}

void LaserPartyEmblem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

void LaserPartyEmblem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}

bool LaserPartyEmblem::isClosed() const
{
    return false;
}

QPointF LaserPartyEmblem::position() const
{
    return QPointF();
}

void LaserPartyEmblem::setBoundingRectWidth(qreal width)
{
    Q_D(LaserPartyEmblem);
    d->radius = width * 0.5;
    computePath();
}

void LaserPartyEmblem::setBoundingRectHeight(qreal height)
{
    Q_D(LaserPartyEmblem);
    d->radius = height * 0.5;
    computePath();
}

