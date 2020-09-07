#include "LaserItem.h"

#include <QSharedData>
#include <QPaintEvent>
#include <QBuffer>
#include <QtMath>

#include <opencv2/opencv.hpp>
#include <Eigen/Core>

#include "LaserScene.h"
#include "laser/LaserDriver.h"
#include "util/ImageUtils.h"
#include "util/PltUtils.h"
#include "util/TypeUtils.h"
#include "util/UnitUtils.h"
#include "util/Utils.h"
#include "widget/LaserViewer.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"

QMap<int, int> LaserPrimitive::g_itemsMaxIndex;

LaserPrimitive::LaserPrimitive(LaserDocument* doc, LaserPrimitiveType type, SizeUnit unit)
    : m_doc(doc)
    , m_layer(nullptr)
    , m_unit(unit)
    , m_isHover(false)
    , m_type(type)
{
    Q_ASSERT(doc);
    setParent(doc);
    setObjectName(utils::createUUID("primitive_"));

    this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    this->setFlag(ItemIsFocusable, true);
    //this->setFlag(ItemAcceptsInputMethod, true);

    this->setAcceptHoverEvents(true);

    if (!g_itemsMaxIndex.contains(m_type))
    {
        g_itemsMaxIndex.insert(m_type, 0);
    }
    int index = ++g_itemsMaxIndex[m_type];
    m_name = QString("%1_%2").arg(typeName(m_type)).arg(index);
}

LaserPrimitive::~LaserPrimitive()
{
    qDebug() << objectName();
}

void LaserPrimitive::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    painter->save();
    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);

    // 绘制外框
    QRectF bounds = boundingRect();
    QPointF topLeft = bounds.topLeft() - QPointF(2, 2);
    QPointF bottomRight = bounds.bottomRight() + QPointF(2, 2);
    bounds = QRectF(topLeft, bottomRight);
    if (isSelected())
    {
        painter->setPen(QPen(Qt::gray, 0.2f, Qt::SolidLine));
        painter->drawRect(bounds);
    }
    //else if (isUnderMouse())
    else if (m_isHover)
    {
        //qDebug() << name() << "redrawing under mouse.";
        painter->setPen(QPen(Qt::green, 0.2f, Qt::SolidLine));
        painter->drawRect(bounds);
    }
    
    // 绘制图元
    QColor color = Qt::blue;
    if (m_layer)
    {
        color = m_layer->color();
    }
    painter->setPen(QPen(color, 50, Qt::SolidLine));
    QTransform t = m_transform * painter->worldTransform();
    painter->setTransform(t);
    draw(painter);

    painter->restore();
    this->scene()->update();
}

QRectF LaserPrimitive::boundingRect() const
{
    QRectF bounds = m_transform.mapRect(m_boundingRect);
    //qDebug() << this->name() << bounds;
    return bounds;
}

QPointF LaserPrimitive::laserStartPos() const
{
    QPointF pos = boundingRect().topLeft();
    pos *= 40;
    return pos;
}

qreal LaserPrimitive::unitToMM() const { return unitUtils::unitToMM(m_unit); }

QString LaserPrimitive::typeName()
{
    return typeName(m_type);
}

//QDataStream & LaserPrimitive::operator<<(QDataStream & stream)
//{
//    stream << objectName()
//        << name()
//        << type();
//    return stream;
//}
//
//QDataStream & LaserPrimitive::operator>>(QDataStream & stream)
//{
//    QString objName;
//    int type;
//    stream >> objName
//        >> m_name
//        >> type;
//    m_type = (LaserPrimitiveType)type;
//    return stream;
//}

QString LaserPrimitive::typeName(LaserPrimitiveType typeId)
{
    static QMap<LaserPrimitiveType, QString> TypeNamesMap{
        { LPT_BITMAP, tr("Bitmap") },
        { LPT_CIRCLE, tr("Circle") },
        { LPT_ELLIPSE, tr("Ellipse") },
        { LPT_LINE, tr("Line") },
        { LPT_PATH, tr("Path") },
        { LPT_POLYGON, tr("Polygon") },
        { LPT_POLYLINE, tr("Polyline") },
        { LPT_RECT, tr("Rect") }
    };
    return TypeNamesMap[typeId];
}

void LaserPrimitive::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    //qDebug() << name() << "hover enter.";
    m_isHover = true;
    update();
}

void LaserPrimitive::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    m_isHover = false;
    update();
}

void LaserPrimitive::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    update();
}

LaserShapeItem::LaserShapeItem(LaserDocument* doc, LaserPrimitiveType type, SizeUnit unit)
    : LaserPrimitive(doc, type, unit)
{
    //m_type = LPT_SHAPE;
}

LaserEllipseItem::LaserEllipseItem(const QRectF bounds, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, LPT_ELLIPSE, unit)
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
    : LaserShapeItem(doc, LPT_RECT, unit)
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
    : LaserShapeItem(doc, LPT_LINE, unit)
    , m_line(line)
{
    m_boundingRect = QRectF(m_line.p1(), m_line.p2());
}

void LaserLineItem::draw(QPainter * painter)
{
    painter->drawLine(m_line);
}

LaserPathItem::LaserPathItem(const QPainterPath & path, LaserDocument * doc, SizeUnit unit)
    : LaserShapeItem(doc, LPT_PATH, unit)
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
    : LaserShapeItem(doc, LPT_POLYLINE, unit)
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
    : LaserShapeItem(doc, LPT_POLYGON, unit)
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
    : LaserPrimitive(doc, LPT_BITMAP, unit)
    , m_image(image)
    , m_bounds(bounds)
{
    m_image = m_image.convertToFormat(QImage::Format_Grayscale8);
    m_boundingRect = m_bounds;
    m_type = LPT_BITMAP;
}

QByteArray LaserBitmapItem::engravingImage(cv::Mat& canvas) 
{ 
    QByteArray ba;

    cv::Mat src(m_image.height(), m_image.width(), CV_8UC1, (void*)m_image.constBits(), m_image.bytesPerLine());
    float mmWidth = 1000.f * m_image.width() / m_image.dotsPerMeterX();
    float mmHeight = 1000.f * m_image.height() / m_image.dotsPerMeterY();

    int scanInterval = 7;
    double yPulseLength = 0.006329114;
    QVariant value;
    if (LaserDriver::instance().getRegister(LaserDriver::RT_ENGRAVING_ROW_STEP, value))
    {
        qDebug() << "row step register:" << value;
        scanInterval = value.toInt();
    }
    if (LaserDriver::instance().getRegister(LaserDriver::RT_Y_AXIS_PULSE_LENGTH, value))
    {
        qDebug() << "y pulse register:" << value;
        yPulseLength = value.toDouble() / 1000.0;
    }
    qreal pixelInterval = scanInterval * yPulseLength;

    int outWidth = mmWidth * MM_TO_INCH * 600;
    int outHeight = std::round(mmHeight / pixelInterval);
    qDebug() << " out width:" << outWidth;
    qDebug() << "out height:" << outHeight;

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(outWidth, outHeight));
    
    cv::Mat outMat = imageUtils::halftone2(resized, layer()->lpi(), layer()->dpi(), 45, layer()->nonlinearCoefficient());
    std::vector<uchar> buffer;
    std::vector<int>param; 
    param.push_back(cv::IMWRITE_PNG_BILEVEL);
    param.push_back(1);
    param.push_back(cv::IMWRITE_PNG_COMPRESSION);
    param.push_back(0);
    cv::imencode(".png", outMat, buffer, param);

    QBuffer baBuf(&ba);
    baBuf.open(QIODevice::WriteOnly);
    baBuf.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());

    QTransform t = m_transform * sceneTransform();
    if (!canvas.empty())
    {
        QRectF bounds = t.mapRect(m_boundingRect);
        cv::Rect roiRect = typeUtils::qtRect2cvRect(bounds, 40);
        roiRect = cv::Rect(roiRect.x, roiRect.y, outMat.cols, outMat.rows);
        cv::Mat roi = canvas(roiRect);
        cv::Mat mat;
        cv::cvtColor(outMat, mat, cv::COLOR_GRAY2BGR);
        mat.copyTo(roi);
    }

    return ba; 
}

void LaserBitmapItem::draw(QPainter * painter)
{
    painter->drawImage(m_bounds, m_image);
}

QDebug operator<<(QDebug debug, const LaserPrimitive & item)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "(" << item.name() << ", " << item.objectName() << ", " << item.type() << ")";
    return debug;
}
