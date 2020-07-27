#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"

#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QTimer>

#include "import/Importer.h"
#include "laser/LaserDriver.h"
#include "scene/LaserDocument.h"
#include "scene/LaserItem.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "state/StateController.h"
#include "ui/LaserLayerDialog.h"
#include "widget/LaserViewer.h"

LaserControllerWindow::LaserControllerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::LaserControllerWindow)
    , m_scene(new LaserScene)
    , m_created(false)
{
    m_ui->setupUi(this);

    m_viewer = new LaserViewer(m_scene.data());
    this->setCentralWidget(m_viewer);

    // initialize layers Tree Widget
    m_ui->treeWidgetLayers->setColumnWidth(0, 90);
    m_ui->treeWidgetLayers->setColumnWidth(1, 15);
    m_ui->treeWidgetLayers->setColumnWidth(2, 15);
    m_ui->treeWidgetLayers->setColumnWidth(3, 15);
    m_ui->treeWidgetLayers->setHeaderLabels(QStringList() << tr("Name") << tr("C") << tr("T") << tr("V"));
    m_ui->treeWidgetLayers->header()->moveSection(0, 2);

    m_ui->toolButtonAddLayer->addAction(m_ui->actionAddEngravingLayer);
    m_ui->toolButtonAddLayer->addAction(m_ui->actionAddCuttingLayer);
    //m_ui->toolButtonRemoveLayer->addAction(m_ui->actionRemoveLayer);
    m_ui->toolButtonRemoveLayer->setDefaultAction(m_ui->actionRemoveLayer);

    connect(m_ui->actionImportSVG, &QAction::triggered, this, &LaserControllerWindow::onActionImportSVG);
    connect(m_ui->actionAddEngravingLayer, &QAction::triggered, this, &LaserControllerWindow::onActionAddEngravingLayer);
    connect(m_ui->actionAddCuttingLayer, &QAction::triggered, this, &LaserControllerWindow::onActionAddCuttingLayer);
    connect(m_ui->actionRemoveLayer, &QAction::triggered, this, &LaserControllerWindow::onActionRemoveLayer);
    connect(m_ui->treeWidgetLayers, &QTreeWidget::itemDoubleClicked, this, &LaserControllerWindow::onTreeWidgetLayersItemDoubleClicked);
    connect(m_ui->actionExportJSON, &QAction::triggered, this, &LaserControllerWindow::onActionExportJson);

    StateControllerInst.initState().addTransition(this, SIGNAL(windowCreated()), &StateControllerInst.normalState());

    bindWidgetsProperties();
}

LaserControllerWindow::~LaserControllerWindow()
{

}

void LaserControllerWindow::onActionAddEngravingLayer(bool)
{
    QString newName = m_scene->document()->newLayerName(LLT_ENGRAVING);
    LaserLayerDialog dialog(m_scene->document(), LLT_ENGRAVING);
    if (dialog.exec() == QDialog::Accepted)
    {
        LaserLayer* layer = dialog.layer();
        m_scene->document()->addLayer(layer);
    }
}

void LaserControllerWindow::onActionAddCuttingLayer(bool checked)
{
    LaserLayerDialog dialog(m_scene->document(), LLT_CUTTING);
    if (dialog.exec() == QDialog::Accepted)
    {
        LaserLayer* layer = dialog.layer();
        m_scene->document()->addLayer(layer);
    }
}

void LaserControllerWindow::onActionRemoveLayer(bool checked)
{
    qDebug() << "removing layer.";
}

void LaserControllerWindow::onTreeWidgetLayersItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    //int i = item->data(0, Qt::UserRole).toInt();
    //qDebug() << i;
    if (item->parent() == nullptr)
    {
        //LaserLayerType type = (LaserLayerType)item->data(1, Qt::UserRole).toInt();
        //LaserLayer* layer = nullptr;
        LaserLayer* layer = item->data(0, Qt::UserRole).value<LaserLayer*>();
        /*if (type == LLT_CUTTING)
        {
            layer = m_scene->document()->cuttingLayers()[i];
        }
        else if (type == LLT_ENGRAVING)
        {
            layer = m_scene->document()->engravingLayers()[i];
        }*/
        LaserLayerDialog dialog(layer);
        if (dialog.exec() == QDialog::Accepted)
        {
            m_ui->treeWidgetLayers->updateItems();
        }
        //qDebug() << type << layer->name();
    }
    else
    {
        LaserPrimitive* laserPrimitive = item->data(0, Qt::UserRole).value<LaserPrimitive*>();
        //qDebug() << type;
    }
}

void LaserControllerWindow::onActionExportJson(bool checked)
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setMimeTypeFilters(QStringList() << "application/json");
    dialog.setWindowTitle(tr("Export"));
    if (dialog.exec() == QFileDialog::AcceptSave)
    {
        QString filename = dialog.selectedFiles().constFirst();
        if (!filename.isEmpty() && !filename.isNull())
            m_scene->document()->exportJSON(filename);

    }
}

void LaserControllerWindow::bindWidgetsProperties()
{
    StateControllerInst.initState().assignProperty(m_ui->actionOpen, "enabled", false);
    StateControllerInst.normalState().assignProperty(m_ui->actionOpen, "enabled", true);
    StateControllerInst.mainState().assignProperty(m_ui->actionOpen, "enabled", true);
    StateControllerInst.machiningState().assignProperty(m_ui->actionOpen, "enabled", false);

    StateControllerInst.initState().assignProperty(m_ui->actionSave, "enabled", false);
    StateControllerInst.normalState().assignProperty(m_ui->actionSave, "enabled", false);
    StateControllerInst.mainState().assignProperty(m_ui->actionSave, "enabled", false);
    StateControllerInst.machiningState().assignProperty(m_ui->actionSave, "enabled", false);

    StateControllerInst.initState().assignProperty(m_ui->actionImportSVG, "enabled", false);
    StateControllerInst.normalState().assignProperty(m_ui->actionImportSVG, "enabled", true);
    StateControllerInst.mainState().assignProperty(m_ui->actionImportSVG, "enabled", true);
    StateControllerInst.machiningState().assignProperty(m_ui->actionImportSVG, "enabled", false);

    StateControllerInst.initState().assignProperty(m_ui->actionExportJSON, "enabled", false);
    StateControllerInst.normalState().assignProperty(m_ui->actionExportJSON, "enabled", false);
    StateControllerInst.mainState().assignProperty(m_ui->actionExportJSON, "enabled", true);
    StateControllerInst.machiningState().assignProperty(m_ui->actionExportJSON, "enabled", false);

    StateControllerInst.initState().assignProperty(m_ui->toolButtonAddLayer, "enabled", false);
    StateControllerInst.normalState().assignProperty(m_ui->toolButtonAddLayer, "enabled", false);
    StateControllerInst.mainState().assignProperty(m_ui->toolButtonAddLayer, "enabled", true);
    StateControllerInst.machiningState().assignProperty(m_ui->toolButtonAddLayer, "enabled", false);

    StateControllerInst.initState().assignProperty(m_ui->actionAddEngravingLayer, "enabled", false);
    StateControllerInst.normalState().assignProperty(m_ui->actionAddEngravingLayer, "enabled", false);
    StateControllerInst.mainState().assignProperty(m_ui->actionAddEngravingLayer, "enabled", true);
    StateControllerInst.machiningState().assignProperty(m_ui->actionAddEngravingLayer, "enabled", false);

    StateControllerInst.initState().assignProperty(m_ui->actionAddCuttingLayer, "enabled", false);
    StateControllerInst.normalState().assignProperty(m_ui->actionAddCuttingLayer, "enabled", false);
    StateControllerInst.mainState().assignProperty(m_ui->actionAddCuttingLayer, "enabled", true);
    StateControllerInst.machiningState().assignProperty(m_ui->actionAddCuttingLayer, "enabled", false);

    StateControllerInst.initState().assignProperty(m_ui->toolButtonRemoveLayer, "enabled", false);
    StateControllerInst.normalState().assignProperty(m_ui->toolButtonRemoveLayer, "enabled", false);
    StateControllerInst.mainState().assignProperty(m_ui->toolButtonRemoveLayer, "enabled", true);
    StateControllerInst.machiningState().assignProperty(m_ui->toolButtonRemoveLayer, "enabled", false);
}

void LaserControllerWindow::showEvent(QShowEvent * event)
{
    if (!m_created)
    {
        m_created = true;
        QTimer::singleShot(100, this, &LaserControllerWindow::windowCreated);
    }
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
    QSignalTransition* t = StateControllerInst.normalState().addTransition(importer.data(), SIGNAL(imported()), &StateControllerInst.mainState());
    LaserDocument* doc = importer->import(filename);
    if (doc)
    {
        m_scene->updateDocument(doc);
        m_ui->treeWidgetLayers->setDocument(doc);
        m_ui->treeWidgetLayers->updateItems();
    }
    StateControllerInst.normalState().removeTransition(reinterpret_cast<QAbstractTransition*>(t));
}
