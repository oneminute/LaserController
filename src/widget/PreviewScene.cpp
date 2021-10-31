#include "PreviewScene.h"

#include "LaserApplication.h"
#include "laser/LaserDevice.h"

#include <QGraphicsItem>
#include <QVector2D>
#include <QPainter>
#include <QPolygon>
#include <QQueue>
#include <QTimer>

struct DrawingItem
{
    QPen pen;
    QPainterPath path;
};

class PreviewScenePrivate
{
    Q_DECLARE_PUBLIC(PreviewScene)
public:
    PreviewScenePrivate(PreviewScene* ptr)
        : q_ptr(ptr)
    {

    }

    ~PreviewScenePrivate()
    {
    }

    PreviewScene* q_ptr;

    QPixmap canvas;
    QTransform canvasTransform;
    QGraphicsPixmapItem* canvasItem;

    QMutex canvasMutex;

    QPen labelPen;
    QFont labelFont;
    QTimer timer;

    QQueue<DrawingItem> drawingItems;
};

PreviewScene::PreviewScene(QObject* parent)
    : QGraphicsScene(parent)
    , d_ptr(new PreviewScenePrivate(this))
{
    reset();
}

PreviewScene::~PreviewScene()
{
}

QPen& PreviewScene::labelPen()
{
    Q_D(PreviewScene);
    return d->labelPen;
}

void PreviewScene::setLabelPen(QPen& pen)
{
    Q_D(PreviewScene);
    d->labelPen = pen;
}

QFont& PreviewScene::labelFont()
{
    Q_D(PreviewScene);
    return d->labelFont;
}

void PreviewScene::setLabelFont(QFont& font)
{
    Q_D(PreviewScene);
    d->labelFont = font;
}

void PreviewScene::reset()
{
    Q_D(PreviewScene);
    clear();
    QRectF boundingRect = LaserApplication::device->boundRectMachining();

    int width = 2000;
    int height = qRound(boundingRect.height() / boundingRect.width() * width);
    d->canvasTransform = QTransform::fromScale(width / boundingRect.width(), height / boundingRect.height());
    setSceneRect(QRect(QPoint(0, 0), QPoint(width, height)));

    d->canvas = QPixmap(width, height);
    d->canvas.fill();

    d->canvasItem = addPixmap(d->canvas);
    setBackgroundBrush(Qt::lightGray);

    connect(&d->timer, &QTimer::timeout, this, &PreviewScene::drawAllItems);
    d->timer.setInterval(200);
    d->timer.start();
}

void PreviewScene::addPath(const QPainterPath& path, QPen pen, const QString& label)
{
    Q_D(PreviewScene);
    QMutexLocker locker(&d->canvasMutex);
    DrawingItem item;
    item.pen = pen;
    item.path = d->canvasTransform.map(path);
    d->drawingItems.enqueue(item);

    //d->canvasItem->update();

    /*QGraphicsPathItem* item = QGraphicsScene::addPath(path);
    pen.setCosmetic(true);
    item->setPen(pen);
    
    if (!label.isEmpty())
    {
        QPointF pos = path.pointAtPercent(0);
        QGraphicsTextItem* textItem = QGraphicsScene::addText(label);
        textItem->setDefaultTextColor(d->labelPen.color());
        textItem->setPos(pos);
    }*/
}

void PreviewScene::addLine(const QLineF& line, QPen pen, const QString& label)
{
    Q_D(PreviewScene);
    QMutexLocker locker(&d->canvasMutex);

    QLineF mappedLine = d->canvasTransform.map(line);

    QPainterPath path;
    path.addEllipse(mappedLine.p1().x() - 3, mappedLine.p1().y() - 3, 6, 6);
    path.moveTo(mappedLine.p1());
    path.lineTo(mappedLine.p2());
    QVector2D dir(mappedLine.p1() - mappedLine.p2());
    dir.normalize();
    QTransform t1, t2;
    t1.rotate(15);
    t2.rotate(-15);
    QLineF arrowLine(QPointF(0, 0), (dir * 18).toPointF());
    QLineF arrowLine1 = t1.map(arrowLine);
    QLineF arrowLine2 = t2.map(arrowLine);
    arrowLine1.translate(mappedLine.p2());
    arrowLine2.translate(mappedLine.p2());
    QLineF arrowLine3(arrowLine1.p2(), arrowLine2.p2());
    path.moveTo(arrowLine1.p1());
    path.lineTo(arrowLine1.p2());
    path.moveTo(arrowLine2.p1());
    path.lineTo(arrowLine2.p2());
    path.moveTo(arrowLine3.p1());
    path.lineTo(arrowLine3.p2());

    DrawingItem item;
    item.pen = pen;
    item.path = path;
    d->drawingItems.append(item);

    //d->canvasItem->update();

    /*QGraphicsLineItem* item = QGraphicsScene::addLine(line);
    pen.setCosmetic(true);
    item->setPen(pen);

    QGraphicsEllipseItem* circleItem = QGraphicsScene::addEllipse(
        line.p1().x() - 1000, line.p1().y() - 1000, 2000, 2000, pen);
    circleItem->setParentItem(item);

    QVector2D dir(line.p1() - line.p2());
    dir.normalize();
    QTransform t1, t2;
    t1.rotate(15);
    t2.rotate(-15);
    QLineF arrowLine(QPointF(0, 0), (dir * 3000).toPointF());
    QLineF arrowLine1 = t1.map(arrowLine);
    QLineF arrowLine2 = t2.map(arrowLine);
    arrowLine1.translate(line.p2());
    arrowLine2.translate(line.p2());
    QLineF arrowLine3(arrowLine1.p2(), arrowLine2.p2());
    QGraphicsLineItem* arrow1 = QGraphicsScene::addLine(arrowLine1, pen);
    QGraphicsLineItem* arrow2 = QGraphicsScene::addLine(arrowLine2, pen);
    QGraphicsLineItem* arrow3 = QGraphicsScene::addLine(arrowLine3, pen);
    arrow1->setParentItem(item);
    arrow2->setParentItem(item);
    arrow3->setParentItem(item);
    
    if (!label.isEmpty())
    {
        QPointF pos = (line.p1() + line.p2()) / 2;
        QGraphicsTextItem* textItem = QGraphicsScene::addText(label);
        textItem->setParentItem(item);
        textItem->setDefaultTextColor(pen.color());
        textItem->setPos(pos);
        textItem->setFont(d->labelFont);
    }*/
}

void PreviewScene::drawAllItems()
{
    Q_D(PreviewScene);
    QMutexLocker locker(&d->canvasMutex);
    QPainter painter(&d->canvas);
    while (!d->drawingItems.empty())
    {
        DrawingItem item = d->drawingItems.dequeue();
        item.pen.setCosmetic(true);
        painter.setPen(item.pen);
        painter.drawPath(item.path);
    }
    d->canvasItem->setPixmap(d->canvas);
}

