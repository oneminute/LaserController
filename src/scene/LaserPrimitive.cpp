#include "LaserPrimitive.h"

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

QString LaserPrimitive::typeLatinName()
{
    return typeLatinName(m_type);
}

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

QString LaserPrimitive::typeLatinName(LaserPrimitiveType typeId)
{
    static QMap<LaserPrimitiveType, QString> TypeLatinNamesMap{
        { LPT_BITMAP, "Bitmap" },
        { LPT_CIRCLE, "Circle" },
        { LPT_ELLIPSE, "Ellipse" },
        { LPT_LINE, "Line" },
        { LPT_PATH, "Path" },
        { LPT_POLYGON, "Polygon" },
        { LPT_POLYLINE, "Polyline" },
        { LPT_RECT, "Rect" }
    };
    return TypeLatinNamesMap[typeId];
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

void LaserPrimitive::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    qDebug() << m_name << "mouse press event";
    update();
}

void LaserPrimitive::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    qDebug() << m_name << "mouse release event";
    update();
}

LaserShape::LaserShape(LaserDocument* doc, LaserPrimitiveType type, SizeUnit unit)
    : LaserPrimitive(doc, type, unit)
{
    //m_type = LPT_SHAPE;
}

LaserEllipse::LaserEllipse(const QRectF bounds, LaserDocument * doc, SizeUnit unit)
    : LaserShape(doc, LPT_ELLIPSE, unit)
    , m_bounds(bounds)
{
    m_boundingRect = m_bounds;
}

std::vector<cv::Point2f> LaserEllipse::cuttingPoints(cv::Mat& canvas)
{
    QPainterPath path = toPath();

    std::vector<cv::Point2f> points;
    pltUtils::pathPoints(path, points, canvas);
    return points;
}

void LaserEllipse::draw(QPainter* painter)
{
    painter->drawEllipse(m_bounds);
}

QPainterPath LaserEllipse::toPath() const
{
    QPainterPath path;

    QTransform t = m_transform * sceneTransform();
    path.addEllipse(t.mapRect(m_bounds));

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);
    return path;
}

LaserRect::LaserRect(const QRectF rect, LaserDocument * doc, SizeUnit unit)
    : LaserShape(doc, LPT_RECT, unit)
    , m_rect(rect)
{
    m_boundingRect = m_rect;
}

void LaserRect::draw(QPainter* painter)
{
    painter->drawRect(m_rect);
}

std::vector<cv::Point2f> LaserRect::cuttingPoints(cv::Mat& canvas)
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
    points.push_back(pt1);

    if (!canvas.empty())
    {
        cv::line(canvas, pt1, pt2, cv::Scalar(0));
        cv::line(canvas, pt2, pt3, cv::Scalar(0));
        cv::line(canvas, pt3, pt4, cv::Scalar(0));
        cv::line(canvas, pt4, pt1, cv::Scalar(0));
    }

    return points;
}

QPainterPath LaserRect::toPath() const
{
    QPainterPath path;
    QTransform t = m_transform * sceneTransform();
    QPolygonF rect = t.map(m_rect);
    path.addPolygon(rect);

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);

    return path;
}

LaserLine::LaserLine(const QLineF & line, LaserDocument * doc, SizeUnit unit)
    : LaserShape(doc, LPT_LINE, unit)
    , m_line(line)
{
    m_boundingRect = QRectF(m_line.p1(), m_line.p2());
}

std::vector<cv::Point2f> LaserLine::cuttingPoints(cv::Mat& canvas)
{
    std::vector<cv::Point2f> points;
    QPainterPath path = toPath();
    
    pltUtils::pathPoints(path, points, canvas);
    
    return points;
}

void LaserLine::draw(QPainter * painter)
{
    painter->drawLine(m_line);
}

QPainterPath LaserLine::toPath() const
{
    QPainterPath path;
    QTransform t = m_transform * sceneTransform();
    QLineF line = t.map(m_line);
    path.moveTo(line.p1());
    path.lineTo(line.p2());

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);

    return path;
}

LaserPath::LaserPath(const QPainterPath & path, LaserDocument * doc, SizeUnit unit)
    : LaserShape(doc, LPT_PATH, unit)
    , m_path(path)
{
    m_boundingRect = path.boundingRect();
}

std::vector<cv::Point2f> LaserPath::cuttingPoints(cv::Mat& canvas)
{
    std::vector<cv::Point2f> points;
    QPainterPath path = toPath();
    
    pltUtils::pathPoints(path, points, canvas);
    
    return points;
}

void LaserPath::draw(QPainter * painter)
{
    painter->drawPath(m_path);
}

QPainterPath LaserPath::toPath() const
{
    QPainterPath path = m_path;
    //if (path.pointAtPercent(0) != path.pointAtPercent(1))
        //path.lineTo(path.pointAtPercent(0));
    QTransform t = m_transform * sceneTransform();
    path = t.map(path);

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);
    return path;
}

LaserPolyline::LaserPolyline(const QPolygonF & poly, LaserDocument * doc, SizeUnit unit)
    : LaserShape(doc, LPT_POLYLINE, unit)
    , m_poly(poly)
{
    m_boundingRect = m_poly.boundingRect();
}

std::vector<cv::Point2f> LaserPolyline::cuttingPoints(cv::Mat & canvas)
{
    std::vector<cv::Point2f> points;
    cv::Point2f lastPt;
    QTransform t = m_transform * sceneTransform();
    for (int i = 0; i < m_poly.size(); i++)
    {
        QPointF pt = t.map(m_poly[i]) * 40;
        cv::Point2f cvPt = typeUtils::qtPointF2CVPoint2f(pt);
        if (i > 0 && !canvas.empty())
        {
            cv::line(canvas, lastPt, cvPt, cv::Scalar(0));
        }
        lastPt = cvPt;
        points.push_back(cvPt);
    }
    if (!canvas.empty())
        cv::line(canvas, points[points.size() - 1], points[0], cv::Scalar(0));
    
    return points;
}

void LaserPolyline::draw(QPainter * painter)
{
    painter->drawPolyline(m_poly);
}

QPainterPath LaserPolyline::toPath() const
{
    QPainterPath path;
    QTransform t = m_transform * sceneTransform();
    QPolygonF rect = t.map(m_poly);
    path.addPolygon(rect);

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);

    return path;
}

LaserPolygon::LaserPolygon(const QPolygonF & poly, LaserDocument * doc, SizeUnit unit)
    : LaserShape(doc, LPT_POLYGON, unit)
    , m_poly(poly)
{
    m_boundingRect = m_poly.boundingRect();
}

std::vector<cv::Point2f> LaserPolygon::cuttingPoints(cv::Mat & canvas)
{
    std::vector<cv::Point2f> points;
    cv::Point2f lastPt;
    QTransform t = m_transform * sceneTransform();
    for (int i = 0; i < m_poly.size(); i++)
    {
        QPointF pt = t.map(m_poly[i]) * 40;
        cv::Point2f cvPt = typeUtils::qtPointF2CVPoint2f(pt);
        if (i > 0 && !canvas.empty())
        {
            cv::line(canvas, lastPt, cvPt, cv::Scalar(0));
        }
        lastPt = cvPt;
        points.push_back(cvPt);
    }
    points.push_back(points[0]);
    if (!canvas.empty())
        cv::line(canvas, points[points.size() - 1], points[0], cv::Scalar(0));
    
    return points;
}

void LaserPolygon::draw(QPainter * painter)
{
    painter->drawPolygon(m_poly);
}

QPainterPath LaserPolygon::toPath() const
{
    QPainterPath path;
    QTransform t = m_transform * sceneTransform();
    QPolygonF poly = t.map(m_poly);
    path.addPolygon(poly);

    QTransform transform = QTransform::fromScale(40, 40);
    path = transform.map(path);

    return path;
}

LaserBitmap::LaserBitmap(const QImage & image, const QRectF& bounds, LaserDocument * doc, SizeUnit unit)
    : LaserPrimitive(doc, LPT_BITMAP, unit)
    , m_image(image)
    , m_bounds(bounds)
{
    m_image = m_image.convertToFormat(QImage::Format_Grayscale8);
    m_boundingRect = m_bounds;
    m_type = LPT_BITMAP;
}

QByteArray LaserBitmap::engravingImage(cv::Mat& canvas) 
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

    qreal boundingWidth = boundingRect().width();
    qreal boundingHeight = boundingRect().height();
    int outWidth = boundingWidth * MM_TO_INCH * 600;
    int outHeight = std::round(boundingHeight / pixelInterval);
    qDebug() << " out width:" << outWidth;
    qDebug() << "out height:" << outHeight;

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(outWidth, outHeight));
    
    cv::Mat outMat = resized;
    if (layer()->useHalftone())
    {
        outMat = imageUtils::halftone3(resized, layer()->lpi(), layer()->dpi(), 45);
    }

    QPointF pos = laserStartPos();
    ba = imageUtils::image2EngravingData(outMat, boundingRect().left(), 
        boundingRect().top(), pixelInterval, boundingWidth);

    QTransform t = m_transform * sceneTransform();
    if (!canvas.empty())
    {
        try
        {
            QRectF bounds = t.mapRect(m_boundingRect);
            cv::Rect roiRect = typeUtils::qtRect2cvRect(bounds, 40);
            roiRect = cv::Rect(roiRect.x, roiRect.y, outMat.cols, outMat.rows);
            cv::Mat roi = canvas(roiRect);
            cv::Mat mat;
            cv::cvtColor(outMat, mat, cv::COLOR_GRAY2BGR);
            mat.copyTo(roi);
        }
        catch (cv::Exception& e)
        {
            std::cout << e.err << std::endl;
        }
    }

    return ba; 
}

void LaserBitmap::draw(QPainter * painter)
{
    painter->drawImage(m_bounds, m_image);
}

QDebug operator<<(QDebug debug, const LaserPrimitive & item)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "(" << item.name() << ", " << item.objectName() << ", " << item.type() << ")";
    return debug;
}

QString FinishRun::toString()
{
    QString text = QObject::tr("Relays: ");
    QStringList nos;
    for (int i = 0; i < 8; i++)
    {
        if (isEnabled(i))
        {
            nos.append(QString::number(i + 1));
        }
    }
    text.append(nos.join(","));
    text.append(" Action: ");

    QString actionStr;
    switch (action)
    {
    case RA_NONE:
        actionStr = QObject::tr("None");
        break;
    case RA_RELEASE:
        actionStr = QObject::tr("Release");
        break;
    case RA_ORIGIN:
        actionStr = QObject::tr("Machining 1");
        break;
    case RA_MACHINING_1:
        actionStr = QObject::tr("Machining 2");
        break;
    case RA_MACHINING_2:
        actionStr = QObject::tr("Machining 3");
        break;
    case RA_MACHINING_3:
        actionStr = QObject::tr("None");
        break;
    }

    text.append(actionStr);
    return text;
}

QByteArray LaserShape::engravingImage(cv::Mat & canvas)
{
    QByteArray bytes;
    QPainterPath path = toPath();
    QRectF boundRect = path.boundingRect();

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

    QList<SliceGroup> groups;

    for (qreal y = boundRect.top(); y <= boundRect.bottom(); y += pixelInterval)
    {
        QLineF hLine(boundRect.left() - 0.01f, y, boundRect.right() + 0.01f, y + 1);
        QRectF hRect(hLine.p1(), hLine.p2());
        QPainterPath linePath;
        linePath.addRect(hRect);
        QPainterPath intersection = path.intersected(linePath);
        for (int i = 0; i < intersection.elementCount(); i++)
        {
            qDebug() << i << intersection.elementAt(i);
        }
    }
    return bytes;
}
