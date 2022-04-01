#include "Utils.h"
#include <QUuid>
#include <QtMath>
#include <QMessageBox>
#include "Eigen/Core"
#include "Eigen/Dense"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

#include "LaserApplication.h"
#include "ui/LaserControllerWindow.h"
#include "laser/LaserDevice.h"
#include "scene/LaserPrimitive.h"
#include "scene/LaserLayer.h"

QString utils::createUUID(const QString& prefix)
{
    QString ret = prefix;
    ret.append(QUuid::createUuid().toString());
    return ret;
}

int utils::parsePortName(const QString & name)
{
    QRegExp re(".*COM(\\d+)");
    re.indexIn(name);
    QString portName = re.cap(1);
    bool ok = false;
    int port = portName.toInt(&ok);
    return port;
}

QFrame * utils::createSeparator(int width, int height, QFrame::Shape shape, QFrame::Shadow shadow)
{
    QFrame* separator = new QFrame;
    separator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    if (width)
    {
        separator->setFixedWidth(width);
    }
    if (height)
    {
        separator->setFixedHeight(height);
    }
    separator->setFrameShape(shape);
    separator->setFrameShadow(shadow);
    return separator;
}

QVector3D utils::putToQuadrant(const QVector3D & pos, QUADRANT quadrant)
{
    float x = pos.x();
    float y = pos.y();
    float z = pos.z();
    switch (quadrant)
    {
    case QUADRANT_1:
    {
        if (x < 0) x = -x;
        if (y < 0) y = -y;
    }
        break;
    case QUADRANT_2:
    {
        if (x > 0) x = -x;
        if (y < 0) y = -y;
    }
        break;
    case QUADRANT_3:
    {
        if (x > 0) x = -x;
        if (y > 0) y = -y;
    }
        break;
    case QUADRANT_4:
    {
        if (x < 0) x = -x;
        if (y > 0) y = -y;
    }
        break;
    }

    return QVector3D(x, y, z);
}

QVector3D utils::limitToLayout(const QVector3D & pos, int quadrant, float width, float height)
{
    float x = pos.x();
    float y = pos.y();
    switch (quadrant)
    {
    case 0:
    {
        x = qBound(0.f, x, width);
        y = qBound(0.f, y, height);
    }
        break;
    case 1:
    {
        x = qBound(-width, x, 0.f);
        y = qBound(0.f, y, height);
    }
        break;
    case 2:
    {
        x = qBound(-width, x, 0.f);
        y = qBound(-height, y, 0.f);
    }
        break;
    case 3:
    {
        x = qBound(0.f, x, width);
        y = qBound(-height, y, 0.f);
    }
        break;
    }

    return QVector3D(x, y, pos.z());
}

bool utils::checkTwoPointEqueal(const QPointF & point1, const QPointF & point2, float scop)
{
	qreal distance = QVector2D(point2 - point1).length();
	if (distance <= scop)
	{
		return true;
	}
	return false;
}

bool utils::fuzzyEquals(const QPointF& pt1, const QPointF& pt2)
{
    qreal length = QVector2D(pt1 - pt2).length();
    return qFuzzyIsNull(length);
}

bool utils::fuzzyEquals(qreal a, qreal b, qreal limit)
{
    return qAbs(a - b) < limit;
}

bool utils::fuzzyCompare(const QPointF& p1, const QPointF& p2)
{
    return qFuzzyCompare(p1.x(), p2.x()) &&
        qFuzzyCompare(p1.y(), p2.y());
}

void utils::sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem * item)
{
	item->setTransform(sceneTransform);
	item->setPos(0, 0);
}
quint32 utils::factorial(quint32 n)
{
    if (n == 0)
        return 1;
    return n * factorial(n - 1);
}

QPointF utils::center(const QVector<QPointF>& points)
{
    QPointF center(0, 0);
    if (points.isEmpty())
        return center;

    for (const QPointF& p : points)
    {
        center += p;
    }
    center /= points.size();
    return center;
}

LaserPoint utils::center(const LaserPointList& points)
{
    LaserPoint center(0, 0/*, 0, 0*/);
    if (points.isEmpty())
        return center;

    for (const LaserPoint& p : points)
    {
        center += p;
    }
    center /= points.size();
    return center;
}

void utils::boundingRect(const QList<QGraphicsItem*>& primitives, QRect& bounding, QRect& boundingAcc, bool exludeUnexport)
{
    int count = 0;
    int accCount = 0;
    bounding = QRect();
    boundingAcc = QRect();
    for (QGraphicsItem* item : primitives)
    {
        LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
        if (exludeUnexport && !primitive->exportable())
            continue;
        if (computeBoundingRect(primitive, bounding, boundingAcc, count, accCount, exludeUnexport)) {
            continue;
        }
    }
}

void utils::boundingRect(const QList<LaserPrimitive*>& primitives, QRect& bounding, QRect& boundingAcc, bool exludeUnexport)
{
    int count = 0;
    int accCount = 0;
    bounding = QRect();
    boundingAcc = QRect();
    for (LaserPrimitive* primitive : primitives)
    {
        if (exludeUnexport && !primitive->exportable())
            continue;
        if (computeBoundingRect(primitive, bounding, boundingAcc, count, accCount, exludeUnexport)) {
            continue;
        }
    }
}

void utils::boundingRect(const QSet<LaserPrimitive*>& primitives, QRect & bounding, QRect & boundingAcc, bool exludeUnexport)
{
    int count = 0;
    int accCount = 0;
    bounding = QRect();
    boundingAcc = QRect();
    for (QSet<LaserPrimitive*>::const_iterator p = primitives.begin();
                                    p != primitives.end(); p++)
    {
        if (exludeUnexport && !(*p)->exportable())
            continue;
        if (computeBoundingRect(*p, bounding, boundingAcc, count, accCount, exludeUnexport)) {
            continue;
        }
    }
}

bool utils::computeBoundingRect(LaserPrimitive* primitive, QRect& bounding, QRect& boundingAcc, int& count, int& accCount, bool exludeUnexport)
{
    QRect rect = primitive->sceneBoundingRect();
    QRect rectAcc = rect;
    LaserLayer* layer = primitive->layer();
    if (primitive->isBitmap() ||
        (layer->type() == LLT_FILLING && layer->fillingType() == FT_Pixel &&
        (primitive->isShape() || primitive->isText())))
    {
        if (accCount++ == 0)
        {
            boundingAcc = rectAcc;
        }
        else
        {
            if (rectAcc.left() < boundingAcc.left())
                boundingAcc.setLeft(rectAcc.left());
            if (rectAcc.top() < boundingAcc.top())
                boundingAcc.setTop(rectAcc.top());
            if (rectAcc.right() > boundingAcc.right())
                boundingAcc.setRight(rectAcc.right());
            if (rectAcc.bottom() > boundingAcc.bottom())
                boundingAcc.setBottom(rectAcc.bottom());
        }
    }

    if (count++ == 0)
    {
        bounding = rect;
        return true;
    }
    if (rect.left() < bounding.left())
        bounding.setLeft(rect.left());
    if (rect.top() < bounding.top())
        bounding.setTop(rect.top());
    if (rect.right() > bounding.right())
        bounding.setRight(rect.right());
    if (rect.bottom() > bounding.bottom())
        bounding.setBottom(rect.bottom());

    return false;
}

LaserLineListList utils::interLines(const QPainterPath& path, qreal rowInterval)
{
    LaserLineListList lineList;
    QRectF boundingRect = path.boundingRect();

    qreal y = boundingRect.top();
    for (; y <= boundingRect.bottom(); y += rowInterval)
    {
        QRectF intersectRect(boundingRect.left() - 10, y, boundingRect.width() + 10, 1);
        QPainterPath intersectPath;
        intersectPath.addRect(intersectRect);

        QPainterPath intersected = intersectPath.intersected(path);
        QMap<qreal, qreal> linePoints;
        for (int i = 0; i < intersected.elementCount(); i++)
        {
            QPainterPath::Element e = intersected.elementAt(i);
            //qDebug() << i << e.x << e.y << e.type;
            if (utils::fuzzyEquals(e.y, y, 0.1))
            {
                linePoints.insert(e.x, e.x);
            }
        }
        qreal last;
        int i = 0;
        LaserLineList lines;
        for (qreal curr : linePoints)
        {
            //qDebug() << curr;
            if (i++ == 0)
            {
                last = curr;
                continue;
            }
            qreal mean = (last + curr) / 2;

            QPointF pt(mean, y);
            if (path.contains(pt))
            {
                qreal x1 = last;
                qreal x2 = curr;
                if (x1 > x2) qSwap(x1, x2);
                lines.append(QLineF(QPointF(x1, y), QPointF(x2, y)).toLine());
            }
            last = curr;
        }
        if (!lines.empty())
            lineList.append(lines);
    }
    qLogD << lineList.count() << " lines generated";
    qLogD << boundingRect.height() / rowInterval << " lines expected";

    return lineList;
}

RELATION utils::determineRelationship(const QPainterPath& a, const QPainterPath& b)
{
    RELATION rel = determineRelationship(a.boundingRect(), b.boundingRect());
    if (rel == RELATION::A_CONTAINS_B)
    {
        if (a.contains(b))
            return rel;
        else
            rel = RELATION::NONE;
    }
    else if (rel == RELATION::B_CONTAINS_A)
    {
        if (b.contains(a))
            return rel;
        else
            rel = RELATION::NONE;
    }
    else
    {
        rel = RELATION::NONE;
    }
    //if (a.contains(b))
    //{
    //    // candidate primitive contains tree node primitive
    //    rel = A_CONTAINS_B;
    //}
    //else if (b.contains(a))
    //{
    //    // tree node primitive contains candidate primitive
    //    rel = B_CONTAINS_A;
    //}
    //else if (a.intersects(b))
    //{
    //    // a intersects with b
    //    rel = INTERSECTION;
    //}
    //else
    //{
    //    // no relationship between candidate primitive and tree node primitive
    //    rel = RELATION::NONE;
    //}
    return rel;
}

RELATION utils::determineRelationship(const QRectF& a, const QRectF& b)
{
    RELATION hRel = RELATION::NONE;
    RELATION vRel = RELATION::NONE;
    qreal left = qMin(a.left(), b.left());
    qreal right = qMax(a.right(), b.right());
    qreal top = qMin(a.top(), b.top());
    qreal bottom = qMax(a.bottom(), b.bottom());
    qreal interWidth = right - left;
    qreal interHeight = bottom - top;
    qreal hSum = a.width() + b.width();
    qreal vSum = a.height() + b.height();

    if (interWidth < hSum)
    {
        hRel = RELATION::INTERSECTION;
        if (interWidth == a.width())
        {
            hRel = RELATION::A_CONTAINS_B;
        }
        else if (interWidth == b.width())
        {
            hRel = RELATION::B_CONTAINS_A;
        }
    }

    if (interHeight < vSum)
    {
        vRel = RELATION::INTERSECTION;
        if (interHeight == a.height())
        {
            vRel = RELATION::A_CONTAINS_B;
        }
        else if (interHeight == b.height())
        {
            vRel = RELATION::B_CONTAINS_A;
        }
    }

    RELATION rel = RELATION::NONE;
    if (vRel == RELATION::INTERSECTION && hRel == RELATION::INTERSECTION)
    {
        rel = RELATION::INTERSECTION;
    }
    if (vRel == A_CONTAINS_B && hRel == A_CONTAINS_B)
    {
        rel = A_CONTAINS_B;
    }
    if (vRel == B_CONTAINS_A && hRel == B_CONTAINS_A)
    {
        rel = B_CONTAINS_A;
    }

    return rel;
}

QTransform utils::leastSquare4d(const PointPairList& pointPairs, const QPointF& offset)
{
    // solve Ax=b
    // 
    // A = | sigma(x1^2 + y1^2)                   0  sigma(x1)  sigma(y1) |
    //     |                  0  sigma(x1^2 + y1^2) -sigma(y1)  sigma(x1) |
    //     |          sigma(x1)          -sigma(y1)          1          0 |
    //     |          sigma(y1)           sigma(x1)          0          1 |
    // 
    // b = | sigma(x1 * x2 + y1 * y2) |
    //     | sigma(x1 * y2 - x2 * y1) |
    //     |                sigma(x2) |
    //     |                sigma(y2) |
    //
    // x = | a |
    //     | b |
    //     | c |
    //     | d |
    //
    // a = cos(angle), b = sin(angle), c = dx, d = dy

    qreal sumX1 = 0;
    qreal sumX2 = 0;
    qreal sumY1 = 0;
    qreal sumY2 = 0;
    qreal sumX12AY12 = 0;
    qreal sumX1X2AY1Y2 = 0;
    qreal sumX1Y2SX2Y1 = 0;
    qreal i = pointPairs.size();

    for (const PointPair& pair : pointPairs)
    {
        qreal x1 = pair.second.x();
        qreal y1 = pair.second.y();
        qreal x2 = pair.first.x() + offset.x();
        qreal y2 = pair.first.y() + offset.y();

        qreal x12 = x1 * x1;
        qreal y12 = y1 * y1;
        qreal x1x2 = x1 * x2;
        qreal y1y2 = y1 * y2;
        qreal x1y2 = x1 * y2;
        qreal x2y1 = x2 * y1;

        sumX1 += x1;
        sumX2 += x2;
        sumY1 += y1;
        sumY2 += y2;
        sumX12AY12 += x12 + y12;
        sumX1X2AY1Y2 += x1x2 + y1y2;
        sumX1Y2SX2Y1 += x1y2 - x2y1;
    }

    Eigen::MatrixXd A(4, 4);
    A << sumX12AY12,          0,  sumX1, sumY1,
                  0, sumX12AY12, -sumY1, sumX1,
              sumX1,      -sumY1,    i,      0,
              sumY1,       sumX1,    0,      i;
    Eigen::Vector4d b;
    b << sumX1X2AY1Y2, 
         sumX1Y2SX2Y1, 
                sumX2, 
                sumY2;
    std::cout << A << std::endl << std::endl;
    std::cout << b << std::endl << std::endl;

    Eigen::MatrixXd x = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
    std::cout << x << std::endl << std::endl;

    qreal angle1 = qRadiansToDegrees(qAcos(x(0)));
    qreal angle2 = qRadiansToDegrees(qAsin(x(1)));
    qLogD << "angle1: " << angle1;
    qLogD << "angle2: " << angle2;

    QTransform t(x(0), x(1), -x(1), x(0), x(2), x(3));
    qLogD << t;
    QLineF vl1(0, 0, 1, 0);
    QLineF vl2 = t.map(vl1);
    qreal sx = vl2.length() / vl1.length();
    QLineF hl1(0, 0, 0, 1);
    QLineF hl2 = t.map(hl1);
    qreal sy = hl2.length() / hl1.length();
    qLogD << "sx: " << sx << ", sy: " << sy;

    QTransform invScaleT = QTransform::fromScale(1 / sx, 1 / sy);
    t = t * invScaleT;
    qLogD << t;

    return t;
}

QTransform utils::leastSquare6d(const PointPairList& pointPairs, const QPointF& offset)
{
    // solve Ax=b
    // 
    // A = | sigma(x^2) sigma(x*y)          0          0    sigma(x)        0 |
    //     | sigma(x*y) sigma(y^2)          0          0    sigma(y)        0 |
    //     |          0          0 sigma(x^2) sigma(x*y)           0 sigma(x) |
    //     |          0          0 sigma(x*y) sigma(y^2)           0 sigma(y) |
    //     |   sigma(x)   sigma(y)          0          0           i        0 |
    //     |          0          0   sigma(x)   sigma(y)           0        i |
    // 
    // b = | sigma(x*m) |
    //     | sigma(y*m) |
    //     | sigma(x*n) |
    //     | sigma(y*n) |
    //     |   sigma(m) |
    //     |   sigma(n) |
    //
    // x = | a |
    //     | b |
    //     | c |
    //     | d |
    //     | e |
    //     | f |
    //
    // a = cos(angle), b = sin(angle), c = dx, d = dy

    qreal sumX = 0;
    qreal sumY = 0;
    qreal sumX2 = 0;
    qreal sumY2 = 0;
    qreal sumXY = 0;
    qreal sumXM = 0;
    qreal sumYM = 0;
    qreal sumXN = 0;
    qreal sumYN = 0;
    qreal sumM = 0;
    qreal sumN = 0;
    qreal i = pointPairs.size();

    for (const PointPair& pair : pointPairs)
    {
        qreal x = pair.second.x();
        qreal y = pair.second.y();
        qreal m = pair.first.x() + offset.x();
        qreal n = pair.first.y() + offset.y();

        sumX += x;
        sumY += y;
        sumX2 += x * x;
        sumY2 += y * y;
        sumXY += x * y;
        sumXM += x * m;
        sumYM += y * m;
        sumXN += x * n;
        sumYN += y * n;
        sumM += m;
        sumN += n;
    }

    Eigen::MatrixXd A(6, 6);
    A << sumX2, sumXY,     0,     0, sumX,    0,
         sumXY, sumY2,     0,     0, sumY,    0,
             0,     0, sumX2, sumXY,    0, sumX,
             0,     0, sumXY, sumY2,    0, sumY,
          sumX,  sumY,     0,     0,    i,    0,
             0,     0,  sumX,  sumY,    0,    i;

    Eigen::VectorXd b(6);
    b << sumXM,
         sumYM,
         sumXN,
         sumYN,
          sumM,
          sumN;

    std::cout << A << std::endl << std::endl;
    std::cout << b << std::endl << std::endl;

    Eigen::MatrixXd x = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
    std::cout << x << std::endl << std::endl;

    qreal angle1 = qRadiansToDegrees(qAcos(x(0)));
    qreal angle2 = qRadiansToDegrees(qAsin(x(1)));
    qLogD << "angle1: " << angle1;
    qLogD << "angle2: " << angle2;

    QTransform t(
        x(0), x(2), 0,
        x(1), x(3), 0,
        x(4), x(5), 1
    );
    qLogD << t;
    QLineF vl1(0, 0, 1, 0);
    QLineF vl2 = t.map(vl1);
    qreal sx = vl2.length() / vl1.length();
    QLineF hl1(0, 0, 0, 1);
    QLineF hl2 = t.map(hl1);
    qreal sy = hl2.length() / hl1.length();
    qLogD << "sx: " << sx << ", sy: " << sy;

    QTransform invScaleT = QTransform::fromScale(1 / sx, 1 / sy);
    t = t * invScaleT;
    qLogD << t;
    QLineF l1(0, 0, 1, 0);
    QLineF l2 = t.map(l1);

    angle1 = qRadiansToDegrees(qAcos(t.m11()));
    angle2 = qRadiansToDegrees(qAsin(t.m12()));
    qLogD << "angle1: " << angle1;
    qLogD << "angle2: " << angle2;

    qreal angle = l2.angleTo(l1);
    qLogD << "angle: " << angle;

    return t;
}

QTransform utils::transformFrom2Points(const PointPairList& pointPairs)
{
    QPointF srcPt1(pointPairs.at(0).first.x(), pointPairs.at(0).first.y());
    QPointF dstPt1(pointPairs.at(0).second.x(), pointPairs.at(0).second.y());
    QPointF srcPt2(pointPairs.at(1).first.x(), pointPairs.at(1).first.y());
    QPointF dstPt2(pointPairs.at(1).second.x(), pointPairs.at(1).second.y());

    QLineF l1(srcPt1, srcPt2);
    QLineF l2(dstPt1, dstPt2);
    qreal angle = l1.angleTo(l2);
    QTransform t;
    t.rotate(angle);
    QLineF l22 = t.map(l2);
    QPointF diff1 = l1.p1() - l22.p1();
    QPointF diff2 = l1.p2() - l22.p2();
    qLogD << "l1: " << l1;
    qLogD << "l2: " << l22;
    qLogD << "diff1: " << diff1 << ", diff2: " << diff2;
    QPointF diff = (diff1 + diff2) / 2;
    t = QTransform(t.m11(), t.m12(), t.m21(), t.m22(), diff.x(), diff.y());
    qLogD << t;

    return t;
}

void utils::rectEdges(QRectF rect, QList<QLineF>& edges)
{
    //selection lines
    edges.append(QLineF(rect.topLeft(), rect.topRight()));
    edges.append(QLineF(rect.topRight(), rect.bottomRight()));
    edges.append(QLineF(rect.bottomLeft(), rect.bottomRight()));
    edges.append(QLineF(rect.topLeft(), rect.bottomLeft()));
}

void utils::warning(const QString& title, const QString& msg, QWidget* parent)
{
    QWidget* parentWnd = parent ? parent : LaserApplication::mainWindow;
    QMessageBox dlg(QMessageBox::Warning, title, msg, QMessageBox::Ok);
    dlg.setButtonText(QMessageBox::Ok, QObject::tr("Ok"));
    dlg.setParent(parent);
    int result = dlg.exec();
}

void utils::makePointsRelative(QList<QPoint>& points, const QPoint& startPos)
{
    for (int i = points.size() - 1; i > 0; i--)
    {
        points[i] = points.at(i) - points.at(i - 1);
    }
    points[0] = points[0] - startPos;
}

QList<QPoint> utils::makePointsRelative(const QList<QPoint>& points, const QPoint& startPos)
{
    QList<QPoint> pointsOut;
    pointsOut.append(points.first() - startPos);
    for (int i = 1; i < points.size(); i++)
    {
        pointsOut.append(points.at(i) - points.at(i - 1));
    }
    return pointsOut;
}
