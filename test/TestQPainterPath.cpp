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

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    //PathTest();
    //interactionTest();
    paintTest();

    return app.exec();
}

