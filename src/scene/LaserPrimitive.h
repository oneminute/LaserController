#ifndef LASERPRIMITIVE_H
#define LASERPRIMITIVE_H

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
#include "LaserDocumentItem.h"
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

    LaserPrimitiveType primitiveType() const;
    QString typeName() const;
    QString typeLatinName() const;
    bool isShape() const;
    bool isBitmap() const;
    bool isText() const;

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

    virtual bool isAvailable() const;
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

class LaserShapePrivate;
class LaserShape : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserShape(LaserShapePrivate* data, LaserDocument* doc, LaserPrimitiveType type, int layerIndex = 1, QTransform transform = QTransform());
    virtual ~LaserShape() { } 
    virtual QByteArray filling(ProgressItem* progress, QPoint& lastPoint) override;
	int layerIndex();
private:
    Q_DISABLE_COPY(LaserShape);
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserShape);
};

class LaserEllipsePrivate;
class LaserEllipse : public LaserShape
{
    Q_OBJECT
public:
    LaserEllipse(const QRect bounds, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserEllipse() {}

    QRectF bounds() const;
    void setBounds(const QRect& bounds);

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

	//virtual QRect sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();
	LaserPrimitive * clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;

private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserEllipse)
    Q_DISABLE_COPY(LaserEllipse)
};

class LaserRectPrivate;
class LaserRect : public LaserShape
{
    Q_OBJECT
public:
    LaserRect(const QRect rect, int cornerRadius, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserRect() {}

    QRect rect() const;
    void setRect(const QRect& rect);

    int cornerRadius() const;
    void setCornerRadius(int cornerRadius);
    void concaveRect(QRect rect, QPainterPath& path, qreal cornerRadius);

    bool isRoundedRect() const;

    virtual void draw(QPainter* painter);
    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);

	//virtual QRect sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();
	virtual LaserPrimitive* clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserRect)
    Q_DISABLE_COPY(LaserRect)
};

class LaserLinePrivate;
class LaserLine : public LaserShape
{
    Q_OBJECT
public:
    LaserLine(const QLine& line, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserLine() {}

    QLine line() const;
    void setLine(const QLine& line);

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

	//virtual QRect sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();
	LaserPrimitive * clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
    Q_DISABLE_COPY(LaserLine);
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserLine);
};

class LaserPathPrivate;
class LaserPath : public LaserShape
{
    Q_OBJECT
public:
    LaserPath(const QPainterPath& path, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserPath() {}

    QPainterPath path() const;
    void setPath(const QPainterPath& path);

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

    virtual QList<QPainterPath> subPaths() const;
	//virtual QRect sceneBoundingRect() const;

    virtual QJsonObject toJson();

	//virtual void reShape();
	QVector<QLineF> edges();
	LaserPrimitive * clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPath);
    Q_DISABLE_COPY(LaserPath);
};

class LaserPolylinePrivate;
class LaserPolyline : public LaserShape
{
    Q_OBJECT
public:
    LaserPolyline(const QPolygon& poly, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserPolyline() {}

    QPolygon polyline() const;
    void setPolyline(const QPolygon& poly);
    //virtual QRect sceneBoundingRect() const;

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

	virtual QJsonObject toJson();
	QVector<QLineF> edges();
	LaserPrimitive * clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPolyline)
    Q_DISABLE_COPY(LaserPolyline)
};

class LaserPolygonPrivate;
class LaserPolygon : public LaserShape
{
    Q_OBJECT
public:
    LaserPolygon(const QPolygon& poly, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserPolygon() {}

    QPolygon polyline() const;
    void setPolyline(const QPolygon& poly);

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

	//virtual QRect sceneBoundingRect() const;

	virtual QJsonObject toJson();
	QVector<QLineF> edges();
	LaserPrimitive * clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPolygon)
    Q_DISABLE_COPY(LaserPolygon)
};

class LaserNurbsPrivate;
class LaserNurbs : public LaserShape
{
    Q_OBJECT
public:
    enum BasisType
    {
        BT_BEZIER,
        BT_BSPLINE
    };

    LaserNurbs(const QList<QPointF> controlPoints, const QList<qreal> knots, const QList<qreal> weights, BasisType basisType, LaserDocument* doc, 
		QTransform transform = QTransform(), int layerIndex = 1);
    ~LaserNurbs() {}

    virtual void draw(QPainter* painter);

	//virtual QRect sceneBoundingRect() const;

    void updateCurve();

	LaserPrimitive * clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserNurbs)
    Q_DISABLE_COPY(LaserNurbs)
};

class LaserBitmapPrivate;
class LaserBitmap : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserBitmap(const QImage& image, const QRect& bounds, LaserDocument* doc, QTransform transform = QTransform(), 
		int layerIndex = 0);
    virtual ~LaserBitmap() {}

    QImage image() const;
    void setImage(const QImage& image);

    QRectF bounds() const;

    virtual QByteArray engravingImage(ProgressItem* parentProgress, QPoint& lastPoint);
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_BITMAP; }
    virtual QString typeName() { return tr("Bitmap"); }
	virtual QJsonObject toJson();

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
	//virtual QRect sceneBoundingRect() const;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

	QVector<QLineF> edges();
	LaserPrimitive * clone(QTransform t);

    virtual bool isClosed() const;
    virtual QPointF position() const;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserBitmap)
    Q_DISABLE_COPY(LaserBitmap)
};
struct LaserTextRowPath {
public:
    LaserTextRowPath() {};
    ~LaserTextRowPath() {};

private:
    QPointF m_leftTop;
    QPainterPath m_path;
    QList<QPainterPath> m_subRowPath;
    QList<QRectF> m_boundList;
public:
    QPointF leftTopPosition() { return m_leftTop; };
    QPainterPath path() { return m_path; };
    QList<QPainterPath>& subRowPathlist() { return m_subRowPath; };
    QList<QRectF> subRowBoundList() { return m_boundList; };

    void setLeftTop(QPointF p) { m_leftTop = p; };
    void setPath(QPainterPath p) { m_path = p; };
    void setSubRowPathlist(QList<QPainterPath> l) { m_subRowPath = l; };
    void setSubRowBoundlist(QList<QRectF> l) { m_boundList = l; };
};
class LaserTextPrivate;
class LaserText : public LaserShape
{
	Q_OBJECT
public:
	LaserText(LaserDocument* doc, QPointF startPos, QFont font, qreal spaceY, int alightHType, int alightVType, QTransform transform = QTransform(),
		int layerIndex = 0);
    virtual ~LaserText();

    QRect rect() const;
    QString content() const;
    void setContent(QString c);
    QPainterPath path() const;
    QVector<QLineF> edges();
    void setFont(QFont font);
    QFont font();
    void setAlignH(int a);
    int alignH();
    void setAlignV(int a);
    int alignV();
    QPointF startPos();
    void setSaveTransform(QTransform t);
    QTransform saveTransform();
    //void setAlignType(int type);
    //int alignType();
    void insertContent(QString str, int index);
    //insertIndex,插入到的index
    void addPath(QString content, int insertIndex);
    void delPath(int index);

    qreal spaceY();
    void setSpacceY(qreal space);

    //void modifyLinePathList();
    void modifyPathList();
    QList<LaserTextRowPath> subPathList();

    //virtual QRectF boundingRect() const;
    //virtual QRect sceneBoundingRect() const;
    QRectF originalBoundingRect(qreal extendPixel = 0) const;
	virtual void draw(QPainter* painter);
	virtual LaserPrimitiveType type() { return LPT_TEXT; }
	virtual QString typeName() { return tr("Text"); }
	LaserPrimitive * clone(QTransform t);
    virtual QJsonObject toJson();

    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual LaserLineListList generateFillData(QPointF& lastPoint);

private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserText)
	Q_DISABLE_COPY(LaserText)
};

QDebug operator<<(QDebug debug, const QRect& rect);

#endif // LASERPRIMITIVE_H