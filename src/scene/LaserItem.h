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
    QTransform transform() { return m_transform; }
    void setTransform(const QTransform& transform) { m_transform = transform; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual QRectF boundingRect() const override;
    virtual void draw(QPainter* painter) = 0;

    qreal unitToMM() const;

protected:
    LaserDocument* m_doc;
    SizeUnit m_unit;
    QTransform m_transform;
    QRectF m_boundingRect;

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

class LaserArcItem : public LaserShapeItem
{
public:
    LaserArcItem(const QPainterPath& path, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);
    virtual ~LaserArcItem();

    QPainterPath path() const { return m_path; }
    void setPath(const QPainterPath& path) { m_path = path; }

    virtual void draw(QPainter* painter);

private:
    QPainterPath m_path;
    Q_DISABLE_COPY(LaserArcItem);
};

class LaserEllipseItem : public LaserShapeItem
{
public:
    LaserEllipseItem(const QRectF bounds, LaserDocument* doc, SizeUnit unit = SizeUnit::SU_MM100);
    virtual ~LaserEllipseItem() {}

    QRectF bounds() const { return m_bounds; }
    void setBounds(const QRectF& bounds) { m_bounds = bounds; }

    virtual void draw(QPainter* painter);

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

    //virtual QRectF boundingRect() const override;
    virtual void draw(QPainter* painter);

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

    //virtual QRectF boundingRect() const override;
    virtual void draw(QPainter* painter);

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

    //virtual QRectF boundingRect() const override;
    virtual void draw(QPainter* painter);

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

    //virtual QRectF boundingRect() const override;
    virtual void draw(QPainter* painter);

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

    //virtual QRectF boundingRect() const override;
    virtual void draw(QPainter* painter);

private:
    QPolygonF m_poly;
    Q_DISABLE_COPY(LaserPolygonItem);
};

#endif // LASERITEM_H