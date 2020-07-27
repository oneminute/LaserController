#include "LaserLayerTreeWidget.h"

#include <QSharedData>

#include "scene/LaserDocument.h"
#include "scene/LaserItem.h"

LaserLayerTreeWidget::LaserLayerTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
}

LaserLayerTreeWidget::~LaserLayerTreeWidget()
{

}

LaserDocument * LaserLayerTreeWidget::document() const
{
    return m_doc;
}

void LaserLayerTreeWidget::setDocument(LaserDocument * doc)
{
    m_doc = doc;

    connect(m_doc, &LaserDocument::layersStructureChanged, this, &LaserLayerTreeWidget::updateItems);
}

void LaserLayerTreeWidget::updateItems()
{
    clear();

    if (m_doc)
    {
        QList<LaserLayer*> layers = document()->cuttingLayers();
        fillLayersTree(layers, "C");
        layers = document()->engravingLayers();
        fillLayersTree(layers, "E");
    }
}

void LaserLayerTreeWidget::fillLayersTree(QList<LaserLayer*>& layers, const QString & type)
{
    QList<QTreeWidgetItem*> treeWidgetItems;
    for (int i = 0; i < layers.size(); i++)
    {
        LaserLayer* layer = layers[i];
        QList<LaserPrimitive*> laserItems = layer->items();
        QTreeWidgetItem* layerWidgetItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, 0);
        layerWidgetItem->setText(0, layer->name());
        layerWidgetItem->setCheckState(1, Qt::Unchecked);
        layerWidgetItem->setText(2, type);
        layerWidgetItem->setText(3, "V");
        layerWidgetItem->setData(0, Qt::UserRole, QVariant::fromValue<LaserLayer*>(layer));
        layerWidgetItem->setData(1, Qt::UserRole, layer->type());
        for (int li = 0; li != laserItems.size(); li++)
        {
            LaserPrimitive* laserItem = laserItems[li];
            QTreeWidgetItem* itemWidgetItem = new QTreeWidgetItem(layerWidgetItem);
            itemWidgetItem->setText(0, laserItem->name());
            itemWidgetItem->setCheckState(1, Qt::Unchecked);
            itemWidgetItem->setText(2, "S");
            itemWidgetItem->setText(3, "V");
            itemWidgetItem->setData(0, Qt::UserRole, QVariant::fromValue<LaserPrimitive*>(laserItem));
            itemWidgetItem->setData(1, Qt::UserRole, laserItem->laserItemType());
        }
        treeWidgetItems.append(layerWidgetItem);
    }
    insertTopLevelItems(0, treeWidgetItems);
}

void LaserLayerTreeWidget::dropEvent(QDropEvent* event)
{
    qDebug() << event;
}
