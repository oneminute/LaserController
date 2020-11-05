#include "LaserViewer.h"

#include <QBoxLayout>
#include <QGraphicsProxyWidget>
#include <QComboBox>
#include <QPainterPath>
#include <QPalette>
#include <QtMath>
#include <QWheelEvent>
#include <QScrollBar>

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

LaserViewer::~LaserViewer()
{
}

void LaserViewer::paintEvent(QPaintEvent * event)
{
    QGraphicsView::paintEvent(event);
    if (StateControllerInst.onState(StateControllerInst.documentSelectingState()))
    {
        QPainter painter(viewport());
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawRect(QRectF(m_selectionStartPoint, m_selectionEndPoint));
    }
}

void LaserViewer::wheelEvent(QWheelEvent * event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        zoomBy(qPow(1.2, event->delta() / 240.0));
    }
    else
        QGraphicsView::wheelEvent(event);
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
    QGraphicsView::mousePressEvent(event);
    if (event->isAccepted())
        return;

    QList<QAbstractState*> states;
    states << StateControllerInst.documentIdleState()
        << StateControllerInst.documentSelectedState();
    if (StateControllerInst.anyState(states))
    {
        if (event->button() == Qt::LeftButton)
        {
            m_selectionStartPoint = event->pos();
            m_selectionEndPoint = m_selectionStartPoint;
            emit beginSelecting();
            qDebug() << "begin to select";
        }
    }
}

void LaserViewer::mouseMoveEvent(QMouseEvent * event)
{
    QPoint point = event->pos();
    if (StateControllerInst.onState(StateControllerInst.documentSelectingState()))
    {
        m_selectionEndPoint = point;
        QPainterPath selectionPath;
        selectionPath.addRect(QRectF(m_selectionStartPoint, m_selectionEndPoint));
        
        m_scene->setSelectionArea(mapToScene(selectionPath));
        return;
    }
    else
    {
        QGraphicsView::mouseMoveEvent(event);
    }
    QPointF pos = mapToScene(point);
    emit mouseMoved(pos);
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
            QGraphicsView::mouseReleaseEvent(event);
        }
        else
        {
            emit endSelecting();
        }
    }
    else
    {
        QGraphicsView::mouseReleaseEvent(event);
    }

    m_mousePressed = false;
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

    QComboBox* comboBoxScale = new QComboBox;
    comboBoxScale->addItem("100%", 1.0);
    comboBoxScale->addItem("200%", 2.0);
    comboBoxScale->addItem("300%", 3.0);
    comboBoxScale->addItem("400%", 4.0);
    comboBoxScale->addItem("500%", 5.0);
    comboBoxScale->addItem("1000%", 10.0);

    //QLayout* layout = horizontalScrollBar()->layout();
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    //horizontalScrollBar()->setLayout(layout);
    //layout->insertWidget(0, comboBoxScale);
    //layout->addWidget(comboBoxScale);

    QScrollBar* newBar = new QScrollBar(Qt::Horizontal);
    QPalette::ColorRole role = newBar->foregroundRole();
    QPalette palette;
    palette.setColor(role, Qt::red);
    newBar->setPalette(palette);
    layout->addWidget(comboBoxScale);
    //layout->addWidget(newBar);
    newBar->setLayout(layout);
    //newBar->setFixedSize(100, 30);
    //newBar->show();
    setHorizontalScrollBar(newBar);

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
