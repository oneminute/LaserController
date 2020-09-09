#include "LaserLayerTableWidget.h"

#include <QPushButton>
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

void LaserLayerTableWidget::setDocument(LaserDocument * doc) 
{ 
    m_doc = doc; 
    connect(m_doc, &LaserDocument::layersStructureChanged, this, &LaserLayerTableWidget::updateItems);
}

void LaserLayerTableWidget::updateItems()
{
    setRowCount(0);

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
        LaserLayer* layer = layers[i];
        QList<LaserPrimitive*> laserItems = layer->items();
        if (laserItems.isEmpty())
            continue;
        setRowCount(row + 1);
        //QTableWidgetItem* layerWidgetItem0 = new QTableWidgetItem();
        //layerWidgetItem0->setBackgroundColor(layer->color());
        //layerWidgetItem0->setData(Qt::UserRole, layer->objectName());
        //setItem(row, 0, layerWidgetItem0);

        QPushButton* button = new QPushButton;
        //button->setFixedWidth(26);
        //button->setFixedHeight(26);
        QPalette pal = button->palette();
        pal.setColor(QPalette::Button, layer->color());
        button->setAutoFillBackground(true);
        button->setPalette(pal);
        setCellWidget(row, 0, button);

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

void LaserLayerTableWidget::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    /*if (selected.count() <= 0)
        return;

    int row = selected.indexes()[0].row();
    if (deselected.count() > 0)
    {
        if (row == deselected.indexes()[0].row())
            return;
    }

    QString layerId = item(row, 0)->data(Qt::UserRole).toString();
    emit layerSelectionChanged(layerId);*/
}
