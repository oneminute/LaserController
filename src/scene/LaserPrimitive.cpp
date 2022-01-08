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

#include "LaserApplication.h"
#include "LaserScene.h"
#include "laser/LaserDriver.h"
#include "laser/LaserDevice.h"
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
#include "ui/LaserControllerWindow.h"
#include "widget/LaserDoubleSpinBox.h"
#include "algorithm/QuadTreeNode.h"
#include "task/ProgressItem.h"
#include "task/ProgressModel.h"
#include "state/StateController.h"

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
        , exportable(true)
        , visible(true)
        , isJoinedGroup(false)
        , joinedGroupList(nullptr)
        , isAlignTarget(false)
    {}

    LaserDocument* doc;
    LaserLayer* layer;
	int layerIndex;
    QRect boundingRect;
    LaserPrimitiveType primitiveType;
    bool isHover;
    QPainterPath outline;
    LaserPointListList machiningPointsList;
    LaserPointListList arrangedPointsList;
    QPoint machiningCenter;
    QList<int> startingIndices;
	QTransform allTransform;
	QRect originalBoundingRect;
	QPainterPath path;
    bool isLocked;
    QList<QuadTreeNode*> treeNodes;
    bool exportable;
    bool visible;
    bool isJoinedGroup;
    bool isAlignTarget;
    QSet<LaserPrimitive*>* joinedGroupList;
    QRect variableBounds;//circleText，horizontalText，verticalText中使用，方便改变外包框
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
    d->name = doc->newPrimitiveName(this);
	d->allTransform = saveTransform;
	d->layerIndex = layerIndex;
}

LaserPrimitive::~LaserPrimitive()
{
    Q_D(LaserPrimitive);
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
    if (!visible())
        return;

    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);

    QRectF bounds = boundingRect();
    QPointF topLeft = bounds.topLeft() - QPointF(2, 2);
    QPointF bottomRight = QPoint(bounds.left() + bounds.width(), bounds.top() + bounds.height()) + QPointF(2, 2);
    QRectF selectionBounds = QRectF(topLeft, bottomRight);
	QColor color = Qt::blue;
    QPen pen(color, 1, Qt::DashLine);

    QPainterPath outline = this->outline();
    QPointF startPos = outline.pointAtPercent(0);
    painter->setPen(QPen(Qt::green, 1, Qt::SolidLine));
    if (Config::Debug::showPrimitiveName())
    {
        painter->drawText(startPos, name());
        QColor color = Qt::blue;
        QPen pen(color, 1, Qt::DashLine);
        painter->drawRect(bounds);
    }
    if (Config::Debug::showPrimitiveFirstPoint())
        painter->drawEllipse(startPos, 2, 2);

    if (d->layer)
    {
        color = d->layer->color();
        if (!exportable())
        {
            color = Qt::lightGray;
        }
    }

	if (isSelected())
	{
		//isSelected();
		QString name = this->metaObject()->className();
        qreal penSize = 1.0;
        if (d->isAlignTarget) {
            penSize = 3.0;
        }
		QPen pen = QPen(color, penSize, Qt::DashLine);
		pen.setCosmetic(true);		
        if (isJoinedGroup()) {
            pen.setStyle(Qt::DashDotDotLine);
        }
        painter->setPen(pen);
        if (name == "LaserBitmap") {
            painter->drawRect(selectionBounds);
        }
	}
    else
    {
		QPen pen = QPen(color, 1.2f, Qt::SolidLine);
		pen.setCosmetic(true);
		painter->setPen(pen);
    }

    if (StateControllerInst.isInState(StateControllerInst.documentPrintAndCutAligningState()))
    {
        QPointF center = d->boundingRect.center();
        QTransform sceneInverted = sceneTransform().inverted();
        QTransform transformInverted = transform().inverted();
        QTransform t;
        //t *= QTransform::fromTranslate(-center.x(), -center.y());
        t *= document()->transform();
        //t *= sceneInverted;
        //t *= transformInverted;
        //t *= QTransform::fromTranslate(center.x(), center.y());
        t *= painter->transform();
        //painter->setTransform(t);
    }
    draw(painter); 
}

int LaserPrimitive::layerIndex()
{
	Q_D(const LaserPrimitive);
	return d->layerIndex;
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
    return d->boundingRect;
}

QRect LaserPrimitive::sceneBoundingRect() const
{
    Q_D(const LaserPrimitive);
    return sceneTransform().map(d->path).boundingRect().toRect();
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
        bool isClosed = machiningPoints.first() == machiningPoints.last();
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
                if (!(lastPoint == currentPoint))
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

LaserLineListList LaserPrimitive::generateFillData() 
{
    Q_D(LaserPrimitive);
    QPainterPath path = sceneTransform().map(d->path);
    // 获取所有的加工线
    LaserLineListList lineList = utils::interLines(path, layer()->fillingRowInterval());

    return lineList;
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

void LaserPrimitive::setBoundingRectWidth(qreal width)
{
}

void LaserPrimitive::setBoundingRectHeight(qreal height)
{
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

bool LaserPrimitive::exportable() const
{
    Q_D(const LaserPrimitive);
    if (layer()->exportable())
    {
        return d->exportable;
    }
    else
    {
        return false;
    }
}

void LaserPrimitive::setExportable(bool value)
{
    Q_D(LaserPrimitive);
    d->exportable = value;
}

bool LaserPrimitive::visible() const
{
    Q_D(const LaserPrimitive);
    if (layer()->visible())
    {
        return d->visible;
    }
    else
    {
        return false;
    }
}

void LaserPrimitive::setVisible(bool value)
{
}

bool LaserPrimitive::isAlignTarget()
{
    Q_D(const LaserPrimitive);
    return d->isAlignTarget;
}

void LaserPrimitive::setAlignTarget(bool value)
{
    Q_D(LaserPrimitive);
    d->isAlignTarget = value;
}

LaserLayer* LaserPrimitive::layer() const
{
    Q_D(const LaserPrimitive);
    return d->layer; 
}

void LaserPrimitive::setLayer(LaserLayer* layer, bool whenNullLayerKeepIndex) 
{
    Q_D(LaserPrimitive);
    if (d->layer) {
        d->layer->primitives().removeOne(this);
    }
    d->layer = layer;
	if (layer) {
        
		d->layerIndex = layer->index();
        d->layer->primitives().append(this);
	}
	else {
        //如果保留原有的layerIndex，删除后可以再恢复回来
        //如果重置为-1，则删除后需要重新添加到层中
        if (!whenNullLayerKeepIndex) {
            d->layerIndex = -1;
        }
	}
    d->doc->updateLayersStructure();
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
	QRect boundingRect)
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
        { LPT_STAR , tr("Star")},
        { LPT_FRAME , tr("Frame") },
        { LPT_RING , tr("Ring") }
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

void LaserPrimitive::setJoinedGroup(QSet<LaserPrimitive*>* joinedGroup)
{
    Q_D(LaserPrimitive);
    /*d->isJoinedGroup = isJoinedGroup;
    if (!isJoinedGroup) {
        d->joinedGroupList.clear();
    }
    else {
        d->joinedGroupList.clear();
        for (QGraphicsItem* item : parentItem()->childItems()) {
            LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
            d->joinedGroupList.append(primitive);
        }
    }*/
    //QSet 唯一性 uniqueness
    if (joinedGroup) {
        joinedGroup->insert(this);
        d->joinedGroupList = joinedGroup;
        d->isJoinedGroup = true;
    }
    else {
        d->isJoinedGroup = false;
        d->joinedGroupList = nullptr;
    }
}

bool LaserPrimitive::isJoinedGroup()
{
    Q_D(const LaserPrimitive);
    return d->isJoinedGroup;
}

QSet<LaserPrimitive*>* LaserPrimitive::joinedGroupList()
{
    Q_D(LaserPrimitive);
    return d->joinedGroupList;
}

QList<QuadTreeNode*>& LaserPrimitive::treeNodesList()
{
    Q_D(LaserPrimitive);
    return d->treeNodes;
}

void LaserPrimitive::addTreeNode(QuadTreeNode* node)
{
    Q_D(LaserPrimitive);
    if (node) {
        d->treeNodes.append(node);
    }
}

void LaserPrimitive::removeAllTreeNode()
{
    Q_D(LaserPrimitive);
    for (QuadTreeNode * node : d->treeNodes) {
        removeOneTreeNode(node);
    }
}

void LaserPrimitive::removeOneTreeNode(QuadTreeNode * node)
{
    Q_D(LaserPrimitive);
    if (node) {
        node->removePrimitive(this);
        if (!d->treeNodes.removeOne(node)) {
            qLogW << "the remove node not exit in the TreeNode";
        }
    }
}

void LaserPrimitive::concaveRect(QRect rect, QPainterPath & path, qreal cornerRadius, int type)
{
    QPainterPath rectPath;
    rectPath.addRect(rect);
    QPoint topLeft = rect.topLeft();
    QPoint topRight(rect.left() + rect.width(), rect.top());
    QPoint bottomLeft(rect.left(), rect.top() + rect.height());
    QPoint bottomRight(rect.left() + rect.width(), rect.top() + rect.height());
    if (type == CRT_Arc) {
        QPainterPath circleLeftTopPath, circleRightTopPath, circleLeftBottomPath, circleRightBottomPath;        
        circleLeftTopPath.addEllipse(topLeft, cornerRadius, cornerRadius);
        circleRightTopPath.addEllipse(topRight, cornerRadius, cornerRadius);
        circleLeftBottomPath.addEllipse(bottomLeft, cornerRadius, cornerRadius);
        circleRightBottomPath.addEllipse(bottomRight, cornerRadius, cornerRadius);
        path.addPath(rectPath.
            subtracted(circleLeftTopPath).
            subtracted(circleRightTopPath).
            subtracted(circleLeftBottomPath).
            subtracted(circleRightBottomPath));
    }
    else if(type == CRT_Line) {
        QPainterPath triangleLeftTopPath, triangleRightTopPath, triangleLeftBottomPath, triangleRightBottomPath;
        triangleLeftTopPath.moveTo(topLeft);
        triangleLeftTopPath.lineTo(QPointF(topLeft.x() + cornerRadius, topLeft.y()));
        triangleLeftTopPath.lineTo(QPointF(topLeft.x(), topLeft.y() + cornerRadius));
        triangleLeftTopPath.closeSubpath();
        triangleRightTopPath.moveTo(topRight);
        triangleRightTopPath.lineTo(QPointF(topRight.x() - cornerRadius, topRight.y()));
        triangleRightTopPath.lineTo(QPointF(topRight.x(), topRight.y() + cornerRadius));
        triangleRightTopPath.closeSubpath();
        triangleLeftBottomPath.moveTo(bottomLeft);
        triangleLeftBottomPath.lineTo(QPointF(bottomLeft.x(), bottomLeft.y() - cornerRadius));
        triangleLeftBottomPath.lineTo(QPointF(bottomLeft.x() + cornerRadius, bottomLeft.y()));
        triangleLeftBottomPath.closeSubpath();
        triangleRightBottomPath.moveTo(bottomRight);
        triangleRightBottomPath.lineTo(QPointF(bottomRight.x() - cornerRadius, bottomRight.y()));
        triangleRightBottomPath.lineTo(QPointF(bottomRight.x(), bottomRight.y() - cornerRadius));
        triangleRightBottomPath.closeSubpath();
        path.addPath(rectPath.
            subtracted(triangleLeftTopPath).
            subtracted(triangleRightTopPath).
            subtracted(triangleLeftBottomPath).
            subtracted(triangleRightBottomPath));
    }
    
}
QPainterPath LaserPrimitive::computeCornerRadius(QRect rect,  int cornerRadius, int type)
{
    Q_D(LaserPrimitive);
    QPainterPath tempPath;
    if (cornerRadius == 0)
    {
        tempPath.addRect(rect);
    }
    else if (type == CRT_Round) {
        tempPath.addRoundedRect(rect, cornerRadius, cornerRadius);
    }
    else {
        concaveRect(rect, tempPath, qAbs(cornerRadius), type);
    }
    return tempPath;
}

bool LaserPrimitive::isAvailable() const
{
    Q_D(const LaserPrimitive);
    if (d->path.isEmpty())
        return false;
    if (qFuzzyCompare(d->boundingRect.width(), 0.0) &&
        qFuzzyCompare(d->boundingRect.height(), 0.0))
        return false;
    return true;
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

QRect LaserPrimitive::variableBounds()
{
    Q_D(LaserPrimitive);
    return d->variableBounds;
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
};

LaserEllipse::LaserEllipse(const QRect bounds, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserEllipsePrivate(this), doc, LPT_ELLIPSE, layerIndex, saveTransform)
{
	
    Q_D(LaserEllipse);
    d->boundingRect = bounds;
	d->originalBoundingRect = bounds;
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
	d->originalBoundingRect = bounds;
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
    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(tr("%1 update machining points").arg(name()), parentProgress);
    machiningUtils::path2Points(progress, path, d->machiningPointsList, indices, d->machiningCenter);

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
	painter->drawPath(d->path);
	painter->setPen(QPen(Qt::black, 1));
}

//QRect LaserEllipse::sceneBoundingRect() const
//{
//	Q_D(const LaserEllipse);
//	return sceneTransform().mapRect(d->boundingRect);
//}

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
	QJsonArray bounds = { d->boundingRect.x(), d->boundingRect.y(),d->boundingRect.width(), d->boundingRect.height() };
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
        , cornerRadius(0)
    {}

    int cornerRadius;
};

LaserRect::LaserRect(const QRect rect, int cornerRadius, LaserDocument * doc, 
    QTransform saveTransform, int layerIndex, int cornerRadiusType)
    : LaserShape(new LaserRectPrivate(this), doc, LPT_RECT, layerIndex, saveTransform)
{
    Q_D(LaserRect);
    d->boundingRect = rect;
	d->originalBoundingRect = rect;
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
	d->originalBoundingRect = rect;
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

void LaserRect::setCornerRadius(int cornerRadius, int type)
{
    Q_D(LaserRect);
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

LaserPointListList LaserRect::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserRect);
    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(tr("%1 update machining points").arg(name()), parentProgress);
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
        points.push_back(LaserPoint(pt1.x(), pt1.y(), qRound(angle11), qRound(angle12)));
        points.push_back(LaserPoint(pt2.x(), pt2.y(), qRound(angle21), qRound(angle22)));
        points.push_back(LaserPoint(pt3.x(), pt3.y(), qRound(angle31), qRound(angle32)));
        points.push_back(LaserPoint(pt4.x(), pt4.y(), qRound(angle41), qRound(angle42)));
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

bool LaserRect::isClosed() const
{
    return true;
}

QPointF LaserRect::position() const
{
    Q_D(const LaserRect);
    return sceneTransform().map(d->boundingRect.topLeft());
}

class LaserLinePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserLine)
public:
    LaserLinePrivate(LaserLine* ptr)
        : LaserShapePrivate(ptr)
    {}

    QLine line;
};

LaserLine::LaserLine(const QLine & line, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserLinePrivate(this), doc, LPT_LINE, layerIndex, saveTransform)
{
    Q_D(LaserLine);
    d->line = line;
	sceneTransformToItemTransform(saveTransform);
    d->path.moveTo(d->line.p1());
    d->path.lineTo(d->line.p2());
    d->boundingRect = d->path.boundingRect().toRect();
	d->originalBoundingRect = d->boundingRect;
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
	d->originalBoundingRect = d->boundingRect;
    d->outline = QPainterPath();
    d->outline.moveTo(d->line.p1());
    d->outline.lineTo(d->line.p2());
}

LaserPointListList LaserLine::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserLine);
    d->machiningPointsList.clear();
    d->startingIndices.clear();

    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(tr("%1 update machining points").arg(name()), parentProgress);
	QTransform t = sceneTransform();
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
    progress->finish();
    
    return d->machiningPointsList;
}

void LaserLine::draw(QPainter * painter)
{
    Q_D(LaserLine);
    painter->drawLine(d->line);
	//painter->drawRect(d->boundingRect);
}

//QRect LaserLine::sceneBoundingRect() const
//{
//	Q_D(const LaserLine);
//	QPainterPath path;
//	QLineF line = sceneTransform().map(d->line);
//	path.moveTo(line.p1());
//	path.lineTo(line.p2());
//	return path.boundingRect().toRect();
//}

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
};

LaserPath::LaserPath(const QPainterPath & path, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserPathPrivate(this), doc,  LPT_PATH, layerIndex, saveTransform)
{
    Q_D(LaserPath);
	//d->path = saveTransform.map(d->path);
    d->path = path;
	sceneTransformToItemTransform(saveTransform);
    d->boundingRect = path.boundingRect().toRect();
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
    d->boundingRect = path.boundingRect().toRect();
	d->originalBoundingRect = d->boundingRect;
    d->outline.addPath(path);
}

LaserPointListList LaserPath::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserPath);
    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(tr("%1 update machining points").arg(name()), parentProgress);
    QPainterPath path = sceneTransform().map(d->path);

    machiningUtils::path2Points(progress, path, d->machiningPointsList, d->startingIndices, d->machiningCenter);

    return d->machiningPointsList;
}

void LaserPath::draw(QPainter * painter)
{
    Q_D(LaserPath);
    painter->drawPath(d->path);
}

QList<QPainterPath> LaserPath::subPaths() const
{
    Q_D(const LaserPath);
    QList<QPainterPath> paths;
    QPainterPath path = sceneTransform().map(d->path);
    QList<QPolygonF> polys = path.toSubpathPolygons();
    for (QPolygonF& poly : polys)
    {
        QPainterPath subPath;
        subPath.addPolygon(poly);
        paths.append(subPath);
    }
    return paths;
}

//QRect LaserPath::sceneBoundingRect() const
//{
//	Q_D(const LaserPath);
//	return sceneTransform().map(d->path).boundingRect().toRect();
//	
//}

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
    QPolygon poly;
};

LaserPolyline::LaserPolyline(const QPolygon & poly, LaserDocument * doc, QTransform saveTransform, int layerIndex)
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

QPolygon LaserPolyline::polyline() const 
{
    Q_D(const LaserPolyline);
    return d->poly; 
}

void LaserPolyline::setPolyline(const QPolygon& poly) 
{
    Q_D(LaserPolyline);
    d->poly = poly; 
    d->path = QPainterPath();
	d->path.addPolygon(d->poly);
    d->boundingRect = d->poly.boundingRect();
	d->originalBoundingRect = d->boundingRect;
    d->outline = QPainterPath();
    d->outline.moveTo(*d->poly.begin());
    for (int i = 1; i < d->poly.count(); i++)
    {
        d->outline.lineTo(d->poly[i]);
    }
}

//QRect LaserPolyline::sceneBoundingRect() const
//{
//    Q_D(const LaserPolyline);
//    return sceneTransform().map(d->path).boundingRect().toRect();
//}

LaserPointListList LaserPolyline::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserPolyline);
    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(tr("%1 update machining points").arg(name()), parentProgress);
    progress->setMaximum(d->poly.size());
    d->machiningPointsList.clear();
    d->startingIndices.clear();
    bool isClosed = this->isClosed();
    LaserPointList points;
    QTransform t = sceneTransform();
    d->machiningCenter = QPoint(0, 0);
    for (int i = 0; i < d->poly.size(); i++)
    {
        QPoint pt = t.map(d->poly.at(i));

        QPoint cPt = pt;
        QPoint nPt = (i == d->poly.size() - 1) ? pt + (pt - d->poly.at(i - 1)) : d->poly.at(i + 1);
        QPoint lPt = (i == 0) ? pt + (pt - d->poly.at(1)) : d->poly.at(i - 1);
        QLineF line1(cPt, nPt);
        QLineF line2(cPt, lPt);
        qreal angle1 = line1.angle();
        qreal angle2 = line2.angle();
        points.append(LaserPoint(pt.x(), pt.y(), qRound(angle1), qRound(angle2)));
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
	painter->drawPath(d->path);
    //painter->drawRect(boundingRect());
}

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

    QPolygon poly;
};

LaserPolygon::LaserPolygon(const QPolygon & poly, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserShape(new LaserPolygonPrivate(this), doc, LPT_POLYGON, layerIndex, saveTransform)
{
    Q_D(LaserPolygon);
	d->poly = poly;
	sceneTransformToItemTransform(saveTransform);
    d->boundingRect = d->poly.boundingRect();
	d->originalBoundingRect = d->boundingRect;
    d->path.addPolygon(d->poly);
    d->outline.addPolygon(d->poly);
}

QPolygon LaserPolygon::polyline() const 
{
    Q_D(const LaserPolygon);
    return d->poly; 
}

void LaserPolygon::setPolyline(const QPolygon& poly) 
{
    Q_D(LaserPolygon);
    d->poly = poly; 
    d->boundingRect = d->poly.boundingRect();
	d->originalBoundingRect = d->boundingRect;
    d->path = QPainterPath();
    d->path.addPolygon(d->poly);
    d->outline = QPainterPath();
    d->outline.addPolygon(d->poly);
}

LaserPointListList LaserPolygon::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserPolygon);
    d->machiningPointsList.clear();
    QPolygon polygon = sceneTransform().map(d->poly);
    polygon.append(polygon.first());
    LaserPointList points;
    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(tr("%1 update machining points").arg(name()), parentProgress);
    machiningUtils::polygon2Points(progress, polygon, points, d->startingIndices, d->machiningCenter);
    d->machiningPointsList.append(points);
    return d->machiningPointsList;
}

void LaserPolygon::draw(QPainter * painter)
{
    Q_D(LaserPolygon);
    painter->drawPolygon(d->poly);
}

//QRect LaserPolygon::sceneBoundingRect() const
//{
//	Q_D(const LaserPolygon);
//	QPainterPath path;
//	path.addPolygon(sceneTransform().map(d->poly));
//	return path.boundingRect().toRect();
//}

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
        for (quint32 i = 0; i <= n; i++)
        {
            iMap[i] = utils::factorial(i);
        }

        qreal u = 0;

        for (int iu = 0; iu < steps; iu++)
        {
            qreal u = iu * 1.0 / steps;
            for (quint32 i = 0; i <= n; i++)
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
            for (quint32 ip = 0; ip <= p; ip++)
            {
                logU.append("\n");
                for (quint32 i = 0; i <= m - 1 - ip; i++)
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

        boundingRect = drawingPath.boundingRect().toRect();
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
    //painter->drawRect(d->boundingRect);
}

//QRect LaserNurbs::sceneBoundingRect() const
//{
//	Q_D(const LaserNurbs);
//	return d->drawingPath.boundingRect().toRect();
//}

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
};

LaserBitmap::LaserBitmap(const QImage & image, const QRect& bounds, LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserPrimitive(new LaserBitmapPrivate(this), doc,  LPT_BITMAP, saveTransform, layerIndex)
{
    Q_D(LaserBitmap);
    d->image = image.convertToFormat(QImage::Format_Grayscale8);
    d->boundingRect = bounds;
    d->path.addRect(bounds);
	d->originalBoundingRect = d->boundingRect;
    d->primitiveType = LPT_BITMAP;
    d->outline.addRect(bounds);
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

QByteArray LaserBitmap::engravingImage(ProgressItem* parentProgress, QPoint& lastPoint)
{ 
    Q_D(LaserBitmap);
    QByteArray ba;

    parentProgress->setMaximum(2);
    QImage srcImage = d->image.copy();
    QImage rotated = srcImage.transformed(sceneTransform(), Qt::SmoothTransformation);
    QImage outImage(rotated.size(), QImage::Format_ARGB32);;
    outImage.fill(Qt::white);
    QPainter p(&outImage);
    p.begin(&outImage);
    p.drawImage(0, 0, rotated);
    p.end();
    outImage = outImage.convertToFormat(QImage::Format_Grayscale8);
    QRect boundingRect = sceneBoundingRect();
    cv::Mat src(outImage.height(), outImage.width(), CV_8UC1, (void*)outImage.constBits(), outImage.bytesPerLine());

    int dpi = d->layer->dpi();
    int pixelWidth = boundingRect.width() * dpi / 25400.0;
    int pixelHeight = boundingRect.height() * dpi / 25400.0;

    int gridSize = qRound(dpi * 1.0 / d->layer->lpi());

    cv::Mat pixelScaled;
    cv::resize(src, pixelScaled, cv::Size(pixelWidth, pixelHeight), 0.0, 0.0, cv::INTER_NEAREST);

    cv::Mat halfToneMat = src;
    if (layer()->useHalftone())
        halfToneMat = imageUtils::halftone6(parentProgress, pixelScaled, this->layer()->halftoneAngles(), gridSize);

    qreal pixelInterval = layer()->engravingRowInterval();

    int outWidth = pixelWidth;
    int outHeight = qRound(boundingRect.height() / pixelInterval);
    qLogD << "bounding rect: " << boundingRect;
    qDebug() << "out width:" << outWidth;
    qDebug() << "out height:" << outHeight;

    cv::Mat resized;
    cv::resize(halfToneMat, resized, cv::Size(outWidth, outHeight), cv::INTER_NEAREST);
    
    qreal accLength = LaserApplication::device->engravingAccLength(layer()->engravingRunSpeed());
    
    ba = imageUtils::image2EngravingData(parentProgress, resized, boundingRect, pixelInterval, 
        lastPoint, accLength);

    parentProgress->finish();
    return ba; 
}

void LaserBitmap::draw(QPainter * painter)
{
    Q_D(LaserBitmap);
	
	//QImage image = d->image.transformed(d->allTransform, Qt::TransformationMode::SmoothTransformation);
	painter->drawImage(d->boundingRect, d->image);
    //painter->drawRect(d->boundingRect);
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

LaserPointListList LaserBitmap::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserBitmap);
    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(tr("%1 update machining points").arg(name()), parentProgress);
    d->machiningPointsList.clear();
    d->startingIndices.clear();

	//QTransform t = sceneTransform() * Global::matrixToMachining();
    QPolygon poly = sceneTransform().map(d->path).toFillPolygon().toPolygon();
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
    points.push_back(LaserPoint(pt1.x(), pt1.y(), qRound(angle11), qRound(angle12)));
    points.push_back(LaserPoint(pt2.x(), pt2.y(), qRound(angle21), qRound(angle22)));
    points.push_back(LaserPoint(pt3.x(), pt3.y(), qRound(angle31), qRound(angle32)));
    points.push_back(LaserPoint(pt4.x(), pt4.y(), qRound(angle41), qRound(angle42)));
    d->machiningCenter = utils::center(points).toPoint();
    points.push_back(points.first());
    d->startingIndices.append(0);
    d->startingIndices.append(1);
    d->startingIndices.append(2);
    d->startingIndices.append(3);
    d->machiningPointsList.append(points);

    progress->finish();
    return d->machiningPointsList;
}

//QRect LaserBitmap::sceneBoundingRect() const
//{
//	Q_D(const LaserBitmap);
//	QPainterPath path;
//	path.addRect(d->boundingRect);
//	return sceneTransform().map(path).boundingRect().toRect();
//}

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
	path.addRect(d->boundingRect);
    path = sceneTransform().map(path);
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

QByteArray LaserShape::filling(ProgressItem* progress, QPoint& lastPoint)
{
    Q_D(LaserShape);
    QByteArray bytes;
    QPainterPath path = sceneTransform().map(d->path);
    QRect boundingRectInDevice = path.boundingRect().toRect();
    qreal ratio = boundingRectInDevice.width() * 1.0 / boundingRectInDevice.height();
    int canvasWidth = qMin(boundingRectInDevice.width(), 8192);
    int canvasHeight = qRound(canvasWidth / ratio);
    if (ratio < 1)
    {
        canvasHeight = qMin(boundingRectInDevice.height(), 8192);
        canvasWidth = qRound(canvasHeight * ratio);
    }

    QTransform t = QTransform::fromScale(
        canvasWidth * 1.0 / boundingRectInDevice.width(),
        canvasHeight * 1.0 / boundingRectInDevice.height()
    );

    path = t.map(path);
    QRect boundingRect = path.boundingRect().toRect();
    t = QTransform::fromTranslate(-boundingRect.x(), -boundingRect.y());
    path = t.map(path);
    QImage canvas(boundingRect.width(), boundingRect.height(), QImage::Format_Grayscale8);
    if (canvas.isNull())
    {
        progress->finish();
        return QByteArray();
    }
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    painter.setBrush(Qt::black);
    painter.drawPath(path);
    canvas.save("tmp/" + name() + "_canvas.png");

    cv::Mat src(canvas.height(), canvas.width(), CV_8UC1, (void*)canvas.constBits(), canvas.bytesPerLine());

    int dpi = d->layer->dpi();
    int pixelWidth = boundingRectInDevice.width() * dpi / 25400.0;
    int pixelHeight = boundingRectInDevice.height() * dpi / 25400.0;

    int pixelInterval = layer()->engravingRowInterval();
    int outWidth = pixelWidth;
    int outHeight = qCeil(boundingRectInDevice.height() * 1.0 / pixelInterval);
    cv::Mat resized;
    cv::resize(src, resized, cv::Size(outWidth, outHeight), 0.0, 0.0, cv::INTER_CUBIC);

    cv::imwrite("tmp/" + name().toStdString() + "_resized.png", resized);

    qLogD << "bounding rect: " << boundingRectInDevice;
    qDebug() << "out width:" << outWidth;
    qDebug() << "out height:" << outHeight;
    
    int accLength = LaserApplication::device->engravingAccLength(layer()->engravingRunSpeed());
    bytes = imageUtils::image2EngravingData(progress, resized, 
        boundingRectInDevice, pixelInterval, lastPoint, accLength);

    return bytes;
}

int LaserShape::layerIndex()
{
	Q_D(LaserShape);
	return d->layerIndex;
}

class LaserTextPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserText)
public:
    LaserTextPrivate(LaserText* ptr)
        : LaserShapePrivate(ptr)
    {
        spaceY = LaserApplication::mainWindow->fontSpaceYDoubleSpinBox()->value();
    }
	QRect rect;
    QString content;
    QString lastContent;
    QPointF startPos;
    //QList<QPainterPath> pathList;
    //QMap<QPointF, QList<QPainterPath>> pathList;
    QList<LaserTextRowPath> pathList;
    //QPainterPath allPath;
    QFont font;
    int alignHType;
    int lastAlignHType;
    int alignVType;
    int lastAlignVType;
    QGraphicsView* view;
    qreal spaceY;
};

LaserText::LaserText(LaserDocument* doc, QPointF startPos, QFont font, qreal spaceY, int alighHType, int alighVType, QTransform saveTransform, int layerIndex)
	: LaserShape(new LaserTextPrivate(this),  doc,  LPT_TEXT, layerIndex, saveTransform)
{
    Q_D(LaserText);
    d->outline.addRect(d->rect);
    d->startPos = startPos;
    d->font = font;
    d->alignHType = alighHType;
    d->lastAlignHType = alighHType;
    d->alignVType = alighVType;
    d->lastAlignVType = alighVType;
    d->startPos = mapFromScene(d->startPos);
    d->view = doc->scene()->views()[0];
    d->allTransform = saveTransform;
    d->spaceY = spaceY;
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
    
    return d->path;
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

qreal LaserText::spaceY()
{
    Q_D(LaserText);
    return d->spaceY;
}

void LaserText::setSpacceY(qreal space)
{
    Q_D(LaserText);
    d->spaceY = space;
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

    int fontSize = d->font.pixelSize();
    qreal letterSpacing = d->font.letterSpacing();
    //qreal wordSpacing = d->font.wordSpacing();
    
    bool isNewLine = true;
    QRectF lastBound;
    //QPainterPath allPath;
    //QPainterPath lastPath;
    qDebug() << d->lastContent;
    qDebug() << d->content;
    //for (QChar c : d->content) {
    
    for (int i = 0; i < d->content.size(); i++) {
        QChar c = d->content[i];
        qreal rowY = (subRowPathList.size() + 1) * fontSize + subRowPathList.size()* spaceY() + d->startPos.y();
        QPointF startP(d->startPos.x(), rowY);
        if (c == "\n") {
            subRowPathList.append(*listPtr);
            rowPathList.append(*rowPathPtr);
            subBoundList.append(*boundListPtr);
            startPosList.append(startP);
            rowY = (subRowPathList.size() + 1) * fontSize + subRowPathList.size()* spaceY() + d->startPos.y();
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
    qreal allHeight = fontSize * subRowPathList.size() + (subRowPathList.size()-1)* spaceY();
    d->pathList.clear();
    d->path = QPainterPath();
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
        d->path.addPath(newRowPath);
    }
    
    d->boundingRect = d->path.boundingRect().toRect();
    document()->updateDocumentBounding();
}

QList<LaserTextRowPath> LaserText::subPathList()
{
    Q_D(const LaserText);

    return d->pathList;
}

//QRectF LaserText::boundingRect() const
//{
//    Q_D(const LaserText);
//    //d->boundingRect = d->path.boundingRect();
//    return sceneTransform().mapRect(path().boundingRect());
//}

//QRect LaserText::sceneBoundingRect() const
//{
//    Q_D(const LaserText);
//    return sceneTransform().mapRect(d->boundingRect);
//}

QRectF LaserText::originalBoundingRect(qreal extendPixel) const
{
    
    Q_D(const LaserPrimitive);
    QRectF boundingRect = path().boundingRect();;
    qreal x = boundingRect.topLeft().x() - extendPixel;
    qreal y = boundingRect.topLeft().y() - extendPixel;
    qreal width = boundingRect.width() + 2 * extendPixel;
    qreal height = boundingRect.height() + 2 * extendPixel;
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
    //painter->drawRect(d->boundingRect);
}

LaserPrimitive * LaserText::clone(QTransform t)
{
	Q_D(LaserText);
	LaserText* text = new LaserText(document(), d->startPos, d->font,d->spaceY, d->alignHType, d->alignVType, sceneTransform(), d->layerIndex);
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
    return false;
}

QPointF LaserText::position() const
{
    return QPointF();
}

LaserPointListList LaserText::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserText);
    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(tr("%1 update machining points").arg(name()), parentProgress);
    int total = 0;
    for (int i = 0; i < d->pathList.size(); i++) {
        QList<QPainterPath> rowPathList = d->pathList[i].subRowPathlist();
        total += rowPathList.length();
    }
    progress->setMaximum(total);

    QTransform transform = sceneTransform();

    d->machiningPointsList.clear();
    d->startingIndices.clear();
    int totalPoints = 0;
    for (int i = 0; i < d->pathList.size(); i++) {
        QList<QPainterPath> rowPathList = d->pathList[i].subRowPathlist();
        for (QPainterPath rowPath : rowPathList) {
            QList<int> indices;
            LaserPointListList pointsList;
            QPoint center;
            machiningUtils::path2Points(nullptr, rowPath, pointsList, indices, center, transform);

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
            progress->increaseProgress();
        }
    }
    
    d->machiningCenter = transform.mapRect(path().boundingRect()).toRect().center();
    progress->finish();
    
    return d->machiningPointsList;
}

LaserLineListList LaserText::generateFillData(QPointF& lastPoint)
{
    Q_D(LaserText);
    QPainterPath path = sceneTransform().map(d->path);
    LaserLineListList lineList = utils::interLines(path, layer()->fillingRowInterval());
    return lineList;
}
class LaserStarPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserStar)
public:
    LaserStarPrivate(LaserStar* ptr)
        : LaserShapePrivate(ptr)
    {        
    }
    qreal radius;
    QPoint centerPoint;
    QPoint points[5];
};

LaserStar::LaserStar(LaserDocument * doc, QPoint centerPos, qreal radius, QTransform transform, int layerIndex)
    : LaserShape(new LaserStarPrivate(this), doc, LPT_STAR, layerIndex, transform)
{
    Q_D(LaserStar);
    d->radius = radius;
    d->centerPoint = centerPos;
    //计算圆上5个点
    qreal cos18 = qCos(M_PI/10);
    qreal sin18 = qSin(M_PI/10);
    qreal cos54 = qCos(M_PI*3 / 10);
    qreal sin54 = qSin(M_PI*3 / 10);
    d->points[0] = QPoint(d->centerPoint.x(), d->centerPoint.y() - d->radius);
    d->points[1] = QPoint(d->centerPoint.x() + d->radius * cos18,
        d->centerPoint.y() - d->radius * sin18);
    d->points[2] = QPoint(d->centerPoint.x() + d->radius * cos54, 
        d->centerPoint.y() + d->radius * sin54);
    d->points[3] = QPoint(d->centerPoint.x() - d->radius * cos54,
        d->centerPoint.y() + d->radius * sin54);
    d->points[4] = QPoint(d->centerPoint.x() - d->radius * cos18,
        d->centerPoint.y() - d->radius * sin18);
    //计算路径
    d->path.moveTo(d->points[0]);
    d->path.lineTo(d->points[2]);
    d->path.lineTo(d->points[4]);
    d->path.lineTo(d->points[1]);
    d->path.lineTo(d->points[3]);
    d->path.closeSubpath();

    d->boundingRect = d->path.boundingRect().toRect();
    d->originalBoundingRect = d->boundingRect;
    setTransform(transform);
}

LaserStar::~LaserStar()
{
}

void LaserStar::draw(QPainter * painter)
{
    Q_D(LaserStar);
    d->path.setFillRule(Qt::WindingFill);
    painter->setBrush(QBrush(this->layer()->color()));
    painter->drawPath(d->path);
    painter->setBrush(Qt::NoBrush);
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
    return object;
}

LaserPrimitive * LaserStar::clone(QTransform t)
{
    Q_D(LaserStar);
    LaserStar* star = new LaserStar(document(),d->centerPoint, d->radius, t, d->layerIndex);
    return star;
}

QVector<QLineF> LaserStar::edges()
{
    Q_D(LaserStar);
    return LaserPrimitive::edges(sceneTransform().map(d->path));
}

void LaserStar::updatePoints()
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
}

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
    LaserViewer* viewer = qobject_cast<LaserViewer*> (views[0]);
    viewer->onEndSelecting();
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

class LaserRingPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserRing)
public:
    LaserRingPrivate(LaserRing* ptr)
        : LaserShapePrivate(ptr)
    {
    }
    qreal outerRadius;
    QPoint centerPoint;
    qreal width;
    QRectF outerBounds;
    QRectF innerBounds;
    QPainterPath outerPath;
    QPainterPath innerPath;
};
LaserRing::LaserRing(LaserDocument* doc, QRectF outerRect, qreal width, QTransform transform, int layerIndex) 
    :LaserShape(new LaserRingPrivate(this), doc, LPT_RING, layerIndex, transform)
{
    Q_D(LaserRing);
    d->width = qAbs(width);
    d->outerBounds = outerRect;
    d->innerBounds = QRectF(QPointF(outerRect.topLeft().x() + d->width, outerRect.topLeft().y() + d->width), 
        QPointF(outerRect.bottomRight().x() - d->width, outerRect.bottomRight().y() - d->width));
    d->outerPath.addEllipse(d->outerBounds);
    d->innerPath.addEllipse(d->innerBounds);
    d->path = d->outerPath - d->innerPath;
    d->boundingRect = d->path.boundingRect().toRect();
    d->originalBoundingRect = d->boundingRect;
    setTransform(transform);
}
LaserRing::~LaserRing()
{
}
void LaserRing::draw(QPainter * painter)
{
    Q_D(LaserRing);
    painter->setBrush(QBrush(this->layer()->color()));
    painter->drawPath(d->path);
    painter->setBrush(Qt::NoBrush);
    
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
    return object;
}
LaserPrimitive * LaserRing::clone(QTransform t)
{
    Q_D(LaserRing);
    LaserRing* ring = new LaserRing(document(), d->outerBounds, d->width, t, d->layerIndex);
    return ring;
}
QVector<QLineF> LaserRing::edges()
{
    Q_D(LaserRing);
    qDebug()<<LaserPrimitive::edges(sceneTransform().map(d->path)).size();
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
    LaserViewer* viewer = qobject_cast<LaserViewer*> (views[0]);
    viewer->onEndSelecting();
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
qreal LaserRing::width() const
{
    Q_D(const LaserRing);
    return d->width;
}

class LaserFramePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserFrame)
public:
    LaserFramePrivate(LaserFrame* ptr)
        : LaserShapePrivate(ptr)
    {
    }
    QRect outerRect;
    QRect innerRect;
    qreal width;
    qreal cornerRadius;
    QPainterPath outerPath;
    QPainterPath innerPath;
    int cornerType;
};

LaserFrame::LaserFrame(LaserDocument* doc, QRect outerRect, qreal width, qreal cornnerRadilus,
    QTransform transform, int layerIndex, int cornerType)
    :LaserShape(new LaserFramePrivate(this), doc, LPT_FRAME, layerIndex, transform)
{
    Q_D(LaserFrame);
    d->outerRect = outerRect;
    d->width = width;
    d->cornerRadius = qAbs(cornnerRadilus);
    d->innerRect = QRect(QPoint(outerRect.topLeft().x() + width, outerRect.topLeft().y() + width), 
        QPoint(outerRect.bottomRight().x() - width, outerRect.bottomRight().y() - width));
    setCornerRadius(d->cornerRadius, cornerType);

    d->boundingRect = d->path.boundingRect().toRect();
    d->originalBoundingRect = d->boundingRect;
    setTransform(transform);
}

LaserFrame::~LaserFrame()
{
}
void LaserFrame::draw(QPainter * painter)
{
    Q_D(LaserFrame);
    painter->setBrush(QBrush(this->layer()->color()));
    painter->drawPath(d->path);
    painter->setBrush(Qt::NoBrush);
    //绘制辅助线
    painter->setPen(QPen(Qt::black, 280, Qt::DotLine));
    qreal halfW = d->innerRect.width() * 0.5;
    qreal halfH = d->innerRect.height() * 0.5;
    QLine hLine(QPoint(d->innerRect.center().x() - halfW, d->innerRect.center().y()),
        QPoint(d->innerRect.center().x() + halfW, d->innerRect.center().y()));
    QLine vLine(QPoint(d->innerRect.center().x(), d->innerRect.center().y() - halfH),
        QPoint(d->innerRect.center().x(), d->innerRect.center().y() + halfH));
    painter->drawLine(hLine);
    painter->drawLine(vLine);
    QPainterPath p; 
    p.addRect(d->innerRect);
    //painter->drawPath(p);

    
}
QJsonObject LaserFrame::toJson()
{
    return QJsonObject();
}
LaserPrimitive * LaserFrame::clone(QTransform t)
{
    Q_D(LaserFrame);
    LaserFrame* frame = new LaserFrame(document(), d->outerRect, d->width, d->cornerRadius, 
        t, d->layerIndex);
    return frame;
}
QVector<QLineF> LaserFrame::edges()
{
    Q_D(LaserFrame);
    return LaserPrimitive::edges(sceneTransform().map(d->path));
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
    LaserViewer* viewer = qobject_cast<LaserViewer*> (views[0]);
    viewer->onEndSelecting();
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
void LaserFrame::setCornerRadius(int cornerRadius, int type)
{
    Q_D(LaserFrame);
    d->cornerRadius = qAbs(cornerRadius);
    d->outerPath = computeCornerRadius(d->outerRect, d->cornerRadius, type);
    d->innerPath = computeCornerRadius(d->innerRect, d->cornerRadius, type);
    d->path = d->outerPath - d->innerPath;
}
QDebug operator<<(QDebug debug, const QRect& rect)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '[' << rect.topLeft() << ", " << rect.bottomRight()
        << ", " << rect.width() << "x" << rect.height() << "]";
    return debug;
}
class LaserCircleTextPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserCircleText)
public:
    LaserCircleTextPrivate(LaserCircleText* ptr)
        : LaserShapePrivate(ptr)
    { 
    }
    QString content;
    QSize size;
    qreal angle;
    QList<QPainterPath> textPathList;
};
LaserCircleText::LaserCircleText(LaserDocument* doc, QString content, QRect bounds,
    QSize size, qreal angle,
    QTransform transform, int layerIndex) 
    :LaserShape(new LaserCircleTextPrivate(this), doc, LPT_CIRCLETEXT, layerIndex, transform)
{
    Q_D(LaserCircleText);
    d->content = content;
    d->boundingRect = bounds;
    setTransform(transform);
    d->originalBoundingRect = d->boundingRect;
    computeTextPath();
    d->path.addRect(d->boundingRect);
}

LaserCircleText::~LaserCircleText()
{
}

void LaserCircleText::computeTextPath()
{
    Q_D(LaserCircleText);
    QFont font;
    font.setWordSpacing(0);
    font.setLetterSpacing(QFont::SpacingType::PercentageSpacing, 0);
    font.setPixelSize(d->boundingRect.height());
    for (int i = 0; i < d->content.size(); i ++) {
        QPainterPath path;
        QChar c = d->content[i];
        path.addText(d->boundingRect.left(), d->boundingRect.height(),
            font, c);
        qreal scaleX = d->boundingRect.width()/path.boundingRect().width() ;
        QTransform t1;
        t1.scale(scaleX, 1);
        path = t1.map(path);
        
        d->textPathList.append(path);
    }
}

void LaserCircleText::draw(QPainter * painter)
{
    Q_D(LaserCircleText);
    painter->drawRect(d->boundingRect);
    for (QPainterPath p : d->textPathList) {
        painter->drawPath(p);
        painter->drawRect(p.boundingRect());
    }
    painter->drawPath(d->path);
}

LaserPrimitive * LaserCircleText::clone(QTransform t)
{
    return nullptr;
}

QJsonObject LaserCircleText::toJson()
{
    return QJsonObject();
}

bool LaserCircleText::isClosed() const
{
    return false;
}

QPointF LaserCircleText::position() const
{
    return QPointF();
}

void LaserCircleText::setBoundingRectWidth(qreal width)
{
    Q_D(LaserCircleText);
    qreal diff = d->boundingRect.width() - width;
    d->boundingRect = QRect(d->boundingRect.x() + diff * 0.5, d->boundingRect.y(), 
        width, d->boundingRect.height());
    QPainterPath p;
    p.addRect(d->boundingRect);
    d->path = p;
}

void LaserCircleText::setBoundingRectHeight(qreal height)
{
    Q_D(LaserCircleText);
    qreal diff = d->boundingRect.height() - height;
    d->boundingRect = QRect(d->boundingRect.x(), d->boundingRect.y() + diff * 0.5,
        d->boundingRect.width(), height);
    QPainterPath p;
    p.addRect(d->boundingRect);
    d->path = p;
}

class LaserHorizontalTextPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserHorizontalText)
public:
    LaserHorizontalTextPrivate(LaserHorizontalText* ptr)
        : LaserShapePrivate(ptr)
    {
    }
    QString content;
    QSize size;
    QList<QPainterPath> textPathList;
};

LaserHorizontalText::LaserHorizontalText(LaserDocument* doc, QString content, QSize size,
    QTransform transform, int layerIndex) 
    :LaserShape(new LaserHorizontalTextPrivate(this), doc, LPT_HORIZONTALTEXT, layerIndex, transform)
{
    Q_D(LaserHorizontalText);
    d->content = content;
    setTransform(transform);
    d->size = size;
    initTextPath();
    d->boundingRect = d->path.boundingRect().toRect();
    d->originalBoundingRect = d->boundingRect;
    d->variableBounds = d->boundingRect;
}

LaserHorizontalText::~LaserHorizontalText()
{
}

void LaserHorizontalText::initTextPath()
{
    Q_D(LaserHorizontalText);
    QFont font;
    font.setWordSpacing(0);
    font.setLetterSpacing(QFont::SpacingType::PercentageSpacing, 0);
    font.setPixelSize(d->size.height());
    
    d->path = QPainterPath();
    for (int i = 0; i < d->content.size(); i++) {
        QPainterPath path;
        path.addText(0, 0, font, QString(d->content[i]));
        QTransform t;
        t.scale(d->size.width() / path.boundingRect().width(), 1);
        path = t.map(path);
        QTransform t1;
        qreal diffX = i * d->size.width();
        t1.translate(diffX, 0);
        path = t1.map(path);
        d->textPathList.append(path);
        d->path.addPath(path);
    }
    
}

void LaserHorizontalText::computeTextPath()
{
    Q_D(LaserHorizontalText);
    
    QPainterPath allPath;
    int index = 0;
    for (QPainterPath& path : d->textPathList) {
        qreal spaceX = (d->variableBounds.width() - d->size.width() * d->textPathList.size()) / (d->textPathList.size() - 1);
        qreal posX = d->variableBounds.left() + index * (d->size.width() + spaceX) ;
        //qreal posX = d->variableBounds.left();
        qreal lastPosX = path.boundingRect().left();
        QTransform t;
        t.translate(posX - lastPosX, 0);
        path = t.map(path);
        allPath.addPath(path);
        index++;
    }
    d->path = allPath;
    d->boundingRect = d->path.boundingRect().toRect();
}

void LaserHorizontalText::draw(QPainter * painter)
{
    Q_D(LaserHorizontalText);
    painter->drawRect(d->variableBounds);
    painter->drawPath(d->path);
}

LaserPrimitive * LaserHorizontalText::clone(QTransform t)
{
    return nullptr;
}

QJsonObject LaserHorizontalText::toJson()
{
    return QJsonObject();
}

bool LaserHorizontalText::isClosed() const
{
    return false;
}

QPointF LaserHorizontalText::position() const
{
    Q_D(const LaserHorizontalText);
    return QPointF();
}

void LaserHorizontalText::setBoundingRectWidth(qreal width)
{
    Q_D(LaserHorizontalText);
    qreal diff = d->variableBounds.width() - width;
    d->variableBounds = QRect(d->variableBounds.x() + diff * 0.5, d->variableBounds.y(),
        width, d->variableBounds.height());
   
    computeTextPath();
}
