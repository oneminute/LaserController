#pragma once

#include "common/common.h"
#include <QObject>
#include <QSharedDataPointer>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QTransform>
#include <QGraphicsObject>
#include <QPolygonF>
#include <QPainterPath>
#include <QVector>
#include <QVector4D>
#include <QTextEdit>
#include "scene/LaserDocumentItem.h"
#include "laser/LaserPointList.h"
#include "laser/LaserLineList.h"

class LaserDocument;
class LaserLayer;
class LaserScene;
class QPaintEvent;
class LaserViewer;
class QuadTreeNode;
class ProgressItem;

class LaserPrimitivePrivate;
class LaserPrimitive : public QGraphicsObject, public ILaserDocumentItem
{
    Q_OBJECT
public:
    LaserPrimitive(LaserPrimitivePrivate* data, LaserDocument* doc = nullptr, 
		LaserPrimitiveType type = LPT_SHAPE, QTransform saveTransform = QTransform(), int layerIndex = 0);
    virtual ~LaserPrimitive();

    LaserDocument* document() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
	int layerIndex();
    QPainterPath getPath();
    virtual QPainterPath getPathForStamp();
	QPainterPath getScenePath();
    
	/// <summary>
	/// [obsolete]
	/// </summary>
	/// <param name="extendPixel"></param>
	/// <returns></returns>
	QRectF originalBoundingRect(qreal extendPixel = 0) const;
	QPolygonF sceneOriginalBoundingPolygon(qreal extendPixel = 0);
    virtual QRectF boundingRect() const override;
    //virtual QPainterPath shape() const override;
    virtual QRect sceneBoundingRect() const;
	void sceneTransformToItemTransform(QTransform sceneTransform);

    virtual void draw(QPainter* painter) {};

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress) { return LaserPointListList(); }
    LaserPointListList machiningPoints() const;
    virtual LaserPointListList arrangeMachiningPoints(LaserPoint& fromPoint, int startingIndex);
    LaserPointListList arrangedPoints() const;
    virtual LaserLineListList generateFillData();
    LaserPoint arrangedStartingPoint() const;
    LaserPoint arrangedEndingPoint() const;
    QList<int> startingIndices() const;
    LaserPointList startingPoints() const;
    LaserPoint firstStartingPoint() const;
    LaserPoint lastStartingPoint() const;
    QPointF centerMachiningPoint() const;
    virtual QByteArray engravingImage(ProgressItem* parentProgress, QPoint& lastPoint) { return QByteArray(); }
    virtual QByteArray filling(ProgressItem* parentProgress, QPoint& lastPoint) { return QByteArray(); }
    virtual bool isClosed() const = 0;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
    LaserPrimitiveType primitiveType() const;
    QString typeName() const;
    QString typeLatinName() const;
    bool isShape() const;
    bool isBitmap() const;
    bool isText() const;
    bool isStamp() const;

    bool exportable() const;
    void setExportable(bool value);
    bool visible() const;
    void setVisible(bool value);
    bool isAlignTarget();
    void setAlignTarget(bool value);

    LaserLayer* layer() const;
    void setLayer(LaserLayer* layer, bool whenNullLayerKeepIndex = true);

    QString toString() const;

    virtual QPainterPath outline() const;

	//virtual void reShape();
	void setData(QPainterPath path,
		QTransform allTransform,
		QTransform transform,
		QRect boundingRect);

	virtual QJsonObject toJson();
	virtual QVector<QLineF> edges(QPainterPath path, bool isPolyline = false);
	virtual QVector<QLineF> edges();
	virtual LaserPrimitive* clone(QTransform t) = 0;

    virtual QPointF position() const;

    static QString typeName(LaserPrimitiveType typeId);
    static QString typeLatinName(LaserPrimitiveType typeId);

    void setLocked(bool isLocked);
    bool isLocked();
    void setJoinedGroup(QSet<LaserPrimitive*>* joinedGroup);
    bool isJoinedGroup();
    QSet<LaserPrimitive*>* joinedGroupList();
    QList<QuadTreeNode*>& treeNodesList();
    void addTreeNode(QuadTreeNode* node);
    void removeAllTreeNode();
    void removeOneTreeNode(QuadTreeNode* node);
    //计算矩形的内切角
    void concaveRect(QRect rect, QPainterPath& path, qreal cornerRadius, int type);
    QPainterPath computeCornerRadius(QRect rect, int cornerRadius, int type);
    virtual bool isAvailable() const;
    virtual int smallCircleIndex() const;

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    
private:
    QRect m_joinedGroupSceneBoundingRect;
	
protected:

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPrimitive)
    Q_DISABLE_COPY(LaserPrimitive)

    friend class LaserDocument;
};

//Q_DECLARE_METATYPE(LaserPrimitive)

QDebug operator<<(QDebug debug, const LaserPrimitive& item);
QDebug operator<<(QDebug debug, const QRect& rect);
