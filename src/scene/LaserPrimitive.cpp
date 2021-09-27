#include "LaserPrimitive.h"

#include <iostream>
#include <QPainterPath>
#include <QSharedData>
#include <QPaintEvent>
#include <QBuffer>
#include <QtMath>
#include <QGraphicsTextItem> 
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
#include <QTextEdit>
#include <QGraphicsSceneMouseEvent>
#include <QJsonArray>
#include <QJsonObject>
#include <QImageReader>
#include <QStack>

#include "LaserScene.h"
#include "laser/LaserDriver.h"
#include "common/Config.h"
#include "util/ImageUtils.h"
#include "util/MachiningUtils.h"
#include "util/TypeUtils.h"
#include "util/UnitUtils.h"
#include "util/Utils.h"
#include "widget/LaserViewer.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserPrimitiveGroup.h"
#include "laser/LaserLineList.h"

class LaserPrimitivePrivate: public ILaserDocumentItemPrivate
{
    Q_DECLARE_PUBLIC(LaserPrimitive)
public:
    LaserPrimitivePrivate(LaserPrimitive* ptr)
        : ILaserDocumentItemPrivate(ptr, LNT_PRIMITIVE)
        , doc(nullptr)
        , layer(nullptr)
        , isHover(false)
        , primitiveType(LPT_UNKNOWN)
        , machiningCenter(0, 0)
        , isLocked(false)
    {}

    LaserDocument* doc;
    LaserLayer* layer;
	int layerIndex;
    QRectF boundingRect;
    LaserPrimitiveType primitiveType;
    bool isHover;
    QPainterPath outline;
    LaserPointListList machiningPointsList;
    LaserPointListList arrangedPointsList;
    QPointF machiningCenter;
    QList<int> startingIndices;
	QTransform allTransform;
	QRectF originalBoundingRect;
	QPainterPath path;
    bool isLocked;
};

LaserPrimitive::LaserPrimitive(LaserPrimitivePrivate* data, LaserDocument* doc, LaserPrimitiveType type, QTransform saveTransform, int layerIndex)
    : ILaserDocumentItem(LNT_PRIMITIVE, data)
{
    Q_D(LaserPrimitive);
    d->doc = doc;
    d->primitiveType = type;
    Q_ASSERT(doc);
    QObject::setParent(doc);

    //this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    //this->setFlag(ItemIsFocusable, true);
    //this->setAcceptHoverEvents(true);
    d->name = newPrimitiveName(type);
	d->allTransform = saveTransform;
	d->layerIndex = layerIndex;
}

LaserPrimitive::~LaserPrimitive()
{
    Q_D(LaserPrimitive);
    qDebug() << d->name;
}

LaserDocument* LaserPrimitive::document() const 
{
    Q_D(const LaserPrimitive);
    return d->doc; 
}
//PolyLine or Polygon
QVector<QLineF> LaserPrimitive::edges(QPainterPath path, bool isPolyline)
{
	QPolygonF polygon = path.toFillPolygon();
	QVector<QLineF> edgeList;
	for (int i = 0; i < polygon.count()-1; i++) {
		
		QLineF edge;
		if (!isPolyline) {
			//polygon
			edge.setPoints(polygon.at(i), polygon.at(i + 1));
		}
		else {
			//polygon
			if (i < polygon.count() - 2) {
				edge.setPoints(polygon.at(i), polygon.at(i + 1));
			}
			else {
				break;
			}
		}
		
		edgeList << edge;
	}

	return edgeList;
}
QVector<QLineF> LaserPrimitive::edges()
{
	return QVector<QLineF>();
}
/*LaserPrimitive* LaserPrimitive::clone(const LaserPrimitive * primitive)
{
	return nullptr;
}*/
void setSelectedInGroup(bool selected) {

}
void LaserPrimitive::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_D(LaserPrimitive);
    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
    //qLogD << "primitive " << d->nodeName << " painter transform : " << painter->transform();

    QRectF bounds = boundingRect();
    QPointF topLeft = bounds.topLeft() - QPointF(2, 2);
    QPointF bottomRight = bounds.bottomRight() + QPointF(2, 2);
    bounds = QRectF(topLeft, bottomRight);
	QColor color = Qt::blue;
	/*QList<QGraphicsView*> views = scene()->views();
	LaserViewer* view = qobject_cast<LaserViewer*>(views[0]);*/
	//painter->setPen(QPen(color, 1, Qt::SolidLine));
    if (d->layer)
    {
        color = d->layer->color();
    }

	if (isSelected())
	{
		//isSelected();
		QString name = this->metaObject()->className();
		QPen pen = QPen(color, 1.2f, Qt::DashLine);
		pen.setCosmetic(true);
		painter->setPen(pen);
		if (name == "LaserBitmap") {
			painter->drawRect(bounds);
		}
	}
	//else if (isUnderMouse())
	else if (d->isHover)
	{
		//painter->setPen(QPen(Qt::green, 0.2f, Qt::SolidLine));
		//painter->drawRect(bounds);
	}
    else
    {
		//painter->setPen(QPen(Qt::GlobalColor::magenta, 0.5f, Qt::SolidLine));
		//painter->drawRect(bounds);
		
		QPen pen = QPen(color, 1.2f, Qt::SolidLine);
		pen.setCosmetic(true);
		painter->setPen(pen);
    }
	
    draw(painter);

    QPainterPath outline = this->outline();

    QPointF startPos = outline.pointAtPercent(0);
    painter->setPen(QPen(Qt::green, 1, Qt::SolidLine));
    if (Config::Debug::showPrimitiveName())
        painter->drawText(startPos, name());
    if (Config::Debug::showPrimitiveFirstPoint())
        painter->drawEllipse(startPos, 2, 2);
}

int LaserPrimitive::layerIndex()
{
	Q_D(const LaserPrimitive);
	return d->layerIndex;
}

QTransform LaserPrimitive::getAllTransform()
{
	Q_D(const LaserPrimitive);
	//return d->allTransform;
	return sceneTransform();
}

QPainterPath LaserPrimitive::getPath()
{
	Q_D(const LaserPrimitive);
	return d->path;
}

QPainterPath LaserPrimitive::getScenePath()
{
    Q_D(const LaserPrimitive);
    return sceneTransform().map(d->path);
}

QRectF LaserPrimitive::originalBoundingRect(qreal extendPixel) const
{
	Q_D(const LaserPrimitive);
    qreal x = d->boundingRect.topLeft().x() - extendPixel;
    qreal y = d->boundingRect.topLeft().y() - extendPixel;
    qreal width = d->boundingRect.width() + 2 * extendPixel;
    qreal height = d->boundingRect.height() + 2 * extendPixel;
    QRectF rect = QRectF(x, y, width, height);
	return rect;
}

QPolygonF LaserPrimitive::sceneOriginalBoundingPolygon(qreal extendPixel)
{
	QPolygonF bounding = sceneTransform().map(boundingRect());
	//QPolygonF bounding = transform().map(originalBoundingRect());
	/*qDebug() << transform();
	qDebug() << getAllTransform();
	qDebug() << sceneTransform();
	qDebug() << "originalBoundingRect(): " << originalBoundingRect();
	qDebug() <<"bounding: " << bounding;*/
	if (extendPixel != 0) {
		QVector2D p12 = QVector2D(bounding[1] - bounding[0]).normalized() * extendPixel;
		QVector2D p23 = QVector2D(bounding[2] - bounding[1]).normalized() * extendPixel;
		QVector2D p34 = QVector2D(bounding[3] - bounding[2]).normalized() * extendPixel;
		QVector2D p41 = QVector2D(bounding[0] - bounding[3]).normalized() * extendPixel;

		QPointF newP1 = bounding[0] - p12.toPointF() + p41.toPointF();
		QPointF newP2 = bounding[1] + p12.toPointF() - p23.toPointF();
		QPointF newP3 = bounding[2] - p34.toPointF() + p23.toPointF();
		QPointF newP4 = bounding[3] + p34.toPointF() - p41.toPointF();

		QPolygonF newBounding;
		newBounding.append(newP1);
		newBounding.append(newP2);
		newBounding.append(newP3);
		newBounding.append(newP4);
		return newBounding;
	}
	return bounding;
}

QRectF LaserPrimitive::boundingRect() const
{
    Q_D(const LaserPrimitive);
	QRectF bounds = d->boundingRect;
    return bounds;
}

QRectF LaserPrimitive::sceneBoundingRect() const
{
    Q_D(const LaserPrimitive);
    QRectF bounds = sceneTransform().mapRect(boundingRect());
    return bounds;
}

void LaserPrimitive::sceneTransformToItemTransform(QTransform sceneTransform)
{
	setTransform(sceneTransform);
	setPos(0, 0);
}

LaserPointListList LaserPrimitive::machiningPoints() const
{
    Q_D(const LaserPrimitive);
    return d->machiningPointsList;
}

LaserPointListList LaserPrimitive::arrangeMachiningPoints(LaserPoint& fromPoint, int startingIndex)
{
    Q_D(LaserPrimitive);
    d->arrangedPointsList.clear();

    int indexBase = 0;
    for (LaserPointList& machiningPoints : d->machiningPointsList)
    {
        int pointsCount = machiningPoints.size();
        int pointIndex = startingIndex - indexBase;
        bool ignore = false;
        if (pointIndex < 0 || pointIndex >= pointsCount)
        {
            // 首点索引不在当前子路径范围内，当前路径的加工点直接复制即可
            pointIndex = 0;
            ignore = true;
        }
        bool isClosed = utils::fuzzyCompare(machiningPoints.first().toPointF(), machiningPoints.last().toPointF());
        LaserPointList points;
        // check closour
        if (isClosed)
        {
            LaserPoint firstPoint = machiningPoints[pointIndex];

            int step = 0;
            if (fromPoint.angle1() >= 0)
            {
                step = -1;
            }
            else
            {
                step = 1;
            }

            int cursor = pointIndex;
            points.reserve(pointsCount);
            LaserPoint lastPoint = firstPoint;
            points.push_back(firstPoint);
            for (int i = 1; i < machiningPoints.length(); i++)
            {
                cursor = (cursor + step + machiningPoints.length()) % machiningPoints.length();
                LaserPoint& currentPoint = machiningPoints[cursor];
                if (!utils::fuzzyCompare(lastPoint.toPointF(), currentPoint.toPointF()))
                    points.push_back(currentPoint);
                lastPoint = currentPoint;
            }
            points.push_back(firstPoint);

            fromPoint = firstPoint;
        }
        else
        {
            if (pointIndex == 0)
            {
                points = machiningPoints;
                fromPoint = machiningPoints.last();
            }
            else if (!ignore)
            {
                for (int i = pointsCount - 1; i >= 0; i--)
                {
                    points.push_back(machiningPoints[i]);
                }
                fromPoint = points.first();
            }
        }

        d->arrangedPointsList.append(points);
        indexBase += pointsCount;
    }
    return d->arrangedPointsList;
}

LaserPointListList LaserPrimitive::arrangedPoints() const
{
    Q_D(const LaserPrimitive);
    return d->arrangedPointsList;
}

QString LaserPrimitive::generateFillData(QPointF& lastPoint) const 
{
    // 获取所有的加工线
    QPainterPath path = toMachiningPath();
    LaserLineList lines = utils::interLines(path, 70);

    // 建立所有线点的kdtree
    lines.buildKdtree();
    
    QByteArray buffer;
    // 从起刀点位置开始依次寻找最优点
    QPointF point = path.boundingRect().topLeft();
    for (int i = 0; i < lines.count(); i++)
    {
        QLineF line = lines.nearestSearch(point);
        QPointF diff1 = line.p1() - point;
        QPointF diff2 = line.p2() - line.p1();
        if (Config::Export::enableRelativeCoordinates())
        {
            buffer.append(QString("PU%1 %2;").arg(qRound(diff1.x())).arg(qRound(diff1.y())));
            buffer.append(QString("PD%1 %2;").arg(qRound(diff2.x())).arg(qRound(diff2.y())));
        }
        else
        {
            buffer.append(QString("PU%1 %2;").arg(qRound(line.p1().x())).arg(qRound(line.p1().y())));
            buffer.append(QString("PD%1 %2;").arg(qRound(line.p2().x())).arg(qRound(line.p2().y())));
        }
        point = line.p2();
        lastPoint = point;
    }
    return buffer; 
}

LaserPoint LaserPrimitive::arrangedStartingPoint() const
{
    Q_D(const LaserPrimitive);
    if (d->arrangedPointsList.isEmpty())
        return LaserPoint();
    return d->arrangedPointsList.first().first();
}

LaserPoint LaserPrimitive::arrangedEndingPoint() const
{
    Q_D(const LaserPrimitive);
    if (d->arrangedPointsList.isEmpty())
        return LaserPoint();
    return d->arrangedPointsList.last().last();
}

QList<int> LaserPrimitive::startingIndices() const
{
    Q_D(const LaserPrimitive);
    return d->startingIndices;
}

LaserPointList LaserPrimitive::startingPoints() const
{
    Q_D(const LaserPrimitive);
    LaserPointList vertices;
    if (d->machiningPointsList.empty())
    {
        return vertices;
    }

    int indexBase = 0;
    for (const LaserPointList& points : d->machiningPointsList)
    {
        for (int i = 0; i < d->startingIndices.length(); i++)
        {
            int index = d->startingIndices[i] - indexBase;
            if (index < points.length() && index >= 0)
                vertices.push_back(points.at(index));
        }
        indexBase += points.length();
    }

    return vertices;
}

LaserPoint LaserPrimitive::firstStartingPoint() const
{
    Q_D(const LaserPrimitive);
    if (d->machiningPointsList.isEmpty() || d->startingIndices.empty())
        return LaserPoint();
    return d->machiningPointsList.first()[d->startingIndices.first()];
}

LaserPoint LaserPrimitive::lastStartingPoint() const
{
    Q_D(const LaserPrimitive);
    if (d->machiningPointsList.isEmpty() || d->startingIndices.empty())
        return LaserPoint();
    int indexBase = 0;
    for (int i = 0; i < d->machiningPointsList.length() - 1; i++)
        indexBase += d->machiningPointsList.at(i).length();
    return d->machiningPointsList.last()[d->startingIndices.last() - indexBase];
}

QPointF LaserPrimitive::centerMachiningPoint() const
{
    Q_D(const LaserPrimitive);
    return d->machiningCenter;
}

LaserPrimitiveType LaserPrimitive::primitiveType() const
{
    Q_D(const LaserPrimitive);
    return d->primitiveType; 
}

QString LaserPrimitive::typeName() const
{
    Q_D(const LaserPrimitive);
    return typeName(d->primitiveType);
}

QString LaserPrimitive::typeLatinName() const
{
    Q_D(const LaserPrimitive);
    return typeLatinName(d->primitiveType);
}

bool LaserPrimitive::isShape() const 
{
    Q_D(const LaserPrimitive);
    return (int)d->primitiveType <= (int)LPT_SHAPE; 
}

bool LaserPrimitive::isBitmap() const 
{
    Q_D(const LaserPrimitive);
    return d->primitiveType == LPT_BITMAP; 
}

bool LaserPrimitive::isText() const
{
    Q_D(const LaserPrimitive);
    return d->primitiveType == LPT_TEXT;
}

LaserLayer* LaserPrimitive::layer() const 
{
    Q_D(const LaserPrimitive);
    return d->layer; 
}

void LaserPrimitive::setLayer(LaserLayer* layer) 
{
    Q_D(LaserPrimitive);
    d->layer = layer;
	if (layer) {
		d->layerIndex = layer->index();
	}
	else {
		d->layerIndex = -1;
	}
	
}

QString LaserPrimitive::toString() const
{
    return QString("[LaserPrimitive: name=%1]").arg(name());
}

QPainterPath LaserPrimitive::outline() const
{
    Q_D(const LaserPrimitive);
    return d->outline;
}

/*void LaserPrimitive::reShape()
{

}*/

void LaserPrimitive::setData(QPainterPath path,
	QTransform allTransform,
	QTransform transform,
	QRectF boundingRect)
{
	Q_D(LaserPrimitive);
	d->path = path;
	d->allTransform = transform;
	d->boundingRect = boundingRect;
	setTransform(transform);
}

QJsonObject LaserPrimitive::toJson()
{
	return QJsonObject();
}

QPointF LaserPrimitive::position() const 
{
    return QPointF(0, 0); 
}

QString LaserPrimitive::typeName(LaserPrimitiveType typeId)
{
    static QMap<LaserPrimitiveType, QString> TypeNamesMap{
        { LPT_BITMAP, tr("Bitmap") },
        { LPT_CIRCLE, tr("Circle") },
        { LPT_ELLIPSE, tr("Ellipse") },
        { LPT_LINE, tr("Line") },
        { LPT_PATH, tr("Path") },
        { LPT_POLYGON, tr("Polygon") },
        { LPT_POLYLINE, tr("Polyline") },
        { LPT_RECT, tr("Rect") },
        { LPT_TEXT, tr("Text") },
        { LPT_NURBS, tr("Nurbs") },
    };
    
    return TypeNamesMap[typeId];
}

QString LaserPrimitive::typeLatinName(LaserPrimitiveType typeId)
{
    static QMap<LaserPrimitiveType, QString> TypeLatinNamesMap{
        { LPT_BITMAP, "Bitmap" },
        { LPT_CIRCLE, "Circle" },
        { LPT_ELLIPSE, "Ellipse" },
        { LPT_LINE, "Line" },
        { LPT_PATH, "Path" },
        { LPT_POLYGON, "Polygon" },
        { LPT_POLYLINE, "Polyline" },
        { LPT_RECT, "Rect" },
        { LPT_TEXT, tr("Text") },
        { LPT_NURBS, tr("Nurbs") },
    };
    return TypeLatinNamesMap[typeId];
}

QString LaserPrimitive::newPrimitiveName(LaserPrimitiveType type) const
{
    Q_D(const LaserPrimitive);
    QMap<LaserPrimitiveType, int> typeCount;
    for (LaserPrimitive* primitive : d->doc->primitives().values())
    {
        if (!typeCount.contains(primitive->primitiveType()))
        {
            typeCount[primitive->primitiveType()] = 0;
        }
        typeCount[primitive->primitiveType()]++;
    }

    if (typeCount.contains(type))
    {
        bool used = true;
        int count = typeCount[type];
        while (true)
        {
            used = false;
            QString name = QString("%1_%2").arg(typeName(type)).arg(count);
            for (LaserPrimitive* primitive : d->doc->primitives().values())
            {
                if (name == primitive->name())
                {
                    used = true;
                    break;
                }
            }
            if (!used)
                break;
            count++;
        }
        return QString("%1_%2").arg(typeName(type)).arg(count);
    }
    return QString("%1_0").arg(typeName(type));
}

void LaserPrimitive::setLocked(bool isLocked)
{
    Q_D(LaserPrimitive);
    d->isLocked = isLocked;
}

bool LaserPrimitive::isLocked()
{
    Q_D(LaserPrimitive);
    return d->isLocked;
}

void LaserPrimitive::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_D(LaserPrimitive);
    d->isHover = true;
    update();
}

void LaserPrimitive::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_D(LaserPrimitive);
    d->isHover = false;
    update();
}

void LaserPrimitive::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	//QGraphicsObject::mousePressEvent(event);
    qLogD << "mousePressEvent: " << name();
	/*if (!this->isSelected()) {
		LaserDocument* document = (LaserDocument*)this->QObject::parent();
		document->scene()->clearSelection();
	}*/
	
    //update();
}

void LaserPrimitive::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
	//QGraphicsObject::mouseMoveEvent(event);
	//event->accept();
	qDebug() << "";
	
}

void LaserPrimitive::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	//QGraphicsObject::mouseReleaseEvent(event);
    //update();
}

class LaserShapePrivate : public LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserShape)
public:
    LaserShapePrivate(LaserShape* ptr)
        : LaserPrimitivePrivate(ptr)
    {}
};

LaserShape::LaserShape(LaserShapePrivate* data, LaserDocument* doc, LaserPrimitiveType type, int layerIndex, QTransform saveTransform)
    : LaserPrimitive(data, doc, type,  saveTransform, layerIndex)
{
	Q_D(LaserShape);
    //m_type = LPT_SHAPE;
	d->layerIndex = layerIndex;
}

class LaserEllipsePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserEllipse)
public:
    LaserEllipsePrivate(LaserEllipse* ptr)
        : LaserShapePrivate(ptr)
    {}

    QRectF bounds;
	//QPainterPath path;
};

LaserEllipse::LaserEllipse(const QRectF bounds, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserEllipsePrivate(this), doc, LPT_ELLIPSE, layerIndex, saveTransform)
{
	
    Q_D(LaserEllipse);
    d->bounds = bounds;
	d->originalBoundingRect = bounds;
	sceneTransformToItemTransform(saveTransform);
	d->path.addEllipse(d->bounds);
	//d->path.addEllipse(d->bounds);
	//d->path = saveTransform.map(d->path);
	d->boundingRect = d->path.boundingRect();
	//��е�ӹ���ʹ�
    d->outline.addEllipse(bounds);
	//d->layerIndex = layerIndex;
    //setLocked(true);
}

QRectF LaserEllipse::bounds() const 
{
    Q_D(const LaserEllipse);
    return d->bounds; 
} 

void LaserEllipse::setBounds(const QRectF& bounds) 
{
    Q_D(LaserEllipse);
    d->bounds = bounds; 
	QPainterPath path;
	path.addEllipse(d->bounds);
	d->path = path;
}

LaserPointListList LaserEllipse::updateMachiningPoints(quint32 progressCode, qreal progressQuota)
{
    Q_D(LaserEllipse);
    QPainterPath path = toMachiningPath();

    QList<int> indices;
    machiningUtils::path2Points(path, d->machiningPointsList, indices, d->machiningCenter);

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
	//painter->drawRect(boundingRect());
	painter->drawPath(d->path);
	//painter->setPen(QPen(Qt::black, 1));
	//painter->drawLine(edges()[0]);
}

QPainterPath LaserEllipse::toMachiningPath() const
{
    Q_D(const LaserEllipse);
    QTransform t = sceneTransform() * Global::matrixToMachining();
    QPainterPath path = t.map(d->path);
    return path;
}

QRectF LaserEllipse::sceneBoundingRect() const
{
	Q_D(const LaserEllipse);
	return sceneTransform().map(d->path).boundingRect();
}

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
	QJsonArray bounds = { d->bounds.x(), d->bounds.y(),d->bounds.width(), d->bounds.height() };
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

LaserPrimitive * LaserEllipse::clone(QTransform t)
{
	Q_D(LaserEllipse);
	LaserEllipse* ellipse = new LaserEllipse(d->boundingRect, document(), t, d->layerIndex);
	return ellipse;
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

class LaserRectPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserRect)
public:
    LaserRectPrivate(LaserRect* ptr)
        : LaserShapePrivate(ptr)
    {}

    QRectF rect;
	//QPainterPath path;
};

LaserRect::LaserRect(const QRectF rect, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserRectPrivate(this), doc, LPT_RECT, layerIndex, saveTransform)
{
    Q_D(LaserRect);
    d->rect = rect;
	d->originalBoundingRect = rect;
	d->path.addRect(rect);
	sceneTransformToItemTransform(saveTransform);
	//d->path = saveTransform.map(d->path);
    d->boundingRect = d->path.boundingRect();
    d->outline.addRect(rect);
}

QRectF LaserRect::rect() const 
{
    Q_D(const LaserRect);
    return d->rect; 
}

void LaserRect::setRect(const QRectF& rect) 
{
    Q_D(LaserRect);
    d->rect = rect; 
	QPainterPath path;
	path.addRect(d->rect);
	d->path = path;
}

void LaserRect::draw(QPainter* painter)
{
    Q_D(LaserRect);
    painter->drawPath(d->path);
}

LaserPointListList LaserRect::updateMachiningPoints(quint32 progressCode, qreal progressQuota)
{
    Q_D(LaserRect);
    d->machiningPointsList.clear();
    d->startingIndices.clear();
	QTransform t = sceneTransform() * Global::matrixToMachining();
    QPolygonF poly = d->path.toFillPolygon(t);
    QPointF pt1 = poly.at(0);
    QPointF pt2 = poly.at(1);
    QPointF pt3 = poly.at(2);
    QPointF pt4 = poly.at(3);

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
    points.push_back(LaserPoint(pt1.x(), pt1.y(), angle11, angle12));
    points.push_back(LaserPoint(pt2.x(), pt2.y(), angle21, angle22));
    points.push_back(LaserPoint(pt3.x(), pt3.y(), angle31, angle32));
    points.push_back(LaserPoint(pt4.x(), pt4.y(), angle41, angle42));
    d->machiningCenter = utils::center(points).toPointF();
    points.push_back(points.first());
    d->startingIndices.append(0);
    d->startingIndices.append(1);
    d->startingIndices.append(2);
    d->startingIndices.append(3);
    d->machiningPointsList.append(points);

    return d->machiningPointsList;
}

QPainterPath LaserRect::toMachiningPath() const
{
    Q_D(const LaserRect);
    QPainterPath path;
    QPolygonF rect = sceneTransform().map(d->rect);
    path.addPolygon(rect);

    path = Global::matrixToMachining().map(path);

    return path;
}

QRectF LaserRect::sceneBoundingRect() const
{
	Q_D(const LaserRect);
	return sceneTransform().map(d->path).boundingRect();
}

/*void LaserRect::reShape()
{
	Q_D(LaserRect);
	d->path = transform().map(d->path);
	d->boundingRect = d->path.boundingRect();
	d->allTransform = d->allTransform * transform();
	setTransform(QTransform());
}*/

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
	//rect
	QJsonArray bounds = { d->rect.x(), d->rect.y(),d->rect.width(), d->rect.height() };
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
	LaserRect* cloneRect = new LaserRect(this->rect(), this->document(), t, d->layerIndex);
	return cloneRect;
}

bool LaserRect::isClosed() const
{
    return true;
}

QPointF LaserRect::position() const
{
    Q_D(const LaserRect);
    return sceneTransform().map(d->rect.topLeft());
}

class LaserLinePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserLine)
public:
    LaserLinePrivate(LaserLine* ptr)
        : LaserShapePrivate(ptr)
    {}

    QLineF line;
};

LaserLine::LaserLine(const QLineF & line, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserLinePrivate(this), doc, LPT_LINE, layerIndex, saveTransform)
{
    Q_D(LaserLine);
    d->line = line;
    d->boundingRect = QRectF(d->line.p1(), d->line.p2());
	sceneTransformToItemTransform(saveTransform);
	d->originalBoundingRect = d->boundingRect;
    d->outline.moveTo(d->line.p1());
    d->outline.lineTo(d->line.p2());
}

QLineF LaserLine::line() const 
{
    Q_D(const LaserLine);
	
    return d->line; 
}

void LaserLine::setLine(const QLineF& line) 
{
    Q_D(LaserLine);
    d->line = line; 
}

LaserPointListList LaserLine::updateMachiningPoints(quint32 progressCode, qreal progressQuota)
{
    Q_D(LaserLine);
    d->machiningPointsList.clear();
    d->startingIndices.clear();

	QTransform t = sceneTransform() * Global::matrixToMachining();
    QPointF pt1 = t.map(d->line.p1());
    QPointF pt2 = t.map(d->line.p2());
    QLineF line1(pt1, pt2);
    QLineF line2(pt2, pt1);
    qreal angle1 = line1.angle();
    qreal angle2 = line2.angle();
    LaserPointList points;
    points.append(LaserPoint(pt1.x(), pt1.y(), angle1, angle2));
    points.append(LaserPoint(pt2.x(), pt2.y(), angle2, angle1));
    d->startingIndices.append(0);
    d->startingIndices.append(1);
    d->machiningPointsList.append(points);
    
    return d->machiningPointsList;
}

void LaserLine::draw(QPainter * painter)
{
    Q_D(LaserLine);
    painter->drawLine(d->line);
	//painter->drawRect(d->boundingRect);
}

QPainterPath LaserLine::toMachiningPath() const
{
    Q_D(const LaserLine);
    QPainterPath path;
    QLineF line = sceneTransform().map(d->line);
    path.moveTo(line.p1());
    path.lineTo(line.p2());

    QTransform transform = Global::matrixToMachining();
    path = transform.map(path);

    return path;
}

QRectF LaserLine::sceneBoundingRect() const
{
	Q_D(const LaserLine);
	QPainterPath path;
	QLineF line = sceneTransform().map(d->line);
	path.moveTo(line.p1());
	path.lineTo(line.p2());
	return path.boundingRect();
}
//after removeFromGroup
/*void LaserLine::reShape()
{
	Q_D(LaserLine);
	setLine(transform().map(d->line));
	QPainterPath path;
	//QLineF line = sceneTransform().map(d->line);
	path.moveTo(d->line.p1());
	path.lineTo(d->line.p2());
	d->boundingRect = path.boundingRect();
	//d->line = d->line;
	d->allTransform = d->allTransform * transform();
	setTransform(QTransform());
}*/

QJsonObject LaserLine::toJson()
{
	Q_D(const LaserLine);
	QJsonObject object;
	//QJsonArray position = { pos().x(), pos().y() };
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

QVector<QLineF> LaserLine::edges()
{
	Q_D(const LaserLine);
	QVector<QLineF>list;
	QLineF line = sceneTransform().map(d->line);
	list.append(line);
	return list;
}

LaserPrimitive * LaserLine::clone(QTransform t)
{
	Q_D(const LaserLine);
	LaserLine* line = new LaserLine(this->line(), document(), t, d->layerIndex);
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

class LaserPathPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserPath)
public:
    LaserPathPrivate(LaserPath* ptr)
        : LaserShapePrivate(ptr)
    {}

    //QPainterPath path;
};

LaserPath::LaserPath(const QPainterPath & path, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserPathPrivate(this), doc,  LPT_PATH, layerIndex, saveTransform)
{
    Q_D(LaserPath);
    d->path = path;
	d->path = saveTransform.map(d->path);
    d->boundingRect = path.boundingRect();
	d->originalBoundingRect = d->boundingRect;
    d->outline.addPath(path);
}

QPainterPath LaserPath::path() const 
{
    Q_D(const LaserPath);
    return d->path; 
}

void LaserPath::setPath(const QPainterPath& path) 
{
    Q_D(LaserPath);
    d->path = path; 
}

LaserPointListList LaserPath::updateMachiningPoints(quint32 progressCode, qreal progressQuota)
{
    Q_D(LaserPath);
    QPainterPath path = toMachiningPath();

    machiningUtils::path2Points(path, d->machiningPointsList, d->startingIndices, d->machiningCenter);

    /*d->machiningPointsList.clear();
    LaserPointList points;
    machiningUtils::path2Points(path, progressCode, progressQuota, points, d->startingIndices, d->machiningCenter, 2,
        Config::Export::maxStartingPoints(), 0);
    d->machiningPointsList.append(points);*/
    
    return d->machiningPointsList;
}

void LaserPath::draw(QPainter * painter)
{
    Q_D(LaserPath);
    painter->drawPath(d->path);
}

QPainterPath LaserPath::toMachiningPath() const
{
    Q_D(const LaserPath);
    QPainterPath path = d->path;
    path = sceneTransform().map(path);

    QTransform transform = Global::matrixToMachining();
    path = transform.map(path);
    return path;
}

QList<QPainterPath> LaserPath::subPaths() const
{
    QList<QPainterPath> paths;
    QPainterPath path = toMachiningPath();
    QList<QPolygonF> polys = path.toSubpathPolygons();
    //qDebug() << "sub polys count:" << polys.count();
    for (QPolygonF& poly : polys)
    {
        QPainterPath subPath;
        subPath.addPolygon(poly);
        paths.append(subPath);
    }
    return paths;
}

QRectF LaserPath::sceneBoundingRect() const
{
	Q_D(const LaserPath);
	return sceneTransform().map(d->path).boundingRect();
	
}

QJsonObject LaserPath::toJson()
{
    Q_D(const LaserPath);
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
    
    /*for (int pIndex = 0; pIndex < d->poly.size(); pIndex++) {
        QPointF point = d->poly[pIndex];
        QJsonArray pointArray = { point.x(), point.y() };
        poly.append(pointArray);
    }*/
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << d->path;

    /*QJsonArray pathArray;
    for (int i = 0; i < d->path.elementCount(); i++) {
        
        QPainterPath::Element element = d->path.elementAt(i);
        QJsonObject obj;
        obj.insert("x", element.x);
        obj.insert("y", element.y);
        obj.insert("type", element.type);
        pathArray.append(obj);
    }*/
    object.insert("path", QLatin1String(buffer.toBase64()));
    object.insert("name", name());
    object.insert("className", this->metaObject()->className());
    //object.insert("position", position);
    object.insert("matrix", matrix);
    int i = layerIndex();
    object.insert("layerIndex", layerIndex());
    return object;
}

/*void LaserPath::reShape()
{
	Q_D(LaserPath);
	d->path = transform().map(d->path);
	d->boundingRect = d->path.boundingRect();
	d->allTransform = d->allTransform * transform();
	setTransform(QTransform());
}*/

QVector<QLineF> LaserPath::edges()
{
	Q_D(LaserPath);
	return LaserPrimitive::edges(sceneTransform().map(d->path));
}

LaserPrimitive * LaserPath::clone(QTransform t)
{
	Q_D(LaserPath);
	LaserPath* path = new LaserPath(this->path(), document(), t, d->layerIndex);
	return path;
}

bool LaserPath::isClosed() const
{
    Q_D(const LaserPath);
    return utils::fuzzyEquals(d->path.pointAtPercent(0), d->path.pointAtPercent(1));
}

QPointF LaserPath::position() const
{
    Q_D(const LaserPath);
    return sceneTransform().map(d->path.pointAtPercent(0));
}

class LaserPolylinePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserPolyline)
public:
    LaserPolylinePrivate(LaserPolyline* ptr)
        : LaserShapePrivate(ptr)
    {}
	//QPainterPath path;
    QPolygonF poly;
};

LaserPolyline::LaserPolyline(const QPolygonF & poly, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserPolylinePrivate(this), doc, LPT_POLYLINE, layerIndex, saveTransform)
{
    Q_D(LaserPolyline);
    d->poly = poly;
	d->path.addPolygon(d->poly);
	sceneTransformToItemTransform(saveTransform);
    d->boundingRect = d->poly.boundingRect();
	d->originalBoundingRect = d->boundingRect;

    d->outline.moveTo(*d->poly.begin());
    for (int i = 1; i < d->poly.count(); i++)
    {
        d->outline.lineTo(d->poly[i]);
    }
}

QPolygonF LaserPolyline::polyline() const 
{
    Q_D(const LaserPolyline);
    return d->poly; 
}

void LaserPolyline::setPolyline(const QPolygonF& poly) 
{
    Q_D(LaserPolyline);
    d->poly = poly; 
}

QRectF LaserPolyline::sceneBoundingRect() const
{
    Q_D(const LaserPolyline);
    return sceneTransform().map(d->path).boundingRect();
}

LaserPointListList LaserPolyline::updateMachiningPoints(quint32 progressCode, qreal progressQuota)
{
    Q_D(LaserPolyline);
    d->machiningPointsList.clear();
    d->startingIndices.clear();
    bool isClosed = this->isClosed();
    LaserPointList points;
    QTransform t = sceneTransform() * Global::matrixToMachining();
    d->machiningCenter = QPointF(0, 0);
    for (int i = 0; i < d->poly.size(); i++)
    {
        QPointF pt = t.map(d->poly.at(i));

        QPointF cPt = pt;
        QPointF nPt = (i == d->poly.size() - 1) ? pt + (pt - d->poly.at(i - 1)) : d->poly.at(i + 1);
        QPointF lPt = (i == 0) ? pt + (pt - d->poly.at(1)) : d->poly.at(i - 1);
        QLineF line1(cPt, nPt);
        QLineF line2(cPt, lPt);
        qreal angle1 = line1.angle();
        qreal angle2 = line2.angle();
        points.append(LaserPoint(pt.x(), pt.y(), angle1, angle2));
        if (isClosed)
        {
            d->startingIndices.append(i);
        }
        d->machiningCenter += pt;
    }
    if (!isClosed)
    {
        d->startingIndices.append(0);
        d->startingIndices.append(points.size() - 1);
    }
        
    d->machiningCenter /= points.size();
    d->machiningPointsList.append(points);
    return d->machiningPointsList;
}

void LaserPolyline::draw(QPainter * painter)
{
    Q_D(LaserPolyline);
	painter->drawPath(d->path);
}

QPainterPath LaserPolyline::toMachiningPath() const
{
    Q_D(const LaserPolyline);
    QPainterPath path;
    QPolygonF rect = transform().map(d->poly);
    path.addPolygon(rect);

    QTransform transform = Global::matrixToMachining();
    path = transform.map(path);

    return path;
}

/*void LaserPolyline::reShape()
{
	Q_D(LaserPolyline);
	d->poly = transform().map(d->poly);
	//QPainterPath path;
	//path.addPolygon(d->poly);
	d->path = transform().map(d->path);
	//d->boundingRect = path.boundingRect();
	d->boundingRect = d->path.boundingRect();
	d->allTransform = d->allTransform*transform();
	qDebug() << transform();
	qDebug() << sceneTransform();
	QGraphicsItem* parentItem = this->parentItem();
	setTransform(QTransform());
}*/

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

class LaserPolygonPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserPolygon)
public:
    LaserPolygonPrivate(LaserPolygon* ptr)
        : LaserShapePrivate(ptr)
    {}

    QPolygonF poly;
};

LaserPolygon::LaserPolygon(const QPolygonF & poly, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserPolygonPrivate(this), doc, LPT_POLYGON, layerIndex, saveTransform)
{
    Q_D(LaserPolygon);
	d->poly = poly;
	sceneTransformToItemTransform(saveTransform);
    //d->poly = saveTransform.map(poly);
    d->boundingRect = d->poly.boundingRect();
	d->originalBoundingRect = d->boundingRect;
    d->outline.addPolygon(d->poly);
}

QPolygonF LaserPolygon::polyline() const 
{
    Q_D(const LaserPolygon);
    return d->poly; 
}

void LaserPolygon::setPolyline(const QPolygonF& poly) 
{
    Q_D(LaserPolygon);
    d->poly = poly; 
}

LaserPointListList LaserPolygon::updateMachiningPoints(quint32 progressCode, qreal progressQuota)
{
    Q_D(LaserPolygon);
    d->machiningPointsList.clear();
    QTransform t = sceneTransform() * Global::matrixToMachining();
    QPolygonF polygon = t.map(d->poly);
    LaserPointList points;
    machiningUtils::polygon2Points(polygon, points, d->startingIndices, d->machiningCenter);
    d->machiningPointsList.append(points);
    return d->machiningPointsList;
}

void LaserPolygon::draw(QPainter * painter)
{
    Q_D(LaserPolygon);
    painter->drawPolygon(d->poly);
}

QPainterPath LaserPolygon::toMachiningPath() const
{
    Q_D(const LaserPolygon);
    QPainterPath path;
    QPolygonF poly = transform().map(d->poly);
    path.addPolygon(poly);

    QTransform transform = Global::matrixToMachining();
    path = transform.map(path);

    return path;
}

QRectF LaserPolygon::sceneBoundingRect() const
{
	Q_D(const LaserPolygon);
	QPainterPath path;
	path.addPolygon(sceneTransform().map(d->poly));
	return path.boundingRect();
}

/*void LaserPolygon::reShape()
{
	Q_D(LaserPolygon);
	d->poly = transform().map(d->poly);
	QPainterPath path;
	path.addPolygon(d->poly);
	d->boundingRect = path.boundingRect();
	d->allTransform = d->allTransform * transform();
	setTransform(QTransform());
}*/

QJsonObject LaserPolygon::toJson()
{
	Q_D(const LaserPolygon);
	QJsonObject object;
	//QJsonArray position = { pos().x(), pos().y() };
	QTransform transform = QTransform();
	QJsonArray matrix = {
		transform.m11(), transform.m12(), transform.m13(),
		transform.m21(), transform.m22(), transform.m23(),
		transform.m31(), transform.m32(), transform.m33()
	};
	QTransform parentTransform = this->sceneTransform();
	QJsonArray parentMatrix = { parentTransform.m11(), parentTransform.m12(), parentTransform.m13(), parentTransform.m21(), parentTransform.m22(), parentTransform.m23(), parentTransform.m31(), parentTransform.m32(), parentTransform.m33() };
	object.insert("parentMatrix", parentMatrix);
	//polygon
	QJsonArray poly;
	for (int pIndex = 0; pIndex < d->poly.size(); pIndex++) {
		QPointF point = d->poly[pIndex];
		QJsonArray pointArray = { point.x(), point.y() };
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

QVector<QLineF> LaserPolygon::edges()
{
	Q_D(const LaserPolygon);
	QPainterPath path;
	path.addPolygon(sceneTransform().map(d->poly));
	return LaserPrimitive::edges(path);
}

LaserPrimitive * LaserPolygon::clone(QTransform t)
{
	Q_D(const LaserPolygon);
	LaserPolygon* polygon = new LaserPolygon(d->poly, document(), t, d->layerIndex);
	return polygon;
}

bool LaserPolygon::isClosed() const
{
    Q_D(const LaserPolygon);
    return utils::fuzzyEquals(d->poly.first(), d->poly.last());
}

QPointF LaserPolygon::position() const
{
    Q_D(const LaserPolygon);
    return sceneTransform().map(d->poly.first());
}

struct UIPair
{
public:
    UIPair(qreal _uIndex = 0, quint32 _i = 0)
        : uIndex(_uIndex)
        , i(_i)
    {}

    quint32 uIndex;
    quint32 i;
};

struct UIPPair
{
public:
    UIPPair(qreal _uIndex = 0, quint32 _i = 0, quint32 _p = 0)
        : uIndex(_uIndex)
        , i(_i)
        , p(_p)
    {}

    quint32 uIndex;
    quint32 i;
    quint32 p;
};

inline bool operator < (const UIPair& v1, const UIPair& v2)
{
    if (v1.i < v2.i)
        return true;
    else if (v1.i > v2.i)
        return false;
    else
    {
        return v1.uIndex < v2.uIndex;
    }
}

inline bool operator < (const UIPPair& v1, const UIPPair& v2)
{
    if (v1.i < v2.i)
        return true;
    else if (v1.i > v2.i)
        return false;
    else
    {
        if (v1.p < v2.p)
            return true;
        else if (v1.p > v2.p)
            return false;
        else
        {
            return v1.uIndex < v2.uIndex;
        }
    }
}

class LaserNurbsPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserNurbs)
public:
    LaserNurbsPrivate(LaserNurbs* ptr, LaserNurbs::BasisType _basisType)
        : LaserShapePrivate(ptr)
        , basisType(_basisType)
        , steps(100)
    {}

    void updateBezierCoefficient()
    {
        coefficients.clear();
        quint32 n = controlPoints.count();
        QMap<quint32, quint32> iMap;
        quint32 nFactorial = utils::factorial(n);
        for (int i = 0; i <= n; i++)
        {
            iMap[i] = utils::factorial(i);
        }

        qreal u = 0;

        for (int iu = 0; iu < steps; iu++)
        {
            qreal u = iu * 1.0 / steps;
            for (int i = 0; i <= n; i++)
            {
                qreal coefficient = nFactorial / (iMap[i] * iMap[n - i]) * qPow(u, i) * qPow(1 - u, n - i);
                coefficients[UIPair(iu, i)] = coefficient;
            }
        }
    }

    void updateBSplineCoefficient()
    {
        coefficients.clear();

        for (int i = 0; i < knots.count(); i++)
        {
            knots[i] = knots[i] / knots.last();
            //knots[i] = 1.0 / (knots.count() - 1) * i;
            qLogD << "knot " << i << " = " << knots[i];
        }

        // m: count of knots minus 1
        // n: count of control points minus 1
        // p: degree of curve
        // m, n, p must saticsfy m = n + p + 1
        // so p = m - n -1
        quint32 m = knots.count() - 1;
        quint32 n = controlPoints.count() - 1; 
        quint32 p = m - n - 1;

        QMap<UIPPair, qreal> nCoefs;

        for (int uIndex = 0; uIndex <= steps; uIndex++)
        {
            qreal u = uIndex * 1.0 / steps;
            QString logU;
            for (int ip = 0; ip <= p; ip++)
            {
                logU.append("\n");
                for (int i = 0; i <= m - 1 - ip; i++)
                {
                    UIPPair key(uIndex, i, ip);
                    if (ip == 0)
                    {
                        qreal uBegin = knots[i];
                        qreal uEnd = knots[i + 1];
                        qreal n = (u >= uBegin && u < uEnd) ? 1 : 0;
                        nCoefs[key] = n;
                        //QString log = QString("N[%1,%2](%3) = %4\n").arg(i).arg(ip).arg(u, 4, 'f', 2).arg(n);
                        //logU.append(log);
                        continue;
                    }

                    UIPPair key1(uIndex, i, ip - 1);
                    UIPPair key2(uIndex, i + 1, ip - 1);

                    qreal exp1 = 0;
                    qreal exp2 = 0;

                    qreal N1 = nCoefs[key1];
                    qreal N2 = nCoefs[key2];
                    qreal u_ip = knots[i + ip];
                    qreal u_ip1 = knots[i + ip + 1];
                    qreal u_i1 = knots[i + 1];
                    qreal u_i = knots[i];

                    qreal u_minus_ui = u - u_i;
                    qreal u_ip_minus_u_i = u_ip - u_i;
                    qreal u_ip1_minus_u = u_ip1 - u;
                    qreal u_ip1_minus_u_i1 = u_ip1 - u_i1;

                    qreal c1 = (u_ip_minus_u_i) ? (u_minus_ui / u_ip_minus_u_i) : 0;
                    exp1 = N1 * c1;
                    qreal c2 = (u_ip1_minus_u_i1) ? (u_ip1_minus_u / u_ip1_minus_u_i1) : 0;
                    exp2 = N2 * c2;

                    qreal N = exp1 + exp2;
                    nCoefs[key] = N;
                    /*QString log = QString(
                        "N[%1,%2] = %3, N[%4,%5] = %6, N[%7,%8] = %9, "
                        "i = %10, i+1 = %11, i+p = %12, i+p+1 = %13, "
                        "u = %14, u(i) = %15, u(i+1) = %16, u(i+p) = %17, u(i+p+1) = %18, "
                        "u-u(i) = %23, u(i+p)-u(i) = %24, u(i+p+1)-u = %25, u(i+p+1)-u(i+1) = %26, "
                        "c1 = %19, c2 = %20, exp1 = %21, exp2 = %22\n")
                        .arg(i).arg(ip).arg(N, 9, 'f', 6)
                        .arg(i).arg(ip - 1).arg(N1, 9, 'f', 6)
                        .arg(i + 1).arg(ip - 1).arg(N2, 9, 'f', 6)
                        .arg(i).arg(i + 1).arg(i + ip).arg(i + ip + 1)
                        .arg(u, 4, 'f', 2).arg(u_i, 4, 'f', 2).arg(u_i1, 4, 'f', 2).arg(u_ip, 4, 'f', 2).arg(u_ip1, 4, 'f', 2)
                        .arg(c1, 9, 'f', 6).arg(c2, 9, 'f', 6).arg(exp1, 9, 'f', 6).arg(exp2, 9, 'f', 6)
                        .arg(u_minus_ui, 5, 'f', 2).arg(u_ip_minus_u_i, 5, 'f', 2).arg(u_ip1_minus_u, 5, 'f', 2).arg(u_ip1_minus_u_i1, 5, 'f', 2);
                    logU.append(log);*/
                    
                    if (ip == p)
                    {
                        coefficients[UIPair(uIndex, i)] = N;
                    }
                }
            }
            //if (uIndex == 100)
                //qLogD << logU;
        }
    }

    void updateDrawingPoints()
    {
        drawingPoints.clear();
        drawingPath = QPainterPath();
        for (int uIndex = 0; uIndex <= steps; uIndex++)
        {
            qreal u = uIndex * 1.0 / steps;
            QPointF point(0, 0);
            if (uIndex == steps)
            {
                point = controlPoints.last();
            }
            else
            {
                qreal sumCoe = 0;
                for (int i = 0; i < controlPoints.count(); i++)
                {
                    UIPair key(uIndex, i);
                    qreal coe = coefficients.contains(key) ? coefficients[key] : 0;
                    sumCoe += coe * weights[i];
                }
                for (int i = 0; i < controlPoints.count(); i++)
                {
                    UIPair key(uIndex, i);
                    qreal coe = coefficients.contains(key) ? coefficients[key] : 0;
                    QPointF pi = controlPoints[i];
                    point += pi * coe * weights[i] / sumCoe;
                }
                qLogD << u << ": " << point << ", coe = " << sumCoe;
            }
            drawingPoints.append(point);
            if (drawingPath.elementCount() == 0)
                drawingPath.moveTo(point);
            else
                drawingPath.lineTo(point);
        }

        boundingRect = drawingPath.boundingRect();
        qLogD << "size of drawingPath elements: " << drawingPath.elementCount();
        qLogD << "bounding rect of drawingPath: " << boundingRect;
    }

    LaserNurbs::BasisType basisType;
    QList<QPointF> controlPoints;
    QList<qreal> knots;
    QList<qreal> weights;

    QMap<UIPair, qreal> coefficients;
    QList<QPointF> drawingPoints;
    QPainterPath drawingPath;
    int steps;
};

LaserNurbs::LaserNurbs(const QList<QPointF> controlPoints, const QList<qreal> knots, const QList<qreal> weights, 
	BasisType basisType, LaserDocument* doc, QTransform transform, int layerIndex)
    : LaserShape(new LaserNurbsPrivate(this, basisType), doc, LPT_NURBS, layerIndex, transform)
{
    Q_D(LaserNurbs);
    d->controlPoints = controlPoints;
    d->knots = knots;
    d->weights = weights;
    d->originalBoundingRect = d->boundingRect;
	utils::sceneTransformToItemTransform(transform, this);
    updateCurve();
}

void LaserNurbs::draw(QPainter* painter)
{
    Q_D(LaserNurbs);
    painter->drawPath(d->drawingPath);
    painter->setPen(QPen(Qt::red));
    QPolygonF polygon;
    for (int i = 0; i < d->controlPoints.count(); i++)
    {
        painter->drawEllipse(d->controlPoints[i], 5, 5);
        polygon.append(d->controlPoints[i]);
    }
    
    painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter->drawPolyline(polygon);
}

QPainterPath LaserNurbs::toMachiningPath() const
{
    Q_D(const LaserNurbs);
    return d->drawingPath;
}

QRectF LaserNurbs::sceneBoundingRect() const
{
	Q_D(const LaserNurbs);
	return d->drawingPath.boundingRect();
}

void LaserNurbs::updateCurve()
{
    Q_D(LaserNurbs);
    switch (d->basisType)
    {
    case BT_BEZIER:
        d->updateBezierCoefficient();
        break;
    case BT_BSPLINE:
        d->updateBSplineCoefficient();
        break;
    }
    d->updateDrawingPoints();
}

LaserPrimitive * LaserNurbs::clone(QTransform t)
{
	Q_D(LaserNurbs);
	LaserNurbs* nurbs = new LaserNurbs(d->controlPoints, d->knots, d->weights, d->basisType, document(), t, d->layerIndex);
	return nurbs;
}

bool LaserNurbs::isClosed() const
{
    Q_D(const LaserNurbs);
    return utils::fuzzyEquals(d->controlPoints.first(), d->controlPoints.last());
}

QPointF LaserNurbs::position() const
{
    Q_D(const LaserNurbs);
    return sceneTransform().map(d->controlPoints.first());
}

class LaserBitmapPrivate : public LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserBitmap)
public:
    LaserBitmapPrivate(LaserBitmap* ptr)
        : LaserPrimitivePrivate(ptr)
    {}

    QImage image;
    //QRectF bounds;
};

LaserBitmap::LaserBitmap(const QImage & image, const QRectF& bounds, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserPrimitive(new LaserBitmapPrivate(this), doc,  LPT_BITMAP, saveTransform, layerIndex)
{
    Q_D(LaserBitmap);
    d->image = image.convertToFormat(QImage::Format_Grayscale8);
    d->boundingRect = bounds;
    d->primitiveType = LPT_BITMAP;
    d->outline.addRect(bounds);
	d->originalBoundingRect = d->boundingRect;
	sceneTransformToItemTransform(saveTransform);

	setFlags(ItemIsSelectable | ItemIsMovable);
	installEventFilter(doc->scene());
	//this->setAcceptedMouseButtons(Qt::LeftButton);
	//setTransform(saveTransform);
}

QImage LaserBitmap::image() const 
{
    Q_D(const LaserBitmap);
    return d->image; 
}



void LaserBitmap::setImage(const QImage& image)
{
    Q_D(LaserBitmap);
    d->image = image;
}

QRectF LaserBitmap::bounds() const 
{
    Q_D(const LaserBitmap);
    return d->boundingRect; 
}

QByteArray LaserBitmap::engravingImage()
{ 
    Q_D(LaserBitmap);
    QByteArray ba;

    QImage srcImage = d->image.copy();
    srcImage.invertPixels();
    QImage outImage = srcImage.transformed(sceneTransform()).convertToFormat(QImage::Format_Grayscale8);
    outImage.invertPixels();
    QRectF boundingRect = sceneBoundingRect();
    outImage.save("tmp\\outImage.png");
    cv::Mat src(outImage.height(), outImage.width(), CV_8UC1, (void*)outImage.constBits(), outImage.bytesPerLine());

    qreal pixelInterval = 0.07;

	qreal boundingWidth = Global::convertToMM(SU_PX, boundingRect.width());
	qreal boundingHeight = Global::convertToMM(SU_PX, boundingRect.height(), Qt::Vertical);
    qreal boundingLeft = Global::convertToMM(SU_PX, boundingRect.left());
    qreal boundingTop = Global::convertToMM(SU_PX, boundingRect.top());
    int dpi = d->layer->dpi();
    int outWidth = boundingWidth * MM_TO_INCH * dpi;
    int outHeight = std::round(boundingHeight / pixelInterval);
    qLogD << "bounding rect: " << boundingRect;
    qDebug() << "out width:" << outWidth;
    qDebug() << "out height:" << outHeight;
    qDebug() << "out left:" << boundingLeft;
    qDebug() << "out top:" << boundingTop;

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(outWidth, outHeight));
    
    cv::Mat outMat = resized;
    if (layer()->useHalftone())
    {
        //outMat = imageUtils::halftone3(resized, layer()->lpi(), layer()->dpi(), 45);
        //outMat = imageUtils::halftone4(resized, 30, 12);
        outMat = imageUtils::halftone5(resized, this->layer()->halftoneAngles(), this->layer()->halftoneGridSize());
    }

    ba = imageUtils::image2EngravingData(outMat, boundingLeft, boundingTop, pixelInterval, boundingWidth);

    return ba; 
}

void LaserBitmap::draw(QPainter * painter)
{
    Q_D(LaserBitmap);
	
	//QImage image = d->image.transformed(d->allTransform, Qt::TransformationMode::SmoothTransformation);
	painter->drawImage(d->boundingRect, d->image);
}

QPainterPath LaserBitmap::toMachiningPath() const
{
    Q_D(const LaserBitmap);
    QPainterPath path;
    QPolygonF rect = sceneTransform().map(d->boundingRect);
    path.addPolygon(rect);

    QTransform transform = Global::matrixToMachining();
    path = transform.map(path);

    return path;
}

QJsonObject LaserBitmap::toJson()
{
	Q_D(const LaserBitmap);
	QJsonObject object;
	//QJsonArray position = { pos().x(), pos().y() };
	QTransform transform = QTransform();
	QJsonArray matrix = {
		transform.m11(), transform.m12(), transform.m13(),
		transform.m21(), transform.m22(), transform.m23(),
		transform.m31(), transform.m32(), transform.m33()
	};
	
	QTransform parentTransform = this->sceneTransform();
	QJsonArray parentMatrix = { parentTransform.m11(), parentTransform.m12(), parentTransform.m13(), parentTransform.m21(), parentTransform.m22(), parentTransform.m23(), parentTransform.m31(), parentTransform.m32(), parentTransform.m33() };
	object.insert("parentMatrix", parentMatrix);
	//rect
	QJsonArray bounds = { d->boundingRect.x(), d->boundingRect.y(),d->boundingRect.width(), d->boundingRect.height() };
	QJsonArray();
	//image
	//QByteArray imageBits(d->image.byteCount(), (char)0);
	QByteArray imageBits;
	QBuffer buffer(&imageBits);
	buffer.open(QIODevice::ReadWrite);
	d->image.save(&buffer, "tiff");
	buffer.close();

	object.insert("name", name());
	object.insert("className", this->metaObject()->className());
	//object.insert("position", position);
	object.insert("matrix", matrix);
	object.insert("bounds", bounds);
	object.insert("image", QLatin1String(imageBits.toBase64()));
	object.insert("layerIndex", layerIndex());
	return object;
}

LaserPointListList LaserBitmap::updateMachiningPoints(quint32 progressCode, qreal progressQuota)
{
    Q_D(LaserBitmap);
    d->machiningPointsList.clear();
    d->startingIndices.clear();

	//QTransform t = sceneTransform() * Global::matrixToMachining();
    QPolygonF poly = this->toMachiningPath().toFillPolygon();
    QPointF pt1 = poly.at(0);
    QPointF pt2 = poly.at(1);
    QPointF pt3 = poly.at(2);
    QPointF pt4 = poly.at(3);

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
    points.push_back(LaserPoint(pt1.x(), pt1.y(), angle11, angle12));
    points.push_back(LaserPoint(pt2.x(), pt2.y(), angle21, angle22));
    points.push_back(LaserPoint(pt3.x(), pt3.y(), angle31, angle32));
    points.push_back(LaserPoint(pt4.x(), pt4.y(), angle41, angle42));
    d->machiningCenter = utils::center(points).toPointF();
    points.push_back(points.first());
    d->startingIndices.append(0);
    d->startingIndices.append(1);
    d->startingIndices.append(2);
    d->startingIndices.append(3);
    d->machiningPointsList.append(points);

    return d->machiningPointsList;
}

QRectF LaserBitmap::sceneBoundingRect() const
{
	Q_D(const LaserBitmap);
	QPainterPath path;
	path.addRect(d->boundingRect);
	return sceneTransform().map(path).boundingRect();
}

void LaserBitmap::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mousePressEvent(event);
	//QList<QGraphicsView*> views = scene()->views();
	//views[0]->viewport()->repaint();
	//event->accept();
	//event->ignore();
}

void LaserBitmap::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mouseMoveEvent(event);
	//QList<QGraphicsView*> views = scene()->views();
	//views[0]->viewport()->repaint();
	//event->accept();
}

void LaserBitmap::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mouseReleaseEvent(event);
	QList<QGraphicsView*> views = scene()->views();
	LaserViewer* viewer = qobject_cast<LaserViewer*> (views[0]);
	viewer->onEndSelecting();
}

QVector<QLineF> LaserBitmap::edges()
{
	Q_D(const LaserBitmap);
	QPainterPath path;
	path.addPolygon(sceneTransform().map(d->boundingRect));
	return LaserPrimitive::edges(path);
}

LaserPrimitive * LaserBitmap::clone(QTransform t)
{
	Q_D(LaserBitmap);
	LaserBitmap* bitmap = new LaserBitmap(d->image, d->boundingRect, document(), t, d->layerIndex);
	return bitmap;
}

bool LaserBitmap::isClosed() const
{
    return true;
}

QPointF LaserBitmap::position() const
{
    return sceneBoundingRect().topLeft();
}

QDebug operator<<(QDebug debug, const LaserPrimitive & item)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "[name = " << item.name() << ", type = " << item.primitiveType() << "]";
    return debug;
}

QString FinishRun::toString()
{
    QString text = QObject::tr("Relays: ");
    QStringList nos;
    for (int i = 0; i < 8; i++)
    {
        if (isEnabled(i))
        {
            nos.append(QString::number(i + 1));
        }
    }
    text.append(nos.join(","));
    text.append(" Action: ");

    QString actionStr;
    switch (action)
    {
    case RA_NONE:
        actionStr = QObject::tr("None");
        break;
    case RA_RELEASE:
        actionStr = QObject::tr("Release");
        break;
    case RA_ORIGIN:
        actionStr = QObject::tr("Machining 1");
        break;
    case RA_MACHINING_1:
        actionStr = QObject::tr("Machining 2");
        break;
    case RA_MACHINING_2:
        actionStr = QObject::tr("Machining 3");
        break;
    case RA_MACHINING_3:
        actionStr = QObject::tr("None");
        break;
    }

    text.append(actionStr);
    return text;
}

QByteArray LaserShape::engravingImage()
{
    QByteArray bytes;
    QPainterPath path = toMachiningPath();
    QRectF boundRect = path.boundingRect();

    int scanInterval = 7;
    double yPulseLength = 0.006329114;
    
    qreal pixelInterval = scanInterval * yPulseLength;

    QList<SliceGroup> groups;

    for (qreal y = boundRect.top(); y <= boundRect.bottom(); y += pixelInterval)
    {
        QLineF hLine(boundRect.left() - 0.01f, y, boundRect.right() + 0.01f, y + 1);
        QRectF hRect(hLine.p1(), hLine.p2());
        QPainterPath linePath;
        linePath.addRect(hRect);
        QPainterPath intersection = path.intersected(linePath);
        for (int i = 0; i < intersection.elementCount(); i++)
        {
            qDebug() << i << intersection.elementAt(i);
        }
    }
    return bytes;
}

int LaserShape::layerIndex()
{
	Q_D(LaserShape);
	return d->layerIndex;
}

class LaserTextPrivate : public LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserText)
public:
    LaserTextPrivate(LaserText* ptr)
        : LaserPrimitivePrivate(ptr)
    {}
	QRect rect;
    QString content;
    QString lastContent;
    QPointF startPos;
    //QList<QPainterPath> pathList;
    //QMap<QPointF, QList<QPainterPath>> pathList;
    QList<LaserTextRowPath> pathList;
    QPainterPath allPath;
    QFont font;
    int alignHType;
    int lastAlignHType;
    int alignVType;
    int lastAlignVType;
    QGraphicsView* view;
};

LaserText::LaserText(LaserDocument* doc, QPointF startPos, QFont font, int alighHType, int alighVType, QTransform saveTransform, int layerIndex)
	: LaserPrimitive(new LaserTextPrivate(this),  doc,  LPT_TEXT, saveTransform, layerIndex)
{
    Q_D(LaserText);
    //d->boundingRect = d->path.boundingRect();
    //d->rect = d->boundingRect.toRect();
    d->outline.addRect(d->rect);
    //d->baseSpace = Global::mm2PixelsXF(2.0);
    //d->textSpace = d->baseSpace + space;;
    d->startPos = startPos;
    d->font = font;
    d->alignHType = alighHType;
    d->lastAlignHType = alighHType;
    d->alignVType = alighVType;
    d->lastAlignVType = alighVType;
    //d->position = d->rect.center();
    d->startPos = mapFromScene(d->startPos);
    d->view = doc->scene()->views()[0];
    d->allTransform = saveTransform;
    sceneTransformToItemTransform(saveTransform);
}

LaserText::~LaserText()
{
}

QRect LaserText::rect() const
{
    Q_D(const LaserText);
    return d->rect; 
}

QString LaserText::content() const 
{
    Q_D(const LaserText);
    return d->content; 
}

void LaserText::setContent(QString c)
{
    Q_D(LaserText);
    d->content = c;
}

QPainterPath LaserText::path() const
{
    Q_D(const LaserText);
    /*QPainterPath paths;

    for (QMap<QPointF, QList<QPainterPath>>::const_iterator i = d->pathList.begin(); i != d->pathList.end(); i++) {
        QPainterPath rowPath;
        QPointF startPos = i.key();
        QList<QPainterPath> subRowPathList = i.value();
        for (QPainterPath path : subRowPathList) {
            rowPath.addPath(path);
            
        }
        
        paths.addPath(rowPath);
    }*/
    
    return d->allPath;
}

QVector<QLineF> LaserText::edges()
{
    Q_D(const LaserText);
    return  LaserPrimitive::edges(sceneTransform().map(path()));
}

void LaserText::setFont(QFont font)
{
    Q_D(LaserText);
    d->font = font;
    modifyPathList();

}

QFont LaserText::font()
{
    Q_D(const LaserText);
    return d->font;
}

void LaserText::setAlignH(int a)
{
    Q_D(LaserText);
    d->alignHType = a;

}

int LaserText::alignH()
{
    Q_D(LaserText);
    return d->alignHType;
}

void LaserText::setAlignV(int a)
{
    Q_D(LaserText);
    d->alignVType = a;
}

int LaserText::alignV()
{
    Q_D(LaserText);
    return d->alignVType;
}

QPointF LaserText::startPos()
{
    Q_D(LaserText);
    return d->startPos;
}

void LaserText::setSaveTransform(QTransform t)
{

}

QTransform LaserText::saveTransform()
{
    return QTransform();
}

/*void LaserText::setAlignType(int type)
{
    Q_D(LaserText);
    d->alignType = type;
}

int LaserText::alignType()
{
    Q_D(LaserText);
    return d->alignType;
}*/

void LaserText::insertContent(QString str, int index)
{
    Q_D(LaserText);
    d->lastContent = d->content;
    d->content.insert(index, str);
}

void LaserText::addPath(QString content, int insertIndex)
{
    Q_D(LaserText);
    insertContent(content, insertIndex);
    modifyPathList();
    d->view->viewport()->repaint();
}

void LaserText::delPath(int index)
{
    Q_D(LaserText);
    d->lastContent = d->content;
    d->content.remove(index, 1);
    modifyPathList();
    d->view->viewport()->repaint();
}

void LaserText::modifyPathList()
{
    Q_D(LaserText);
    QList<QList<QPainterPath>> subRowPathList;
    QList<QList<QRectF>> subBoundList;
    QList<QPainterPath> rowPathList;
    QList<QPointF> startPosList;

    QList<QPainterPath>* listPtr = &QList<QPainterPath>();
    QList<QRectF>* boundListPtr = &QList<QRectF>();

    QPainterPath rowPath;
    QPainterPath* rowPathPtr = &rowPath;

    qreal fontSize = d->font.pixelSize();
    qreal letterSpacing = d->font.letterSpacing();
    qreal wordSpacing = d->font.wordSpacing();
    
    bool isNewLine = true;
    QRectF lastBound;
    //QPainterPath allPath;
    //QPainterPath lastPath;
    qDebug() << d->lastContent;
    qDebug() << d->content;
    //for (QChar c : d->content) {
    
    for (int i = 0; i < d->content.size(); i++) {
        QChar c = d->content[i];
        qreal rowY = (subRowPathList.size() + 1) * fontSize + subRowPathList.size()* wordSpacing + d->startPos.y();
        QPointF startP(d->startPos.x(), rowY);
        if (c == "\n") {
            subRowPathList.append(*listPtr);
            rowPathList.append(*rowPathPtr);
            subBoundList.append(*boundListPtr);
            startPosList.append(startP);
            rowY = (subRowPathList.size() + 1) * fontSize + subRowPathList.size()* wordSpacing + d->startPos.y();
            startP .setY(rowY);
            //换行
            isNewLine = true;
            listPtr = &QList<QPainterPath>();
            rowPathPtr = &QPainterPath();
            boundListPtr = &QList<QRectF>();
        }
        else {
            QPainterPath path;
            QTransform pathT;
            QRectF bound;
            QPointF pos;
            //top left
            if (isNewLine) {
                pos = startP;
            }
            else {
                pos = QPointF(lastBound.right() + d->font.letterSpacing(), rowY);
            }
            if (c == " ") {
                QFontMetrics m(d->font);
                qreal width = m.averageCharWidth();
                bound = QRectF(pos.x(), pos.y(), width, d->font.pixelSize());
            }
            else {
                path.addText(pos, d->font, c);
                bound = path.boundingRect();
            }
            
            listPtr->append(path);
            rowPathPtr->addPath(path);
            boundListPtr->append(bound);
            lastBound = bound;
            isNewLine = false;
        }
        if (i == d->content.size() - 1) {
            subRowPathList.append(*listPtr);
            rowPathList.append(*rowPathPtr);
            subBoundList.append(*boundListPtr);
            startPosList.append(startP);
        }
        //allPath.addPath(path);
        
        
    }
    
    qreal allHeight = fontSize * subRowPathList.size() + (fontSize * subRowPathList.size()-1)* wordSpacing;
    d->pathList.clear();
    d->allPath = QPainterPath();
    for (int j = 0; j < subRowPathList.size(); j++) {
        QList<QPainterPath> subRowPath = subRowPathList[j];
        QList<QRectF> subRowBound = subBoundList[j];
        QPainterPath rowPath = rowPathList[j];
        QPointF startLeftTop = startPosList[j];
        qreal rowWidth = rowPath.boundingRect().width();
        QPointF diff;
        
        switch (d->alignHType) {
        case Qt::AlignLeft: {
            switch (d->alignVType) {
                case Qt::AlignTop: {
                    diff = QPointF(0, 0);
                    break;
                }
                case Qt::AlignBottom: {
                    diff = QPointF(0, -allHeight);
                    break;
                }
                case Qt::AlignVCenter: {
                    diff = QPointF(0, -allHeight*0.5);
                    break;
                }
            }
            break;
        }
        case Qt::AlignRight: {
            switch (d->alignVType) {
                case Qt::AlignTop: {
                    diff = QPointF(-rowWidth, 0);
                    break;
                }
                case Qt::AlignBottom: {
                    diff = QPointF(-rowWidth, -allHeight);
                    break;
                }
                case Qt::AlignVCenter: {
                    diff = QPointF(-rowWidth, -allHeight*0.5);
                    break;
                }   
            }
            break;
        }
        case Qt::AlignHCenter: {
            switch (d->alignVType) {
                case Qt::AlignTop: {
                    diff = QPointF(-rowWidth*0.5, 0);
                    break;
                }
                case Qt::AlignBottom: {
                    diff = QPointF(-rowWidth * 0.5, -allHeight);
                    break;
                }
                case Qt::AlignVCenter: {
                    diff = QPointF(-rowWidth * 0.5, -allHeight*0.5);
                    break;
                }
            }
            break;
        }
        }
        
        LaserTextRowPath rowPathStruct;
        QPainterPath newRowPath;
        QList<QPainterPath> newRowSubPaths;
        QList<QRectF> newRowBounds;
        QPointF rowLeftTop = startLeftTop + diff;
        for (int subIndex = 0; subIndex < subRowPath.size(); subIndex++ ) {
            QPainterPath subPath = subRowPath[subIndex];
            QRectF subBound = subRowBound[subIndex];

            qDebug() << diff;
            subPath.translate(diff.x(), diff.y());
            subBound.translate(diff.x(), diff.y());

            newRowPath.addPath(subPath);
            newRowSubPaths.append(subPath);
            newRowBounds.append(subBound);
        }
        rowPathStruct.setPath(newRowPath);
        rowPathStruct.setLeftTop(rowLeftTop);
        rowPathStruct.setSubRowPathlist(newRowSubPaths);
        rowPathStruct.setSubRowBoundlist(newRowBounds);
        d->pathList.append(rowPathStruct);
        d->allPath.addPath(newRowPath);
    }
    
}

QList<LaserTextRowPath> LaserText::subPathList()
{
    Q_D(const LaserText);

    return d->pathList;
}

QRectF LaserText::boundingRect() const
{
    Q_D(const LaserText);
    //d->boundingRect = d->path.boundingRect();
    return path().boundingRect();
}

QRectF LaserText::sceneBoundingRect() const
{
    Q_D(const LaserText);
    return sceneTransform().map(path()).boundingRect();
}

QRectF LaserText::originalBoundingRect(qreal extendPixel) const
{
    
    Q_D(const LaserPrimitive);
    qreal x = boundingRect().topLeft().x() - extendPixel;
    qreal y = boundingRect().topLeft().y() - extendPixel;
    qreal width = boundingRect().width() + 2 * extendPixel;
    qreal height = boundingRect().height() + 2 * extendPixel;
    QRectF rect = QRectF(x, y, width, height);
    return rect;
}

void LaserText::draw(QPainter * painter)
{
    Q_D(LaserText);
    painter->drawPath(mapFromScene(sceneTransform().map(path())));
    //painter->drawPolygon(mapFromScene(sceneTransform().map(originalBoundingRect(Global::mm2PixelsYF(10.0)))));
    
    /*for (int i = 0; i < d->pathList.size(); i++) {
        painter->setPen(QPen(Qt::green));
        QColor c(0, 0, 0, 0);
        painter->setPen(QPen(c));
        QList<QPainterPath> rowPathList = d->pathList[i].subRowPathlist();
        painter->drawPolygon(mapFromScene(sceneTransform().map(d->pathList[i].path().boundingRect())));
        for (QPainterPath path : rowPathList) {
            painter->setPen(QPen(Qt::red));
            //painter->drawPath(mapFromScene(sceneTransform().map(path)));
            painter->drawPolygon(mapFromScene(sceneTransform().map(path.boundingRect())));

        }
    }*/
}

LaserPrimitive * LaserText::clone(QTransform t)
{
	Q_D(LaserText);
	LaserText* text = new LaserText(document(), d->startPos, d->font, d->alignHType, d->alignVType, sceneTransform(), d->layerIndex);
    text->setContent(d->content);
    text->modifyPathList();
	return text;
}

QJsonObject LaserText::toJson()
{
    Q_D(const LaserText);
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
    object.insert("name", name());
    object.insert("className", this->metaObject()->className());
    object.insert("matrix", matrix);
    object.insert("layerIndex", layerIndex());
    //content
    object.insert("content", d->content);
    QJsonArray startPosArray{ d->startPos.x(), d->startPos.y() };
    object.insert("startPos", startPosArray);
    //font
    QJsonObject font;
    font.insert("family", d->font.family());
    font.insert("size", d->font.pixelSize());
    font.insert("bold", d->font.bold());
    font.insert("italic", d->font.italic());
    font.insert("upper", d->font.capitalization());
    font.insert("spaceX", d->font.letterSpacing());
    font.insert("letterSpaceTpye", d->font.letterSpacingType());
    font.insert("spaceY", d->font.wordSpacing());
    font.insert("alignH", d->alignHType);
    font.insert("alignV", d->alignVType);
    object.insert("font", font);
    return object;
}

bool LaserText::isClosed() const
{
    return true;
}

QPointF LaserText::position() const
{
    return QPointF();
}

QPainterPath LaserText::toMachiningPath() const
{
    Q_D(const LaserText);
    QPainterPath path = d->allPath;
    path = sceneTransform().map(path);

    QTransform transform = Global::matrixToMachining();
    path = transform.map(path);
    return path;
}

LaserPointListList LaserText::updateMachiningPoints(quint32 progressCode, qreal progressQuota)
{
    Q_D(LaserText);

    QTransform transform = Global::matrixToMachining();

    d->machiningPointsList.clear();
    d->startingIndices.clear();
    int totalPoints = 0;
    for (int i = 0; i < d->pathList.size(); i++) {
        QList<QPainterPath> rowPathList = d->pathList[i].subRowPathlist();
        for (QPainterPath rowPath : rowPathList) {
            rowPath = sceneTransform().map(rowPath);

            QList<int> indices;
            LaserPointListList pointsList;
            machiningUtils::path2Points(rowPath, pointsList, indices, QPointF(), transform);

            if (indices.length() <= Config::PathOptimization::maxStartingPoints())
            {
                for (int index : indices)
                {
                    d->startingIndices.append(index + totalPoints);
                }
            }
            else
            {
                for (int i = 0; i < Config::PathOptimization::maxStartingPoints(); i++)
                {
                    int index = i * indices.length() / Config::PathOptimization::maxStartingPoints();
                    d->startingIndices.append(indices.at(index) + totalPoints);
                }
            }
            
            d->machiningPointsList.append(pointsList);
            for (LaserPointList& list : pointsList)
            {
                totalPoints += list.count();
            }
        }
    }
    
    d->machiningCenter = transform.mapRect(sceneBoundingRect()).center();
    
    return d->machiningPointsList;
}

