#include "PreviewViewer.h"
#include <QWheelEvent>
#include <QScrollBar>
#include <qdebug.h>
#include <QMouseEvent>

PreviewViewer::PreviewViewer(QWidget* parent)
    : QGraphicsView(parent)
    ,m_scene(new QGraphicsScene())
{
    setScene(m_scene);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setEnabled(false);
    horizontalScrollBar()->setEnabled(false);
    reset();
}

PreviewViewer::~PreviewViewer()
{
    m_scene = nullptr;
}

void PreviewViewer::zoomIn()
{
    QRectF rect = this->rect();
    QPointF center = rect.center();
    zoomBy(1.1, center);
}

void PreviewViewer::zoomOut()
{
    QRectF rect = this->rect();
    QPointF center = rect.center();
    zoomBy(0.9, center);
}

void PreviewViewer::resetZoom()
{
    if (!qFuzzyCompare(zoomValue(), qreal(1))) {
        //resetTransform();
        setTransform(QTransform());
        setZoomValue(1);//zoomBy()中更新网格和标尺
                        //emit zoomChanged(mapFromScene(m_scene->backgroundItem()->QGraphicsItemGroup::pos()));
    }
}

qreal PreviewViewer::zoomValue() const
{
    qDebug()<<transform();
    return transform().m11();
}

void PreviewViewer::setZoomValue(qreal zoomScale)
{
    zoomBy(zoomScale / this->zoomValue(), this->rect().center());
}

bool PreviewViewer::zoomBy(qreal factor, QPointF zoomAnchor, bool zoomAnchorCenter)
{
    const qreal currentZoom = zoomValue();
    if ((factor < 1 && currentZoom < 0.00001) || (factor > 1 && currentZoom > 58))
        return false;
    
    QTransform t = transform();
    QTransform t1;
    t1.scale(factor, factor);
    QTransform t2;
    QPointF diff;

    QPointF newZoomAnchor = zoomAnchor - m_anchorPoint;//映射到以m_anchorPoint为（0， 0）点的坐标系
    QPointF scaled = t1.map(newZoomAnchor);
    QPointF newPoint = newZoomAnchor;
    if (zoomAnchorCenter) {

        newPoint = this->rect().center();
        scaled += m_anchorPoint;
    }
    diff = newPoint - scaled;
    t2.translate(diff.x(), diff.y());
    setTransform(t*t1*t2);
    //更新网格
    //backgroundItem->onChangeGrids();
    //emit zoomChanged(mapFromScene(m_scene->backgroundItem()->QGraphicsItemGroup::pos()));
    //emit scaleChanged(zoomValue());
    return true;
}

void PreviewViewer::reset()
{
    setSceneRect(QRectF(QPointF(-50000000000000, -50000000000000), QPointF(50000000000000, 50000000000000)));
    QRectF rect1 = rect();
    setTransformationAnchor(QGraphicsView::NoAnchor);
    m_anchorPoint =  mapFromScene(QPointF(0, 0)); //NoAnchor以scene的(0, 0)点为坐标原点进行变换
    setTransform(QTransform());
    QPixmap cMap(":/ui/icons/images/drag_hand.png");
    this->setCursor(cMap.scaled(30, 30, Qt::KeepAspectRatio));
    //centerOn(rect().center());
}

void PreviewViewer::wheelEvent(QWheelEvent * event)
{
    QGraphicsView::wheelEvent(event);
    if (!m_scene) {
        return;
    }
    setTransformationAnchor(QGraphicsView::NoAnchor);
    
    qreal wheelZoomValue = 1 + event->delta() / 120.0 * 0.1;
    QPointF mousePos = mapFromGlobal(QCursor::pos());
    zoomBy(wheelZoomValue, mousePos);
    this->viewport()->repaint();
}

void PreviewViewer::mousePressEvent(QMouseEvent* event)
{
    //View Drag Ready
    QPixmap cMap(":/ui/icons/images/dragging_hand.png");
    this->setCursor(cMap.scaled(30, 30, Qt::KeepAspectRatio));
    m_lastViewDragPoint = event->pos();

}

void PreviewViewer::mouseMoveEvent(QMouseEvent * event)
{
    QPointF viewDragPoint = event->pos();
    QPointF diff = viewDragPoint - m_lastViewDragPoint;
    QTransform t = transform();
    QTransform t1;
    t1.translate(diff.x(), diff.y());
    setTransform(t * t1);
    m_lastViewDragPoint = viewDragPoint;
}

void PreviewViewer::mouseReleaseEvent(QMouseEvent * event)
{
    QGraphicsView::mouseReleaseEvent(event);
    QPixmap cMap(":/ui/icons/images/drag_hand.png");
    this->setCursor(cMap.scaled(30, 30, Qt::KeepAspectRatio));
}

void PreviewViewer::paintEvent(QPaintEvent * event)
{
    QGraphicsView::paintEvent(event);
    QPainter painter(viewport());
    painter.setPen(QPen(Qt::black));
    QRectF rect(0, 0, 100, 100);

    painter.drawRect(sceneRect());
}
