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
#include "scene/LaserNodePrivate.h"

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

class LaserPrimitivePrivate: public LaserNodePrivate
{
    Q_DECLARE_PUBLIC(LaserPrimitive)
public:
    LaserPrimitivePrivate(LaserPrimitive* ptr)
        : LaserNodePrivate(ptr)
        , doc(nullptr)
        , layer(nullptr)
        , isHover(false)
        , type(LPT_UNKNOWN)
    {}

    LaserDocument* doc;
    LaserLayer* layer;
    QRectF boundingRect;
    LaserPrimitiveType type;
    bool isHover;
    QPainterPath outline;
    std::vector<cv::Point2f> mechiningPoints;
    QList<int> startingIndices;
};

LaserPrimitive::LaserPrimitive(LaserPrimitivePrivate* data, LaserDocument* doc, LaserPrimitiveType type)
    : LaserNode(data, LNT_PRIMITIVE)
{
    Q_D(LaserPrimitive);
    d->doc = doc;
    d->type = type;
    Q_ASSERT(doc);
    QObject::setParent(doc);

    g_counter[type]++;

    //this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    //this->setFlag(ItemIsFocusable, true);

    //this->setAcceptHoverEvents(true);
	this->setAcceptedMouseButtons(false);

    d->nodeName = QString("%1_%2").arg(typeLatinName(type)).arg(g_counter[type]);
}

LaserPrimitive::~LaserPrimitive()
{
    Q_D(LaserPrimitive);
    qDebug() << d->nodeName;
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
    
    
    QColor color = Qt::blue;
    if (d->layer)
    {
        color = d->layer->color();
    }

    painter->setPen(QPen(color, 1, Qt::SolidLine));
	if (isSelected())
	{
		painter->setPen(QPen(Qt::black, 1.2f, Qt::DashLine));
	
		//painter->drawRect(bounds);
	}
	//else if (isUnderMouse())
	else if (d->isHover)
	{
		painter->setPen(QPen(Qt::green, 0.2f, Qt::SolidLine));
		painter->drawRect(bounds);
	}
    draw(painter);

    QPainterPath outline = this->outline();
    QPointF startPos = outline.pointAtPercent(0);
    painter->setPen(QPen(Qt::green, 1, Qt::SolidLine));
    painter->drawText(startPos, name());
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

std::vector<cv::Point2f> LaserPrimitive::mechiningPoints() const
{
    Q_D(const LaserPrimitive);
    return d->mechiningPoints;
}

std::vector<cv::Point2f> LaserPrimitive::mechiningPoints(cv::Point2f& lastPoint, int pointIndex, cv::Mat& canvas) const
{
    Q_D(const LaserPrimitive);
    int pointsCount = d->mechiningPoints.size();
    cv::Point2f head = d->mechiningPoints[0];
    cv::Point2f tail = d->mechiningPoints[pointsCount - 1];
    double distBetweenHeadAndTail = cv::norm(head - tail);
    std::vector<cv::Point2f> points;

    // check the primitive is whether closour
    if (qFuzzyCompare(distBetweenHeadAndTail, 0))
    {
        int prevIndex = (pointIndex - 1 + pointsCount) % pointsCount;
        int nextIndex = (pointIndex + 1) % pointsCount;
        cv::Point2f currentPoint = d->mechiningPoints[pointIndex];
        cv::Point2f prevPoint = d->mechiningPoints[prevIndex];
        cv::Point2f nextPoint = d->mechiningPoints[nextIndex];

        cv::Vec2f inDir = currentPoint - lastPoint;
        cv::Vec2f prevDir = prevPoint - currentPoint;
        cv::Vec2f nextDir = nextPoint - currentPoint;

        int prevScore = 0;
        if (inDir[0] * prevDir[0] >= 0)
            prevScore++;
        if (inDir[1] * prevDir[1] >= 0)
            prevScore++;

        int nextScore = 0;
        if (inDir[0] * nextDir[0] >= 0)
            nextScore++;
        if (inDir[1] * nextDir[1] >= 0)
            nextScore++;

        int step = 0;
        if (nextScore >= prevScore)
        {
            step = 1;
        }
        else
        {
            step = -1;
        }

        int cursor = pointIndex;
        points.reserve(pointsCount);
        for (int i = 0; i < pointsCount; i++)
        {
            cv::Point2f point = d->mechiningPoints[cursor];
            points.push_back(point);
            cursor = (cursor + step + pointsCount) % pointsCount;
        }

        lastPoint = currentPoint;
    }
    else
    {
        if (pointIndex == 0)
        {
            points = d->mechiningPoints;
            lastPoint = tail;
        }
        else
        {
            for (int i = pointsCount - 1; i >= 0; i--)
            {
                points.push_back(d->mechiningPoints[i]);
            }
            lastPoint = head;
        }
    }

    if (!canvas.empty())
    {
        cv::Mat pointsMat(points);
        pointsMat.convertTo(pointsMat, CV_32S);
        cv::polylines(canvas, pointsMat, true, cv::Scalar(0, 0, 255), 5);
    }
    
    return points;
}

QList<int> LaserPrimitive::startingIndices() const
{
    Q_D(const LaserPrimitive);
    return d->startingIndices;
}

std::vector<cv::Point2f> LaserPrimitive::startingPoints() const
{
    Q_D(const LaserPrimitive);

    std::vector<cv::Point2f> indices;
    if (d->mechiningPoints.empty())
    {
        return indices;
    }

    for (int i = 0; i < d->startingIndices.length(); i++)
    {
        indices.push_back(d->mechiningPoints[d->startingIndices[i]]);
    }

    return indices;
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
    return d->nodeName; 
}

void LaserPrimitive::setName(const QString& name) 
{
    Q_D(LaserPrimitive);
    d->nodeName = name; 
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

QString LaserPrimitive::toString() const
{
    return QString("[LaserPrimitive: name=%1]").arg(nodeName());
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
    qLogD << "mousePressEvent: " << nodeName();
	/*if (!this->isSelected()) {
		LaserDocument* document = (LaserDocument*)this->QObject::parent();
		document->scene()->clearSelection();
	}*/
	
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

    d->position = bounds.center();
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
    Q_D(LaserEllipse);
    QPainterPath path = toPath();

    std::vector<cv::Point2f> points;
    pltUtils::path2Points(path, points, canvas);
    d->mechiningPoints = points;
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
    d->position = rect.center();
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

    d->mechiningPoints = points;
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
    d->position = line.center();
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
    Q_D(LaserLine);
    std::vector<cv::Point2f> points;
    QPainterPath path = toPath();
    
    pltUtils::path2Points(path, points, canvas);
    
    d->mechiningPoints = points;
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
    d->position = d->boundingRect.center();
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
    Q_D(LaserPath);
    std::vector<cv::Point2f> points;
    QPainterPath path = toPath();
    
    pltUtils::path2Points(path, points, canvas);
    
    d->mechiningPoints = points;
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
    //qDebug() << "sub polys count:" << polys.count();
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

    d->position = *poly.begin();
    d->outline.moveTo(*poly.begin());
    for (int i = 1; i < poly.count(); i++)
    {
        d->position += poly[i];
        d->outline.lineTo(poly[i]);
    }

    d->position /= poly.count();
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
    
    d->mechiningPoints = points;
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
    
    for (int i = 0; i < poly.count(); i++)
    {
        d->position += poly[i];
    }
    d->position /= poly.count();
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
    
    d->mechiningPoints = points;
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
    //QRectF bounds;
};

LaserBitmap::LaserBitmap(const QImage & image, const QRectF& bounds, LaserDocument * doc)
    : LaserPrimitive(new LaserBitmapPrivate(this), doc, LPT_BITMAP)
{
    Q_D(LaserBitmap);
    d->image = image.convertToFormat(QImage::Format_Grayscale8);
    d->boundingRect = bounds;
    d->type = LPT_BITMAP;
    d->outline.addRect(bounds);
    d->position = bounds.center();
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
    return d->boundingRect; 
}

QByteArray LaserBitmap::engravingImage(cv::Mat& canvas)
{ 
    Q_D(LaserBitmap);
    QByteArray ba;

    cv::Mat src(d->image.height(), d->image.width(), CV_8UC1, (void*)d->image.constBits(), d->image.bytesPerLine());
    //float mmWidth = 1000.f * d->image.width() / d->image.dotsPerMeterX();
    //float mmHeight = 1000.f * d->image.height() / d->image.dotsPerMeterY();

    int scanInterval = 7;
    double yPulseLength = 0.006329114;
    QVariant value;
    if (LaserDriver::instance().getRegister(LaserDriver::RT_ENGRAVING_ROW_STEP, value))
    {
        qDebug() << "row step register:" << value;
        scanInterval = value.toInt();
    }
    if (LaserDriver::instance().getRegister(LaserDriver::RT_X_AXIS_PULSE_LENGTH, value))
    {
        qDebug() << "y pulse register:" << value;
        yPulseLength = value.toDouble() / 1000.0;
    }
    qreal pixelInterval = scanInterval * yPulseLength;

	qreal boundingWidth = Global::convertToMM(SU_PX, d->boundingRect.width());
	qreal boundingHeight = Global::convertToMM(SU_PX, d->boundingRect.height(), Qt::Vertical);
    qreal boundingLeft = Global::convertToMM(SU_PX, d->boundingRect.left());
    qreal boundingTop = Global::convertToMM(SU_PX, d->boundingRect.top());
    int outWidth = boundingWidth * MM_TO_INCH * 600;
    int outHeight = std::round(boundingHeight / pixelInterval);
    qLogD << "bounding rect: " << d->boundingRect;
    qDebug() << "out width:" << outWidth;
    qDebug() << "out height:" << outHeight;
    qDebug() << "out left:" << boundingLeft;
    qDebug() << "out top:" << boundingTop;

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(outWidth, outHeight));
    
    cv::Mat outMat = resized;
    if (layer()->useHalftone())
    {
        outMat = imageUtils::halftone3(resized, layer()->lpi(), layer()->dpi(), 45);
    }

    ba = imageUtils::image2EngravingData(outMat, boundingLeft, boundingTop, pixelInterval, boundingWidth);

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
    painter->drawImage(d->boundingRect, d->image);
}

std::vector<cv::Point2f> LaserBitmap::cuttingPoints(cv::Mat& canvas)
{
    Q_D(LaserBitmap);
	QTransform t = transform() * Global::matrixToMM(SU_PX, 40, 40);
    std::vector<cv::Point2f> points;
    cv::Point2f pt1 = typeUtils::qtPointF2CVPoint2f(t.map(d->boundingRect.topLeft()));
    cv::Point2f pt2 = typeUtils::qtPointF2CVPoint2f(t.map(d->boundingRect.topRight()));
    cv::Point2f pt3 = typeUtils::qtPointF2CVPoint2f(t.map(d->boundingRect.bottomRight()));
    cv::Point2f pt4 = typeUtils::qtPointF2CVPoint2f(t.map(d->boundingRect.bottomLeft()));
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

    d->mechiningPoints = points;
    return points;
}

QDebug operator<<(QDebug debug, const LaserPrimitive & item)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "(" << item.name() << ", " << item.type() << ")";
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
    d->position = rect.center();
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
