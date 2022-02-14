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
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
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
    //计算矩形的内切角
    void concaveRect(QRect rect, QPainterPath& path, qreal cornerRadius, int type);
    QPainterPath computeCornerRadius(QRect rect, int cornerRadius, int type);
    virtual bool isAvailable() const;
    //circleText，horizontalText，verticalText中使用，方便改变外包框
    //QRect variableBounds();
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
    LaserRect(const QRect rect, int cornerRadius, LaserDocument* doc, 
        QTransform transform = QTransform(), int layerIndex = 1, int cornerRadiusType = CRT_Round);
    virtual ~LaserRect() {}

    QRect rect() const;
    void setRect(const QRect& rect);

    int cornerRadius() const;
    void setCornerRadius(int cornerRadius, int type);
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

class LaserStarPrivate;
class LaserStar : public LaserShape {
    Q_OBJECT
public:
    LaserStar(LaserDocument* doc, QPoint centerPos, qreal radius, QTransform transform = QTransform(),
        int layerIndex = 0);
    virtual ~LaserStar();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_STAR; }
    virtual QString typeName() { return tr("Star"); }
    virtual QJsonObject toJson();
    LaserPrimitive * clone(QTransform t);
    QVector<QLineF> edges();
    void updatePoints();
    QPoint* points();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual bool isClosed() const;
    virtual QPointF position() const;
    //virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    //virtual LaserLineListList generateFillData(QPointF& lastPoint);
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStar)
    Q_DISABLE_COPY(LaserStar)
};
class LaserRingPrivate;
class LaserRing : public LaserShape {
    Q_OBJECT
public:
    LaserRing(LaserDocument* doc, QRectF outerRect, qreal width, QTransform transform = QTransform(),
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
    void setBorderWidth(qreal w);
    qreal borderWidth();
    void computePath();
    //virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    //virtual LaserLineListList generateFillData(QPointF& lastPoint);
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserRing)
    Q_DISABLE_COPY(LaserRing)
};
class LaserFramePrivate;
class LaserFrame : public LaserShape {
    Q_OBJECT
public:
    //cornnerRadilus 为正是圆角，为副是内角
    //内角分用圆形且和用正方形切
    LaserFrame(LaserDocument* doc, QRect outerRect, qreal width, qreal cornerRadilus,
        QTransform transform = QTransform(),int layerIndex = 0, int cornerType = CRT_Round);
    virtual~LaserFrame();

    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_FRAME; }
    virtual QString typeName() { return tr("Frame"); }
    virtual QJsonObject toJson();
    LaserPrimitive * clone(QTransform t);
    QVector<QLineF> edges();
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
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserFrame)
    Q_DISABLE_COPY(LaserFrame)
};
//LaserCircleText, LaserHorizontalText, LaserVerticalText等印章文字的父类
class LaserStampTextPrivate;
class LaserStampText : public LaserShape {
    Q_OBJECT
public:
    LaserStampText(LaserStampTextPrivate* ptr, LaserDocument* doc, LaserPrimitiveType type, 
        QString content, QTransform transform = QTransform(), int layerIndex = 0, QSize size = QSize(), qreal space = 0, 
        bool bold = false, bool italic = false, bool uppercase = false, QString family = "Times New Roman");
    virtual~LaserStampText();
    virtual void recompute() = 0;
    void setContent(QString content);
    QString getContent();
    void setBold(bool bold);
    bool bold();
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
        bool bold = false, bool italic = false, bool uppercase = false, QString family = "Times New Roman",
        bool isInit = true, qreal maxRadian = 0, qreal minRadian = 0, QSize size = QSize(), QTransform transform = QTransform(), int layerIndex = 0);
    virtual ~LaserCircleText();
    void computeTextPath(qreal angle, QSize textSize,  bool needInit = true);
    QPointF computeEllipsePoint(qreal rRadian);
    void translateText(QPointF& lastPoint, QPointF& curPoint, qreal interval, qreal index);
    QTransform scaleText(QPainterPath path);
    QTransform rotateText(int i,QPointF textPos);
    void transformText(QPainterPath path, QPointF textPos, int i);
    void transformTextByCenter(QPainterPath path, QPointF textPos, int i);
    QRectF textArcRect();
    void initAngle();
    void setAngle(qreal angle, bool needInit = true);
    void setTextSize(QSize size, bool needInit = true);
    qreal mapToAffineCircleAngle(qreal radian);
    void moveTextToEllipse(qreal lengthByPercent);
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
        QPointF bottomLeft, bool bold = false, bool italic = false, bool uppercase = false, QString family = "Times New Roman",
        qreal space = 0,  QTransform transform = QTransform(), int layerIndex = 0);
    virtual ~LaserHorizontalText();
    void initTextPath();
    void computeTextPathProcess();
    void computeTextPath();
    
    void toBottomLeft();
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
        QPointF topLeft,bool bold = false, bool italic = false, bool uppercase = false, QString family = "Times New Roman",
        qreal space = 0, QTransform transform = QTransform(), int layerIndex = 0);
    virtual ~LaserVerticalText();
    void initTextPath();
    void computeTextPath();
    void toTopLeft();
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

QDebug operator<<(QDebug debug, const QRect& rect);

#endif // LASERPRIMITIVE_H