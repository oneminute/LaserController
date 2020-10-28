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

#include <opencv2/opencv.hpp>

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

class LaserPrimitive : public QGraphicsObject
{
    Q_OBJECT
public:
    LaserPrimitive(LaserDocument* doc = nullptr, LaserPrimitiveType type = LPT_SHAPE, SizeUnit unit = SizeUnit::SU_MM100);
    virtual ~LaserPrimitive();

    LaserDocument* document() const { return m_doc; }
    SizeUnit unit() const { return m_unit; }
    QTransform extraTransform() { return m_transform; }
    void setTransform(const QTransform& transform) { m_transform = transform; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual QRectF boundingRect() const override;
    QPointF laserStartPos() const;

    virtual void draw(QPainter* painter) {};

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat()) { return std::vector<cv::Point2f>(); }
    virtual QByteArray engravingImage(cv::Mat& canvas = cv::Mat()) { return QByteArray(); }

    qreal unitToMM() const;

    LaserPrimitiveType laserItemType() const { return m_type; }
    QString typeName();
    QString typeLatinName();
    bool isShape() const { return (int)m_type <= (int)LPT_SHAPE; }
    bool isBitmap() const { return m_type == LPT_BITMAP; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    LaserLayer* layer() const { return m_layer; }
    void setLayer(LaserLayer* layer) { m_layer = layer; }

protected:
    QString typeName(LaserPrimitiveType typeId);
    QString typeLatinName(LaserPrimitiveType typeId);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

protected:
    LaserDocument* m_doc;
    LaserLayer* m_layer;
    SizeUnit m_unit;
    QTransform m_transform;
    QRectF m_boundingRect;
    LaserPrimitiveType m_type;
    QString m_name;

    bool m_isHover;

    static QMap<int, int> g_itemsMaxIndex;

private:
    Q_DISABLE_COPY(LaserPrimitive);

    friend class LaserDocument;
};

//Q_DECLARE_METATYPE(LaserPrimitive)

QDebug operator<<(QDebug debug, const LaserPrimitive& item);

class LaserShape : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserShape(LaserDocument* doc, LaserPrimitiveType type, SizeUnit unit = SizeUnit::SU_MM100);
    virtual QPainterPath toPath() const = 0;
    virtual QByteArray engravingImage(cv::Mat& canvas = cv::Mat()) override;

private:
    Q_DISABLE_COPY(LaserShape);
};

class LaserEllipse : public LaserShape
{
    Q_OBJECT
public:
    LaserEllipse(const QRectF bounds, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);
    virtual ~LaserEllipse() {}

    QRectF bounds() const { return m_bounds; }
    void setBounds(const QRectF& bounds) { m_bounds = bounds; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

private:
    QRectF m_bounds;
    Q_DISABLE_COPY(LaserEllipse)
};

class LaserRect : public LaserShape
{
    Q_OBJECT
public:
    LaserRect(const QRectF rect, LaserDocument* doc,  SizeUnit unit = SizeUnit::SU_MM100);

    QRectF rect() const { return m_rect; }
    void setRect(const QRectF& rect) { m_rect = rect; }

    virtual void draw(QPainter* painter);
    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());

    virtual QPainterPath toPath() const;

private:
    QRectF m_rect;
    Q_DISABLE_COPY(LaserRect);
};

class LaserLine : public LaserShape
{
    Q_OBJECT
public:
    LaserLine(const QLineF& line, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QLineF line() const { return m_line; }
    void setLine(const QLineF& line) { m_line = line; }

    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

private:
    QLineF m_line;
    Q_DISABLE_COPY(LaserLine);
};

class LaserPath : public LaserShape
{
    Q_OBJECT
public:
    LaserPath(const QPainterPath& path, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QPainterPath path() const { return m_path; }
    void setPath(const QPainterPath& path) { m_path = path; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

private:
    QPainterPath m_path;
    Q_DISABLE_COPY(LaserPath);
};

class LaserPolyline : public LaserShape
{
    Q_OBJECT
public:
    LaserPolyline(const QPolygonF& poly, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QPolygonF polyline() const { return m_poly; }
    void setPolyline(const QPolygonF& poly) { m_poly = poly; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

private:
    QPolygonF m_poly;
    Q_DISABLE_COPY(LaserPolyline);
};

class LaserPolygon : public LaserShape
{
    Q_OBJECT
public:
    LaserPolygon(const QPolygonF& poly, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QPolygonF polyline() const { return m_poly; }
    void setPolyline(const QPolygonF& poly) { m_poly = poly; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);

    virtual QPainterPath toPath() const;

private:
    QPolygonF m_poly;
    Q_DISABLE_COPY(LaserPolygon);
};

class LaserBitmap : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserBitmap(const QImage& image, const QRectF& bounds, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QImage image() const { return m_image; }
    void setImage(const QImage& image) { m_image = m_image; }

    QRectF bounds() const { return m_bounds; }

    virtual QByteArray engravingImage(cv::Mat& canvas = cv::Mat());
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_BITMAP; }
    virtual QString typeName() { return tr("Bitmap"); }

private:
    QImage m_image;
    QRectF m_bounds;
    Q_DISABLE_COPY(LaserBitmap);
};

#endif // LASERPRIMITIVE_H