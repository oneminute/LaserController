#include "GraphicsViewEx.h"

#include <QWheelEvent>
#include <QMouseEvent>

#include "common/common.h"

GraphicsViewEx::GraphicsViewEx(QWidget* parent)
    : QGraphicsView(parent)
    , m_mousePressed(false)
{
    //this->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontClipPainter | QGraphicsView::DontSavePainterState);	
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setAlignment(Qt::AlignCenter);
}

GraphicsViewEx::~GraphicsViewEx()
{

}

void GraphicsViewEx::fitZoom()
{
    fitInView(sceneRect());
}

void GraphicsViewEx::zoomToActual()
{
    
}

void GraphicsViewEx::wheelEvent(QWheelEvent* e)
{
    qreal zoomValue = 1 + e->delta() / 1200.0;
    QPointF scenePos = mapToScene(e->pos());
    this->scale(zoomValue, zoomValue);

    QSize viewportSize = viewport()->size();
    QPoint viewportCenter = viewport()->rect().center();
    QPointF deltaPos = mapToScene(e->pos()) - mapToScene(viewportCenter);
    centerOn(scenePos - deltaPos);

    e->accept();
}

void GraphicsViewEx::mousePressEvent(QMouseEvent* e)
{
    m_mousePressed = true;
    m_mousePressedPos = m_lastMousePos = e->pos();
}

void GraphicsViewEx::mouseReleaseEvent(QMouseEvent* e)
{
    m_mousePressed = false;
    QPoint delta = e->pos() - m_mousePressedPos;
    if (delta.manhattanLength() == 0)
    {
        emit clicked(mapToScene(e->pos()));
    }
}

void GraphicsViewEx::mouseMoveEvent(QMouseEvent* e)
{
    if (m_mousePressed)
    {
        QPointF p1 = mapToScene(e->pos());
        QPointF p2 = mapToScene(m_lastMousePos);
        QPointF deltaPos = p1 - p2;
        m_lastMousePos = e->pos();
        QTransform t = transform();
        QTransform tt = QTransform::fromTranslate(deltaPos.x(), deltaPos.y());
        setTransform(t * tt);
    }
    //e->accept();
}
