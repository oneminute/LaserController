#include <QtCore>
#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QGuiApplication>
#include <QPainterPath>
#include <QPointF>
#include <QTransform>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QMainWindow>
#include "TestQPainterPath.h"

bool fuzzyEqual(qreal a, qreal b)
{
    return qAbs(a - b) < 0.0001;
}

void interactionTest()
{
    QGraphicsView* view = new QGraphicsView;
    QGraphicsScene* scene = new QGraphicsScene;
    view->setScene(scene);

    QPainterPath path;
    path.addEllipse(QRectF(-100, -50, 100, 100));
    path.addEllipse(QRectF(0, -50, 100, 100));
    QFont f;
    f.setPixelSize(144);
    //path.addText(QPointF(-100, 50), f, "OW");
    QRectF boundingRect = path.boundingRect();

    QRectF rect(boundingRect.left(), boundingRect.top() + 40, boundingRect.width(), 10);
    QPainterPath path2;
    path2.addRect(rect);

    QTransform t = QTransform::fromTranslate(0, 100);
    QPainterPath intersected = path2.intersected(path);

    QMap<qreal, qreal> linePoints;
    for (int i = 0; i < intersected.elementCount(); i++)
    {
        QPainterPath::Element e = intersected.elementAt(i);
        qDebug() << i << e.x << e.y << e.type;
        if (fuzzyEqual(e.y, rect.top()))
        {
            linePoints.insert(e.x, e.x);
        }
    }
    //QList<qreal> points = linePoints.toList();
    QList<QLineF> lines;
    qreal last;
    int i = 0;
    for (qreal curr: linePoints)
    {
        qDebug() << curr;
        if (i++ == 0)
        {
            last = curr;
            continue;
        }
        qreal mean = (last + curr) / 2;

        QPointF pt(mean, rect.top());
        if (path.contains(pt))
        {
            lines.append(QLineF(QPointF(last, rect.top()), QPointF(curr, rect.top())));
        }
        last = curr;
    }
    for (QLineF& line : lines)
    {
        qDebug() << line;
        QGraphicsLineItem* lineItem = scene->addLine(line);
        int r = QRandomGenerator::global()->bounded(255);
        int g = QRandomGenerator::global()->bounded(255);
        int b = QRandomGenerator::global()->bounded(255);
        QColor color(r, g, b);
        QPen pen(color, 10, Qt::SolidLine);
        pen.setCosmetic(true);
        lineItem->setPen(pen);
    }

    t.map(intersected);

    //view->setSceneRect(QRectF(-3, -3, 6, 6));

    scene->addPath(path)/*->setBrush(QBrush(Qt::blue))*/;
    //scene->addPath(path2);
    scene->addPath(t.map(intersected));
    view->show();
}

void pathTest()
{
    QPainterPath path;
    path.addEllipse(QPointF(0, 0), 10, 10);

    for (int i = 0; i < path.elementCount(); i++)
    {
        QPainterPath::Element element = path.elementAt(i);
        qDebug() << "path1: " << element.type << element;
    }

    QPainterPath path2;
    path2.moveTo(-5, 0);
    path2.cubicTo(QPointF(-2.5, 8), QPointF(2.5, 8), QPointF(5, 0));
    qDebug();
    for (int i = 0; i < path2.elementCount(); i++)
    {
        QPainterPath::Element element = path2.elementAt(i);
        qDebug() << "path2: " << element.type << element;
    }

    path = QTransform::fromScale(-1, 1).map(path);
    for (qreal i = 0; i <= 1; i += 0.01)
    {
        qreal angle = path.angleAtPercent(i);
        qDebug() << i << "angle:" << angle;
    }
}

void paintTest()
{
    QPixmap canvas(200, 200);
    QPen pen;
    QPainter painter(&canvas);
    painter.setPen(pen);
    painter.drawLine(QPointF(1, 0), QPointF(0, 1));
}

void subPathPolygonsTest1()
{
    QPainterPath path;
    QRect rect(QPoint(0, 0), QPoint(10, 10));
    path.addRect(rect);
    QList<QPolygonF> polys = path.toSubpathPolygons();
    Q_ASSERT(polys.length() == 1);
    qDebug() << polys[0].length();
    for (int i = 0; i < polys[0].length(); i++)
    {
        qDebug() << polys[0].at(i);
    }
}

void subPathPolygonsTest2()
{
    QPainterPath path;
    QPolygonF polyIn;
    polyIn.append(QPointF(0, 2));
    polyIn.append(QPointF(2, 3));
    polyIn.append(QPointF(3, 5));
    polyIn.append(QPointF(20, 5));
    polyIn.append(QPointF(12, 8));
    polyIn.append(QPointF(2, 3));
    path.addPolygon(polyIn);
    QPolygonF polys = path.toFillPolygon();
    //Q_ASSERT(polys.length() == 1);
    qDebug() << polys.length();
    for (int i = 0; i < polys.length(); i++)
    {
        qDebug() << polys.at(i);
    }
}

qreal percentAtLengthPercent(const QPainterPath& curve, qreal curveLength, qreal lengthPercent)
{
    if (lengthPercent < 0)
        lengthPercent += 1;
    if (lengthPercent > 1)
        lengthPercent -= 1;
    qreal length = curveLength * lengthPercent;
    qreal percent = curve.percentAtLength(length);
    return percent;
}

void testCurveTrace()
{
    QGraphicsScene* scene = new QGraphicsScene;
    //scene->setSceneRect(-5, -5, 10, 10);
    QGraphicsView* view = new QGraphicsView(scene);
    view->resize(800, 600);
    view->show();

    qreal a = 500;
    qreal b = 100;
    qreal a2 = a * a;
    qreal b2 = b * b;
    qreal a4 = a2 * a2;
    qreal b4 = b2 * b2;
    qreal a8 = a4 * a4;
    qreal b8 = b4 * b4;
    QPainterPath curve;
    curve.addEllipse(QPointF(0, 0), a, b);

    QPen pen(Qt::black, 1);
    pen.setCosmetic(true);
    QPen pen2(Qt::blue, 1);
    pen2.setCosmetic(true);
    QGraphicsPathItem* item = scene->addPath(curve);
    item->setPen(pen);
    view->fitInView(item, Qt::KeepAspectRatio);

    int count = 32;
    qreal delta = 1.0 / count;
    qreal smallPercent = 0.001;
    qreal curveLength = curve.length();
    for (qreal i = 0; i <= count; i++)
    {
        qreal lengthPercent = i * delta;
        qreal percent = percentAtLengthPercent(curve, curveLength, lengthPercent);
        qreal diffPercent = lengthPercent - percent;
        QPointF pt = curve.pointAtPercent(percent);
        qreal angle = curve.angleAtPercent(percent);
        qreal slope = curve.slopeAtPercent(percent);
        //qreal angle2 = qAtan2(pt.y(), pt.x());
        qreal angle2 = qRadiansToDegrees(qAtan(slope));
        //qreal angle2 = angle + 0;
        QGraphicsRectItem* ptItem = scene->addRect(QRectF(QPointF(-20, -20), QPointF(20, 20)));
        QGraphicsLineItem* ptLine = scene->addLine(QLineF(QPointF(), QPointF(30, 0)));
        QGraphicsTextItem* textItem = scene->addText(QString::number(i));
        ptItem->setBrush(Qt::NoBrush);
        ptItem->setPen(pen2);
        ptLine->setPen(pen2);
        textItem->setPos(pt);
        //QTransform t;
        //t.rotate(angle2);
        //ptItem->setTransform(t * QTransform::fromTranslate(pt.x(), pt.y()));
        ptItem->setPos(pt);
        //ptItem->setTransformOriginPoint(pt);
        ptItem->setRotation(angle2);

        //QTransform t2;
        //t2.rotate(angle2);
        //ptLine->setTransform(t2);
        ptLine->setPos(pt);
        ptLine->setRotation(angle2 + 90);
        QGraphicsRectItem* lineEnd = scene->addRect(QRectF(QPointF(-5, -5), QPointF(5, 5)));
        lineEnd->setPen(pen2);
        lineEnd->setPos(ptLine->sceneTransform().map(ptLine->line().p2()));

        qreal percentPrev = percentAtLengthPercent(curve, curveLength, lengthPercent - smallPercent);
        qreal percentNext = percentAtLengthPercent(curve, curveLength, lengthPercent + smallPercent);
        qreal deltaPercent = qAbs(percent - percentPrev);
        QPointF ptPrev = curve.pointAtPercent(percentPrev);
        QPointF ptNext = curve.pointAtPercent(percentNext);
        qreal anglePrev = curve.angleAtPercent(percentPrev);
        qreal angleNext = curve.angleAtPercent(percentNext);
        qreal angleDeltaPrev = qAbs(angle - anglePrev);
        qreal angleDeltaNext = qAbs(angleNext - angle);
        qreal deltaAngle = qAbs(angleDeltaNext - angleDeltaPrev);
        //qreal deltaAngle = qAbs(angleNext - anglePrev);
        qreal x = pt.x();
        qreal y = pt.y();
        qreal x2 = x * x;
        qreal y2 = y * y;
        qreal y4 = y2 * y2;
        qreal d1 = -b2 * x / (a2 * y);
        qreal d2 = -b2 / (a4 * y * y2);
        qreal rho = -qPow((1 + d1 * d1), 1.5) / d2;
        qreal curvature = 1 / rho;
        qreal w = qLn(curvature);
        qDebug().nospace() << qSetFieldWidth(11)
            << "lengthPercent: " << lengthPercent
            << ", percent: " << percent
            << ", diffPercent: " << diffPercent
            //<< ", deltaPercent: " << deltaPercent
            //<< ", pos: " << pt
            << ", angle: " << angle
            << ", angle2: " << angle2
            ;
    }
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    //PathTest();
    //interactionTest();
    //paintTest();
    //subPathPolygonsTest2();
    testCurveTrace();

    return app.exec();
}

