#include "LaserPrimitive.h"

#include <iostream>

#include <QSharedData>
#include <QPaintEvent>
#include <QBuffer>
#include <QtMath>
#include <QGraphicsTextItem> 
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
#include <QTextEdit>

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
QMap<LaserPrimitiveType, int> g_counter{
    { LPT_LINE, 0 },
    { LPT_CIRCLE, 0},
    { LPT_ELLIPSE, 0},
    { LPT_RECT, 0},
    { LPT_POLYLINE, 0},
    { LPT_POLYGON, 0},
    { LPT_PATH, 0},
    { LPT_BITMAP, 0},
    { LPT_TEXT, 0},
};

class LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserPrimitive)
public:
    LaserPrimitivePrivate(LaserPrimitive* ptr)
        : q_ptr(ptr)
        , doc(nullptr)
        , layer(nullptr)
        , isHover(false)
        , type(LPT_UNKNOWN)
    {}

    LaserDocument* doc;
    LaserLayer* layer;
    QRectF boundingRect;
    LaserPrimitiveType type;
    QString name;
    bool isHover;
    QPainterPath outline;

    LaserPrimitive* q_ptr;
};

LaserPrimitive::LaserPrimitive(LaserPrimitivePrivate* data, LaserDocument* doc, LaserPrimitiveType type)
    : d_ptr(data)
{
    Q_D(LaserPrimitive);
    d->doc = doc;
    d->type = type;
    Q_ASSERT(doc);
    setParent(doc);

    g_counter[type]++;
    QString objName = QString("%1_%2").arg(typeLatinName(type)).arg(g_counter[type]);
    setObjectName(objName);

    this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    this->setFlag(ItemIsFocusable, true);

    this->setAcceptHoverEvents(true);

    if (!g_itemsMaxIndex.contains(d->type))
    {
        g_itemsMaxIndex.insert(d->type, 0);
    }
    int index = ++g_itemsMaxIndex[d->type];
    d->name = QString("%1_%2").arg(typeName(d->type)).arg(index);
}

LaserPrimitive::~LaserPrimitive()
{
    qDebug() << objectName();
}

LaserDocument* LaserPrimitive::document() const 
{
    Q_D(const LaserPrimitive);
    return d->doc; 
}

void LaserPrimitive::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_D(LaserPrimitive);
    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);

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
    else if (d->isHover)
    {
        painter->setPen(QPen(Qt::green, 0.2f, Qt::SolidLine));
        painter->drawRect(bounds);
    }
    
    QColor color = Qt::blue;
    if (d->layer)
    {
        color = d->layer->color();
    }

    painter->setPen(QPen(color, 1, Qt::SolidLine));
    draw(painter);

    QPainterPath outline = this->outline();
    QPointF startPos = outline.pointAtPercent(0);
    painter->drawText(startPos, this->objectName());
}

QRectF LaserPrimitive::boundingRect() const
{
    Q_D(const LaserPrimitive);
    QRectF bounds = transform().mapRect(d->boundingRect);
    return bounds;
}

QPointF LaserPrimitive::laserStartPos() const
{
    QPointF pos = boundingRect().topLeft();
    pos *= 40;
    return pos;
}

LaserPrimitiveType LaserPrimitive::primitiveType() const 
{
    Q_D(const LaserPrimitive);
    return d->type; 
}

QString LaserPrimitive::typeName() const
{
    Q_D(const LaserPrimitive);
    return typeName(d->type);
}

QString LaserPrimitive::typeLatinName() const
{
    Q_D(const LaserPrimitive);
    return typeLatinName(d->type);
}

bool LaserPrimitive::isShape() const 
{
    Q_D(const LaserPrimitive);
    return (int)d->type <= (int)LPT_SHAPE; 
}

bool LaserPrimitive::isBitmap() const 
{
    Q_D(const LaserPrimitive);
    return d->type == LPT_BITMAP; 
}

QString LaserPrimitive::name() const 
{
    Q_D(const LaserPrimitive);
    return d->name; 
}

void LaserPrimitive::setName(const QString& name) 
{
    Q_D(LaserPrimitive);
    d->name = name; 
}

LaserLayer* LaserPrimitive::layer() const 
{
    Q_D(const LaserPrimitive);
    return d->layer; 
}

void LaserPrimitive::setLayer(LaserLayer* layer) 
{
    Q_D(LaserPrimitive);
    d->layer = layer; 
}

QPainterPath LaserPrimitive::outline() const
{
    Q_D(const LaserPrimitive);
    return d->outline;
}

QString LaserPrimitive::typeName(LaserPrimitiveType typeId) const
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

QString LaserPrimitive::typeLatinName(LaserPrimitiveType typeId) const
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
    Q_D(LaserPrimitive);
    d->isHover = true;
    update();
}

void LaserPrimitive::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_D(LaserPrimitive);
    d->isHover = false;
    update();
}

void LaserPrimitive::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsObject::mousePressEvent(event);
	if (!this->isSelected()) {
		LaserDocument* document = (LaserDocument*)this->parent();
		document->scene()->clearSelection();
	}
	
    update();
}

void LaserPrimitive::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsObject::mouseReleaseEvent(event);
    update();
}

class LaserShapePrivate : public LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserShape)
public:
    LaserShapePrivate(LaserShape* ptr)
        : LaserPrimitivePrivate(ptr)
    {}
};

LaserShape::LaserShape(LaserShapePrivate* data, LaserDocument* doc, LaserPrimitiveType type)
    : LaserPrimitive(data, doc, type)
{
    //m_type = LPT_SHAPE;
}

class LaserEllipsePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserEllipse)
public:
    LaserEllipsePrivate(LaserEllipse* ptr)
        : LaserShapePrivate(ptr)
    {}

    QRectF bounds;
};

LaserEllipse::LaserEllipse(const QRectF bounds, LaserDocument * doc)
    : LaserShape(new LaserEllipsePrivate(this), doc, LPT_ELLIPSE)
{
    Q_D(LaserEllipse);
    d->bounds = bounds;
    d->boundingRect = bounds;
    d->outline.addEllipse(bounds);
}

QRectF LaserEllipse::bounds() const 
{
    Q_D(const LaserEllipse);
    return d->bounds; 
} 

void LaserEllipse::setBounds(const QRectF& bounds) 
{
    Q_D(LaserEllipse);
    d->bounds = bounds; 
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
    Q_D(LaserEllipse);
    painter->drawEllipse(d->bounds);
}

QPainterPath LaserEllipse::toPath() const
{
    Q_D(const LaserEllipse);
    QPainterPath path;

    path.addEllipse(transform().mapRect(d->bounds));

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
    path = transform.map(path);
    return path;
}

class LaserRectPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserRect)
public:
    LaserRectPrivate(LaserRect* ptr)
        : LaserShapePrivate(ptr)
    {}

    QRectF rect;
};

LaserRect::LaserRect(const QRectF rect, LaserDocument * doc)
    : LaserShape(new LaserRectPrivate(this), doc, LPT_RECT)
{
    Q_D(LaserRect);
    d->rect = rect;
    d->boundingRect = rect;
    d->outline.addRect(rect);
}

QRectF LaserRect::rect() const 
{
    Q_D(const LaserRect);
    return d->rect; 
}

void LaserRect::setRect(const QRectF& rect) 
{
    Q_D(LaserRect);
    d->rect = rect; 
}

void LaserRect::draw(QPainter* painter)
{
    Q_D(LaserRect);
    painter->drawRect(d->rect);
}

std::vector<cv::Point2f> LaserRect::cuttingPoints(cv::Mat& canvas)
{
    Q_D(LaserRect);
	QTransform t = transform() * Global::matrixToMM(SU_PX, 40, 40);
    std::vector<cv::Point2f> points;
    cv::Point2f pt1 = typeUtils::qtPointF2CVPoint2f(t.map(d->rect.topLeft()));
    cv::Point2f pt2 = typeUtils::qtPointF2CVPoint2f(t.map(d->rect.topRight()));
    cv::Point2f pt3 = typeUtils::qtPointF2CVPoint2f(t.map(d->rect.bottomRight()));
    cv::Point2f pt4 = typeUtils::qtPointF2CVPoint2f(t.map(d->rect.bottomLeft()));
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
    Q_D(const LaserRect);
    QPainterPath path;
    QPolygonF rect = transform().map(d->rect);
    path.addPolygon(rect);

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
    path = transform.map(path);

    return path;
}

class LaserLinePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserLine)
public:
    LaserLinePrivate(LaserLine* ptr)
        : LaserShapePrivate(ptr)
    {}

    QLineF line;
};

LaserLine::LaserLine(const QLineF & line, LaserDocument * doc)
    : LaserShape(new LaserLinePrivate(this), doc, LPT_LINE)
{
    Q_D(LaserLine);
    d->line = line;
    d->boundingRect = QRectF(d->line.p1(), d->line.p2());
    d->outline.moveTo(line.p1());
    d->outline.lineTo(line.p2());
}

QLineF LaserLine::line() const 
{
    Q_D(const LaserLine);
    return d->line; 
}

void LaserLine::setLine(const QLineF& line) 
{
    Q_D(LaserLine);
    d->line = line; 
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
    Q_D(LaserLine);
    painter->drawLine(d->line);
}

QPainterPath LaserLine::toPath() const
{
    Q_D(const LaserLine);
    QPainterPath path;
    QLineF line = transform().map(d->line);
    path.moveTo(line.p1());
    path.lineTo(line.p2());

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
    path = transform.map(path);

    return path;
}

class LaserPathPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserPath)
public:
    LaserPathPrivate(LaserPath* ptr)
        : LaserShapePrivate(ptr)
    {}

    QPainterPath path;
};

LaserPath::LaserPath(const QPainterPath & path, LaserDocument * doc)
    : LaserShape(new LaserPathPrivate(this), doc, LPT_PATH)
{
    Q_D(LaserPath);
    d->path = path;
    d->boundingRect = path.boundingRect();
    d->outline.addPath(path);
}

QPainterPath LaserPath::path() const 
{
    Q_D(const LaserPath);
    return d->path; 
}

void LaserPath::setPath(const QPainterPath& path) 
{
    Q_D(LaserPath);
    d->path = path; 
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
    Q_D(LaserPath);
    painter->drawPath(d->path);
}

QPainterPath LaserPath::toPath() const
{
    Q_D(const LaserPath);
    QPainterPath path = d->path;
    path = transform().map(path);

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
    path = transform.map(path);
    return path;
}

QList<QPainterPath> LaserPath::subPaths() const
{
    QList<QPainterPath> paths;
    QPainterPath path = toPath();
    QList<QPolygonF> polys = path.toSubpathPolygons();
    qDebug() << "sub polys count:" << polys.count();
    for (QPolygonF& poly : polys)
    {
        QPainterPath subPath;
        subPath.addPolygon(poly);
        paths.append(subPath);
    }
    return paths;
}

class LaserPolylinePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserPolyline)
public:
    LaserPolylinePrivate(LaserPolyline* ptr)
        : LaserShapePrivate(ptr)
    {}

    QPolygonF poly;
};

LaserPolyline::LaserPolyline(const QPolygonF & poly, LaserDocument * doc)
    : LaserShape(new LaserPolylinePrivate(this), doc, LPT_POLYLINE)
{
    Q_D(LaserPolyline);
    d->poly = poly;
    d->boundingRect = poly.boundingRect();

    d->outline.moveTo(*poly.begin());
    for (int i = 1; i < poly.count(); i++)
    {
        d->outline.lineTo(poly[i]);
    }
}

QPolygonF LaserPolyline::polyline() const 
{
    Q_D(const LaserPolyline);
    return d->poly; 
}

void LaserPolyline::setPolyline(const QPolygonF& poly) 
{
    Q_D(LaserPolyline);
    d->poly = poly; 
}

std::vector<cv::Point2f> LaserPolyline::cuttingPoints(cv::Mat & canvas)
{
    Q_D(LaserPolyline);
    std::vector<cv::Point2f> points;
    cv::Point2f lastPt;
    QTransform t = transform() * Global::matrixToMM(SU_PX, 40, 40);
    for (int i = 0; i < d->poly.size(); i++)
    {
        QPointF pt = t.map(d->poly[i]);
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
    Q_D(LaserPolyline);
    painter->drawPolyline(d->poly);
}

QPainterPath LaserPolyline::toPath() const
{
    Q_D(const LaserPolyline);
    QPainterPath path;
    QPolygonF rect = transform().map(d->poly);
    path.addPolygon(rect);

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
    path = transform.map(path);

    return path;
}

class LaserPolygonPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserPolygon)
public:
    LaserPolygonPrivate(LaserPolygon* ptr)
        : LaserShapePrivate(ptr)
    {}

    QPolygonF poly;
};

LaserPolygon::LaserPolygon(const QPolygonF & poly, LaserDocument * doc)
    : LaserShape(new LaserPolygonPrivate(this), doc, LPT_POLYGON)
{
    Q_D(LaserPolygon);
    d->poly = poly;
    d->boundingRect = poly.boundingRect();
    d->outline.addPolygon(poly);
}

QPolygonF LaserPolygon::polyline() const 
{
    Q_D(const LaserPolygon);
    return d->poly; 
}

void LaserPolygon::setPolyline(const QPolygonF& poly) 
{
    Q_D(LaserPolygon);
    d->poly = poly; 
}

std::vector<cv::Point2f> LaserPolygon::cuttingPoints(cv::Mat & canvas)
{
    Q_D(LaserPolygon);
    std::vector<cv::Point2f> points;
    cv::Point2f lastPt;
    QTransform t = transform() * Global::matrixToMM(SU_PX, 40, 40);
    for (int i = 0; i < d->poly.size(); i++)
    {
        QPointF pt = t.map(d->poly[i]);
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
    Q_D(LaserPolygon);
    painter->drawPolygon(d->poly);
}

QPainterPath LaserPolygon::toPath() const
{
    Q_D(const LaserPolygon);
    QPainterPath path;
    QPolygonF poly = transform().map(d->poly);
    path.addPolygon(poly);

    QTransform transform = Global::matrixToMM(SU_PX, 40, 40);
    path = transform.map(path);

    return path;
}

class LaserBitmapPrivate : public LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserBitmap)
public:
    LaserBitmapPrivate(LaserBitmap* ptr)
        : LaserPrimitivePrivate(ptr)
    {}

    QImage image;
    QRectF bounds;
};

LaserBitmap::LaserBitmap(const QImage & image, const QRectF& bounds, LaserDocument * doc)
    : LaserPrimitive(new LaserBitmapPrivate(this), doc, LPT_BITMAP)
{
    Q_D(LaserBitmap);
    d->image = image.convertToFormat(QImage::Format_Grayscale8);
    d->boundingRect = bounds;
    d->type = LPT_BITMAP;
    d->outline.addRect(bounds);
}

QImage LaserBitmap::image() const 
{
    Q_D(const LaserBitmap);
    return d->image; 
}

void LaserBitmap::setImage(const QImage& image)
{
    Q_D(LaserBitmap);
    d->image = image;
}

QRectF LaserBitmap::bounds() const 
{
    Q_D(const LaserBitmap);
    return d->bounds; 
}

QByteArray LaserBitmap::engravingImage(cv::Mat& canvas)
{ 
    Q_D(LaserBitmap);
    QByteArray ba;

    cv::Mat src(d->image.height(), d->image.width(), CV_8UC1, (void*)d->image.constBits(), d->image.bytesPerLine());
    float mmWidth = 1000.f * d->image.width() / d->image.dotsPerMeterX();
    float mmHeight = 1000.f * d->image.height() / d->image.dotsPerMeterY();

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

	qreal boundingWidth = Global::convertToMM(SU_PX, boundingRect().width());
	qreal boundingHeight = Global::convertToMM(SU_PX, boundingRect().height(), Qt::Vertical);
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

    QTransform t = transform().scale(Global::convertToMM(SU_PX, 1), Global::convertToMM(SU_PX, 1, Qt::Vertical));
    if (!canvas.empty())
    {
        try
        {
            QRectF bounds = t.mapRect(d->boundingRect);
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
    Q_D(LaserBitmap);
    painter->drawImage(d->bounds, d->image);
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

QByteArray LaserShape::engravingImage(cv::Mat& canvas)
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

class LaserTextPrivate : public LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserText)
public:
    LaserTextPrivate(LaserText* ptr)
        : LaserPrimitivePrivate(ptr)
    {}

	QString content;
	QRect rect;
};

LaserText::LaserText(const QRect rect, const QString content, LaserDocument* doc, LaserPrimitiveType type)
	: LaserPrimitive(new LaserTextPrivate(this), doc, LPT_TEXT)
{
    Q_D(LaserText);
    d->rect = rect;
    d->content = content;
	d->boundingRect = rect;
    d->outline.addRect(rect);
}

QRect LaserText::rect() const 
{
    Q_D(const LaserText);
    return d->rect; 
}

QString LaserText::content() const 
{
    Q_D(const LaserText);
    return d->content; 
}

void LaserText::draw(QPainter * painter)
{
    Q_D(LaserText);
	QTextDocument doc;
	doc.setHtml(d->content);
	painter->translate(d->rect.topLeft());
	doc.drawContents(painter, QRect(0, 0, d->rect.width(), d->rect.height()));
}
