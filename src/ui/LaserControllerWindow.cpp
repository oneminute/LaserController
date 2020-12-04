#include "LaserControllerWindow.h"
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
//#include <QCustomPlot>

#include "import/Importer.h"
#include "laser/LaserDriver.h"
#include "scene/LaserDocument.h"
#include "scene/LaserPrimitive.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "state/StateController.h"
#include "task/ConnectionTask.h"
#include "task/DisconnectionTask.h"
#include "task/MachiningTask.h"
#include "ui/LaserLayerDialog.h"
#include "ui/HalftoneDialog.h"
#include "ui/ParameterDialog.h"
#include "ui/RegistersDialog.h"
#include "util/ImageUtils.h"
#include "util/Utils.h"
#include "widget/LaserViewer.h"
#include "widget/LayerButton.h"
#include "widget/PropertiesHelperManager.h"

LaserControllerWindow::LaserControllerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::LaserControllerWindow)
    , m_created(false)
    , m_useLoadedJson(false)
{
    m_ui->setupUi(this);
    setDockNestingEnabled(true);

    QList<QColor> colors;
    colors << QColor(Qt::red)
        << QColor(Qt::blue)
        << QColor(Qt::darkRed)
        << QColor(Qt::darkBlue)
        << QColor(Qt::green)
        << QColor(Qt::cyan)
        << QColor(Qt::darkGreen)
        << QColor(Qt::darkCyan)
        << QColor(Qt::magenta)
        << QColor(Qt::yellow)
        << QColor(Qt::darkMagenta)
        << QColor(Qt::darkYellow)
        << QColor(qRgb(172, 0, 0))
        << QColor(qRgb(0, 172, 0))
        << QColor(qRgb(0, 0, 196))
        << QColor(qRgb(196, 48, 172));

    m_viewer = reinterpret_cast<LaserViewer*>(m_ui->graphicsView);
    m_scene = reinterpret_cast<LaserScene*>(m_viewer->scene());
    
    int colorTick = 360 / LaserDocument::layersCount();
    for (int i = 0; i < LaserDocument::layersCount(); i++)
    {
        LayerButton* button = new LayerButton;
        button->setMinimumWidth(30);
        button->setFixedHeight(30);
        button->setColor(colors[i]);
        button->setText(QString(tr("%1")).arg(i + 1, 2, 10, QLatin1Char('0')));
        button->update();
        m_ui->layoutLayerButtons->addWidget(button);
        m_layerButtons.append(button);

        connect(button, &LayerButton::colorUpdated, m_ui->tableWidgetLayers, &LaserLayerTableWidget::updateItems);
    }
    m_ui->layoutLayerButtons->addStretch();

    //removeDockWidget(m_ui->dockWidgetLayerButtons);
    removeDockWidget(m_ui->dockWidgetLayers);
    removeDockWidget(m_ui->dockWidgetProperties);
    removeDockWidget(m_ui->dockWidgetOperations);

    //addDockWidget(Qt::RightDockWidgetArea, m_ui->dockWidgetLayerButtons);
    splitDockWidget(m_ui->dockWidgetLayerButtons, m_ui->dockWidgetLayers, Qt::Horizontal);
    splitDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetOperations, Qt::Vertical);

    tabifyDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetMovement);
    tabifyDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetProperties);

    //m_ui->dockWidgetLayerButtons->show();
    m_ui->dockWidgetLayers->show();
    m_ui->dockWidgetProperties->show();
    m_ui->dockWidgetOperations->show();

    // initialize layers Tree Widget
    m_ui->tableWidgetLayers->setColumnWidth(0, 45);
    m_ui->tableWidgetLayers->setColumnWidth(1, 30);
    m_ui->tableWidgetLayers->setColumnWidth(2, 75);
    m_ui->tableWidgetLayers->setColumnWidth(3, 45);
    m_ui->tableWidgetLayers->setColumnWidth(4, 75);
    m_ui->tableWidgetLayers->setColumnWidth(5, 30);
    m_ui->tableWidgetLayers->setColumnWidth(6, 30);

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
    m_ui->actionPause->setCheckable(true);
    m_ui->toolButtonStop->setDefaultAction(m_ui->actionStop);
    m_ui->toolButtonSpotShot->setDefaultAction(m_ui->actionLaserSpotShot);
    m_ui->actionLaserSpotShot->setCheckable(true);
    m_ui->toolButtonCut->setDefaultAction(m_ui->actionLaserCut);
    m_ui->toolButtonMoveToOrigin->setDefaultAction(m_ui->actionMoveToOrigin);
    m_ui->toolButtonMoveLeft->setDefaultAction(m_ui->actionMoveLeft);
    m_ui->toolButtonMoveRight->setDefaultAction(m_ui->actionMoveRight);
    m_ui->toolButtonMoveTop->setDefaultAction(m_ui->actionMoveTop);
    m_ui->toolButtonMoveBottom->setDefaultAction(m_ui->actionMoveBottom);
    m_ui->toolButtonMoveTopLeft->setDefaultAction(m_ui->actionMoveTopLeft);
    m_ui->toolButtonMoveTopRight->setDefaultAction(m_ui->actionMoveTopRight);
    m_ui->toolButtonMoveBottomLeft->setDefaultAction(m_ui->actionMoveBottomLeft);
    m_ui->toolButtonMoveBottomRight->setDefaultAction(m_ui->actionMoveBottomRight);
    m_ui->toolButtonMoveUp->setDefaultAction(m_ui->actionMoveUp);
    m_ui->toolButtonMoveDown->setDefaultAction(m_ui->actionMoveDown);
    
    m_ui->toolButtonMoveLayerUp->setDefaultAction(m_ui->actionMoveLayerUp);
    m_ui->toolButtonMoveLayerDown->setDefaultAction(m_ui->actionMoveLayerDown);

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

    m_ui->tableWidgetParameters->setColumnWidth(0, 100);
    m_ui->tableWidgetParameters->setColumnWidth(1, 300);
	//设置为可选择
	m_ui->actionRectangleTool->setCheckable(true);

    connect(m_ui->actionImportSVG, &QAction::triggered, this, &LaserControllerWindow::onActionImportSVG);
    connect(m_ui->actionImportCorelDraw, &QAction::triggered, this, &LaserControllerWindow::onActionImportCorelDraw);
    connect(m_ui->actionRemoveLayer, &QAction::triggered, this, &LaserControllerWindow::onActionRemoveLayer);
    connect(m_ui->actionExportJSON, &QAction::triggered, this, &LaserControllerWindow::onActionExportJson);
    connect(m_ui->actionLoadJson, &QAction::triggered, this, &LaserControllerWindow::onActionLoadJson);
    connect(m_ui->actionMachining, &QAction::triggered, this, &LaserControllerWindow::onActionMachining);
    connect(m_ui->actionPause, &QAction::triggered, this, &LaserControllerWindow::onActionPauseMechining);
    connect(m_ui->actionStop, &QAction::triggered, this, &LaserControllerWindow::onActionStopMechining);
    connect(m_ui->actionLaserSpotShot, &QAction::triggered, this, &LaserControllerWindow::onActionLaserSpotShot);
    connect(m_ui->actionLaserCut, &QAction::triggered, this, &LaserControllerWindow::onActionLaserCut);
    connect(m_ui->actionLaserMove, &QAction::triggered, this, &LaserControllerWindow::onActionLaserMove);

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
    connect(m_ui->actionMoveLayerUp, &QAction::triggered, this, &LaserControllerWindow::onActionMoveLayerUp);
    connect(m_ui->actionMoveLayerDown, &QAction::triggered, this, &LaserControllerWindow::onActionMoveLayerDown);
    connect(m_ui->actionShowRegisters, &QAction::triggered, this, &LaserControllerWindow::onActionShowRegisters);

	connect(m_ui->actionRectangleTool, &QAction::triggered, this, &LaserControllerWindow::onActionRectangle);

    connect(m_ui->tableWidgetLayers, &QTableWidget::cellDoubleClicked, this, &LaserControllerWindow::onTableWidgetLayersCellDoubleClicked);
    connect(m_ui->tableWidgetLayers, &QTableWidget::itemSelectionChanged, this, &LaserControllerWindow::onTableWidgetItemSelectionChanged);

    connect(m_scene, &LaserScene::selectionChanged, this, &LaserControllerWindow::onLaserSceneSelectedChanged);
    connect(m_viewer, &LaserViewer::mouseMoved, this, &LaserControllerWindow::onLaserViewerMouseMoved);

    connect(&LaserDriver::instance(), &LaserDriver::comPortsFetched, this, &LaserControllerWindow::onDriverComPortsFetched);
    connect(&LaserDriver::instance(), &LaserDriver::comPortConnected, this, &LaserControllerWindow::onDriverComPortConnected);
    connect(&LaserDriver::instance(), &LaserDriver::comPortDisconnected, this, &LaserControllerWindow::onDriverComPortDisconnected);
    //connect(&LaserDriver::instance(), &LaserDriver::machiningStarted, this, &LaserControllerWindow::onStarted);
    //connect(&LaserDriver::instance(), &LaserDriver::machiningPaused, this, &LaserControllerWindow::onPaused);
    //connect(&LaserDriver::instance(), &LaserDriver::downloading, this, &LaserControllerWindow::onDownloading);
    //connect(&LaserDriver::instance(), &LaserDriver::downloaded, this, &LaserControllerWindow::onDownloaded);
    //connect(&LaserDriver::instance(), &LaserDriver::machiningStopped, this, &LaserControllerWindow::onStopped);
    //connect(&LaserDriver::instance(), &LaserDriver::machiningCompleted, this, &LaserControllerWindow::onCompleted);

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

    //m_ui->actionShowRegisters->setDisabled(true);
}

LaserControllerWindow::~LaserControllerWindow()
{
    //m_ui->tableWidgetLayers->clearSelection();
}

void LaserControllerWindow::onActionImportSVG(bool checked)
{
    QString filename = getFilename(tr("Open SVG File"), QStringList() << "image/svg+xml" << "image/svg+xml-compressed");
    if (filename.isEmpty())
        return;
    QSharedPointer<Importer> importer = Importer::getImporter(this, Importer::SVG);
    LaserDocument* doc = importer->import(filename, m_scene);
    if (doc)
    {
        doc->bindLayerButtons(m_layerButtons);
        m_scene->updateDocument(doc);
        m_ui->tableWidgetLayers->setDocument(doc);
        m_ui->tableWidgetLayers->updateItems();
    }
}

void LaserControllerWindow::onActionImportCorelDraw(bool checked)
{
    QSharedPointer<Importer> importer = Importer::getImporter(this, Importer::CORELDRAW);
    QVariantMap params;
    params["parent_winid"] = winId();
    params["parent_win"] = QVariant::fromValue<QMainWindow*>(this);
    LaserDocument* doc = importer->import("", m_scene, params);
    if (doc)
    {
        doc->bindLayerButtons(m_layerButtons);
        m_scene->updateDocument(doc);
        m_ui->tableWidgetLayers->setDocument(doc);
        m_ui->tableWidgetLayers->updateItems();
    }
}

void LaserControllerWindow::onActionRemoveLayer(bool checked)
{
    qDebug() << "removing layer.";
    QTableWidgetItem* item = m_ui->tableWidgetLayers->currentItem();
}

void LaserControllerWindow::onTableWidgetLayersCellDoubleClicked(int row, int column)
{
    QTableWidgetItem* item = m_ui->tableWidgetLayers->item(row, 0);
    int index = item->data(Qt::UserRole).toInt();

    LaserLayer* layer = m_scene->document()->layers()[index];

    LaserLayerDialog dialog(layer);
    if (dialog.exec() == QDialog::Accepted)
    {
        m_ui->tableWidgetLayers->updateItems();
    }
}

void LaserControllerWindow::onTableWidgetItemSelectionChanged()
{
    QList<QTableWidgetItem*> items = m_ui->tableWidgetLayers->selectedItems();
    if (items.isEmpty())
        return;

    int row = items[0]->row();
    QTableWidgetItem* item = m_ui->tableWidgetLayers->item(row, 0);
    int index = item->data(Qt::UserRole).toInt();

    LaserLayer* layer = m_scene->document()->layers()[index];
    m_scene->blockSignals(true);
    m_scene->clearSelection();
    for (LaserPrimitive* primitive : layer->items())
    {
        primitive->setSelected(true);
    }
    m_scene->blockSignals(false);
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
        //MachiningTask* task = LaserDriver::instance().createMachiningTask(m_currentJson, false);
        //task->start();
        LaserDriver::instance().loadDataFromFile(m_currentJson);
    }
    else
    {
        if (m_scene->document() == nullptr)
        {
            QMessageBox::warning(this, tr("Alert"), tr("No active document. Please open or import a document to mechining"));
            return;
        }
        QString filename = "export.json";
        //QTemporaryFile file;
        //if (file.open())
        //{
            //QString filename = file.fileName();
        qDebug() << "exporting to temporary json file:" << filename;
        m_scene->document()->exportJSON(filename);
        qDebug() << "export temp json file for machining" << filename;
        LaserDriver::instance().loadDataFromFile(filename);
        //MachiningTask* task = LaserDriver::instance().createMachiningTask(filename, false);
        //task->start();
        //}
    }
    m_useLoadedJson = false;
}

void LaserControllerWindow::onActionPauseMechining(bool checked)
{
    //LaserDriver::instance().pauseContinueMachining(!checked);
    int result = LaserDriver::instance().pauseContinueMachining(!checked);
    qDebug() << "pause result:" << result << ", checked state:" << checked;
    m_ui->actionPause->blockSignals(true);
    if (result == 1)
    {
        m_ui->actionPause->setChecked(true);
    }
    else
    {
        m_ui->actionPause->setChecked(false);
    }
    m_ui->actionPause->blockSignals(false);
}

void LaserControllerWindow::onActionStopMechining(bool checked)
{
    LaserDriver::instance().stopMachining();
    m_ui->actionPause->setChecked(false);
}

void LaserControllerWindow::onActionLaserSpotShot(bool checked)
{
    int result = LaserDriver::instance().testLaserLight(checked);
    if (result == 5)
    {
        qDebug() << "Light on.";
        m_ui->actionLaserSpotShot->setChecked(true);
    }
    else if (result == 6)
    {
        qDebug() << "Light off.";
        m_ui->actionLaserSpotShot->setChecked(false);
    }
}

void LaserControllerWindow::onActionLaserCut(bool checked)
{
}

void LaserControllerWindow::onActionLaserMove(bool checked)
{
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
            
            imageUtils::halftone3(resized, dialog.lpi(), dialog.dpi(), dialog.degrees());
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

void LaserControllerWindow::onActionShowRegisters(bool checked)
{
    ParameterDialog dialog;
    dialog.exec();
}

void LaserControllerWindow::onActionHome(bool checked)
{
    ParameterDialog dialog;
    dialog.exec();
}

void LaserControllerWindow::onActionRectangle(bool checked)
{
	if (checked) {
		m_ui->actionRectangleTool->setEnabled(false);
		//StateController::
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

void LaserControllerWindow::onActionMoveLayerUp(bool checked)
{
    QList<QTableWidgetItem*> selectedItems = m_ui->tableWidgetLayers->selectedItems();
    if (selectedItems.isEmpty())
        return;

    int row = selectedItems[0]->row();
    if (row == 0)
        return;

    QTableWidgetItem* current = m_ui->tableWidgetLayers->item(row, 0);
    QTableWidgetItem* target = m_ui->tableWidgetLayers->item(row - 1, 0);
    m_scene->document()->swapLayers(target->data(Qt::UserRole).toInt(), current->data(Qt::UserRole).toInt());
    m_ui->tableWidgetLayers->selectRow(row - 1);
}

void LaserControllerWindow::onActionMoveLayerDown(bool checked)
{
    QList<QTableWidgetItem*> selectedItems = m_ui->tableWidgetLayers->selectedItems();
    if (selectedItems.isEmpty())
        return;

    int row = selectedItems[0]->row();
    if (row >= m_ui->tableWidgetLayers->rowCount() - 1)
        return;

    QTableWidgetItem* current = m_ui->tableWidgetLayers->item(row, 0);
    QTableWidgetItem* target = m_ui->tableWidgetLayers->item(row + 1, 0);
    m_scene->document()->swapLayers(target->data(Qt::UserRole).toInt(), current->data(Qt::UserRole).toInt());
    m_ui->tableWidgetLayers->selectRow(row + 1);
}

void LaserControllerWindow::onLaserSceneSelectedChanged()
{
    QList<LaserPrimitive*> items = m_scene->selectedPrimitives();
    if (items.isEmpty())
        return;

    m_ui->tableWidgetLayers->blockSignals(true);
    for (LaserPrimitive* item : items)
    {
        int row = items[0]->layer()->row();
        if (row != -1)
            m_ui->tableWidgetLayers->selectRow(row);
    }
    m_ui->tableWidgetLayers->blockSignals(false);

    if (items.count() == 1)
    {
        LaserPrimitive* item = items[0];
        PropertiesHelperManager::primitivePropertiesHelper().resetProperties(item, m_ui->tableWidgetParameters);
    }
}

void LaserControllerWindow::onLaserViewerMouseMoved(const QPointF & pos)
{
    QString posStr = QString("%1mm,%2mm").arg(pos.x()).arg(pos.y());
    m_statusBarLocation->setText(posStr);
}

//void LaserControllerWindow::updateLayerButtons()
//{
//    if (!m_scene->document())
//        return;
//
//    QList<LaserLayer*> layers = m_scene->document()->layers();
//    for (int i = 0; i < layers.count(); i++)
//    {
//        m_layerButtons[i]->setUserData(0, layers[i]);
//        layers[i]->setColor(m_layerButtons[i]->palette().color(QPalette::Button));
//    }
//}

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
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, devicePausedState);
    // end actionMachining

    // actionPause
    /*BIND_PROP_TO_STATE(m_ui->actionPause, "setChecked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "setChecked", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "setChecked", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "setChecked", false, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "setChecked", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "setChecked", true, devicePausedState);*/
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, devicePausedState);
    // end actionPause

    // actionStop
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, devicePausedState);
    // end actionStop

    // actionLaserSpotShot
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, devicePausedState);
    // end actionLaserSpotShot

    // actionLaserCut
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", true, devicePausedState);
    // end actionLaserCut

    // actionLaserMove
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, devicePausedState);
    // end actionLaserMove

	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentWorkingState);


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


