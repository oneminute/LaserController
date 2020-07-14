#include "LaserItem.h"

#include <QSharedData>
#include <QPaintEvent>

#include "LaserScene.h"
#include "util/UnitUtils.h"
#include "widget/LaserViewer.h"
#include "scene/LaserDocument.h"

LaserItem::LaserItem(LaserDocument* doc, SizeUnit unit)
    : m_doc(doc)
    , m_unit(unit)
{

}

LaserItem::~LaserItem()
{
    
}

void LaserItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    painter->save();
    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);

    // 绘制外框
    painter->setPen(QPen(Qt::green, 1, Qt::SolidLine));
    QRectF bounds = boundingRect();
    painter->drawRect(bounds);

    // 绘制图元
    painter->setPen(QPen(Qt::blue, 50, Qt::SolidLine));
    //QTransform t = m_transform.scale(m_doc->scale(), m_doc->scale());
    //QTransform t = m_transform.scale(m_doc->scale(), m_doc->scale());
    QTransform t = m_transform * painter->worldTransform();
    painter->setTransform(t);
    draw(painter);

    painter->restore();
}

QRectF LaserItem::boundingRect() const
{
    QTransform t(m_transform);
    //t = t.scale(m_doc->scale(), m_doc->scale());
    QRectF bounds = t.mapRect(m_boundingRect);
    return bounds;
}

qreal LaserItem::unitToMM() const { return unitUtils::unitToMM(m_unit); }

LaserArcItem::LaserArcItem(const QPainterPath & path, LaserDocument* doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_path(path)
{
}

LaserArcItem::~LaserArcItem()
{
}

void LaserArcItem::draw(QPainter* painter)
{
}

LaserShapeItem::LaserShapeItem(LaserDocument* doc, SizeUnit unit)
    : LaserItem(doc, unit)
{
}

LaserEllipseItem::LaserEllipseItem(const QRectF bounds, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_bounds(bounds)
{
    m_boundingRect = m_bounds;
}

void LaserEllipseItem::draw(QPainter* painter)
{
    painter->drawEllipse(m_bounds);
}

LaserRectItem::LaserRectItem(const QRectF rect, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_rect(rect)
{
    m_boundingRect = m_rect;
}

void LaserRectItem::draw(QPainter* painter)
{
    painter->drawRect(m_rect);
}

LaserLineItem::LaserLineItem(const QLineF & line, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_line(line)
{
    m_boundingRect = QRectF(m_line.p1(), m_line.p2());
}

void LaserLineItem::draw(QPainter * painter)
{
    painter->drawLine(m_line);
}

LaserPathItem::LaserPathItem(const QPainterPath & path, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_path(path)
{
    m_boundingRect = path.boundingRect();
}

void LaserPathItem::draw(QPainter * painter)
{
    painter->drawPath(m_path);
}

LaserPolylineItem::LaserPolylineItem(const QPolygonF & poly, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_poly(poly)
{
    m_boundingRect = m_poly.boundingRect();
}

void LaserPolylineItem::draw(QPainter * painter)
{
    painter->drawPolyline(m_poly);
}

LaserPolygonItem::LaserPolygonItem(const QPolygonF & poly, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_poly(poly)
{
    m_boundingRect = m_poly.boundingRect();
}

void LaserPolygonItem::draw(QPainter * painter)
{
    painter->drawPolygon(m_poly);
}


