#include "PreviewViewer.h"

PreviewViewer::PreviewViewer(QWidget* parent)
    : QGraphicsView(parent)
{
    setScene(new QGraphicsScene());
}

PreviewViewer::~PreviewViewer()
{
}
