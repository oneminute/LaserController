#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"

#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QErrorMessage>
#include <QMessageBox>

#include "import/Importer.h"
#include "laser/LaserDriver.h"
#include "scene/LaserDocument.h"
#include "scene/LaserItem.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "state/StateController.h"
#include "ui/LaserLayerDialog.h"
#include "ui/ConnectionDialog.h"
#include "widget/LaserViewer.h"
#include "util/Utils.h"

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
    m_ui->toolButtonRemoveLayer->setDefaultAction(m_ui->actionRemoveLayer);

    // set up tools buttons
    QToolButton* toolButtonSelectionTool = new QToolButton();
    QToolButton* toolButtonRectangleTool = new QToolButton();
    QToolButton* toolButtonEllipseTool = new QToolButton();
    QToolButton* toolButtonPolygonTool = new QToolButton();
    QToolButton* toolButtonTextTool = new QToolButton();
    QToolButton* toolButtonLineTool = new QToolButton();
    QToolButton* toolButtonSplineTool = new QToolButton();
    QToolButton* toolButtonBitmapTool = new QToolButton();

    toolButtonSelectionTool->setDefaultAction(m_ui->actionSelectionTool);
    toolButtonRectangleTool->setDefaultAction(m_ui->actionRectangleTool);
    toolButtonEllipseTool->setDefaultAction(m_ui->actionEllipseTool);
    toolButtonPolygonTool->setDefaultAction(m_ui->actionPolygonTool);
    toolButtonTextTool->setDefaultAction(m_ui->actionTextTool);
    toolButtonLineTool->setDefaultAction(m_ui->actionLineTool);
    toolButtonSplineTool->setDefaultAction(m_ui->actionSplineTool);
    toolButtonBitmapTool->setDefaultAction(m_ui->actionBitmapTool);

    m_ui->toolBarTools->addWidget(toolButtonSelectionTool);
    m_ui->toolBarTools->addWidget(toolButtonRectangleTool);
    m_ui->toolBarTools->addWidget(toolButtonEllipseTool);
    m_ui->toolBarTools->addWidget(toolButtonPolygonTool);
    m_ui->toolBarTools->addWidget(toolButtonTextTool);
    m_ui->toolBarTools->addWidget(toolButtonLineTool);
    m_ui->toolBarTools->addWidget(toolButtonSplineTool);
    m_ui->toolBarTools->addWidget(toolButtonBitmapTool);

    connect(m_ui->actionImportSVG, &QAction::triggered, this, &LaserControllerWindow::onActionImportSVG);
    connect(m_ui->actionImportCorelDraw, &QAction::triggered, this, &LaserControllerWindow::onActionImportCorelDraw);
    connect(m_ui->actionAddEngravingLayer, &QAction::triggered, this, &LaserControllerWindow::onActionAddEngravingLayer);
    connect(m_ui->actionAddCuttingLayer, &QAction::triggered, this, &LaserControllerWindow::onActionAddCuttingLayer);
    connect(m_ui->actionRemoveLayer, &QAction::triggered, this, &LaserControllerWindow::onActionRemoveLayer);
    connect(m_ui->treeWidgetLayers, &QTreeWidget::itemDoubleClicked, this, &LaserControllerWindow::onTreeWidgetLayersItemDoubleClicked);
    connect(m_ui->actionExportJSON, &QAction::triggered, this, &LaserControllerWindow::onActionExportJson);
    connect(m_ui->actionMachining, &QAction::triggered, this, &LaserControllerWindow::onActionMechining);
    connect(m_ui->actionPause, &QAction::triggered, this, &LaserControllerWindow::onActionPauseMechining);
    connect(m_ui->actionStop, &QAction::triggered, this, &LaserControllerWindow::onActionStopMechining);
    connect(m_ui->actionConnect, &QAction::triggered, this, &LaserControllerWindow::onActionConnect);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &LaserControllerWindow::onActionDisconnect);
    connect(m_ui->actionLoadMotor, &QAction::triggered, this, &LaserControllerWindow::onActionLoadMotor);
    connect(m_ui->actionUnloadMotor, &QAction::triggered, this, &LaserControllerWindow::onActionUnloadMotor);

    ADD_TRANSITION(initState, workingState, this, SIGNAL(windowCreated()));

    bindWidgetsProperties();

    // check tmp folder
    QDir appDir(QCoreApplication::applicationDirPath());
    m_tmpDir = QDir(QCoreApplication::applicationDirPath() + "/tmp");
    if (!m_tmpDir.exists())
    {
        appDir.mkpath("tmp");
    }
}

LaserControllerWindow::~LaserControllerWindow()
{

}

void LaserControllerWindow::onActionImportSVG(bool checked)
{
    QString filename = getFilename(tr("Open SVG File"), QStringList() << "image/svg+xml" << "image/svg+xml-compressed");
    if (filename.isEmpty())
        return;
    QSharedPointer<Importer> importer = Importer::getImporter(Importer::SVG);
    //QSignalTransition* t = StateControllerInst.normalState().addTransition(importer.data(), SIGNAL(imported()), &StateControllerInst.mainState());
    LaserDocument* doc = importer->import(filename);
    if (doc)
    {
        m_scene->updateDocument(doc);
        m_ui->treeWidgetLayers->setDocument(doc);
        m_ui->treeWidgetLayers->updateItems();
    }
    //StateControllerInst.normalState().removeTransition(reinterpret_cast<QAbstractTransition*>(t));
}

void LaserControllerWindow::onActionImportCorelDraw(bool checked)
{
    QSharedPointer<Importer> importer = Importer::getImporter(Importer::CORELDRAW);
    LaserDocument* doc = importer->import();
    if (doc)
    {
        m_scene->updateDocument(doc);
        m_ui->treeWidgetLayers->setDocument(doc);
        m_ui->treeWidgetLayers->updateItems();
    }
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
    QTreeWidgetItem* item = m_ui->treeWidgetLayers->currentItem();
    if (item == nullptr)
        return;
    if (item->parent() == nullptr)
    {
        QString id = item->data(0, Qt::UserRole).toString();
        LaserLayer* layer = m_scene->document()->laserLayer(id);
        
        if (layer->type() == LLT_CUTTING)
        {
            if (m_scene->document()->cuttingLayers()[0] == layer)
            {
                QMessageBox::warning(this, tr("Remove layer"), tr("You can not remove base cutting layer."), QMessageBox::StandardButton::Ok, QMessageBox::NoButton);
                return;
            }
        }
        else if (layer->type() == LLT_ENGRAVING)
        {
            if (m_scene->document()->engravingLayers()[0] == layer)
            {
                QMessageBox::warning(this, tr("Remove layer"), tr("You can not remove base engraving layer."), QMessageBox::StandardButton::Ok, QMessageBox::NoButton);
                return;
            }
        }
        int result = QMessageBox::question(this, tr("Remove layer"), tr("Do you want to remove this layer?"),
            QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Cancel);
        if (result == QMessageBox::StandardButton::Apply)
        {
            m_scene->document()->removeLayer(layer);
        }
    }
}

void LaserControllerWindow::onTreeWidgetLayersItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    QString id = item->data(0, Qt::UserRole).toString();
    if (item->parent() == nullptr)
    {
        LaserLayer* layer = m_scene->document()->laserLayer(id);
        
        LaserLayerDialog dialog(layer);
        if (dialog.exec() == QDialog::Accepted)
        {
            m_ui->treeWidgetLayers->updateItems();
        }
        //qDebug() << type << layer->name();
    }
    else
    {
        LaserPrimitive* laserPrimitive = m_scene->document()->laserPrimitive(id);
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

void LaserControllerWindow::onActionMechining(bool checked)
{
    //QString filename = utils::createUUID("json_") + ".json";

    QString filename = "export.json";
    filename = m_tmpDir.absoluteFilePath(filename);
    m_scene->document()->exportJSON(filename);
    qDebug() << "export temp json file for machining" << filename;
    LaserDriver::instance().loadDataFromFile(filename);
    LaserDriver::instance().startMachining(true);
    //m_tmpDir.remove(filename);
}

void LaserControllerWindow::onActionPauseMechining(bool checked)
{
    LaserDriver::instance().pauseContinueMachining(checked);
}

void LaserControllerWindow::onActionStopMechining(bool checked)
{
    LaserDriver::instance().stopMachining();
}

void LaserControllerWindow::onActionConnect(bool checked)
{
    ConnectionDialog dialog;
    if (dialog.exec() == QDialog::Accepted)
    {
        if (!LaserDriver::instance().isConnected())
        {
            QErrorMessage dialog;
            dialog.showMessage(tr("Can not connect to laser machine."));
        }
    }
}

void LaserControllerWindow::onActionDisconnect(bool checked)
{
    if (QMessageBox::Apply == QMessageBox::question(this, tr("Disconnect"), tr("Do you want to disconnect from laser machine?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {
        LaserDriver::instance().unInitComPort();
    }
}

void LaserControllerWindow::onActionLoadMotor(bool checked)
{
    LaserDriver::instance().controlMotor(true);
}

void LaserControllerWindow::onActionUnloadMotor(bool checked)
{
    if (QMessageBox::Apply == QMessageBox::question(this, tr("Unload motor"), tr("Do you want to unload motor?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {
        LaserDriver::instance().controlMotor(false);
    }
}

void LaserControllerWindow::bindWidgetsProperties()
{
    // actionOpen
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", false, documentWorkingState);
    // end actionOpen

    // actionImportSVG
    BIND_PROP_TO_STATE(m_ui->actionImportSVG, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionImportSVG, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionImportSVG, "enabled", false, documentWorkingState);
    // end actionImportSVG

    // actionExportJSON
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentWorkingState);
    // end actionExportJSON

    // actionSave
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentWorkingState);
    // end actionSave

    // actionSaveAs
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentWorkingState);
    // end actionSaveAs

    // actionCloseDocument
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentWorkingState);
    // end actionCloseDocument

    // toolButtonAddLayer
    BIND_PROP_TO_STATE(m_ui->toolButtonAddLayer, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->toolButtonAddLayer, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->toolButtonAddLayer, "enabled", true, documentWorkingState);
    // end toolButtonAddLayer

    // toolButtonRemoveLayer
    BIND_PROP_TO_STATE(m_ui->toolButtonRemoveLayer, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->toolButtonRemoveLayer, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->toolButtonRemoveLayer, "enabled", true, documentWorkingState);
    // end toolButtonRemoveLayer

    // actionAddEngravingLayer
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", true, documentWorkingState);
    // end actionAddEngravingLayer

    // actionAddCuttingLayer
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", true, documentWorkingState);
    // end actionAddCuttingLayer

    // actionConnect
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", true, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, devicePauseState);
    // end actionConnect

    // actionDisconnect
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, devicePauseState);
    // end actionDisconnect

    // actionDisconnect
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, devicePauseState);
    // end actionDisconnect

    // actionDisconnect
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, devicePauseState);
    // end actionDisconnect

    // actionStop
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, devicePauseState);
    // end actionStop
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


