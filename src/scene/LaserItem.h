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

class LaserItem : public QGraphicsObject
{
public:
    LaserItem(LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);
    virtual ~LaserItem();

    LaserDocument* doc() const { return m_doc; }
    SizeUnit unit() const { return m_unit; }
    QTransform extraTransform() { return m_transform; }
    void setTransform(const QTransform& transform) { m_transform = transform; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual QRectF boundingRect() const override;
    QPointF laserStartPos() const;

    virtual void draw(QPainter* painter) = 0;
    virtual LaserItemType type() = 0;
    virtual QString typeName() = 0;

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat()) { return std::vector<cv::Point2f>(); }
    virtual QByteArray engravingImage() { return QByteArray(); }

    qreal unitToMM() const;

    LaserItemType laserItemType() const { return m_type; }
    bool isShape() const { return (int)m_type <= (int)LIT_SHAPE; }
    bool isBitmap() const { return m_type == LIT_BITMAP; }

protected:
    LaserDocument* m_doc;
    SizeUnit m_unit;
    QTransform m_transform;
    QRectF m_boundingRect;
    LaserItemType m_type;

private:
    Q_DISABLE_COPY(LaserItem);
};

class LaserShapeItem : public LaserItem
{
public:
    LaserShapeItem(LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

private:
    Q_DISABLE_COPY(LaserShapeItem);
};

class LaserEllipseItem : public LaserShapeItem
{
public:
    LaserEllipseItem(const QRectF bounds, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);
    virtual ~LaserEllipseItem() {}

    QRectF bounds() const { return m_bounds; }
    void setBounds(const QRectF& bounds) { m_bounds = bounds; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());
    virtual void draw(QPainter* painter);
    virtual LaserItemType type() { return LIT_ELLIPSE; }
    virtual QString typeName() { return "Ellipse"; }

private:
    QRectF m_bounds;
    Q_DISABLE_COPY(LaserEllipseItem)
};

class LaserRectItem : public LaserShapeItem
{
public:
    LaserRectItem(const QRectF rect, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QRectF rect() const { return m_rect; }
    void setRect(const QRectF& rect) { m_rect = rect; }

    virtual void draw(QPainter* painter);
    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());
    virtual LaserItemType type() { return LIT_RECT; }
    virtual QString typeName() { return "Rect"; }

private:
    QRectF m_rect;
    Q_DISABLE_COPY(LaserRectItem);
};

class LaserLineItem : public LaserShapeItem
{
public:
    LaserLineItem(const QLineF& line, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QLineF line() const { return m_line; }
    void setLine(const QLineF& line) { m_line = line; }

    virtual void draw(QPainter* painter);
    virtual LaserItemType type() { return LIT_LINE; }
    virtual QString typeName() { return "Line"; }

private:
    QLineF m_line;
    Q_DISABLE_COPY(LaserLineItem);
};

class LaserPathItem : public LaserShapeItem
{
public:
    LaserPathItem(const QPainterPath& path, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QPainterPath path() const { return m_path; }
    void setPath(const QPainterPath& path) { m_path = path; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());
    virtual void draw(QPainter* painter);
    virtual LaserItemType type() { return LIT_PATH; }
    virtual QString typeName() { return "Path"; }

private:
    QPainterPath m_path;
    Q_DISABLE_COPY(LaserPathItem);
};

class LaserPolylineItem : public LaserShapeItem
{
public:
    LaserPolylineItem(const QPolygonF& poly, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QPolygonF polyline() const { return m_poly; }
    void setPolyline(const QPolygonF& poly) { m_poly = poly; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());
    virtual void draw(QPainter* painter);
    virtual LaserItemType type() { return LIT_POLYLINE; }
    virtual QString typeName() { return "Polyline"; }

private:
    QPolygonF m_poly;
    Q_DISABLE_COPY(LaserPolylineItem);
};

class LaserPolygonItem : public LaserShapeItem
{
public:
    LaserPolygonItem(const QPolygonF& poly, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QPolygonF polyline() const { return m_poly; }
    void setPolyline(const QPolygonF& poly) { m_poly = poly; }

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat());
    virtual void draw(QPainter* painter);
    virtual LaserItemType type() { return LIT_POLYGON; }
    virtual QString typeName() { return "Polygon"; }

private:
    QPolygonF m_poly;
    Q_DISABLE_COPY(LaserPolygonItem);
};

class LaserBitmapItem : public LaserItem
{
public:
    LaserBitmapItem(const QImage& image, const QRectF& bounds, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QImage image() const { return m_image; }
    void setImage(const QImage& image) { m_image = m_image; }

    QRectF bounds() const { return m_bounds; }

    virtual QByteArray engravingImage();
    virtual void draw(QPainter* painter);
    virtual LaserItemType type() { return LIT_BITMAP; }
    virtual QString typeName() { return "Bitmap"; }

private:
    QImage m_image;
    QRectF m_bounds;
    Q_DISABLE_COPY(LaserBitmapItem);
};

#endif // LASERITEM_H