#include "LaserViewer.h"

LaserViewer::LaserViewer(QWidget* parent)
    : QGraphicsView(parent)
{

}

LaserViewer::LaserViewer(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{

}

LaserViewer::~LaserViewer()
{

}