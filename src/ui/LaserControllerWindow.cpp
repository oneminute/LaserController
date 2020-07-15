#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"

#include <QFileDialog>

#include "widget/LaserViewer.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include "import/Importer.h"
#include "ui/LaserLayerDialog.h"

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
}

LaserControllerWindow::~LaserControllerWindow()
{

}

void LaserControllerWindow::onToolButtonAddEngravingLayer(bool)
{
    QString newName = m_scene->document()->newLayerName(LaserLayer::LLT_ENGRAVING);
    LaserLayerDialog dialog(newName, LaserLayer::LLT_ENGRAVING);
    dialog.exec();

    LaserLayer layer = dialog.layer();
    m_scene->document()->addLayer(layer);
    updateLayers();
}

void LaserControllerWindow::onToolButtonAddCuttingLayer(bool checked)
{
    QString newName = m_scene->document()->newLayerName(LaserLayer::LLT_CUTTING);
    LaserLayerDialog dialog(newName, LaserLayer::LLT_CUTTING);
    dialog.exec();

    LaserLayer layer = dialog.layer();
    m_scene->document()->addLayer(layer);
    updateLayers();
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
    for (QList<LaserLayer>::iterator i = layers.begin(); i != layers.end(); i++)
    {
        LaserLayer layer = *i;
        QList<LaserItem*> laserItems = layer.items();
        QTreeWidgetItem* layerWidgetItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, 0);
        layerWidgetItem->setText(0, layer.id());
        layerWidgetItem->setText(1, type);
        layerWidgetItem->setText(2, "V");
        for (QList<LaserItem*>::iterator li = laserItems.begin(); li != laserItems.end(); li++)
        {
            QTreeWidgetItem* itemWidgetItem = new QTreeWidgetItem(layerWidgetItem);
            itemWidgetItem->setText(0, "Item");
            itemWidgetItem->setText(1, "S");
            itemWidgetItem->setText(2, "V");
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
