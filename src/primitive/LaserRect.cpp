#include "LaserRect.h"
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

class LaserRectPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserRect)
public:
    LaserRectPrivate(LaserRect* ptr)
        : LaserShapePrivate(ptr)
        , cornerRadius(0)
    {}
    int cornerType;
    int cornerRadius;
};

LaserRect::LaserRect(const QRect rect, int cornerRadius, LaserDocument * doc, 
    QTransform saveTransform, int layerIndex, int cornerRadiusType)
    : LaserShape(new LaserRectPrivate(this), doc, LPT_RECT, layerIndex, saveTransform)
{
    Q_D(LaserRect);
    d->boundingRect = rect;
	//d->originalBoundingRect = rect;
    setCornerRadius(qAbs(cornerRadius), cornerRadiusType);
	sceneTransformToItemTransform(saveTransform);
    d->outline.addRect(rect);
}

QRect LaserRect::rect() const 
{
    Q_D(const LaserRect);
    return d->boundingRect; 
}

void LaserRect::setRect(const QRect& rect) 
{
    Q_D(LaserRect);
    d->boundingRect = rect; 
	//d->originalBoundingRect = rect;
	d->path = QPainterPath();
	d->path.addRect(d->boundingRect);
    d->outline = QPainterPath();
    d->outline.addRect(rect);
}

int LaserRect::cornerRadius() const
{
    Q_D(const LaserRect);
    return d->cornerRadius;
}

int LaserRect::cornerType() const
{
    Q_D(const LaserRect);
    return d->cornerType;
}

void LaserRect::setCornerRadius(int cornerRadius, int type)
{
    Q_D(LaserRect);
    d->cornerType = type;
    d->cornerRadius = cornerRadius;
    d->path = computeCornerRadius(d->boundingRect, cornerRadius, type);
}

bool LaserRect::isRoundedRect() const
{
    Q_D(const LaserRect);
    return !utils::fuzzyEquals(d->cornerRadius, 0, 0.0001);
}

void LaserRect::draw(QPainter* painter)
{
    Q_D(LaserRect);
    painter->drawPath(d->path);
}

//QRect LaserRect::sceneBoundingRect() const
//{
//	Q_D(const LaserRect);
//    QTransform t = sceneTransform();
//	return t.mapRect(d->boundingRect);
//}

QJsonObject LaserRect::toJson()
{
	Q_D(const LaserRect);
	QJsonObject object;
	//QTransform transform = d->allTransform;
	QTransform transform = QTransform();
	QJsonArray matrix = {
		transform.m11(), transform.m12(), transform.m13(),
		transform.m21(), transform.m22(), transform.m23(),
		transform.m31(), transform.m32(), transform.m33()
	};
	QTransform parentTransform = sceneTransform();
	QJsonArray parentMatrix = { parentTransform.m11(), parentTransform.m12(), parentTransform.m13(), parentTransform.m21(), parentTransform.m22(), parentTransform.m23(), parentTransform.m31(), parentTransform.m32(), parentTransform.m33() };
	object.insert("parentMatrix", parentMatrix);
    object.insert("cornerRadius", d->cornerRadius);
	//rect
	QJsonArray bounds = { d->boundingRect.x(), d->boundingRect.y(),d->boundingRect.width(), d->boundingRect.height() };
	//QJsonArray();
	object.insert("name", name());
	object.insert("className", this->metaObject()->className());
	//object.insert("position", position);
	object.insert("matrix", matrix);
	object.insert("bounds", bounds);
	object.insert("layerIndex", layerIndex());
	return object;
}

QVector<QLineF> LaserRect::edges()
{
	Q_D(const LaserRect);
	return LaserPrimitive::edges(sceneTransform().map(d->path));
}

LaserPrimitive * LaserRect::clone(QTransform t)
{
	Q_D(const LaserRect);
	LaserRect* cloneRect = new LaserRect(this->rect(), this->cornerRadius(), this->document(), t, d->layerIndex);
	return cloneRect;
}

void LaserRect::setBoundingRectWidth(qreal width)
{
    Q_D(LaserRect);
    //d->boundingRect = QRect(d->boundingRect.left(), d->boundingRect.top(), width, d->boundingRect.height());
    setRect(QRect(d->boundingRect.left(), d->boundingRect.top(), width, d->boundingRect.height()));
}

void LaserRect::setBoundingRectHeight(qreal height)
{
    Q_D(LaserRect);
    setRect(QRect(d->boundingRect.left(), d->boundingRect.top(), d->boundingRect.width(), height));
}

bool LaserRect::isClosed() const
{
    return true;
}

QPointF LaserRect::position() const
{
    Q_D(const LaserRect);
    return sceneTransform().map(d->boundingRect.topLeft());
}

LaserPointListList LaserRect::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserRect);
    ProgressItem* progress = new ProgressItem(tr("%1 update machining points").arg(name()), ProgressItem::PT_Simple, parentProgress);
    d->machiningPointsList.clear();
    d->startingIndices.clear();
    if (isRoundedRect())
    {
        machiningUtils::path2Points(progress, sceneTransform().map(d->path), d->machiningPointsList, d->startingIndices, d->machiningCenter);
    }
    else
    {
        QPolygon poly = d->path.toFillPolygon(sceneTransform()).toPolygon();
        QPoint pt1 = poly.at(0);
        QPoint pt2 = poly.at(1);
        QPoint pt3 = poly.at(2);
        QPoint pt4 = poly.at(3);

        QLineF line11(pt1, pt2);
        QLineF line12(pt1, pt4);
        qreal angle11 = line11.angle();
        qreal angle12 = line12.angle();

        QLineF line21(pt2, pt3);
        QLineF line22(pt2, pt1);
        qreal angle21 = line21.angle();
        qreal angle22 = line22.angle();

        QLineF line31(pt3, pt4);
        QLineF line32(pt3, pt2);
        qreal angle31 = line31.angle();
        qreal angle32 = line32.angle();

        QLineF line41(pt4, pt1);
        QLineF line42(pt4, pt3);
        qreal angle41 = line41.angle();
        qreal angle42 = line42.angle();

        LaserPointList points;
        points.push_back(LaserPoint(pt1.x(), pt1.y()/*, qRound(angle11), qRound(angle12)*/));
        points.push_back(LaserPoint(pt2.x(), pt2.y()/*, qRound(angle21), qRound(angle22)*/));
        points.push_back(LaserPoint(pt3.x(), pt3.y()/*, qRound(angle31), qRound(angle32)*/));
        points.push_back(LaserPoint(pt4.x(), pt4.y()/*, qRound(angle41), qRound(angle42)*/));
        d->machiningCenter = utils::center(points).toPoint();
        points.push_back(points.first());
        d->startingIndices.append(0);
        d->startingIndices.append(1);
        d->startingIndices.append(2);
        d->startingIndices.append(3);
        d->machiningPointsList.append(points);
        progress->finish();
    }

    return d->machiningPointsList;
}


