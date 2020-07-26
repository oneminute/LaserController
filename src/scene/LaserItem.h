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
    Q_OBJECT
public:
    LaserItem(LaserDocument* doc, LaserItemType type, SizeUnit unit = SizeUnit::SU_MM100);
    virtual ~LaserItem();

    LaserDocument* document() const { return m_doc; }
    SizeUnit unit() const { return m_unit; }
    QTransform extraTransform() { return m_transform; }
    void setTransform(const QTransform& transform) { m_transform = transform; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual QRectF boundingRect() const override;
    QPointF laserStartPos() const;

    virtual void draw(QPainter* painter) = 0;

    virtual std::vector<cv::Point2f> cuttingPoints(cv::Mat& mat = cv::Mat()) { return std::vector<cv::Point2f>(); }
    virtual QByteArray engravingImage() { return QByteArray(); }

    qreal unitToMM() const;

    LaserItemType laserItemType() const { return m_type; }
    QString typeName();
    bool isShape() const { return (int)m_type <= (int)LIT_SHAPE; }
    bool isBitmap() const { return m_type == LIT_BITMAP; }

protected:
    QString typeName(LaserItemType typeId);

protected:
    LaserDocument* m_doc;
    SizeUnit m_unit;
    QTransform m_transform;
    QRectF m_boundingRect;
    LaserItemType m_type;

    bool m_isHover;

    static QMap<int, int> g_itemsMaxIndex;

private:
    Q_DISABLE_COPY(LaserItem);

    friend class LaserDocument;
};

class LaserShapeItem : public LaserItem
{
    Q_OBJECT
public:
    LaserShapeItem(LaserDocument* doc, LaserItemType type, SizeUnit unit = SizeUnit::SU_MM100);

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

class LaserBitmapItem : public LaserItem
{
    Q_OBJECT
public:
    LaserBitmapItem(const QImage& image, const QRectF& bounds, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);

    QImage image() const { return m_image; }
    void setImage(const QImage& image) { m_image = m_image; }

    QRectF bounds() const { return m_bounds; }

    virtual QByteArray engravingImage();
    virtual void draw(QPainter* painter);
    virtual LaserItemType type() { return LIT_BITMAP; }
    virtual QString typeName() { return tr("Bitmap"); }

private:
    QImage m_image;
    QRectF m_bounds;
    Q_DISABLE_COPY(LaserBitmapItem);
};

#endif // LASERITEM_H