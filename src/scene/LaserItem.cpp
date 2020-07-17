#include "LaserItem.h"

#include <QSharedData>
#include <QPaintEvent>
#include <QBuffer>
#include <QtMath>

#include <opencv2/opencv.hpp>
#include <Eigen/Core>

#include "LaserScene.h"
#include "util/PltUtils.h"
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

    // �������
    painter->setPen(QPen(Qt::green, 0.2f, Qt::SolidLine));
    QRectF bounds = boundingRect();
    painter->drawRect(bounds);

    // ����ͼԪ
    painter->setPen(QPen(Qt::blue, 50, Qt::SolidLine));
    QTransform t = m_transform * painter->worldTransform();
    painter->setTransform(t);
    draw(painter);

    painter->restore();
}

QRectF LaserItem::boundingRect() const
{
    QRectF bounds = m_transform.mapRect(m_boundingRect);
    return bounds;
}

QPointF LaserItem::laserStartPos() const
{
    QPointF pos = boundingRect().topLeft();
    pos *= 40;
    return pos;
}

qreal LaserItem::unitToMM() const { return unitUtils::unitToMM(m_unit); }

LaserShapeItem::LaserShapeItem(LaserDocument* doc, SizeUnit unit)
    : LaserItem(doc, unit)
{
    m_type = LIT_SHAPE;
}

LaserEllipseItem::LaserEllipseItem(const QRectF bounds, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, unit)
    , m_bounds(bounds)
{
    m_boundingRect = m_bounds;
}

std::vector<cv::Point2f> LaserEllipseItem::cuttingPoints(cv::Mat& mat)
{
    QPainterPath path;

    path.addEllipse(m_transform.mapRect(m_bounds));

    QTransform transform = QTransform::fromScale(40, 40);

    std::vector<cv::Point2f> points;
    pltUtils::pathPoints(path, points, mat);
    return points;
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

std::vector<cv::Point2f> LaserRectItem::cuttingPoints(cv::Mat& mat)
{
    std::vector<cv::Point2f> points;
    QPainterPath path;
    QPolygonF poly;
    poly.append(m_transform.map(m_rect.topLeft()));
    poly.append(m_transform.map(m_rect.topRight()));
    poly.append(m_transform.map(m_rect.bottomRight()));
    poly.append(m_transform.map(m_rect.bottomLeft()));
    poly.append(m_transform.map(m_rect.topLeft()));
    path.addPolygon(poly);

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);

    pltUtils::pathPoints(path, points, mat);

    /*Eigen::Matrix3d transform;
    transform << m_transform.m11(), m_transform.m21(), m_transform.m31(),
        m_transform.m12(), m_transform.m22(), m_transform.m32(),
        m_transform.m13(), m_transform.m23(), m_transform.m33();
    int count = pltUtils::linePoints(m_rect.topLeft().x(), m_rect.topLeft().y(), m_rect.topRight().x(), m_rect.topRight().y(), points, 40, transform);
    count += pltUtils::linePoints(m_rect.topRight().x(), m_rect.topRight().y(), m_rect.bottomRight().x(), m_rect.bottomRight().y(), points, 40, transform);
    count += pltUtils::linePoints(m_rect.bottomRight().x(), m_rect.bottomRight().y(), m_rect.bottomLeft().x(), m_rect.bottomLeft().y(), points, 40, transform);
    count += pltUtils::linePoints(m_rect.bottomLeft().x(), m_rect.bottomLeft().y(), m_rect.topLeft().x(), m_rect.topLeft().y(), points, 40, transform);
    points.push_back(points[0]);
    std::cout << points[count - 1] << std::endl;
    std::cout << points[count] << std::endl;
    std::cout << points[0] << std::endl;*/

    return points;
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

std::vector<cv::Point2f> LaserPathItem::cuttingPoints(cv::Mat& mat)
{
    std::vector<cv::Point2f> points;
    if (m_path.pointAtPercent(0) != m_path.pointAtPercent(1))
        m_path.lineTo(m_path.pointAtPercent(0));
    QPainterPath path = m_transform.map(m_path);

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);
    
    pltUtils::pathPoints(path, points, mat);
    
    return points;
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

std::vector<cv::Point2f> LaserPolylineItem::cuttingPoints(cv::Mat & mat)
{
    std::vector<cv::Point2f> points;
    QPolygonF poly(m_poly);
    if (poly.first() != poly.last())
        poly.append(poly.first());
    QPainterPath path;
    path.addPolygon(poly);
    path = m_transform.map(path);

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);
    
    pltUtils::pathPoints(path, points, mat);
    
    return points;
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

std::vector<cv::Point2f> LaserPolygonItem::cuttingPoints(cv::Mat & mat)
{
    std::vector<cv::Point2f> points;
    QPolygonF poly(m_poly);
    if (poly.first() != poly.last())
        poly.append(poly.first());
    QPainterPath path;
    path.addPolygon(poly);
    path = m_transform.map(path);

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);
    
    pltUtils::pathPoints(path, points, mat);
    
    return points;
}

void LaserPolygonItem::draw(QPainter * painter)
{
    painter->drawPolygon(m_poly);
}

LaserBitmapItem::LaserBitmapItem(const QImage & image, const QRectF& bounds, LaserDocument * doc, SizeUnit unit)
    : LaserItem(doc, unit)
    , m_image(image)
    , m_bounds(bounds)
{
    m_boundingRect = m_bounds;
    m_type = LIT_BITMAP;
}

QByteArray LaserBitmapItem::engravingImage() 
{ 
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    m_image.save(&buffer, "PNG");
    return ba; 
}

void LaserBitmapItem::draw(QPainter * painter)
{
    painter->drawImage(m_bounds, m_image);
}
