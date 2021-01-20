#include "LaserLayerTreeWidget.h"

#include <QDropEvent>
#include <QSharedData>
#include <QMimeData>

#include "scene/LaserDocument.h"
#include "scene/LaserPrimitive.h"

LaserLayerTreeWidget::LaserLayerTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    // initialize layers Tree Widget
    //setColumnWidth(0, 45);
    //setColumnWidth(1, 45);
    //setColumnWidth(2, 45);
    //setColumnWidth(3, 45);
    //setHeaderLabels(QStringList() << tr("Name") << tr("C") << tr("T") << tr("V"));
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
        //QList<LaserLayer*> layers = document()->cuttingLayers();
        //fillLayersTree(layers, tr("C"));
        //layers = document()->engravingLayers();
        //fillLayersTree(layers, tr("E"));
    }
}

void LaserLayerTreeWidget::fillLayersTree(QList<LaserLayer*>& layers, const QString & type)
{
    QList<QTreeWidgetItem*> treeWidgetItems;
    for (int i = 0; i < layers.size(); i++)
    {
        LaserLayer* layer = layers[i];
        QList<LaserPrimitive*> laserItems = layer->primitives();
        QTreeWidgetItem* layerWidgetItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, 0);
        //layerWidgetItem->setText(0, "    ");
        layerWidgetItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
        layerWidgetItem->setBackgroundColor(0, layer->color());
        //layerWidgetItem->setBackground(0, QBrush(layer->color()));
        //layerWidgetItem->setCheckState(1, Qt::Unchecked);
        layerWidgetItem->setText(1, type);
        layerWidgetItem->setText(2, layer->name());
        layerWidgetItem->setBackgroundColor(2, layer->color());
        layerWidgetItem->setText(3, QString::number(layer->primitives().count()));
        layerWidgetItem->setData(0, Qt::UserRole, layer->objectName());
        //layerWidgetItem->setData(1, Qt::UserRole, layer->type());
        //for (int li = 0; li != laserItems.size(); li++)
        //{
        //    LaserPrimitive* laserItem = laserItems[li];
        //    QTreeWidgetItem* itemWidgetItem = new QTreeWidgetItem(layerWidgetItem);
        //    itemWidgetItem->setText(0, laserItem->name());
        //    itemWidgetItem->setCheckState(1, Qt::Unchecked);
        //    itemWidgetItem->setText(2, "S");
        //    itemWidgetItem->setText(3, "V");
        //    itemWidgetItem->setData(0, Qt::UserRole, laserItem->objectName());
        //    //itemWidgetItem->setData(1, Qt::UserRole, laserItem->laserItemType());
        //}
        treeWidgetItems.append(layerWidgetItem);
    }
    insertTopLevelItems(0, treeWidgetItems);
}

//void LaserLayerTreeWidget::startDrag(Qt::DropActions supportedActions)
//{
//    
//}

void LaserLayerTreeWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* data = event->mimeData();
    QList<QTreeWidgetItem*> selItems = selectedItems();
    for (QList<QTreeWidgetItem*>::iterator i = selItems.begin(); i != selItems.end(); i++)
    {
        QString id = (*i)->data(0, Qt::UserRole).toString();
        qDebug() << id;
    }
}
