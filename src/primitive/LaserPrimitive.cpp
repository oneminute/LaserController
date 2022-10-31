#include "LaserPrimitive.h"
#include "LaserPrimitivePrivate.h"
#include "LaserPrimitiveGroup.h"

#include <iostream>

#include <QBitmap>
#include <QBuffer>
#include <QGraphicsTextItem> 
#include <QGraphicsSceneMouseEvent>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainterPath>
#include <QPaintEvent>
#include <QSharedData>
#include <QStack>
#include <QTextEdit>
#include <QtMath>

#include <opencv2/opencv.hpp>

#include "LaserApplication.h"
#include "algorithm/QuadTreeNode.h"
#include "common/common.h"
#include "common/Config.h"
#include "laser/LaserDriver.h"
#include "laser/LaserDevice.h"
#include "laser/LaserLineList.h"
#include "primitive/LaserPrimitiveHeaders.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "state/StateController.h"
#include "task/ProgressItem.h"
#include "task/ProgressModel.h"
#include "ui/LaserControllerWindow.h"
#include "util/ImageUtils.h"
#include "util/MachiningUtils.h"
#include "util/TypeUtils.h"
#include "util/UnitUtils.h"
#include "util/Utils.h"
#include "widget/LaserDoubleSpinBox.h"
#include "widget/LaserViewer.h"


void setSelectedInGroup(bool selected) {

}

LaserPrimitive::LaserPrimitive(LaserPrimitivePrivate* data, LaserDocument* doc, 
    LaserPrimitiveType type, QTransform saveTransform, int layerIndex)
    : ILaserDocumentItem(LNT_PRIMITIVE, data)
{
    Q_D(LaserPrimitive);
    d->doc = doc;
    d->primitiveType = type;
    QObject::setParent(doc);

    this->setFlag(ItemIsSelectable, true);
    d->name = doc->newPrimitiveName(this);
	d->allTransform = saveTransform;
	d->layerIndex = layerIndex;

    qLogD << "primitive " << id() << "created. " << typeName();
}

LaserPrimitive::~LaserPrimitive()
{
    Q_D(LaserPrimitive);
    qLogD << "primitive " << id() << "deleted";
}

LaserDocument* LaserPrimitive::document() const 
{
    Q_D(const LaserPrimitive);
    return d->doc; 
}

//PolyLine or Polygon
QVector<QLine> LaserPrimitive::edges(QPainterPath path, bool isPolyline)
{
	QPolygon polygon = path.toFillPolygon().toPolygon();
	QVector<QLine> edgeList;
	for (int i = 0; i < polygon.count()-1; i++) {
		
		QLine edge;
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

QVector<QLine> LaserPrimitive::edges()
{
	return QVector<QLine>();
}

void LaserPrimitive::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_D(LaserPrimitive);
    QGraphicsScene* scene = this->scene();
    if (!scene)
    {
        return;
    }
    QString className = scene->metaObject()->className();
    if (className == "LaserScene") {
        if (!visible())
            return;
    }

    painter->save();
    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);

    QRectF bounds = boundingRect();
    QPointF topLeft = bounds.topLeft() - QPointF(2, 2);
    QPointF bottomRight = QPoint(bounds.left() + bounds.width(), bounds.top() + bounds.height()) + QPointF(2, 2);
    QRectF selectionBounds = QRectF(topLeft, bottomRight);
    QColor color = Qt::blue;
    QPen pen(color, 1, Qt::DashLine);

    QPainterPath outline = this->outline();
    QPointF startPos = outline.pointAtPercent(0);
    QPen greenPen(Qt::green, 1, Qt::SolidLine);
    greenPen.setCosmetic(true);
    painter->setPen(greenPen);
    if (Config::Debug::showPrimitiveName())
    {
        QFont font = painter->font();
        font.setPointSize(5000);
        painter->setFont(font);
        painter->drawText(startPos, name());
        QColor color = Qt::blue;
        QPen pen(color, 1, Qt::DashLine);
        painter->drawRect(bounds);
    }
    if (Config::Debug::showPrimitiveFirstPoint())
        painter->drawEllipse(startPos, 2, 2);

    LaserLayer* layer = this->layer();
    if (layer)
    {
        color = layer->color();
        if (!exportable())
        {
            color = Qt::lightGray;
        }
    }
    else {
        color = Qt::transparent;
    }

    if (isSelected())
    {
        //isSelected();
        QString name = this->metaObject()->className();
        qreal penSize = 1;
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

    if (layer && layer->type() == LLT_FILLING)
    {
        QBrush brush(color, Qt::SolidPattern);
        painter->setBrush(brush);
    }
    if (className == "QGraphicsScene") {
        QBrush brush(color, Qt::SolidPattern);
        painter->setBrush(brush);
    }
    if (StateControllerInst.isInState(StateControllerInst.documentPrintAndCutAligningState()))
    {
        //QPointF center = d->boundingRect.center();
        //QTransform sceneInverted = sceneTransform().inverted();
        //QTransform transformInverted = transform().inverted();
        //QTransform t;
        //t *= QTransform::fromTranslate(-center.x(), -center.y());
        //t *= document()->transform();
        //t *= sceneInverted;
        //t *= transformInverted;
        //t *= QTransform::fromTranslate(center.x(), center.y());
        //t *= painter->transform();
        //painter->setTransform(t);
    }

    draw(painter);
    painter->restore();
}

int LaserPrimitive::layerIndex()
{
	Q_D(const LaserPrimitive);
	return d->layerIndex;
}

bool LaserPrimitive::isEditing() const
{
    Q_D(const LaserPrimitive);
    return d->isEditing;
}

void LaserPrimitive::setEditing(bool editing)
{
    Q_D(LaserPrimitive);
    d->isEditing = editing;
}

QUndoCommand* LaserPrimitive::beginCreating(const QString& commandName, 
    LaserViewer* viewer, LaserScene* scene, const QPoint& position)
{
    return nullptr;
}

void LaserPrimitive::endCreating(const QString& commandName,
    LaserViewer* viewer, LaserScene* scene)
{
}

QPainterPath LaserPrimitive::getPath()
{
    Q_D(const LaserPrimitive);
    return d->path;
}

QPainterPath LaserPrimitive::getPathForStamp()
{
    Q_D(LaserPrimitive);
    QPainterPath pathForStamp;
    QPointF center = d->path.boundingRect().center();
    QTransform t;
    t.scale(-1, 1);
    pathForStamp = t.map(d->path);
    QPointF center1 = pathForStamp.boundingRect().center();
    QTransform t1;
    t1.translate(center.x() - center1.x(), center.y() - center1.y());
    pathForStamp = t1.map(pathForStamp);

    return pathForStamp;
}

QPainterPath LaserPrimitive::getScenePath()
{
    Q_D(const LaserPrimitive);
    return sceneTransform().map(d->path);
}

QPolygon LaserPrimitive::sceneOriginalBoundingPolygon(qreal extendPixel)
{
	QPolygon bounding = sceneTransform().map(QRectF(boundingRect())).toPolygon();
	
	if (extendPixel != 0) {
		QVector2D p12 = QVector2D(bounding[1] - bounding[0]).normalized() * extendPixel;
		QVector2D p23 = QVector2D(bounding[2] - bounding[1]).normalized() * extendPixel;
		QVector2D p34 = QVector2D(bounding[3] - bounding[2]).normalized() * extendPixel;
		QVector2D p41 = QVector2D(bounding[0] - bounding[3]).normalized() * extendPixel;

		QPoint newP1 = bounding[0] - p12.toPoint() + p41.toPoint();
		QPoint newP2 = bounding[1] + p12.toPoint() - p23.toPoint();
		QPoint newP3 = bounding[2] - p34.toPoint() + p23.toPoint();
		QPoint newP4 = bounding[3] + p34.toPoint() - p41.toPoint();

		QPolygon newBounding;
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

void LaserPrimitive::setAllTransform(const QTransform& t)
{
    Q_D(LaserPrimitive);
    d->allTransform = t;
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

            int step = 1;
            /*if (fromPoint.angle1() >= 0)
            {
                step = -1;
            }
            else
            {
                step = 1;
            }*/

            int cursor = pointIndex;
            points.reserve(pointsCount);
            LaserPoint lastPoint = firstPoint;
            points.push_back(firstPoint);
            for (int i = 1; i < machiningPoints.length(); i++)
            {
                cursor = (cursor + step + machiningPoints.length()) % machiningPoints.length();
                LaserPoint& currentPoint = machiningPoints[cursor];
                if (lastPoint != currentPoint)
                    points.push_back(currentPoint);
                lastPoint = currentPoint;
            }
            if (lastPoint != firstPoint)
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
    return d->primitiveType == LPT_LINE ||
        d->primitiveType == LPT_CIRCLE ||
        d->primitiveType == LPT_ELLIPSE ||
        d->primitiveType == LPT_RECT ||
        d->primitiveType == LPT_POLYLINE ||
        d->primitiveType == LPT_POLYGON ||
        d->primitiveType == LPT_PATH ||
        d->primitiveType == LPT_NURBS;
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

bool LaserPrimitive::isStamp() const
{
    Q_D(const LaserPrimitive);
    return d->primitiveType == LPT_STAR ||
        d->primitiveType == LPT_PARTYEMBLEM ||
        d->primitiveType == LPT_FRAME ||
        d->primitiveType == LPT_RING ||
        d->primitiveType == LPT_CIRCLETEXT ||
        d->primitiveType == LPT_HORIZONTALTEXT ||
        d->primitiveType == LPT_VERTICALTEXT;
}

LaserPrimitive* LaserPrimitive::clone()
{
    LaserPrimitive* primitive = cloneImplement();
    primitive->setId(this->id());
    primitive->setName(this->name());
    primitive->setLayer(this->layer());
    primitive->setEditing(this->isEditing());
    return primitive;
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
    LaserLayer* layer = d->doc->layerById(d->layerId);
    if (!layer)
        layer = d->doc->layerByIndex(d->layerIndex);
    return layer;
}

void LaserPrimitive::setLayer(LaserLayer* layer) 
{
    Q_D(LaserPrimitive);
    
    //d->layer = layer;
	if (layer) {
        
		d->layerIndex = layer->index();
        d->layerId = layer->id();
        //d->layer->primitives().append(this);
	}
	else {
        //如果保留原有的layerIndex，删除后可以再恢复回来
        //如果重置为-1，则删除后需要重新添加到层中
        //if (!whenNullLayerKeepIndex) {
        d->layerIndex = -1;
        d->layerId = QString();
        //}
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

int LaserPrimitive::smallCircleIndex() const
{
    if (!Config::Export::enableSmallDiagonal())
        return -1;
    QSize boundingSize = sceneBoundingRect().size();
    int length = boundingSize.width() >= boundingSize.height() ? boundingSize.width() : boundingSize.height();
    int index = Config::Export::smallDiagonalLimitation()->indexOf(length / 1000.0);
    return index;
}

LaserPrimitive* LaserPrimitive::createPrimitive(LaserPrimitiveType type,
    LaserDocument* doc)
{
    LaserPrimitive* primitive = nullptr;
    LaserLayer* layer = nullptr;
    layer = doc->getCurrentOrCapableLayer(type);
    int layerIndex = layer->index();
    if (layer)
    {
        switch (type)
        {
        case LPT_LINE:
            primitive = new LaserLine(doc, QTransform(), layerIndex);
            break;
        case LPT_ELLIPSE:
            primitive = new LaserEllipse(doc, QTransform(), layerIndex);
            break;
        case LPT_RECT:
            primitive = new LaserRect(doc, QTransform(), layerIndex);
            break;
        case LPT_POLYLINE:
            primitive = new LaserPolyline(doc, QTransform(), layerIndex);
            break;
        case LPT_POLYGON:
            primitive = new LaserPolygon(doc, QTransform(), layerIndex);
            break;
        case LPT_PATH:
            primitive = new LaserPath(doc, QTransform(), layerIndex);
            break;
        case LPT_BITMAP:
            primitive = new LaserBitmap(doc, QTransform(), layerIndex);
            break;
        case LPT_TEXT:
        {
            primitive = new LaserText(doc, QTransform(), layerIndex);
            break;
        }
        default:
            primitive = nullptr;
            break;
        }
    }
    return primitive;
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

void LaserPrimitive::sceneMousePressEvent(
    LaserViewer* viewer,
    LaserScene* scene, 
    const QPoint& point,
    QMouseEvent* event)
{
}

void LaserPrimitive::sceneMouseMoveEvent(
    LaserViewer* viewer,
    LaserScene* scene, 
    const QPoint& point,
    QMouseEvent* event,
    bool isPressed)
{
}

//void LaserPrimitive::sceneMouseReleaseEvent(
//    LaserViewer* viewer,
//    LaserScene* scene, 
//    const QPoint& point,
//    QMouseEvent* event,
//    bool isPressed)
//{
//}

void LaserPrimitive::sceneKeyPressEvent(
    LaserViewer* viewer,
    QKeyEvent* event)
{
}

void LaserPrimitive::sceneKeyReleaseEvent(
    LaserViewer* viewer,
    QKeyEvent* event)
{
}

QDebug operator<<(QDebug debug, const LaserPrimitive & item)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "[name = " << item.name() << ", type = " << item.primitiveType() << "]";
    return debug;
}




