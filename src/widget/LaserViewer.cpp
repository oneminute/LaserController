#include "LaserViewer.h"

#include <QtMath>
#include <QWheelEvent>
#include <QPainterPath>

#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include "state/StateController.h"

LaserViewer::LaserViewer(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new LaserScene)
    , m_rubberBandActive(false)
    , m_mousePressed(false)
{
    setScene(m_scene.data());
    init();
}

//LaserViewer::LaserViewer(LaserScene* scene, QWidget* parent)
//    : QGraphicsView(scene, parent)
//    , m_scene(scene)
//{
//    init();
//}

LaserViewer::~LaserViewer()
{
    init();
}

void LaserViewer::paintEvent(QPaintEvent * event)
{
    if (StateControllerInst.onState(StateControllerInst.documentSelectingState()))
    {
        QPainter painter(viewport());
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawRect(QRectF(m_selectionStartPoint, m_selectionEndPoint));
    }
    QGraphicsView::paintEvent(event);
}

void LaserViewer::wheelEvent(QWheelEvent * event)
{
    zoomBy(qPow(1.2, event->delta() / 240.0));
}

void LaserViewer::zoomBy(qreal factor)
{
    const qreal currentZoom = zoomFactor();
    if ((factor < 1 && currentZoom < 0.01) || (factor > 1 && currentZoom > 10))
        return;
    scale(factor, factor);
    m_scene->document()->setScale(factor);
    emit zoomChanged();
}

void LaserViewer::mousePressEvent(QMouseEvent * event)
{
    QList<QAbstractState*> states;
    states << StateControllerInst.documentIdleState()
        << StateControllerInst.documentSelectedState();
    if (StateControllerInst.anyState(states))
    {
        if (event->button() == Qt::LeftButton)
        {
            m_selectionStartPoint = event->pos();
            m_selectionEndPoint = m_selectionStartPoint;
            event->accept();
            emit beginSelecting();
            qDebug() << "begin to select";
        }
        else
        {
            QWidget::mousePressEvent(event);
        }
    }
    else
    {
        QWidget::mousePressEvent(event);
    }
}

void LaserViewer::mouseMoveEvent(QMouseEvent * event)
{
    if (StateControllerInst.onState(StateControllerInst.documentSelectingState()))
    {
        m_selectionEndPoint = event->pos();
        QPainterPath selectionPath;
        selectionPath.addRect(QRectF(m_selectionStartPoint, m_selectionEndPoint));
        
        m_scene->setSelectionArea(mapToScene(selectionPath));
        return;
    }
    event->accept();
}

void LaserViewer::mouseReleaseEvent(QMouseEvent * event)
{
    if (StateControllerInst.onState(StateControllerInst.documentSelectingState()))
    {
        qreal distance = (m_selectionEndPoint - m_selectionStartPoint).manhattanLength();
        qDebug() << "distance:" << distance;
        if (distance <= 0.000001f)
        {
            m_scene->clearSelection();
            emit cancelSelecting();
        }
        else
        {
            emit endSelecting();
        }
    }

    m_mousePressed = false;
    event->accept();
}

qreal LaserViewer::zoomFactor() const
{
    return transform().m11();
}

void LaserViewer::init()
{
    setRubberBandSelectionMode(Qt::ItemSelectionMode::IntersectsItemBoundingRect);
    setInteractive(true);
    setMouseTracking(true);

    ADD_TRANSITION(documentIdleState, documentSelectingState, this, &LaserViewer::beginSelecting);
    ADD_TRANSITION(documentSelectingState, documentSelectedState, this, &LaserViewer::endSelecting);
    ADD_TRANSITION(documentSelectedState, documentSelectingState, this, &LaserViewer::beginSelecting);
    ADD_TRANSITION(documentSelectedState, documentIdleState, this, &LaserViewer::cancelSelecting);
    ADD_TRANSITION(documentSelectingState, documentIdleState, this, &LaserViewer::cancelSelecting);
}

void LaserViewer::zoomIn()
{
    zoomBy(2);
}

void LaserViewer::zoomOut()
{
    zoomBy(0.5);
}

void LaserViewer::resetZoom()
{
    if (!qFuzzyCompare(zoomFactor(), qreal(1))) {
        resetTransform();
        emit zoomChanged();
    }
}
