#include "LaserLayerTableWidget.h"

#include <QLabel>
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
        QList<LaserLayer*> layers = document()->layers();
        for (int i = 0; i < layers.size(); i++)
        {
            int row = rowCount();
            LaserLayer* layer = layers[i];
            QList<LaserPrimitive*> laserItems = layer->items();
            if (laserItems.isEmpty())
                continue;
            setRowCount(row + 1);

            QString type;
            if (layer->type() == LLT_CUTTING)
            {
                type = tr("C");
            }
            else if (layer->type() == LLT_ENGRAVING)
            {
                type = tr("E");
            }

            QTableWidgetItem* layerWidgetItem0 = new QTableWidgetItem();
            layerWidgetItem0->setBackgroundColor(layer->color());
            setItem(row, 0, layerWidgetItem0);

            QTableWidgetItem* layerWidgetItem1 = new QTableWidgetItem();
            layerWidgetItem1->setText(type);
            layerWidgetItem1->setData(Qt::UserRole, i);
            setItem(row, 1, layerWidgetItem1);

            QTableWidgetItem* layerWidgetItem2 = new QTableWidgetItem();
            layerWidgetItem2->setText(layer->name());
            setItem(row, 2, layerWidgetItem2);

            QTableWidgetItem* layerWidgetItem3 = new QTableWidgetItem();
            layerWidgetItem3->setText(QString::number(layer->items().count()));
            setItem(row, 3, layerWidgetItem3);
        }
    }
}


