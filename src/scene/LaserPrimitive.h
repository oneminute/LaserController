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
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
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
    LaserRect(const QRect rect, int cornerRadius, LaserDocument* doc, 
        QTransform transform = QTransform(), int layerIndex = 1, int cornerRadiusType = CRT_Round);
    virtual ~LaserRect() {}

    QRect rect() const;
    void setRect(const QRect& rect);

    int cornerRadius() const;
    int cornerType() const;
    void setCornerRadius(int cornerRadius, int type);
    bool isRoundedRect() const;

    virtual void draw(QPainter* painter);
    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);

	//virtual QRect sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();
	virtual LaserPrimitive* clone(QTransform t);
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
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
    void setRect(QRect rect);

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
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);

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
class LaserStampBasePrivate;
class LaserStampBase : public LaserShape {
    Q_OBJECT
public:
    LaserStampBase(LaserStampBasePrivate* ptr, LaserDocument* doc, LaserPrimitiveType type, bool stampIntaglio, QTransform transform,
        int layerIndex, int antiFakeType = 0, int antiFakeLine = 0, bool isAverageDistribute = false, qreal lineWidth = 0,
        bool surpassOuter = false, bool surpassInner = false, bool randomMove = false);
    ~LaserStampBase();
    void setFingerMap(QPixmap map);
    QPixmap& fingerMap();
    void setFingerMapDensity(qreal density);
    qreal fingerMapDensity();
    void setStampBrush(QPainter* painter, QColor color, QSize size, QTransform otherTransform = QTransform(), bool isResetColor = false);
    virtual void setAntiFakePath(QPainterPath path);
    bool stampIntaglio();
    virtual void setStampIntaglio(bool bl);
    int antiFakeType();
    void setAntiFakeType(int type);
    int antiFakeLine();
    void setAntiFakeLine(int count);
    bool isAverageDistribute();
    void setIsAverageDistribute(bool bl);
    qreal AntiFakeLineWidth();
    void setAntiFakeLineWidth(qreal width);
    bool surpassOuter();
    void setSurpassOuter(bool bl);
    bool surpassInner();
    void setSurpassInner(bool bl);
    bool randomMove();
    void setRandomMove(bool bl);
    void createAntiFakePath(int antiFakeType, int antiFakeLine, bool isAverageDistribute, qreal lineWidth,
        bool surpassOuter = false, bool surpassInner = false, bool randomMove = false);
    void createAntifakeLineByBounds();
    void createAntifakeLineByArc(qreal lineWidthRate);
    QPainterPath createBasePathByArc(qreal borderWidth, QLineF& baseLine);
    QPainterPath createCurveLine(QRectF bounds, qreal a, QLineF line);
    QPainterPath transformAntifakeLineByBounds(QPainterPath basePath, qreal intervalRate, qreal start, qreal end);
    QPainterPath transformAntifakeLineByArc(QPainterPath basePath, QLineF baseLine, qreal lineWidthRate, qreal startPercent);
protected:
    void stampBaseClone(LaserStampBase* sp);
    void stampBaseToJson(QJsonObject& object);

private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStampBase)
        Q_DISABLE_COPY(LaserStampBase)
};

class LaserStarPrivate;
class LaserStar : public LaserStampBase {
    Q_OBJECT
public:
    LaserStar(LaserDocument* doc, QPoint centerPos, qreal radius, bool stampIntaglio = false, QTransform transform = QTransform(),
        int layerIndex = 0);
    virtual ~LaserStar();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_STAR; }
    virtual QString typeName() { return tr("Star"); }
    virtual QJsonObject toJson();
    LaserPrimitive * clone(QTransform t);
    QVector<QLineF> edges();
    void computePath();
    qreal radius();
    //void updatePoints();
    //QPoint* points();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStar)
    Q_DISABLE_COPY(LaserStar)
};
class LaserPartyEmblemPrivate;
class LaserPartyEmblem : public LaserStampBase {
    Q_OBJECT
public:
    LaserPartyEmblem(LaserDocument* doc, QPoint centerPos, qreal radius, bool stampIntaglio = false, QTransform transform = QTransform(),
        int layerIndex = 0);
    virtual ~LaserPartyEmblem();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_PARTYEMBLEM; }
    virtual QString typeName() { return tr("PartyEmblem"); }
    virtual QJsonObject toJson();
    LaserPrimitive* clone(QTransform t);
    QVector<QLineF> edges();
    void computePath();
    qreal radius();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserPartyEmblem)
        Q_DISABLE_COPY(LaserPartyEmblem)
};
class LaserRingPrivate;
class LaserRing : public LaserStampBase {
    Q_OBJECT
public:
    LaserRing(LaserDocument* doc, QRectF outerRect, qreal width,bool stampIntaglio = false, QTransform transform = QTransform(),
        int layerIndex = 0);
    virtual ~LaserRing();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_RING; }
    virtual QString typeName() { return tr("Ring"); }
    virtual QJsonObject toJson();
    LaserPrimitive * clone(QTransform t);
    QVector<QLineF> edges();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    QRectF innerRect();
    QPainterPath outerPath();
    QPainterPath innerPath();
    
    void setInner(bool bl);
    bool isInner();
    void setBorderWidth(qreal w);
    qreal borderWidth();
    void computePath();
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserRing)
    Q_DISABLE_COPY(LaserRing)
};
class LaserFramePrivate;
class LaserFrame : public LaserStampBase {
    Q_OBJECT
public:
    //cornnerRadilus 为正是圆角，为副是内角
    //内角分用圆形且和用正方形切
    LaserFrame(LaserDocument* doc, QRect outerRect, qreal width, qreal cornerRadilus, bool stampIntaglio = false,
        QTransform transform = QTransform(),int layerIndex = 0, int cornerType = CRT_Round);
    virtual~LaserFrame();

    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_FRAME; }
    virtual QString typeName() { return tr("Frame"); }
    virtual QJsonObject toJson();
    LaserPrimitive * clone(QTransform t);
    QVector<QLineF> edges();
    void setInner(bool bl);
    bool isInner();
    QPainterPath outerPath();
    QPainterPath innerPath();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    void setCornerRadius(qreal cornerRadius, int type);
    qreal cornerRadius();
    int cornerRadiusType();
    QRectF innerRect();
    void setBorderWidth(qreal w);
    qreal borderWidth();
    void computePath();
    bool needAuxiliaryLine();
    void setNeedAuxiliaryLine(bool bl);
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);

private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserFrame)
    Q_DISABLE_COPY(LaserFrame)
};
//LaserCircleText, LaserHorizontalText, LaserVerticalText等印章文字的父类
class LaserStampTextPrivate;
class LaserStampText : public LaserStampBase {
    Q_OBJECT
public:
    LaserStampText(LaserStampTextPrivate* ptr, LaserDocument* doc, LaserPrimitiveType type, 
        QString content, QTransform transform = QTransform(), int layerIndex = 0, QSize size = QSize(), qreal space = 0, 
        bool bold = false, bool italic = false, bool uppercase = false, bool stampIntaglio = false, QString family = "Times New Roman", qreal weight = 0);
    virtual~LaserStampText();
    virtual void recompute() = 0;
    virtual void draw(QPainter* painter);
    void setContent(QString content);
    QString getContent();
    void setBold(bool bold);
    bool bold();
    void setWeight(qreal w);
    qreal weight();
    void setItalic(bool italic);
    bool italic();
    
    void setUppercase(bool uppercase);
    bool uppercase();
    void setFamily(QString family);
    QString family();
    qreal space();
    virtual void setTextHeight(qreal diff) = 0;
    virtual void setTextWidth(qreal diff) = 0;
    QSize textSize();
    virtual void setSpace(qreal space) = 0;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStampText)
    Q_DISABLE_COPY(LaserStampText)
};
class LaserCircleTextPrivate;
class LaserCircleText : public LaserStampText {
    Q_OBJECT
public:
    LaserCircleText(LaserDocument* doc, QString content, QRectF bounds, qreal angle,
        bool bold = false, bool italic = false, bool uppercase = false, bool stampIntaglio = false, QString family = "Times New Roman",qreal space = 0,
        bool isInit = true, qreal maxRadian = 0, qreal minRadian = 0, QSize size = QSize(), QTransform transform = QTransform(), int layerIndex = 0, qreal weight = 0);
    virtual ~LaserCircleText();
    void computeTextPath(qreal angle, QSize textSize,  bool needInit = true);
    //QPointF computeEllipsePoint(qreal rRadian);
    void translateText(QPointF& lastPoint, QPointF& curPoint, qreal interval, qreal index);
    QTransform scaleText(QPainterPath path);
    QTransform rotateText(int i,QPointF textPos);
    void transformText(QPainterPath path, QPointF textPos, int i);
    void transformTextByCenter(QPainterPath path, QPointF textPos, int i);
    QRectF textArcRect();
    void initAngle();
    void setAngle(qreal angle, bool needInit = true);
    void setOffsetRotateAngle(qreal offsetAngle);
    void setTextSize(QSize size, bool needInit = true);
    qreal mapToAffineCircleAngle(qreal radian);
    void moveTextToEllipse();
    void computeTextByPercent(int intervalCount);
    void computeMoveTextPath(qreal diffAngle);
    void computeChangeAngle(qreal angle);
    void resizeRadian();
    QPainterPath* textArc();
    qreal angle();
    //QPointF startPoint();
    //QPointF endPoint();
    //QPointF centerPoint();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_CIRCLETEXT; }
    virtual QString typeName() { return tr("CircleText"); }
    LaserPrimitive * clone(QTransform t);
    virtual QJsonObject toJson();
    QVector<QLineF> edges();

    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
    virtual void recompute();
    virtual void setSpace(qreal space);
    QRectF circleBounds();
    void setTextHeight(qreal height);
    void setTextWidth(qreal width);
    
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserCircleText)
        Q_DISABLE_COPY(LaserCircleText)
};

class LaserHorizontalTextPrivate;
class LaserHorizontalText : public LaserStampText {
    Q_OBJECT
public:
    LaserHorizontalText(LaserDocument* doc, QString content,QSize size,
        QPointF center, bool bold = false, bool italic = false, bool uppercase = false,bool stampIntaglio = false, QString family = "Times New Roman",
        qreal space = 0,  QTransform transform = QTransform(), int layerIndex = 0, qreal weight = 0);
    virtual ~LaserHorizontalText();
    //void initTextPath();
    void computeTextPathProcess();
    void computeTextPath();
    
    void toCenter();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_HORIZONTALTEXT; }
    virtual QString typeName() { return tr("HorizontalText"); }
    LaserPrimitive * clone(QTransform t);
    virtual QJsonObject toJson();
    QVector<QLineF> edges();
    virtual void recompute();
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setSpace(qreal space);
    void setTextHeight(qreal height);
    void setTextWidth(qreal width);
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserHorizontalText)
        Q_DISABLE_COPY(LaserHorizontalText)
};
class LaserVerticalTextPrivate;
class LaserVerticalText : public LaserStampText {
    Q_OBJECT
public:
    LaserVerticalText(LaserDocument* doc, QString content, QSize size,
        QPointF center,bool bold = false, bool italic = false, bool uppercase = false,bool stampIntaglio = false, QString family = "Times New Roman",
        qreal space = 0, QTransform transform = QTransform(), int layerIndex = 0, qreal weight = 0);
    virtual ~LaserVerticalText();
    void computeTextPathProcess();
    void computeTextPath();
    void toCenter();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_VERTICALTEXT; }
    virtual QString typeName() { return tr("VerticalText"); }
    LaserPrimitive * clone(QTransform t);
    virtual QJsonObject toJson();
    QVector<QLineF> edges();
    virtual void recompute();
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectHeight(qreal height);
    virtual void setSpace(qreal space);
    void setTextHeight(qreal height);
    void setTextWidth(qreal width);
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserVerticalText)
    Q_DISABLE_COPY(LaserVerticalText)
};
class LaserStampBitmapPrivate;
class LaserStampBitmap : public LaserStampBase {
    Q_OBJECT
public:
    LaserStampBitmap(const QImage& image, const QRect& bounds, bool stampIntaglio, LaserDocument* doc, QTransform transform = QTransform(),
        int layerIndex = 0);
    ~LaserStampBitmap();
    void computeImage(bool generateStamp = false);
    virtual void setStampIntaglio(bool bl);
    virtual bool isClosed() const { return true; };
    virtual LaserPrimitive* clone(QTransform t);
    virtual QJsonObject toJson();
    virtual void draw(QPainter* painter);
    void setOriginalImage(QImage image);
    void setFingerprint();
    void computeMask();
    void setBounds(QRect bounds);
    QImage generateStampImage();

    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
    virtual void setAntiFakePath(QPainterPath path);
    virtual void setAntiFakeImage(QImage image);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStampBitmap)
        Q_DISABLE_COPY(LaserStampBitmap)
};

QDebug operator<<(QDebug debug, const QRect& rect);

#endif // LASERPRIMITIVE