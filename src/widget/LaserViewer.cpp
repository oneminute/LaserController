#include "LaserViewer.h"

#include <QtMath>
#include <QWheelEvent>

#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"

LaserViewer::LaserViewer(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(nullptr)
{

}

LaserViewer::LaserViewer(LaserScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , m_scene(scene)
{

}

LaserViewer::~LaserViewer()
{

}

void LaserViewer::paintEvent(QPaintEvent * event)
{
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

qreal LaserViewer::zoomFactor() const
{
    return transform().m11();
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
