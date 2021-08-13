#include "LaserPrimitive.h"

#include <iostream>

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
#include "util/ImageUtils.h"
#include "util/PltUtils.h"
#include "util/TypeUtils.h"
#include "util/UnitUtils.h"
#include "util/Utils.h"
#include "widget/LaserViewer.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserNodePrivate.h"
#include "scene/LaserPrimitiveGroup.h"

QMap<LaserPrimitiveType, int> g_counter{
    { LPT_LINE, 0 },
    { LPT_CIRCLE, 0},
    { LPT_ELLIPSE, 0},
    { LPT_RECT, 0},
    { LPT_POLYLINE, 0},
    { LPT_POLYGON, 0},
    { LPT_PATH, 0},
    { LPT_BITMAP, 0},
    { LPT_TEXT, 0},
};

class LaserPrimitivePrivate: public LaserNodePrivate
{
    Q_DECLARE_PUBLIC(LaserPrimitive)
public:
    LaserPrimitivePrivate(LaserPrimitive* ptr)
        : LaserNodePrivate(ptr)
        , doc(nullptr)
        , layer(nullptr)
        , isHover(false)
        , type(LPT_UNKNOWN)
    {}

    LaserDocument* doc;
    LaserLayer* layer;
    QRectF boundingRect;
    LaserPrimitiveType type;
    bool isHover;
    QPainterPath outline;
    QVector<QPointF> updateMachiningPoints;
    QList<int> startingIndices;
	QTransform allTransform;
	QRectF originalBoundingRect;
	QPainterPath path;//��Ϊ�����ͼ��
};

LaserPrimitive::LaserPrimitive(LaserPrimitivePrivate* data, LaserDocument* doc, LaserPrimitiveType type, QTransform saveTransform)
    : LaserNode(data, LNT_PRIMITIVE)
{
    Q_D(LaserPrimitive);
    d->doc = doc;
    d->type = type;
    Q_ASSERT(doc);
    QObject::setParent(doc);

    g_counter[type]++;

    //this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    //this->setFlag(ItemIsFocusable, true);
    //this->setAcceptHoverEvents(true);
    d->nodeName = QString("%1_%2").arg(typeLatinName(type)).arg(g_counter[type]);
	d->allTransform = saveTransform;
}

LaserPrimitive::~LaserPrimitive()
{
    Q_D(LaserPrimitive);
    qDebug() << d->nodeName;
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
    //painter->setPen(QPen(Qt::green, 1, Qt::SolidLine));
    //painter->drawText(startPos, name());
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

QRectF LaserPrimitive::originalBoundingRect() const
{
	Q_D(const LaserPrimitive);
	return d->originalBoundingRect;
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

QPointF LaserPrimitive::laserStartPos() const
{
    QPointF pos = boundingRect().topLeft();
    pos *= 40;
    return pos;
}

void LaserPrimitive::sceneTransformToItemTransform(QTransform sceneTransform)
{
	
	QTransform t(
		sceneTransform.m11(), sceneTransform.m12(), sceneTransform.m13(),
		sceneTransform.m21(), sceneTransform.m22(), sceneTransform.m23(),
		0, 0, sceneTransform.m33());
	setTransform(t);
	setPos(sceneTransform.m31(), sceneTransform.m32());
}

QVector<QPointF> LaserPrimitive::machiningPoints() const
{
    Q_D(const LaserPrimitive);
    return d->updateMachiningPoints;
}

QVector<QPointF> LaserPrimitive::machiningPoints(QPointF& lastPoint, int pointIndex, cv::Mat& canvas) const
{
    Q_D(const LaserPrimitive);
    int pointsCount = d->updateMachiningPoints.size();
    QPointF head = d->updateMachiningPoints[0];
    QPointF tail = d->updateMachiningPoints[pointsCount - 1];
    double distBetweenHeadAndTail = QVector2D(head - tail).length();
    QVector<QPointF> points;

    // check the primitive is whether closour
    if (qFuzzyCompare(distBetweenHeadAndTail, 0))
    {
        int prevIndex = (pointIndex - 1 + pointsCount) % pointsCount;
        int nextIndex = (pointIndex + 1) % pointsCount;
        QPointF currentPoint = d->updateMachiningPoints[pointIndex];
        QPointF prevPoint = d->updateMachiningPoints[prevIndex];
        QPointF nextPoint = d->updateMachiningPoints[nextIndex];

        QVector2D inDir(currentPoint - lastPoint);
        QVector2D prevDir(prevPoint - currentPoint);
        QVector2D nextDir(nextPoint - currentPoint);

        int prevScore = 0;
        if (inDir[0] * prevDir[0] >= 0)
            prevScore++;
        if (inDir[1] * prevDir[1] >= 0)
            prevScore++;

        int nextScore = 0;
        if (inDir[0] * nextDir[0] >= 0)
            nextScore++;
        if (inDir[1] * nextDir[1] >= 0)
            nextScore++;

        int step = 0;
        if (nextScore >= prevScore)
        {
            step = 1;
        }
        else
        {
            step = -1;
        }

        int cursor = pointIndex;
        points.reserve(pointsCount);
        for (int i = 0; i < pointsCount; i++)
        {
            QPointF point = d->updateMachiningPoints[cursor];
            points.push_back(point);
            cursor = (cursor + step + pointsCount) % pointsCount;
        }

        lastPoint = currentPoint;
    }
    else
    {
        if (pointIndex == 0)
        {
            points = d->updateMachiningPoints;
            lastPoint = tail;
        }
        else
        {
            for (int i = pointsCount - 1; i >= 0; i--)
            {
                points.push_back(d->updateMachiningPoints[i]);
            }
            lastPoint = head;
        }
    }

    if (!canvas.empty())
    {
        cv::Mat pointsMat(points.count(), 2, CV_32F, static_cast<void*>(points.data()));
        pointsMat.convertTo(pointsMat, CV_32S);
        cv::polylines(canvas, pointsMat, true, cv::Scalar(0, 0, 255), 5);
    }
    
    return points;
}

QList<int> LaserPrimitive::startingIndices() const
{
    Q_D(const LaserPrimitive);
    return d->startingIndices;
}

QVector<QPointF> LaserPrimitive::startingPoints() const
{
    Q_D(const LaserPrimitive);

    QVector<QPointF> vertices;
    if (d->updateMachiningPoints.empty())
    {
        return vertices;
    }

    for (int i = 0; i < d->startingIndices.length(); i++)
    {
        vertices.push_back(d->updateMachiningPoints[d->startingIndices[i]]);
    }

    return vertices;
}

LaserPrimitiveType LaserPrimitive::primitiveType() const
{
    Q_D(const LaserPrimitive);
    return d->type; 
}

QString LaserPrimitive::typeName() const
{
    Q_D(const LaserPrimitive);
    return typeName(d->type);
}

QString LaserPrimitive::typeLatinName() const
{
    Q_D(const LaserPrimitive);
    return typeLatinName(d->type);
}

bool LaserPrimitive::isShape() const 
{
    Q_D(const LaserPrimitive);
    return (int)d->type <= (int)LPT_SHAPE; 
}

bool LaserPrimitive::isBitmap() const 
{
    Q_D(const LaserPrimitive);
    return d->type == LPT_BITMAP; 
}

QString LaserPrimitive::name() const 
{
    Q_D(const LaserPrimitive);
    return d->nodeName; 
}

void LaserPrimitive::setName(const QString& name) 
{
    Q_D(LaserPrimitive);
    d->nodeName = name; 
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
}

QString LaserPrimitive::toString() const
{
    return QString("[LaserPrimitive: name=%1]").arg(nodeName());
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

QString LaserPrimitive::typeName(LaserPrimitiveType typeId) const
{
    static QMap<LaserPrimitiveType, QString> TypeNamesMap{
        { LPT_BITMAP, tr("Bitmap") },
        { LPT_CIRCLE, tr("Circle") },
        { LPT_ELLIPSE, tr("Ellipse") },
        { LPT_LINE, tr("Line") },
        { LPT_PATH, tr("Path") },
        { LPT_POLYGON, tr("Polygon") },
        { LPT_POLYLINE, tr("Polyline") },
        { LPT_RECT, tr("Rect") }
    };
    
    return TypeNamesMap[typeId];
}

QString LaserPrimitive::typeLatinName(LaserPrimitiveType typeId) const
{
    static QMap<LaserPrimitiveType, QString> TypeLatinNamesMap{
        { LPT_BITMAP, "Bitmap" },
        { LPT_CIRCLE, "Circle" },
        { LPT_ELLIPSE, "Ellipse" },
        { LPT_LINE, "Line" },
        { LPT_PATH, "Path" },
        { LPT_POLYGON, "Polygon" },
        { LPT_POLYLINE, "Polyline" },
        { LPT_RECT, "Rect" }
    };
    return TypeLatinNamesMap[typeId];
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
    qLogD << "mousePressEvent: " << nodeName();
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

LaserShape::LaserShape(LaserShapePrivate* data, LaserDocument* doc, LaserPrimitiveType type, QTransform saveTransform)
    : LaserPrimitive(data, doc, type, saveTransform)
{
    //m_type = LPT_SHAPE;
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

LaserEllipse::LaserEllipse(const QRectF bounds, LaserDocument * doc, QTransform saveTransform)
    : LaserShape(new LaserEllipsePrivate(this), doc, LPT_ELLIPSE, saveTransform)
{
	
    Q_D(LaserEllipse);
    d->bounds = bounds;
	d->originalBoundingRect = bounds;
	sceneTransformToItemTransform(saveTransform);
	d->path.addEllipse(d->bounds);
	//d->path.addEllipse(d->bounds);
	//d->path = saveTransform.map(d->path);
	d->boundingRect = d->path.boundingRect();
	//��е�ӹ���ʹ��
    d->outline.addEllipse(bounds);
    d->position = bounds.center();
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

QVector<QPointF> LaserEllipse::updateMachiningPoints(cv::Mat& canvas)
{
    Q_D(LaserEllipse);
    QPainterPath path = toPath();

    QVector<QPointF> points;
    machiningUtils::path2Points(path, points, canvas);
    d->updateMachiningPoints = points;
    return points;
}

void LaserEllipse::draw(QPainter* painter)
{
    Q_D(LaserEllipse);
	//painter->drawRect(boundingRect());
	painter->drawPath(d->path);
}

QPainterPath LaserEllipse::toPath() const
{
    Q_D(const LaserEllipse);
    QPainterPath path;
    QTransform t = sceneTransform() * Global::matrixToMM(SU_PX, 40, 40);
    path = t.map(d->path);
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

	return object;
}

QVector<QLineF> LaserEllipse::edges()
{
	Q_D(const LaserEllipse);
	return LaserPrimitive::edges(sceneTransform().map(d->path));
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

LaserRect::LaserRect(const QRectF rect, LaserDocument * doc, QTransform saveTransform)
    : LaserShape(new LaserRectPrivate(this), doc, LPT_RECT, saveTransform)
{
    Q_D(LaserRect);
    d->rect = rect;
	d->originalBoundingRect = rect;
	d->path.addRect(rect);
	sceneTransformToItemTransform(saveTransform);
	//d->path = saveTransform.map(d->path);
    d->boundingRect = d->path.boundingRect();
    d->outline.addRect(rect);
    d->position = rect.center();
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

QVector<QPointF> LaserRect::updateMachiningPoints(cv::Mat& canvas)
{
    Q_D(LaserRect);
	QTransform t = sceneTransform() * Global::matrixToMM(SU_PX, 40, 40);
    QVector<QPointF> points;
    QPolygonF poly = d->path.toFillPolygon(t);
    QPointF pt1 = poly.at(0);
    QPointF pt2 = poly.at(1);
    QPointF pt3 = poly.at(2);
    QPointF pt4 = poly.at(3);
    points.push_back(pt1);
    points.push_back(pt2);
    points.push_back(pt3);
    points.push_back(pt4);
    points.push_back(pt1);

    if (!canvas.empty())
    {
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt1), typeUtils::qtPointF2CVPoint2f(pt2), cv::Scalar(0));
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt2), typeUtils::qtPointF2CVPoint2f(pt3), cv::Scalar(0));
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt3), typeUtils::qtPointF2CVPoint2f(pt4), cv::Scalar(0));
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt4), typeUtils::qtPointF2CVPoint2f(pt1), cv::Scalar(0));
    }

    d->updateMachiningPoints = points;
    return points;
}

QPainterPath LaserRect::toPath() const
{
    Q_D(const LaserRect);
    QPainterPath path;
    QPolygonF rect = transform().map(d->rect);
    path.addPolygon(rect);

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
    path = transform.map(path);

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
	return object;
}

QVector<QLineF> LaserRect::edges()
{
	Q_D(const LaserRect);
	return LaserPrimitive::edges(sceneTransform().map(d->path));
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

LaserLine::LaserLine(const QLineF & line, LaserDocument * doc, QTransform saveTransform)
    : LaserShape(new LaserLinePrivate(this), doc, LPT_LINE, saveTransform)
{
    Q_D(LaserLine);
    d->line = line;
    d->boundingRect = QRectF(d->line.p1(), d->line.p2());
	sceneTransformToItemTransform(saveTransform);
	d->originalBoundingRect = d->boundingRect;
    d->outline.moveTo(d->line.p1());
    d->outline.lineTo(d->line.p2());
    d->position = d->line.center();
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

QVector<QPointF> LaserLine::updateMachiningPoints(cv::Mat& canvas)
{
    Q_D(LaserLine);
    QVector<QPointF> points;
    QPainterPath path = toPath();
    
    machiningUtils::path2Points(path, points, canvas);
    
    d->updateMachiningPoints = points;
    return points;
}

void LaserLine::draw(QPainter * painter)
{
    Q_D(LaserLine);
    painter->drawLine(d->line);
	//painter->drawRect(d->boundingRect);
}

QPainterPath LaserLine::toPath() const
{
    Q_D(const LaserLine);
    QPainterPath path;
    QLineF line = sceneTransform().map(d->line);
    path.moveTo(line.p1());
    path.lineTo(line.p2());

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
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

class LaserPathPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserPath)
public:
    LaserPathPrivate(LaserPath* ptr)
        : LaserShapePrivate(ptr)
    {}

    //QPainterPath path;
};

LaserPath::LaserPath(const QPainterPath & path, LaserDocument * doc, QTransform saveTransform)
    : LaserShape(new LaserPathPrivate(this), doc, LPT_PATH, saveTransform)
{
    Q_D(LaserPath);
    d->path = path;
	d->path = saveTransform.map(d->path);
    d->boundingRect = path.boundingRect();
	d->originalBoundingRect = d->boundingRect;
    d->outline.addPath(path);
    d->position = d->boundingRect.center();
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

QVector<QPointF> LaserPath::updateMachiningPoints(cv::Mat& canvas)
{
    Q_D(LaserPath);
    QVector<QPointF> points;
    QPainterPath path = toPath();
    
    machiningUtils::path2Points(path, points, canvas);
    
    d->updateMachiningPoints = points;
    return points;
}

void LaserPath::draw(QPainter * painter)
{
    Q_D(LaserPath);
    painter->drawPath(d->path);
}

QPainterPath LaserPath::toPath() const
{
    Q_D(const LaserPath);
    QPainterPath path = d->path;
    path = sceneTransform().map(path);

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
    path = transform.map(path);
    return path;
}

QList<QPainterPath> LaserPath::subPaths() const
{
    QList<QPainterPath> paths;
    QPainterPath path = toPath();
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

LaserPolyline::LaserPolyline(const QPolygonF & poly, LaserDocument * doc, QTransform saveTransform)
    : LaserShape(new LaserPolylinePrivate(this), doc, LPT_POLYLINE, saveTransform)
{
    Q_D(LaserPolyline);
    d->poly = poly;
	d->path.addPolygon(d->poly);
	sceneTransformToItemTransform(saveTransform);
	//d->path = saveTransform.map(d->path);
	//d->poly = saveTransform.map(d->poly);
    d->boundingRect = d->poly.boundingRect();
	d->originalBoundingRect = d->boundingRect;

    d->position = *d->poly.begin();
    d->outline.moveTo(*d->poly.begin());
    for (int i = 1; i < d->poly.count(); i++)
    {
        d->position += d->poly[i];
        d->outline.lineTo(d->poly[i]);
    }

    d->position /= d->poly.count();
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

QVector<QPointF> LaserPolyline::updateMachiningPoints(cv::Mat & canvas)
{
    Q_D(LaserPolyline);
    QVector<QPointF> points;
    cv::Point2f lastPt;
    QTransform t = sceneTransform() * Global::matrixToMM(SU_PX, 40, 40);
    for (int i = 0; i < d->poly.size(); i++)
    {
        QPointF pt = t.map(d->poly[i]);
        cv::Point2f cvPt = typeUtils::qtPointF2CVPoint2f(pt);
        if (i > 0 && !canvas.empty())
        {
            cv::line(canvas, lastPt, cvPt, cv::Scalar(0));
        }
        lastPt = cvPt;
        points.push_back(pt);
    }
    if (!canvas.empty())
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(points[points.size() - 1]), typeUtils::qtPointF2CVPoint2f(points[0]), cv::Scalar(0));
    
    d->updateMachiningPoints = points;
    return points;
}

void LaserPolyline::draw(QPainter * painter)
{
    Q_D(LaserPolyline);
    //painter->drawPolyline(d->poly);
	painter->drawPath(d->path);
}

QPainterPath LaserPolyline::toPath() const
{
    Q_D(const LaserPolyline);
    QPainterPath path;
    QPolygonF rect = transform().map(d->poly);
    path.addPolygon(rect);

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
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
	return object;
}

QVector<QLineF> LaserPolyline::edges()
{
	Q_D(const LaserPolyline);
	QPainterPath path;
	path.addPolygon(sceneTransform().map(d->poly));
	return LaserPrimitive::edges(path, true);
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

LaserPolygon::LaserPolygon(const QPolygonF & poly, LaserDocument * doc, QTransform saveTransform)
    : LaserShape(new LaserPolygonPrivate(this), doc, LPT_POLYGON, saveTransform)
{
    Q_D(LaserPolygon);
	d->poly = poly;
	sceneTransformToItemTransform(saveTransform);
    //d->poly = saveTransform.map(poly);
    d->boundingRect = d->poly.boundingRect();
	d->originalBoundingRect = d->boundingRect;
    d->outline.addPolygon(d->poly);
    
    for (int i = 0; i < d->poly.count(); i++)
    {
        d->position += d->poly[i];
    }
    d->position /= d->poly.count();
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

QVector<QPointF> LaserPolygon::updateMachiningPoints(cv::Mat & canvas)
{
    Q_D(LaserPolygon);
    QVector<QPointF> points;
    cv::Point2f lastPt;
    QTransform t = sceneTransform() * Global::matrixToMM(SU_PX, 40, 40);
    for (int i = 0; i < d->poly.size(); i++)
    {
        QPointF pt = t.map(d->poly[i]);
        cv::Point2f cvPt = typeUtils::qtPointF2CVPoint2f(pt);
        if (i > 0 && !canvas.empty())
        {
            cv::line(canvas, lastPt, cvPt, cv::Scalar(0));
        }
        lastPt = cvPt;
        points.push_back(pt);
    }
    points.push_back(points[0]);
    if (!canvas.empty())
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(points[points.size() - 1]), typeUtils::qtPointF2CVPoint2f(points[0]), cv::Scalar(0));
    
    d->updateMachiningPoints = points;
    return points;
}

void LaserPolygon::draw(QPainter * painter)
{
    Q_D(LaserPolygon);
    painter->drawPolygon(d->poly);
}

QPainterPath LaserPolygon::toPath() const
{
    Q_D(const LaserPolygon);
    QPainterPath path;
    QPolygonF poly = transform().map(d->poly);
    path.addPolygon(poly);

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
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
	return object;
}

QVector<QLineF> LaserPolygon::edges()
{
	Q_D(const LaserPolygon);
	QPainterPath path;
	path.addPolygon(sceneTransform().map(d->poly));
	return LaserPrimitive::edges(path);
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

LaserNurbs::LaserNurbs(const QList<QPointF> controlPoints, const QList<qreal> knots, const QList<qreal> weights, BasisType basisType, LaserDocument* doc, QTransform transform)
    : LaserShape(new LaserNurbsPrivate(this, basisType), doc, LPT_NURBS, transform)
{
    Q_D(LaserNurbs);
    d->controlPoints = controlPoints;
    d->knots = knots;
    d->weights = weights;
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

QPainterPath LaserNurbs::toPath() const
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

LaserBitmap::LaserBitmap(const QImage & image, const QRectF& bounds, LaserDocument * doc, QTransform saveTransform)
    : LaserPrimitive(new LaserBitmapPrivate(this), doc, LPT_BITMAP, saveTransform)
{
    Q_D(LaserBitmap);
    d->image = image.convertToFormat(QImage::Format_Grayscale8);
    d->boundingRect = bounds;
    d->type = LPT_BITMAP;
    d->outline.addRect(bounds);
    d->position = bounds.center();
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

QByteArray LaserBitmap::engravingImage(cv::Mat& canvas)
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
    //float mmWidth = 1000.f * d->image.width() / d->image.dotsPerMeterX();
    //float mmHeight = 1000.f * d->image.height() / d->image.dotsPerMeterY();

    int scanInterval = 7;
    double yPulseLength = 0.006329114;
    QVariant value;
    /*if (LaserDriver::instance().getRegister(LaserDriver::RT_SCAN_ROW_SPACING, value))
    {
        qDebug() << "row step register:" << value;
        scanInterval = value.toInt();
    }
    if (LaserDriver::instance().getRegister(LaserDriver::RT_SCAN_ROW_SPEED, value))
    {
        qDebug() << "y pulse register:" << value;
        yPulseLength = value.toDouble() / 1000.0;
    }*/
    qreal pixelInterval = scanInterval * yPulseLength;

	qreal boundingWidth = Global::convertToMM(SU_PX, boundingRect.width());
	qreal boundingHeight = Global::convertToMM(SU_PX, boundingRect.height(), Qt::Vertical);
    qreal boundingLeft = Global::convertToMM(SU_PX, boundingRect.left());
    qreal boundingTop = Global::convertToMM(SU_PX, boundingRect.top());
    int outWidth = boundingWidth * MM_TO_INCH * 600;
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
        outMat = imageUtils::halftone3(resized, layer()->lpi(), layer()->dpi(), 45);
    }

    ba = imageUtils::image2EngravingData(outMat, boundingLeft, boundingTop, pixelInterval, boundingWidth);

    QTransform t = transform().scale(Global::convertToMM(SU_PX, 1), Global::convertToMM(SU_PX, 1, Qt::Vertical));
    if (!canvas.empty())
    {
        try
        {
            QRectF bounds = t.mapRect(boundingRect);
            cv::Rect roiRect = typeUtils::qtRect2cvRect(bounds, 40);
            roiRect = cv::Rect(roiRect.x, roiRect.y, outMat.cols, outMat.rows);
            cv::Mat roi = canvas(roiRect);
            cv::Mat mat;
            cv::cvtColor(outMat, mat, cv::COLOR_GRAY2BGR);
            mat.copyTo(roi);
        }
        catch (cv::Exception& e)
        {
            std::cout << e.err << std::endl;
        }
    }

    return ba; 
}

void LaserBitmap::draw(QPainter * painter)
{
    Q_D(LaserBitmap);
	
	//QImage image = d->image.transformed(d->allTransform, Qt::TransformationMode::SmoothTransformation);
	painter->drawImage(d->boundingRect, d->image);
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
	return object;
}

QVector<QPointF> LaserBitmap::updateMachiningPoints(cv::Mat& canvas)
{
    Q_D(LaserBitmap);
	QTransform t = transform() * Global::matrixToMM(SU_PX, 40, 40);
    QVector<QPointF> points;
    QPointF pt1 = t.map(d->boundingRect.topLeft());
    QPointF pt2 = t.map(d->boundingRect.topRight());
    QPointF pt3 = t.map(d->boundingRect.bottomRight());
    QPointF pt4 = t.map(d->boundingRect.bottomLeft());
    points.push_back(pt1);
    points.push_back(pt2);
    points.push_back(pt3);
    points.push_back(pt4);
    points.push_back(pt1);

    if (!canvas.empty())
    {
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt1), typeUtils::qtPointF2CVPoint2f(pt2), cv::Scalar(0));
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt2), typeUtils::qtPointF2CVPoint2f(pt3), cv::Scalar(0));
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt3), typeUtils::qtPointF2CVPoint2f(pt4), cv::Scalar(0));
        cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt4), typeUtils::qtPointF2CVPoint2f(pt1), cv::Scalar(0));
    }

    d->updateMachiningPoints = points;
    return points;
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

QDebug operator<<(QDebug debug, const LaserPrimitive & item)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "(" << item.name() << ", " << item.type() << ")";
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

QByteArray LaserShape::engravingImage(cv::Mat& canvas)
{
    QByteArray bytes;
    QPainterPath path = toPath();
    QRectF boundRect = path.boundingRect();

    int scanInterval = 7;
    double yPulseLength = 0.006329114;
    QVariant value;
    /*if (LaserDriver::instance().getRegister(LaserDriver::RT_ENGRAVING_ROW_STEP, value))
    {
        qDebug() << "row step register:" << value;
        scanInterval = value.toInt();
    }
    if (LaserDriver::instance().getRegister(LaserDriver::RT_Y_AXIS_PULSE_LENGTH, value))
    {
        qDebug() << "y pulse register:" << value;
        yPulseLength = value.toDouble() / 1000.0;
    }*/
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

class LaserTextPrivate : public LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserText)
public:
    LaserTextPrivate(LaserText* ptr)
        : LaserPrimitivePrivate(ptr)
    {}

	QString content;
	QRect rect;
};

LaserText::LaserText(const QRect rect, const QString content, LaserDocument* doc, LaserPrimitiveType type, QTransform saveTransform)
	: LaserPrimitive(new LaserTextPrivate(this), doc, LPT_TEXT, saveTransform)
{
    Q_D(LaserText);
    d->rect = rect;
    d->content = content;
	d->boundingRect = rect;
    d->outline.addRect(rect);
    d->position = rect.center();
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

void LaserText::draw(QPainter * painter)
{
    Q_D(LaserText);
	QTextDocument doc;
	doc.setHtml(d->content);
	painter->translate(d->rect.topLeft());
	doc.drawContents(painter, QRect(0, 0, d->rect.width(), d->rect.height()));
}
