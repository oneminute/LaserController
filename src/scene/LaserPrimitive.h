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
#include <QTextEdit>
#include <opencv2/opencv.hpp>

#include "LaserNode.h"

class LaserDocument;
class LaserLayer;
class LaserScene;
class QPaintEvent;
class LaserViewer;

struct Slice
{
    qreal y;
    qreal xStart;
    qreal xEnd;
    FillStyleAndPixelsCount fspc;
    QVector<quint8> pixels;

    void reverse()
    {
        qSwap(xStart, xEnd);
        std::reverse(pixels.begin(), pixels.end());
    }

    qreal left() const
    {
        if (xStart < xEnd)
            return xEnd;
        else
            return xStart;
    }

    qreal right() const
    {
        if (xStart > xEnd)
            return xStart;
        else
            return xEnd;
    }

    qreal length() const
    {
        return right() - left();
    }
};

struct SliceGroup
{
    QVector<Slice> slices;

    Slice& first() { return slices.first(); }
    Slice& last() { return slices.last(); }

    void append(Slice& slice)
    {
        qreal startDiff = qAbs(slice.xStart - last().xEnd);
        qreal endDiff = qAbs(slice.xEnd - last().xEnd);

        if (startDiff > endDiff)
        {
            slice.reverse();
        }

        slices.append(slice);
    }

    bool adjacent(const Slice& slice, qreal vInterval, qreal hInterval = 0.1f)
    {
        if (isEmpty())
            return false;

        // determine vertical interval
        if (slice.y - last().y > vInterval)
            return false;

        // determine horizontal interval
        qreal left = qMin(slice.left(), last().left());
        qreal right = qMin(slice.right(), last().right());

        if (right - left - (slice.length() + last().length()) > hInterval)
            return false;

        return true;
    }

    bool isEmpty() const { return slices.isEmpty(); }
};

class LaserPrimitivePrivate;
class LaserPrimitive : public QGraphicsObject, public LaserNode
{
    Q_OBJECT
public:
    LaserPrimitive(LaserPrimitivePrivate* data, LaserDocument* doc = nullptr, LaserPrimitiveType type = LPT_SHAPE, QTransform saveTransform = QTransform());
    virtual ~LaserPrimitive();

    LaserDocument* document() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	QTransform getAllTransform();
	QPainterPath getPath();
	QRectF originalBoundingRect() const;
	QPolygonF sceneOriginalBoundingPolygon(qreal extendPixel = 0);
    virtual QRectF boundingRect() const override;
    virtual QRectF sceneBoundingRect() const;
    QPointF laserStartPos() const;
	void sceneTransformToItemTransform(QTransform sceneTransform);//����sceneTransfromת��ΪItem��transform��position

    virtual void draw(QPainter* painter) {};

    virtual QVector<QPointF> cuttingPoints(cv::Mat& canvas = cv::Mat()) { return QVector<QPointF>(); }
    QVector<QPointF> mechiningPoints() const;
    QVector<QPointF> mechiningPoints(QPointF& lastPoint, int pointIndex, cv::Mat& canvas = cv::Mat()) const;
    QList<int> startingIndices() const;
    QVector<QPointF> startingPoints() const;
    virtual QByteArray engravingImage(cv::Mat& canvas = cv::Mat()) { return QByteArray(); }

    LaserPrimitiveType primitiveType() const;
    QString typeName() const;
    QString typeLatinName() const;
    bool isShape() const;
    bool isBitmap() const;

    QString name() const;
    void setName(const QString& name);

    LaserLayer* layer() const;
    void setLayer(LaserLayer* layer);

    QString toString() const;

    virtual QList<QPainterPath> subPaths() const { return QList<QPainterPath>(); }

    virtual QPainterPath outline() const;

	//virtual void reShape();
	void setData(QPainterPath path,
		QTransform allTransform,
		QTransform transform,
		QRectF boundingRect);

	virtual QJsonObject toJson();
	virtual QVector<QLineF> edges(QPainterPath path, bool isPolyline = false);
	virtual QVector<QLineF> edges();
	/*void setScaleValue(qreal x, qreal y);
	void setScaleTranslate(qreal x, qreal y);
	void setSelectedEditingState(int state);
	void setSelectedEditingMatrix(QMatrix mat);*/
protected:
    QString typeName(LaserPrimitiveType typeId) const;
    QString typeLatinName(LaserPrimitiveType typeId) const;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	
	
protected:
    static QMap<int, int> g_itemsMaxIndex;

    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserPrimitive)
    Q_DISABLE_COPY(LaserPrimitive)

    friend class LaserDocument;
	/*int m_selectedEditingState;
	qreal m_scaleX = 1;
	qreal m_scaleY = 1;
	qreal m_scaleTX = 0;
	qreal m_scaleTY = 0;
	QMatrix m_selectedEditingMatrix;*/
};

//Q_DECLARE_METATYPE(LaserPrimitive)

QDebug operator<<(QDebug debug, const LaserPrimitive& item);

class LaserShapePrivate;
class LaserShape : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserShape(LaserShapePrivate* data, LaserDocument* doc, LaserPrimitiveType type, QTransform transform = QTransform());
    virtual ~LaserShape() { } 
    virtual QPainterPath toPath() const = 0;
    virtual QByteArray engravingImage(cv::Mat& canvas = cv::Mat()) override;
	

private:
    Q_DISABLE_COPY(LaserShape);
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserShape);
};

class LaserEllipsePrivate;
class LaserEllipse : public LaserShape
{
    Q_OBJECT
public:
    LaserEllipse(const QRectF bounds, LaserDocument* doc, QTransform transform = QTransform());
    virtual ~LaserEllipse() {}

    QRectF bounds() const;
    void setBounds(const QRectF& bounds);

    virtual QVector<QPointF> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;
	virtual QRectF sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserEllipse)
    Q_DISABLE_COPY(LaserEllipse)
};

class LaserRectPrivate;
class LaserRect : public LaserShape
{
    Q_OBJECT
public:
    LaserRect(const QRectF rect, LaserDocument* doc, QTransform transform = QTransform());
    virtual ~LaserRect() {}

    QRectF rect() const;
    void setRect(const QRectF& rect);

    virtual void draw(QPainter* painter);
    virtual QVector<QPointF> cuttingPoints(cv::Mat& canvas = cv::Mat());

    virtual QPainterPath toPath() const;
	virtual QRectF sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();
private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserRect)
    Q_DISABLE_COPY(LaserRect)
};

class LaserLinePrivate;
class LaserLine : public LaserShape
{
    Q_OBJECT
public:
    LaserLine(const QLineF& line, LaserDocument* doc, QTransform transform = QTransform());
    virtual ~LaserLine() {}

    QLineF line() const;
    void setLine(const QLineF& line);

    virtual QVector<QPointF> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;
	virtual QRectF sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();

private:
    Q_DISABLE_COPY(LaserLine);
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserLine);
};

class LaserPathPrivate;
class LaserPath : public LaserShape
{
    Q_OBJECT
public:
    LaserPath(const QPainterPath& path, LaserDocument* doc, QTransform transform = QTransform());
    virtual ~LaserPath() {}

    QPainterPath path() const;
    void setPath(const QPainterPath& path);

    virtual QVector<QPointF> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

    virtual QList<QPainterPath> subPaths() const;
	virtual QRectF sceneBoundingRect() const;

	//virtual void reShape();
	QVector<QLineF> edges();

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserPath);
    Q_DISABLE_COPY(LaserPath);
};

class LaserPolylinePrivate;
class LaserPolyline : public LaserShape
{
    Q_OBJECT
public:
    LaserPolyline(const QPolygonF& poly, LaserDocument* doc, QTransform transform = QTransform());
    virtual ~LaserPolyline() {}

    QPolygonF polyline() const;
    void setPolyline(const QPolygonF& poly);
    virtual QRectF sceneBoundingRect() const;

    virtual QVector<QPointF> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserPolyline)
    Q_DISABLE_COPY(LaserPolyline)
};

class LaserPolygonPrivate;
class LaserPolygon : public LaserShape
{
    Q_OBJECT
public:
    LaserPolygon(const QPolygonF& poly, LaserDocument* doc, QTransform transform = QTransform());
    virtual ~LaserPolygon() {}

    QPolygonF polyline() const;
    void setPolyline(const QPolygonF& poly);

    virtual QVector<QPointF> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

	virtual QRectF sceneBoundingRect() const;

	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserPolygon)
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

    LaserNurbs(const QList<QPointF> controlPoints, const QList<qreal> knots, const QList<qreal> weights, BasisType basisType, LaserDocument* doc, QTransform transform = QTransform());
    ~LaserNurbs() {}

    virtual void draw(QPainter* painter);
    virtual QPainterPath toPath() const;

	virtual QRectF sceneBoundingRect() const;

    void updateCurve();

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserNurbs)
    Q_DISABLE_COPY(LaserNurbs)
};

class LaserBitmapPrivate;
class LaserBitmap : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserBitmap(const QImage& image, const QRectF& bounds, LaserDocument* doc, QTransform transform = QTransform());
    virtual ~LaserBitmap() {}

    QImage image() const;
    void setImage(const QImage& image);

    QRectF bounds() const;

    virtual QByteArray engravingImage(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_BITMAP; }
    virtual QString typeName() { return tr("Bitmap"); }
	virtual QJsonObject toJson();

    virtual QVector<QPointF> cuttingPoints(cv::Mat& canvas = cv::Mat());
	virtual QRectF sceneBoundingRect() const;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

	QVector<QLineF> edges();

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserBitmap)
    Q_DISABLE_COPY(LaserBitmap)
};

class LaserTextPrivate;
class LaserText : public LaserPrimitive
{
	Q_OBJECT
public:
	LaserText(const QRect rect, const QString content, LaserDocument* doc, LaserPrimitiveType type, QTransform transform = QTransform());
    virtual ~LaserText() {}

    QRect rect() const;
    QString content() const;

	virtual void draw(QPainter* painter);
	virtual LaserPrimitiveType type() { return LPT_TEXT; }
	virtual QString typeName() { return tr("Text"); }
private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserText)
	Q_DISABLE_COPY(LaserText)
};

#endif // LASERPRIMITIVE_H