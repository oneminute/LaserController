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
    painter->setPen(QPen(Qt::blue, 50, Qt::SolidLine));
    QRectF bounds = boundingRect();
    QTransform t = m_transform.scale(m_doc->scale(), m_doc->scale());
    painter->setTransform(m_transform);
    draw(painter);
    painter->restore();
}

QRectF LaserItem::boundingRect() const
{
    QTransform t(m_transform);
    t = t.scale(m_doc->scale(), m_doc->scale());
    return t.mapRect(m_boundingRect);
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
    //qreal ratio = unitUtils::unitToMM(m_unit);
    //m_bounds = QRectF(bounds.x() * ratio, bounds.y() * ratio, bounds.width() * ratio, bounds.height() * ratio);
    //m_transformedBounds = m_bounds;
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
    //qreal ratio = unitUtils::unitToMM(m_unit);
    //m_rect = QRectF(rect.x() * ratio, rect.y() * ratio, rect.width() * ratio, rect.height() * ratio);
    m_boundingRect = m_rect;
}

//QRectF LaserRectItem::boundingRect() const
//{
//    return m_transform.mapRect(m_rect);
//}

void LaserRectItem::draw(QPainter* painter)
{
    painter->drawRect(m_rect);
}

LaserLineItem::LaserLineItem(const QLineF & line, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_line(line)
{
    //qreal ratio = unitToMM();
    //m_line = QLineF(line.p1() * ratio, line.p2() * ratio);
    m_boundingRect = QRectF(m_line.p1(), m_line.p2());
}

//QRectF LaserLineItem::boundingRect() const
//{
//    return QRectF(m_line.p1(), m_line.p2());
//}

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

//QRectF LaserPathItem::boundingRect() const
//{
//    QRectF bounds = m_path.boundingRect();
//    qreal ratio = unitToMM();
//    return QRectF(bounds.x() * ratio, bounds.y() * );
//}

void LaserPathItem::draw(QPainter * painter)
{
    painter->drawPath(m_path);
}

LaserPolylineItem::LaserPolylineItem(const QPolygonF & poly, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_poly(poly)
{
    /*qreal ratio = unitToMM();
    for (int i = 0; i < poly.size(); i++)
    {
        QPointF pt = poly[i];
        m_poly.append(pt * ratio);
    }*/
    m_boundingRect = m_poly.boundingRect();
}

//QRectF LaserPolylineItem::boundingRect() const
//{
//    return m_poly.boundingRect();
//}

void LaserPolylineItem::draw(QPainter * painter)
{
    painter->drawPolyline(m_poly);
}

LaserPolygonItem::LaserPolygonItem(const QPolygonF & poly, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_poly(poly)
{
    /*qreal ratio = unitToMM();
    for (int i = 0; i < poly.size(); i++)
    {
        QPointF pt = poly[i];
        m_poly.append(pt * ratio);
    }*/
    m_boundingRect = m_poly.boundingRect();
}

//QRectF LaserPolygonItem::boundingRect() const
//{
//    return m_poly.boundingRect();
//}

void LaserPolygonItem::draw(QPainter * painter)
{
    painter->drawPolygon(m_poly);
}


