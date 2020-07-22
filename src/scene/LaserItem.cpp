#include "LaserItem.h"

#include <QSharedData>
#include <QPaintEvent>
#include <QBuffer>
#include <QtMath>

#include <opencv2/opencv.hpp>
#include <Eigen/Core>

#include "LaserScene.h"
#include "util/PltUtils.h"
#include "util/TypeUtils.h"
#include "util/UnitUtils.h"
#include "widget/LaserViewer.h"
#include "scene/LaserDocument.h"

LaserItem::LaserItem(LaserDocument* doc, SizeUnit unit)
    : m_doc(doc)
    , m_unit(unit)
{
    this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    this->setFlag(ItemIsFocusable, true);
}

LaserItem::~LaserItem()
{
    
}

void LaserItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    painter->save();
    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);

    // 绘制外框
    painter->setPen(QPen(Qt::green, 0.2f, Qt::SolidLine));
    QRectF bounds = boundingRect();
    painter->drawRect(bounds);

    // 绘制图元
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

    QTransform t = m_transform * sceneTransform();
    path.addEllipse(t.mapRect(m_bounds));

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);

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
    QTransform t = m_transform * sceneTransform();
    std::vector<cv::Point2f> points;
    cv::Point2f pt1 = typeUtils::qtPointF2CVPoint2f(t.map(m_rect.topLeft()) * 40);
    cv::Point2f pt2 = typeUtils::qtPointF2CVPoint2f(t.map(m_rect.topRight()) * 40);
    cv::Point2f pt3 = typeUtils::qtPointF2CVPoint2f(t.map(m_rect.bottomRight()) * 40);
    cv::Point2f pt4 = typeUtils::qtPointF2CVPoint2f(t.map(m_rect.bottomLeft()) * 40);
    points.push_back(pt1);
    points.push_back(pt2);
    points.push_back(pt3);
    points.push_back(pt4);

    cv::line(mat, pt1, pt2, cv::Scalar(0));
    cv::line(mat, pt2, pt3, cv::Scalar(0));
    cv::line(mat, pt3, pt4, cv::Scalar(0));
    cv::line(mat, pt4, pt1, cv::Scalar(0));

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
    QTransform t = m_transform * sceneTransform();
    QPainterPath path = t.map(m_path);

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
    cv::Point2f lastPt;
    QTransform t = m_transform * sceneTransform();
    for (int i = 0; i < m_poly.size(); i++)
    {
        QPointF pt = t.map(m_poly[i]) * 40;
        cv::Point2f cvPt = typeUtils::qtPointF2CVPoint2f(pt);
        if (i > 0)
        {
            cv::line(mat, lastPt, cvPt, cv::Scalar(0));
        }
        lastPt = cvPt;
        points.push_back(cvPt);
    }
    cv::line(mat, points[points.size() - 1], points[0], cv::Scalar(0));
    
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
    cv::Point2f lastPt;
    QTransform t = m_transform * sceneTransform();
    for (int i = 0; i < m_poly.size(); i++)
    {
        QPointF pt = t.map(m_poly[i]) * 40;
        cv::Point2f cvPt = typeUtils::qtPointF2CVPoint2f(pt);
        if (i > 0)
        {
            cv::line(mat, lastPt, cvPt, cv::Scalar(0));
        }
        lastPt = cvPt;
        points.push_back(cvPt);
    }
    cv::line(mat, points[points.size() - 1], points[0], cv::Scalar(0));
    
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
