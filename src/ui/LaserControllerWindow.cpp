#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"

#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QErrorMessage>
#include <QMessageBox>
#include <QThread>
#include <QTemporaryFile>
#include <QImage>

#include "import/Importer.h"
#include "laser/LaserDriver.h"
#include "scene/LaserDocument.h"
#include "scene/LaserItem.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "state/StateController.h"
#include "task/ConnectionTask.h"
#include "task/DisconnectionTask.h"
#include "task/MachiningTask.h"
#include "ui/LaserLayerDialog.h"
#include "util/ImageUtils.h"
#include "util/Utils.h"
#include "widget/LaserViewer.h"

#include "halftone/SSIM.h"
#include "halftone/Ostromoukhov.h"
#include "halftone/SAHalftoner.h"

LaserControllerWindow::LaserControllerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::LaserControllerWindow)
    , m_scene(new LaserScene)
    , m_created(false)
    , m_useLoadedJson(false)
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
    connect(m_ui->actionLoadJson, &QAction::triggered, this, &LaserControllerWindow::onActionLoadJson);
    connect(m_ui->actionMachining, &QAction::triggered, this, &LaserControllerWindow::onActionMachining);
    connect(m_ui->actionPause, &QAction::triggered, this, &LaserControllerWindow::onActionPauseMechining);
    connect(m_ui->actionStop, &QAction::triggered, this, &LaserControllerWindow::onActionStopMechining);
    connect(m_ui->actionConnect, &QAction::triggered, this, &LaserControllerWindow::onActionConnect);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &LaserControllerWindow::onActionDisconnect);
    connect(m_ui->actionLoadMotor, &QAction::triggered, this, &LaserControllerWindow::onActionLoadMotor);
    connect(m_ui->actionUnloadMotor, &QAction::triggered, this, &LaserControllerWindow::onActionUnloadMotor);
    connect(m_ui->actionDownload, &QAction::triggered, this, &LaserControllerWindow::onActionDownload);
    connect(m_ui->actionWorkState, &QAction::triggered, this, &LaserControllerWindow::onActionWorkState);
    connect(m_ui->actionMoveToOriginalPoint, &QAction::triggered, this, &LaserControllerWindow::onActionMoveToOriginalPoint);
    connect(m_ui->actionHalfTone, &QAction::triggered, this, &LaserControllerWindow::onActionHalfTone);

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
    if (dialog.exec() == QFileDialog::Accepted)
    {
        QString filename = dialog.selectedFiles().constFirst();
        if (!filename.isEmpty() && !filename.isNull())
            m_scene->document()->exportJSON(filename);

    }
}

void LaserControllerWindow::onActionLoadJson(bool checked)
{
    QFileDialog dialog(this);
    //dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters(QStringList() << "application/json");
    dialog.setWindowTitle(tr("Load Json"));
    if (dialog.exec() == QFileDialog::Accepted)
    {
        QString filename = dialog.selectedFiles().constFirst();
        if (!filename.isEmpty() && !filename.isNull())
        {
            m_currentJson = filename;
            m_useLoadedJson = true;
            qDebug() << m_currentJson << m_useLoadedJson;
        }
    }
}

void LaserControllerWindow::onActionMachining(bool checked)
{
    if (m_useLoadedJson)
    {
        qDebug() << "export temp json file for machining" << m_currentJson;
        MachiningTask* task = LaserDriver::instance().createMachiningTask(m_currentJson, false);
        task->start();
    }
    else
    {
        QString filename = "export.json";
        //QTemporaryFile file;
        //if (file.open())
        //{
            //QString filename = file.fileName();
            qDebug() << "exporting to temporary json file:" << filename;
            m_scene->document()->exportJSON(filename);
            qDebug() << "export temp json file for machining" << filename;
            MachiningTask* task = LaserDriver::instance().createMachiningTask(filename, false);
            task->start();
        //}
    }
    m_useLoadedJson = false;
}

void LaserControllerWindow::onActionPauseMechining(bool checked)
{
    LaserDriver::instance().pauseContinueMachining(!checked);
}

void LaserControllerWindow::onActionStopMechining(bool checked)
{
    LaserDriver::instance().stopMachining();
}

void LaserControllerWindow::onActionConnect(bool checked)
{
    ConnectionTask* task = LaserDriver::instance().createConnectionTask(this);
    task->start();
}

void LaserControllerWindow::onActionDisconnect(bool checked)
{
    DisconnectionTask* task = LaserDriver::instance().createDisconnectionTask(this);
    task->start();
}

void LaserControllerWindow::onActionDownload(bool checked)
{
    QString filename = "export.json";
    filename = m_tmpDir.absoluteFilePath(filename);
    m_scene->document()->exportJSON(filename);
    qDebug() << "export temp json file for machining" << filename;
    LaserDriver::instance().loadDataFromFile(filename);
}

void LaserControllerWindow::onActionLoadMotor(bool checked)
{
    LaserDriver::instance().controlMotor(false);
}

void LaserControllerWindow::onActionUnloadMotor(bool checked)
{
    if (QMessageBox::Apply == QMessageBox::question(this, tr("Unload motor"), tr("Do you want to unload motor?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {
        LaserDriver::instance().controlMotor(false);
    }
}

void LaserControllerWindow::onActionWorkState(bool checked)
{
    LaserDriver::instance().getDeviceWorkState();
}

void LaserControllerWindow::onActionMoveToOriginalPoint(bool checked)
{
    LaserDriver::instance().lPenMoveToOriginalPoint(50);
}

void LaserControllerWindow::onActionHalfTone(bool checked)
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), QString(), tr("Images (*.png *.bmp *.jpg)"));
    if (!filename.isEmpty() && !filename.isNull())
    {
        QImage image(filename);
        image = image.convertToFormat(QImage::Format_Grayscale8);
        cv::Mat src(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        float mmWidth = 1000.f * image.width() / image.dotsPerMeterX();
        float mmHeight = 1000.f * image.height() / image.dotsPerMeterY();
        //qDebug() << "dpiX:" << dpiX << ", dpiY:" << dpiY;
        //imageUtils::floydSteinberg(src, mmWidth, mmHeight);
        imageUtils::halftone2(src, mmWidth, mmHeight, 50, 600, 45);

        //SAHer saher(src);
        //saher.ComputeSAH();
        //cv::imshow("sah", saher.GetResult());
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
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, devicePausedState);
    // end actionConnect

    // actionDisconnect
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, devicePausedState);
    // end actionDisconnect

    // actionDisconnect
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, devicePausedState);
    // end actionDisconnect

    // actionPause
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "setChecked", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "setChecked", true, devicePausedState);
    // end actionPause

    // actionStop
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, devicePausedState);
    // end actionStop

    // actionLoadJson

    // end actionLoadJson
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


