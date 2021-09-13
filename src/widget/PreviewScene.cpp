#include "PreviewScene.h"

#include "LaserApplication.h"
#include "laser/LaserDevice.h"

#include <QGraphicsItem>
#include <QVector2D>

class PreviewScenePrivate
{
    Q_DECLARE_PUBLIC(PreviewScene)
public:
    PreviewScenePrivate(PreviewScene* ptr)
        : q_ptr(ptr)
    {

    }

    PreviewScene* q_ptr;

    QPen labelPen;
    QFont labelFont;
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
    clear();
    addRect(LaserApplication::device->boundRectMachining());
}

void PreviewScene::addPath(const QPainterPath& path, QPen pen, const QString& label)
{
    Q_D(PreviewScene);
    QGraphicsPathItem* item = QGraphicsScene::addPath(path);
    pen.setCosmetic(true);
    item->setPen(pen);
    
    if (!label.isEmpty())
    {
        QPointF pos = path.pointAtPercent(0);
        QGraphicsTextItem* textItem = QGraphicsScene::addText(label);
        textItem->setDefaultTextColor(d->labelPen.color());
        textItem->setPos(pos);
    }
}

void PreviewScene::addLine(const QLineF& line, QPen pen, const QString& label)
{
    Q_D(PreviewScene);
    QGraphicsLineItem* item = QGraphicsScene::addLine(line);
    pen.setCosmetic(true);
    item->setPen(pen);

    QGraphicsEllipseItem* circleItem = QGraphicsScene::addEllipse(
        line.p1().x() - 50, line.p1().y() - 50, 100, 100, pen);
    circleItem->setParentItem(item);

    QVector2D dir(line.p1() - line.p2());
    dir.normalize();
    QTransform t1, t2;
    t1.rotate(15);
    t2.rotate(-15);
    QLineF arrowLine(QPointF(0, 0), (dir * 150).toPointF());
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
    }
}
