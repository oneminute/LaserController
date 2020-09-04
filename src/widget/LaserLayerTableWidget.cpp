#include "LaserLayerTableWidget.h"

#include "scene/LaserDocument.h"
#include "scene/LaserItem.h"

LaserLayerTableWidget::LaserLayerTableWidget(QWidget* parent)
    : QTableWidget(parent)
    , m_doc(nullptr)
{

}

LaserLayerTableWidget::~LaserLayerTableWidget()
{

}

void LaserLayerTableWidget::updateItems()
{
    clear();

    if (m_doc)
    {
        QList<LaserLayer*> layers = document()->cuttingLayers();
        fillLayers(layers, tr("C"));
        layers = document()->engravingLayers();
        fillLayers(layers, tr("E"));
    }
}

void LaserLayerTableWidget::fillLayers(QList<LaserLayer*>& layers, const QString& type)
{
    for (int i = 0; i < layers.size(); i++)
    {
        int row = rowCount();
        setRowCount(row + 1);
        LaserLayer* layer = layers[i];
        QList<LaserPrimitive*> laserItems = layer->items();
        QTableWidgetItem* layerWidgetItem0 = new QTableWidgetItem();
        layerWidgetItem0->setBackgroundColor(layer->color());
        layerWidgetItem0->setData(Qt::UserRole, layer->objectName());
        setItem(row, 0, layerWidgetItem0);

        QTableWidgetItem* layerWidgetItem1 = new QTableWidgetItem();
        layerWidgetItem1->setText(type);
        setItem(row, 1, layerWidgetItem1);

        QTableWidgetItem* layerWidgetItem2 = new QTableWidgetItem();
        layerWidgetItem2->setText(layer->name());
        setItem(row, 2, layerWidgetItem2);

        QTableWidgetItem* layerWidgetItem3 = new QTableWidgetItem();
        layerWidgetItem3->setText(QString::number(layer->items().count()));
        setItem(row, 3, layerWidgetItem3);
    }
}
