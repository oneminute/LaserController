#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"

#include <QFileDialog>
#include <QTreeWidgetItem>

#include "import/Importer.h"
#include "scene/LaserDocument.h"
#include "scene/LaserItem.h"
#include "scene/LaserScene.h"
#include "ui/LaserLayerDialog.h"
#include "widget/LaserViewer.h"

LaserControllerWindow::LaserControllerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::LaserControllerWindow)
    , m_scene(new LaserScene)
{
    m_ui->setupUi(this);

    m_viewer = new LaserViewer(m_scene.data());
    this->setCentralWidget(m_viewer);

    // initialize layers Tree Widget
    m_ui->treeWidgetLayers->setColumnWidth(0, 90);
    m_ui->treeWidgetLayers->setColumnWidth(1, 15);
    m_ui->treeWidgetLayers->setColumnWidth(2, 15);
    m_ui->treeWidgetLayers->setHeaderLabels(QStringList() << tr("Name") << tr("T") << tr("V"));
    m_ui->treeWidgetLayers->header()->moveSection(0, 2);

    connect(m_ui->actionImportSVG, &QAction::triggered, this, &LaserControllerWindow::onActionImportSVG);
    connect(m_ui->toolButtonAddEngravingLayer, &QToolButton::clicked, this, &LaserControllerWindow::onToolButtonAddEngravingLayer);
    connect(m_ui->toolButtonAddCuttingLayer, &QToolButton::clicked, this, &LaserControllerWindow::onToolButtonAddCuttingLayer);
    connect(m_ui->treeWidgetLayers, &QTreeWidget::itemDoubleClicked, this, &LaserControllerWindow::onTreeWidgetLayersItemDoubleClicked);
}

LaserControllerWindow::~LaserControllerWindow()
{

}

void LaserControllerWindow::onToolButtonAddEngravingLayer(bool)
{
    QString newName = m_scene->document()->newLayerName(LaserLayer::LLT_ENGRAVING);
    LaserLayerDialog dialog(newName, LaserLayer::LLT_ENGRAVING);
    if (dialog.exec() == QDialog::Accepted)
    {
        LaserLayer layer = dialog.layer();
        m_scene->document()->addLayer(layer);
        updateLayers();
    }
}

void LaserControllerWindow::onToolButtonAddCuttingLayer(bool checked)
{
    QString newName = m_scene->document()->newLayerName(LaserLayer::LLT_CUTTING);
    LaserLayerDialog dialog(newName, LaserLayer::LLT_CUTTING);
    if (dialog.exec() == QDialog::Accepted)
    {
        LaserLayer layer = dialog.layer();
        m_scene->document()->addLayer(layer);
        updateLayers();
    }
}

void LaserControllerWindow::onTreeWidgetLayersItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    int i = item->data(0, Qt::UserRole).toInt();
    qDebug() << i;
    if (item->parent() == nullptr)
    {
        LaserLayer::LayerType type = (LaserLayer::LayerType)item->data(1, Qt::UserRole).toInt();
        LaserLayer layer;
        if (type == LaserLayer::LLT_CUTTING)
        {
            layer = m_scene->document()->cuttingLayers()[i];
        }
        else if (type == LaserLayer::LLT_ENGRAVING)
        {
            layer = m_scene->document()->engravingLayers()[i];
        }
        LaserLayerDialog dialog(layer);
        if (dialog.exec() == QDialog::Accepted)
        {
            updateLayers();
        }
        qDebug() << type << layer.id();
    }
    else
    {
        LaserItemType type = (LaserItemType)item->data(1, Qt::UserRole).toInt();
        qDebug() << type;
    }
}

void LaserControllerWindow::updateLayers()
{
    m_ui->treeWidgetLayers->clear();

    QList<LaserLayer> layers = m_scene->document()->cuttingLayers();
    fillLayersTree(layers, "C");
    layers = m_scene->document()->engravingLayers();
    fillLayersTree(layers, "E");
}

void LaserControllerWindow::fillLayersTree(QList<LaserLayer> &layers, const QString& type)
{
    QList<QTreeWidgetItem*> treeWidgetItems;
    for (int i = 0; i < layers.size(); i++)
    {
        LaserLayer layer = layers[i];
        QList<LaserItem*> laserItems = layer.items();
        QTreeWidgetItem* layerWidgetItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, 0);
        layerWidgetItem->setText(0, layer.id());
        layerWidgetItem->setText(1, type);
        layerWidgetItem->setText(2, "V");
        layerWidgetItem->setData(0, Qt::UserRole, i);
        layerWidgetItem->setData(1, Qt::UserRole, layer.type());
        for (int li = 0; li != laserItems.size(); li++)
        {
            LaserItem* laserItem = laserItems[li];
            QTreeWidgetItem* itemWidgetItem = new QTreeWidgetItem(layerWidgetItem);
            itemWidgetItem->setText(0, "Item");
            itemWidgetItem->setText(1, "S");
            itemWidgetItem->setText(2, "V");
            itemWidgetItem->setData(0, Qt::UserRole, li);
            itemWidgetItem->setData(1, Qt::UserRole, laserItem->laserItemType());
        }
        treeWidgetItems.append(layerWidgetItem);
    }
    m_ui->treeWidgetLayers->insertTopLevelItems(0, treeWidgetItems);
}

QString LaserControllerWindow::getFilename(const QString& title, const QStringList & mime)
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters(QStringList() << "image/svg+xml" << "image/svg+xml-compressed");
    dialog.setWindowTitle(tr("Open SVG File"));
    if (dialog.exec() == QDialog::Accepted)
        return dialog.selectedFiles().constFirst();
    else
        return "";
}

void LaserControllerWindow::onActionImportSVG(bool checked)
{
    QString filename = getFilename(tr("Open SVG File"), QStringList() << "image/svg+xml" << "image/svg+xml-compressed");
    if (filename.isEmpty())
        return;
    QSharedPointer<Importer> importer = Importer::getImporter(Importer::SVG);
    LaserDocument* doc = importer->import(filename);
    m_scene->updateDocument(doc);
    updateLayers();
}
