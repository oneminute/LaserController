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
    LaserPrimitive(LaserPrimitivePrivate* data, LaserDocument* doc = nullptr, LaserPrimitiveType type = LPT_SHAPE);
    virtual ~LaserPrimitive();

    LaserDocument* document() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual QRectF boundingRect() const override;
    QPointF laserStartPos() const;

    virtual void draw(QPainter* painter) {};

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat()) { return std::vector<cv::Point2f>(); }
    std::vector<cv::Point2f> mechiningPoints(cv::Point2f& lastPoint, int pointIndex, cv::Mat& canvas = cv::Mat()) const;
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

    virtual QList<QPainterPath> subPaths() const { return QList<QPainterPath>(); }

    virtual QPainterPath outline() const;

protected:
    QString typeName(LaserPrimitiveType typeId) const;
    QString typeLatinName(LaserPrimitiveType typeId) const;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

protected:
    static QMap<int, int> g_itemsMaxIndex;

    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserPrimitive)
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
    LaserShape(LaserShapePrivate* data, LaserDocument* doc, LaserPrimitiveType type);
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
    LaserEllipse(const QRectF bounds, LaserDocument* doc);
    virtual ~LaserEllipse() {}

    QRectF bounds() const;
    void setBounds(const QRectF& bounds);

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserEllipse)
    Q_DISABLE_COPY(LaserEllipse)
};

class LaserRectPrivate;
class LaserRect : public LaserShape
{
    Q_OBJECT
public:
    LaserRect(const QRectF rect, LaserDocument* doc);
    virtual ~LaserRect() {}

    QRectF rect() const;
    void setRect(const QRectF& rect);

    virtual void draw(QPainter* painter);
    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());

    virtual QPainterPath toPath() const;

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserRect)
    Q_DISABLE_COPY(LaserRect)
};

class LaserLinePrivate;
class LaserLine : public LaserShape
{
    Q_OBJECT
public:
    LaserLine(const QLineF& line, LaserDocument* doc);
    virtual ~LaserLine() {}

    QLineF line() const;
    void setLine(const QLineF& line);

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

private:
    Q_DISABLE_COPY(LaserLine);
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserLine);
};

class LaserPathPrivate;
class LaserPath : public LaserShape
{
    Q_OBJECT
public:
    LaserPath(const QPainterPath& path, LaserDocument* doc);
    virtual ~LaserPath() {}

    QPainterPath path() const;
    void setPath(const QPainterPath& path);

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

    virtual QList<QPainterPath> subPaths() const;

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserPath);
    Q_DISABLE_COPY(LaserPath);
};

class LaserPolylinePrivate;
class LaserPolyline : public LaserShape
{
    Q_OBJECT
public:
    LaserPolyline(const QPolygonF& poly, LaserDocument* doc);
    virtual ~LaserPolyline() {}

    QPolygonF polyline() const;
    void setPolyline(const QPolygonF& poly);

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserPolyline)
    Q_DISABLE_COPY(LaserPolyline)
};

class LaserPolygonPrivate;
class LaserPolygon : public LaserShape
{
    Q_OBJECT
public:
    LaserPolygon(const QPolygonF& poly, LaserDocument* doc);
    virtual ~LaserPolygon() {}

    QPolygonF polyline() const;
    void setPolyline(const QPolygonF& poly);

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserPolygon)
    Q_DISABLE_COPY(LaserPolygon)
};

class LaserBitmapPrivate;
class LaserBitmap : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserBitmap(const QImage& image, const QRectF& bounds, LaserDocument* doc);
    virtual ~LaserBitmap() {}

    QImage image() const;
    void setImage(const QImage& image);

    QRectF bounds() const;

    virtual QByteArray engravingImage(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_BITMAP; }
    virtual QString typeName() { return tr("Bitmap"); }

private:
    Q_DECLARE_PRIVATE_D(LaserNode::d_ptr, LaserBitmap)
    Q_DISABLE_COPY(LaserBitmap)
};

class LaserTextPrivate;
class LaserText : public LaserPrimitive
{
	Q_OBJECT
public:
	LaserText(const QRect rect, const QString content, LaserDocument* doc, LaserPrimitiveType type);
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