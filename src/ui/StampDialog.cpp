#include "StampDialog.h"
#include "scene/LaserScene.h"
#include "scene/LaserPrimitiveGroup.h"

StampDialog::StampDialog(LaserScene* scene, QWidget* parent) 
    :QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint),
    m_scene(scene)
{
    m_viewer = qobject_cast<LaserViewer*>( m_scene->views()[0]);
}
StampDialog::~StampDialog()
{
}

void StampDialog::okBtnAccept()
{    
    m_viewer->addPrimitiveAndExamRegionByBounds(createStampPrimitive());
    m_viewer->zoomToSelection();
}

void StampDialog::previewBtnAccept()
{
    m_preview->setScene(new QGraphicsScene(this));
    m_preview->scene()->clear();
    QList<LaserPrimitive*> list = createStampPrimitive();
    LaserPrimitiveGroup* group = new LaserPrimitiveGroup();
    for (LaserPrimitive* p : list) {
        group->addToGroup(p);
    }
    m_preview->scene()->addItem(group);
    QRectF bounds = group->boundingRect();
    qreal longer = bounds.width();
    qreal longer_o = m_preview->rect().width();
    QPointF stampCenter = bounds.center();
    if (longer < bounds.height()) {
        longer = bounds.height();
        longer_o = m_preview->rect().height();
    }
    qreal scale = (longer_o - 20) / longer;
    group->setScale(scale);
    QPointF center = m_preview->rect().center();
    QTransform t;
    t.translate(center.x() - stampCenter.x(), center.y() - stampCenter.y());
    group->setTransform(t * group->transform());
    m_preview->repaint();
}
