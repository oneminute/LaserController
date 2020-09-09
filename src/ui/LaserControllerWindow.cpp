﻿#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"

#include <QErrorMessage>
#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QTreeWidgetItem>

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
#include "ui/HalftoneDialog.h"
#include "util/ImageUtils.h"
#include "util/Utils.h"
#include "widget/LaserViewer.h"

LaserControllerWindow::LaserControllerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::LaserControllerWindow)
    , m_created(false)
    , m_useLoadedJson(false)
{
    m_ui->setupUi(this);

    m_viewer = reinterpret_cast<LaserViewer*>(m_ui->graphicsView);
    m_scene = reinterpret_cast<LaserScene*>(m_viewer->scene());
    
    for (int i = 0; i < LaserDocument::engravingLayersCount(); i++)
    {
        QPushButton* button = new QPushButton;
        button->setFixedWidth(30);
        button->setFixedHeight(30);
        QColor color(QColor::Hsv);
        color.setHsv(i * 179 / LaserDocument::engravingLayersCount(), 255, 255);
        QPalette pal = button->palette();
        pal.setColor(QPalette::Button, color);
        button->setAutoFillBackground(true);
        button->setPalette(pal);
        button->update();
        m_ui->horizontalLayoutLayerButtons->addWidget(button);
        connect(button, &QPushButton::clicked, [=](bool checked = false)
            {
                qDebug() << "engraving" << i;
                if (m_scene->selectedPrimitives().count() > 0 &&
                    QMessageBox::Apply == QMessageBox::question(this, tr("Move primitives?"), tr("Do you want to move primitives to selected layer?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
                {
                    for (LaserPrimitive* primitive : m_scene->selectedPrimitives())
                    {
                        m_scene->document()->addItem(primitive, m_scene->document()->engravingLayers()[i]);
                    }
                }
            }
        );
    }

    for (int i = 0; i < LaserDocument::cuttingLayersCount(); i++)
    {
        QPushButton* button = new QPushButton;
        button->setFixedWidth(30);
        button->setFixedHeight(30);
        QColor color(QColor::Hsv);
        color.setHsv(i * 179 / LaserDocument::cuttingLayersCount() + 180, 255, 255);
        QPalette pal = button->palette();
        pal.setColor(QPalette::Button, color);
        button->setAutoFillBackground(true);
        button->setPalette(pal);
        button->update();
        m_ui->horizontalLayoutLayerButtons->addWidget(button);
        connect(button, &QPushButton::clicked, [=](bool checked = false)
            {
                qDebug() << "cutting" << i;
                if (m_scene->selectedPrimitives().count() > 0 &&
                    QMessageBox::Apply == QMessageBox::question(this, tr("Move primitives?"), tr("Do you want to move primitives to selected layer?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
                {
                    for (LaserPrimitive* primitive : m_scene->selectedPrimitives())
                    {
                        m_scene->document()->addItem(primitive, m_scene->document()->cuttingLayers()[i]);
                    }
                }
            }
        );
    }

    m_ui->horizontalLayoutLayerButtons->addStretch();

    // initialize layers Tree Widget
    m_ui->tableWidgetLayers->setColumnWidth(0, 30);
    m_ui->tableWidgetLayers->setColumnWidth(1, 45);
    m_ui->tableWidgetLayers->setColumnWidth(2, 60);
    m_ui->tableWidgetLayers->setColumnWidth(3, 75);

    //m_ui->toolButtonAddLayer->addAction(m_ui->actionAddEngravingLayer);
    //m_ui->toolButtonAddLayer->addAction(m_ui->actionAddCuttingLayer);
    //m_ui->toolButtonRemoveLayer->setDefaultAction(m_ui->actionRemoveLayer);

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

    m_ui->toolButtonConnect->setDefaultAction(m_ui->actionConnect);
    m_ui->toolButtonDisconnect->setDefaultAction(m_ui->actionDisconnect);
    m_ui->toolButtonStart->setDefaultAction(m_ui->actionMachining);
    m_ui->toolButtonPause->setDefaultAction(m_ui->actionPause);
    m_ui->toolButtonStop->setDefaultAction(m_ui->actionStop);

    // init status bar
    m_statusBarStatus = new QLabel;
    m_statusBarStatus->setText(tr("Tips"));
    m_statusBarStatus->setMinimumWidth(120);
    m_statusBarStatus->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarStatus);
    m_ui->statusbar->addWidget(utils::createSeparator());

    m_statusBarTips = new QLabel;
    m_statusBarTips->setText(tr("Welcome!"));
    m_statusBarTips->setMinimumWidth(120);
    m_statusBarTips->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarTips);
    m_ui->statusbar->addWidget(utils::createSeparator());

    m_statusBarCoordinate = new QLabel;
    m_statusBarCoordinate->setText(tr("0,0"));
    m_statusBarCoordinate->setMinimumWidth(45);
    m_statusBarCoordinate->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarCoordinate);
    m_ui->statusbar->addWidget(utils::createSeparator());
    
    m_statusBarLocation = new QLabel;
    m_statusBarLocation->setText(tr("Top Left"));
    m_statusBarLocation->setMinimumWidth(100);
    m_statusBarLocation->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addPermanentWidget(utils::createSeparator());
    m_ui->statusbar->addPermanentWidget(m_statusBarLocation);

    m_statusBarPageInfo = new QLabel;
    m_statusBarPageInfo->setText(tr("Page Size(mm): 210x320"));
    m_statusBarPageInfo->setMinimumWidth(150);
    m_statusBarPageInfo->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addPermanentWidget(utils::createSeparator());
    m_ui->statusbar->addPermanentWidget(m_statusBarPageInfo);

    m_statusBarCopyright = new QLabel;
    m_statusBarCopyright->setText(tr(u8"Copyright \u00a9 2020 Super Laser Technologies, Inc. All Rights Reserved."));
    m_statusBarCopyright->setMinimumWidth(200);
    m_statusBarCopyright->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addPermanentWidget(utils::createSeparator());
    m_ui->statusbar->addPermanentWidget(m_statusBarCopyright);

    connect(m_ui->actionImportSVG, &QAction::triggered, this, &LaserControllerWindow::onActionImportSVG);
    connect(m_ui->actionImportCorelDraw, &QAction::triggered, this, &LaserControllerWindow::onActionImportCorelDraw);
    connect(m_ui->actionAddEngravingLayer, &QAction::triggered, this, &LaserControllerWindow::onActionAddEngravingLayer);
    connect(m_ui->actionAddCuttingLayer, &QAction::triggered, this, &LaserControllerWindow::onActionAddCuttingLayer);
    connect(m_ui->actionRemoveLayer, &QAction::triggered, this, &LaserControllerWindow::onActionRemoveLayer);
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
    connect(m_ui->actionDeletePrimitive, &QAction::triggered, this, &LaserControllerWindow::onActionDeletePrimitive);
    connect(m_ui->actionCloseDocument, &QAction::triggered, this, &LaserControllerWindow::onActionCloseDocument);

    connect(m_ui->tableWidgetLayers, &QTableWidget::cellDoubleClicked, this, &LaserControllerWindow::onTableWidgetLayersCellDoubleClicked);
    connect(m_ui->tableWidgetLayers, &LaserLayerTableWidget::layerSelectionChanged, this, &LaserControllerWindow::onTableWidgetLayersSelectionChanged);

    connect(&LaserDriver::instance(), &LaserDriver::comPortsFetched, this, &LaserControllerWindow::onDriverComPortsFetched);
    connect(&LaserDriver::instance(), &LaserDriver::comPortConnected, this, &LaserControllerWindow::onDriverComPortConnected);
    connect(&LaserDriver::instance(), &LaserDriver::comPortDisconnected, this, &LaserControllerWindow::onDriverComPortDisconnected);

    //connect(this, &LaserControllerWindow::windowCreated, this, &LaserControllerWindow::onWindowCreated);
    connect(StateController::instance().deviceUnconnectedState(), &QState::entered, this, &LaserControllerWindow::onEnterDeviceUnconnectedState);

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
        m_ui->tableWidgetLayers->setDocument(doc);
        m_ui->tableWidgetLayers->updateItems();
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
        m_ui->tableWidgetLayers->setDocument(doc);
        m_ui->tableWidgetLayers->updateItems();
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
    QTableWidgetItem* item = m_ui->tableWidgetLayers->currentItem();
    /*if (item == nullptr)
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
    }*/
}

void LaserControllerWindow::onTableWidgetLayersCellDoubleClicked(int row, int column)
{
    QTableWidgetItem* item = m_ui->tableWidgetLayers->item(row, 0);
    QString id = item->data(Qt::UserRole).toString();

    LaserLayer* layer = m_scene->document()->laserLayer(id);

    LaserLayerDialog dialog(layer);
    if (dialog.exec() == QDialog::Accepted)
    {
        m_ui->tableWidgetLayers->updateItems();
    }
}

void LaserControllerWindow::onTableWidgetLayersSelectionChanged(const QString & layerId)
{
    
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
    //ConnectionTask* task = LaserDriver::instance().createConnectionTask(this);
    //task->start();
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

        HalftoneDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted)
        {
            float pixelInterval = dialog.pixelInterval() * dialog.yPulseLength();

            int outWidth = mmWidth * MM_TO_INCH * dialog.dpi();
            int outHeight = std::round(mmHeight / pixelInterval);
            qDebug() << " out width:" << outWidth;
            qDebug() << "out height:" << outHeight;

            cv::Mat resized;
            cv::resize(src, resized, cv::Size(outWidth, outHeight));
            
            imageUtils::halftone2(resized, dialog.lpi(), dialog.dpi(), dialog.degrees(), dialog.nonlinearCoefficient());
        }
    }
}

void LaserControllerWindow::onActionDeletePrimitive(bool checked)
{
    if (QMessageBox::Apply == QMessageBox::question(this, tr("Delete primitives?"), tr("Do you want to delete primitives to selected layer?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {
        for (LaserPrimitive* primitive : m_scene->selectedPrimitives())
        {
            m_scene->document()->removeItem(primitive);
        }
    }
}

void LaserControllerWindow::onActionCloseDocument(bool checked)
{
    if (QMessageBox::Apply == QMessageBox::question(this, tr("Close document?"), tr("Do you want to close current document?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {
        m_scene->clearDocument(true);
    }
}

void LaserControllerWindow::onDriverComPortsFetched(const QStringList & ports)
{
    for (int i = 0; i < ports.size(); i++)
    {
        m_ui->comboBoxDevices->addItem(ports[i], utils::parsePortName(ports[i]));
    }

    if (!ports.isEmpty())
    {
        LaserDriver::instance().initComPort(ports[0]);
    }
}

void LaserControllerWindow::onDriverComPortConnected()
{
    m_statusBarStatus->setText(tr("Device Connected"));
}

void LaserControllerWindow::onDriverComPortDisconnected(bool isError, const QString & errorMsg)
{
    m_statusBarStatus->setText(tr("Device Disconnected"));
}

void LaserControllerWindow::onWindowCreated()
{
}

void LaserControllerWindow::onEnterDeviceUnconnectedState()
{
    static bool first = true;
    if (first)
    {
        LaserDriver::instance().load();
        LaserDriver::instance().init(this);
        LaserDriver::instance().getPortListAsyn();
        first = false;
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

    // actionImportCorelDraw
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, documentWorkingState);
    // end actionImportCorelDraw

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
    //BIND_PROP_TO_STATE(m_ui->toolButtonAddLayer, "enabled", false, initState);
    //BIND_PROP_TO_STATE(m_ui->toolButtonAddLayer, "enabled", false, documentEmptyState);
    //BIND_PROP_TO_STATE(m_ui->toolButtonAddLayer, "enabled", true, documentWorkingState);
    // end toolButtonAddLayer

    // toolButtonRemoveLayer
    //BIND_PROP_TO_STATE(m_ui->toolButtonRemoveLayer, "enabled", false, initState);
    //BIND_PROP_TO_STATE(m_ui->toolButtonRemoveLayer, "enabled", false, documentEmptyState);
    //BIND_PROP_TO_STATE(m_ui->toolButtonRemoveLayer, "enabled", true, documentWorkingState);
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

    // toolButtonConnect
    BIND_PROP_TO_STATE(m_ui->toolButtonConnect, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->toolButtonConnect, "enabled", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonConnect, "enabled", true, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonConnect, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->toolButtonConnect, "enabled", false, devicePausedState);
    // end toolButtonConnect

    // toolButtonDisconnect
    BIND_PROP_TO_STATE(m_ui->toolButtonDisconnect, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->toolButtonDisconnect, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonDisconnect, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonDisconnect, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->toolButtonDisconnect, "enabled", true, devicePausedState);
    // end toolButtonDisconnect

    // actionMachining
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, devicePausedState);
    // end actionMachining

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


