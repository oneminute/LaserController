#include "LaserLayerTableWidget.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QLabel>

#include "scene/LaserDocument.h"
#include "scene/LaserPrimitive.h"
#include "LaserApplication.h"
#include "LaserHeaderView.h"
#include "scene/LaserScene.h"
#include "widget/LaserViewer.h"
#include "widget/UndoCommand.h"
#include "LaserApplication.h"

LaserLayerTableWidget::LaserLayerTableWidget(QWidget* parent)
    : QTableWidget(parent)
    , m_doc(nullptr)
{
    QStringList columnHeaders;
    columnHeaders 
        << tr("#")
        << tr("Layer")
        << tr("Mode")
        << tr("Count")
        << tr("Speed/Power")
        << tr("Output")
        << tr("Visible");

    setColumnCount(7);
    setHorizontalHeaderLabels(columnHeaders);
    
    
    
    /*setColumnWidth(0, 45);
    setColumnWidth(1, 30);
    setColumnWidth(2, 75);
    setColumnWidth(3, 45);
    setColumnWidth(4, 75);
    setColumnWidth(5, 30);
    setColumnWidth(6, 30);*/

    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setShowGrid(false);
    verticalHeader()->setVisible(false);
    //My
    horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter);
    QAbstractItemModel* itemmode = horizontalHeader()->model();
    // initialize layers Tree Widget
    //setting icon
    LaserHeaderView* h = new LaserHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(h);
    
    for (int i = 0; i < columnHeaders.size(); i++) {

        QTableWidgetItem* header = horizontalHeaderItem(i);
        header->setToolTip(header->text());
        header->setText("");
        setColumnWidth(i, 35);
        
    }
    setColumnWidth(2, 78);
    setColumnWidth(4, 68);
    
}

LaserLayerTableWidget::~LaserLayerTableWidget()
{

}

void LaserLayerTableWidget::setDocument(LaserDocument * doc) 
{ 
    if (!doc)
        return;
    m_doc = doc; 
    connect(m_doc, &LaserDocument::layersStructureChanged, this, &LaserLayerTableWidget::updateItems);
    connect(m_doc, &LaserDocument::closed, this, &LaserLayerTableWidget::laserDocumentClosed);
}

void LaserLayerTableWidget::laserDocumentClosed()
{
    m_doc = nullptr;
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
            QList<LaserPrimitive*> laserItems = layer->primitives();
            if (laserItems.isEmpty())
                continue;
            setRowCount(row + 1);
            layer->setRow(row);

            QString type;
            QString fullType;
            LaserLayerType laserType = layer->type();
            if (laserType == LLT_CUTTING)
            {
                type = tr("C");
                fullType = tr("Cutting");
            }
            else if (laserType == LLT_ENGRAVING)
            {
                type = tr("E");
                fullType = tr("Engraving");
            }
            else if (laserType == LLT_FILLING)
            {
                type = tr("C+F");
                fullType = tr("Filling");
            }

            QTableWidgetItem* itemColor = new QTableWidgetItem();
            Qt::ItemFlags flags = itemColor->flags();
            flags &= ~Qt::ItemIsSelectable;
            itemColor->setFlags(flags);
            itemColor->setBackgroundColor(layer->color());
            itemColor->setTextAlignment(Qt::AlignCenter);

            QTableWidgetItem* itemType = new QTableWidgetItem();
            itemType->setText(fullType);
            itemType->setTextAlignment(Qt::AlignCenter);

            QTableWidgetItem* itemName = new QTableWidgetItem();
            itemName->setText(QString("%1%2").arg(type).arg(layer->name()));
            itemName->setTextAlignment(Qt::AlignCenter);

            QTableWidgetItem* itemCount = new QTableWidgetItem();
            itemCount->setText(QString::number(layer->primitives().count()));
            itemCount->setTextAlignment(Qt::AlignCenter);

            QTableWidgetItem* itemSpeedPower = new QTableWidgetItem();
            if (layer->type() == LLT_CUTTING)
            {
                itemSpeedPower->setText(QString("%1/%2").arg(layer->cuttingRunSpeed()).arg(layer->cuttingRunSpeedPower()));
            }
            else if (layer->type() == LLT_ENGRAVING)
            {
                itemSpeedPower->setText(QString("%1/%2").arg(layer->engravingRunSpeed()).arg(layer->engravingLaserPower()));
            }
            else if (layer->type() == LLT_FILLING)
            {
                itemSpeedPower->setText(QString("%1/%2").arg(layer->fillingRunSpeed()).arg(layer->fillingRunSpeedPower()));
            }
            itemSpeedPower->setTextAlignment(Qt::AlignCenter);

            QCheckBox* exportable = new QCheckBox();
            QWidget* exportablePanel = new QWidget();
            QHBoxLayout* exportableLayout = new QHBoxLayout(exportablePanel);
            exportableLayout->addWidget(exportable);
            exportableLayout->setAlignment(Qt::AlignCenter);
            exportableLayout->setContentsMargins(0, 0, 0, 0);
            exportablePanel->setLayout(exportableLayout);
            exportable->setChecked(layer->exportable());
            connect(exportable, &QCheckBox::toggled, [=](bool checked) 
                {
                    layer->setExportable(checked);
                    LaserViewer* view = qobject_cast<LaserViewer*>(m_doc->scene()->views()[0]);
                    view->viewport()->update();
                }
            );
            setCellWidget(row, 5, exportablePanel);

            QCheckBox* visible = new QCheckBox();
            QWidget* visiblePanel = new QWidget();
            QHBoxLayout* visibleLayout = new QHBoxLayout(visiblePanel);
            visibleLayout->addWidget(visible);
            visibleLayout->setAlignment(Qt::AlignCenter);
            visibleLayout->setContentsMargins(0, 0, 0, 0);
            visiblePanel->setLayout(visibleLayout);
            visible->setChecked(layer->visible());
            layer->setCheckBox(visible);
            connect(visible, &QCheckBox::clicked, [=](bool checked)
                {
                    //layer->setVisible(checked);
                    LaserViewer* view = qobject_cast<LaserViewer*>(m_doc->scene()->views()[0]);
                    LayerVisibleCommand* cmd = new LayerVisibleCommand(view, layer, checked);
                    view->undoStack()->push(cmd);
                    view->viewport()->update();
                }
            );
            setCellWidget(row, 6, visiblePanel);

            setItem(row, 0, itemName);
            setItem(row, 1, itemColor);
            setItem(row, 2, itemType);
            setItem(row, 3, itemCount);
            setItem(row, 4, itemSpeedPower);

            item(row, 0)->setData(Qt::UserRole, i);
        }
    }
}


