#ifndef LASERITEM_H
#define LASERITEM_H

#include "common/common.h"
#include <QObject>
#include <QSharedDataPointer>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QTransform>
#include <QGraphicsObject>
#include <QPolygonF>
#include <QPainterPath>

#include <opencv2/opencv.hpp>

class LaserDocument;
class LaserScene;
class QPaintEvent;
class LaserViewer;

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
    bool isShape() const { return (int)m_type <= (int)LPT_SHAPE; }
    bool isBitmap() const { return m_type == LPT_BITMAP; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    //QDataStream& operator<<(QDataStream& stream);
    //QDataStream& operator>>(QDataStream& stream);

protected:
    QString typeName(LaserPrimitiveType typeId);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

protected:
    LaserDocument* m_doc;
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

class LaserShapeItem : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserShapeItem(LaserDocument* doc, LaserPrimitiveType type, SizeUnit unit = SizeUnit::SU_MM100);

private:
    Q_DISABLE_COPY(LaserShapeItem);
};

class LaserEllipseItem : public LaserShapeItem
{
    Q_OBJECT
public:
    LaserEllipseItem(const QRectF bounds, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);
    virtual ~LaserEllipseItem() {}

    QRectF bounds() const { return m_bounds; }
    void setBounds(const QRectF& bounds) { m_bounds = bounds; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());
    virtual void draw(QPainter* painter);

private:
    QRectF m_bounds;
    Q_DISABLE_COPY(LaserEllipseItem)
};

class LaserRectItem : public LaserShapeItem
{
    Q_OBJECT
public:
    LaserRectItem(const QRectF rect, LaserDocument* doc,  SizeUnit unit = SizeUnit::SU_MM100);

    QRectF rect() const { return m_rect; }
    void setRect(const QRectF& rect) { m_rect = rect; }

    virtual void draw(QPainter* painter);
    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());

private:
    QRectF m_rect;
    Q_DISABLE_COPY(LaserRectItem);
};

class LaserLineItem : public LaserShapeItem
{
    Q_OBJECT
public:
    LaserLineItem(const QLineF& line, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QLineF line() const { return m_line; }
    void setLine(const QLineF& line) { m_line = line; }

    virtual void draw(QPainter* painter);

private:
    QLineF m_line;
    Q_DISABLE_COPY(LaserLineItem);
};

class LaserPathItem : public LaserShapeItem
{
    Q_OBJECT
public:
    LaserPathItem(const QPainterPath& path, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QPainterPath path() const { return m_path; }
    void setPath(const QPainterPath& path) { m_path = path; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());
    virtual void draw(QPainter* painter);

private:
    QPainterPath m_path;
    Q_DISABLE_COPY(LaserPathItem);
};

class LaserPolylineItem : public LaserShapeItem
{
    Q_OBJECT
public:
    LaserPolylineItem(const QPolygonF& poly, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QPolygonF polyline() const { return m_poly; }
    void setPolyline(const QPolygonF& poly) { m_poly = poly; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());
    virtual void draw(QPainter* painter);

private:
    QPolygonF m_poly;
    Q_DISABLE_COPY(LaserPolylineItem);
};

class LaserPolygonItem : public LaserShapeItem
{
    Q_OBJECT
public:
    LaserPolygonItem(const QPolygonF& poly, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QPolygonF polyline() const { return m_poly; }
    void setPolyline(const QPolygonF& poly) { m_poly = poly; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());
    virtual void draw(QPainter* painter);

private:
    QPolygonF m_poly;
    Q_DISABLE_COPY(LaserPolygonItem);
};

class LaserBitmapItem : public LaserPrimitive
{
    Q_OBJECT
public:
    LaserBitmapItem(const QImage& image, const QRectF& bounds, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

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
    Q_DISABLE_COPY(LaserBitmapItem);
};

#endif // LASERITEM_H