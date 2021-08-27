#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"
#include "widget/UndoCommand.h"

#include <DockAreaTabBar.h>
#include <DockAreaTitleBar.h>
#include <DockAreaWidget.h>
#include <DockComponentsFactory.h>
#include <QFileDialog> 
#include <FloatingDockContainer.h>
#include <QCheckBox>
#include <QComboBox>
#include <QErrorMessage>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStack>
#include <QtMath>
#include <QRadioButton>
#include <QScrollBar>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidgetItem>
#include <QUndoStack>
#include <QWindow>

#include "LaserApplication.h"
#include "algorithm/OptimizeNode.h"
#include "common/common.h"
#include "common/Config.h"
#include "import/Importer.h"
#include "laser/LaserDevice.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserPrimitive.h"
#include "scene/LaserPrimitiveGroup.h"
#include "scene/LaserScene.h"
#include "state/StateController.h"
#include "ui/ConfigDialog.h"
#include "ui/HalftoneDialog.h"
#include "ui/LaserLayerDialog.h"
#include "ui/MainCardInfoDialog.h"
#include "ui/RegistersDialog.h"
#include "util/ImageUtils.h"
#include "util/Utils.h"
#include "widget/FloatEditDualSlider.h"
#include "widget/FloatEditSlider.h"
#include "widget/LaserLayerTableWidget.h"
#include "widget/LaserViewer.h"
#include "widget/LayerButton.h"
#include "widget/PropertiesHelperManager.h"
#include "widget/RulerWidget.h"

using namespace ads;

LaserControllerWindow::LaserControllerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::LaserControllerWindow)
    , m_created(false)
    , m_useLoadedJson(false)
	, m_unitIsMM(true)
	, m_selectionOriginalState(SelectionOriginalCenter)
	, m_windowTitle("Laser Controller")
{
    m_ui->setupUi(this);

    // initialize Dock Manager
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    m_dockManager = new CDockManager(this);

    createCentralDockPanel();
    createLayersDockPanel();
    createCameraDockPanel();
    createOperationsDockPanel();
    createOutlineDockPanel();
    createMovementDockPanel();

    m_dockAreaLayers->setCurrentIndex(0);
    
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

    int colorTick = 360 / Config::Layers::maxLayersCount();
    for (int i = 0; i < Config::Layers::maxLayersCount(); i++)
    {
        LayerButton* button = new LayerButton(m_viewer);
        button->setMinimumWidth(Config::Ui::colorButtonWidth());
        button->setFixedHeight(Config::Ui::colorButtonHeight());
        button->setColor(colors[i]);
        button->setText(QString(tr("%1")).arg(i + 1, 2, 10, QLatin1Char('0')));
        button->update();
		button->setEnabled(false);
        m_ui->layoutLayerButtons->addWidget(button);
        m_layerButtons.append(button);

        connect(button, &LayerButton::colorUpdated, m_tableWidgetLayers, &LaserLayerTableWidget::updateItems);
    }
    m_ui->layoutLayerButtons->addStretch();

    //removeDockWidget(m_ui->dockWidgetLayerButtons);
    //removeDockWidget(m_ui->dockWidgetLayers);
    //removeDockWidget(m_ui->dockWidgetProperties);
    //removeDockWidget(m_ui->dockWidgetOperations);
	//removeDockWidget(m_ui->dockWidgetOutline);

    //addDockWidget(Qt::RightDockWidgetArea, m_ui->dockWidgetLayerButtons);
    //addDockWidget(Qt::RightDockWidgetArea, m_ui->dockWidgetLayers);
    //splitDockWidget(m_ui->dockWidgetLayerButtons, m_ui->dockWidgetLayers, Qt::Horizontal);
    //splitDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetLayerButtons, Qt::Horizontal);
    //splitDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetOperations, Qt::Vertical);

    //tabifyDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetMovement);
    //tabifyDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetProperties);
    //tabifyDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetOutline);

    //m_ui->dockWidgetLayerButtons->show();
    //m_ui->dockWidgetLayers->show();
    //m_ui->dockWidgetProperties->show();
    //m_ui->dockWidgetOperations->show();
    //m_ui->dockWidgetOutline->show();

    // set up tools buttons
    QToolButton* toolButtonSelectionTool = new QToolButton;
    QToolButton* toolButtonRectangleTool = new QToolButton;
    QToolButton* toolButtonEllipseTool = new QToolButton;
    QToolButton* toolButtonPolygonTool = new QToolButton;
    QToolButton* toolButtonTextTool = new QToolButton;
    QToolButton* toolButtonLineTool = new QToolButton;
    QToolButton* toolButtonSplineTool = new QToolButton;
    QToolButton* toolButtonBitmapTool = new QToolButton;
	//QToolButton* toolButtonViewDragTool = new QToolButton;

	
    toolButtonSelectionTool->setDefaultAction(m_ui->actionSelectionTool);
	//toolButtonViewDragTool->setDefaultAction(m_ui->actionDragView);
    toolButtonRectangleTool->setDefaultAction(m_ui->actionRectangleTool);
    toolButtonEllipseTool->setDefaultAction(m_ui->actionEllipseTool);
    toolButtonPolygonTool->setDefaultAction(m_ui->actionPolygonTool);
    toolButtonTextTool->setDefaultAction(m_ui->actionTextTool);
    toolButtonLineTool->setDefaultAction(m_ui->actionLineTool);
    toolButtonSplineTool->setDefaultAction(m_ui->actionSplineTool);
	toolButtonSplineTool->addAction(m_ui->actionEditSplineTool);
    toolButtonBitmapTool->setDefaultAction(m_ui->actionBitmapTool);

    m_ui->toolBarTools->addWidget(toolButtonSelectionTool);
    m_ui->toolBarTools->addWidget(toolButtonRectangleTool);
    m_ui->toolBarTools->addWidget(toolButtonEllipseTool);
    m_ui->toolBarTools->addWidget(toolButtonPolygonTool);
    m_ui->toolBarTools->addWidget(toolButtonTextTool);
    m_ui->toolBarTools->addWidget(toolButtonLineTool);
    m_ui->toolBarTools->addWidget(toolButtonSplineTool);
    m_ui->toolBarTools->addWidget(toolButtonBitmapTool);
	//m_ui->toolBarTools->addWidget(toolButtonViewDragTool);

    // init status bar
    m_statusBarStatus = new QLabel;
    m_statusBarStatus->setText(tr("Tips"));
    m_statusBarStatus->setMinimumWidth(60);
    m_statusBarStatus->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarStatus);
    m_ui->statusbar->addWidget(utils::createSeparator());

    m_statusBarRegister = new QLabel;
    m_statusBarRegister->setText(tr("Unregistered"));
    m_statusBarRegister->setMinimumWidth(60);
    m_statusBarRegister->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarRegister);
    m_ui->statusbar->addWidget(utils::createSeparator());

    m_statusBarActivation = new QLabel;
    m_statusBarActivation->setText(tr("Inactivated"));
    m_statusBarActivation->setMinimumWidth(60);
    m_statusBarActivation->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarActivation);
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

	//设置为可选择
    m_ui->actionSelectionTool->setCheckable(true);
	m_ui->actionRectangleTool->setCheckable(true);
	m_ui->actionEllipseTool->setCheckable(true);
	m_ui->actionLineTool->setCheckable(true);
	m_ui->actionPolygonTool->setCheckable(true);
	m_ui->actionSplineTool->setCheckable(true);
	m_ui->actionEditSplineTool->setCheckable(true);
	m_ui->actionTextTool->setCheckable(true);
	m_ui->actionDragView->setCheckable(true);
	//init selected items properties
	m_propertyLayout = new QGridLayout(m_ui->properties);
	m_propertyLayout->setMargin(0);
	m_propertyLayout->setSpacing(3);
	//posx, posy, lock, unlcok
	m_posXLabel = new QLabel("XPos");
	m_posYLabel = new QLabel("YPos");
	m_posXBox = new LaserDoubleSpinBox();
	m_posXBox->setMinimum(-DBL_MAX);
	m_posXBox->setMaximum(DBL_MAX);
	m_posYBox = new LaserDoubleSpinBox();
	m_posYBox->setMinimum(-DBL_MAX);
	m_posYBox->setMaximum(DBL_MAX);
	m_posXBox->setDecimals(3);
	m_posYBox->setDecimals(3);
	m_posXUnit = new QLabel("mm");
	m_posYUnit = new QLabel("mm");
	m_lockOrUnlock = new QToolButton();
	m_lockOrUnlock->setDefaultAction(m_ui->actionLock);
	m_lockOrUnlock->setEnabled(false);

	m_propertyLayout->addWidget(m_posXLabel, 0, 0);
	m_propertyLayout->addWidget(m_posYLabel, 1, 0);
	m_propertyLayout->addWidget(m_posXBox, 0, 1);
	m_propertyLayout->addWidget(m_posYBox, 1, 1);
	m_propertyLayout->addWidget(m_posXUnit, 0, 2);
	m_propertyLayout->addWidget(m_posYUnit, 1, 2);
	m_propertyLayout->addWidget(m_lockOrUnlock, 0, 3, 2, 1);

	//width, height
	QLabel* widthLabel = new QLabel("Width");
	QLabel* heightLabel = new QLabel("Height");
	m_widthBox = new LaserDoubleSpinBox();
	m_widthBox->setMinimum(DBL_MIN);
	m_widthBox->setMaximum(DBL_MAX);
	m_heightBox = new LaserDoubleSpinBox();
	m_heightBox->setMinimum(DBL_MIN);
	m_heightBox->setMaximum(DBL_MAX);
	m_heightBox->setDecimals(3);
	//m_heightBox->setDecimals(DBL_MAX_10_EXP);
	m_widthUnit = new QLabel("mm");
	m_heightUnit = new QLabel("mm");

	m_propertyLayout->addWidget(widthLabel, 0, 4);
	m_propertyLayout->addWidget(heightLabel, 1, 4);
	m_propertyLayout->addWidget(m_widthBox, 0, 5);
	m_propertyLayout->addWidget(m_heightBox, 1, 5);
	m_propertyLayout->addWidget(m_widthUnit, 0, 6);
	m_propertyLayout->addWidget(m_heightUnit, 1, 6);

	//rate
	m_xRateBox = new LaserDoubleSpinBox();
	m_yRateBox = new LaserDoubleSpinBox();

	QLabel* xRateLabel = new QLabel("%");
	QLabel* yRateLabel = new QLabel("%");
	
	m_propertyLayout->addWidget(m_xRateBox, 0, 7);
	m_propertyLayout->addWidget(m_yRateBox, 1, 7);
	m_propertyLayout->addWidget(xRateLabel, 0, 8);
	m_propertyLayout->addWidget(yRateLabel, 1, 8);

	m_xRateBox->setMaximum(DBL_MAX);
	m_xRateBox->setMinimum(DBL_MIN);
	m_yRateBox->setMaximum(DBL_MAX);
	m_yRateBox->setMinimum(DBL_MIN);
	m_xRateBox->setDecimals(3);
	m_yRateBox->setDecimals(3);
	m_xRateBox->setValue(100);
	m_yRateBox->setValue(100);
	//rotate
	QGridLayout*  rotateLayout = new QGridLayout();
	rotateLayout->setMargin(0);
	rotateLayout->setSpacing(0);
	rotateLayout->setAlignment(Qt::AlignCenter);
	m_topLeftBtn = new QRadioButton();
	m_topCenterBtn = new QRadioButton();
	m_topRightBtn = new QRadioButton();
	m_leftCenterBtn = new QRadioButton();
	m_centerBtn = new QRadioButton();
	m_rightCenterBtn = new QRadioButton();
	m_bottomLeftBtn = new QRadioButton();
	m_bottomCenterBtn = new QRadioButton();
	m_bottomRightBtn = new QRadioButton();

	rotateLayout->addWidget(m_topLeftBtn, 0, 0);
	rotateLayout->addWidget(m_topCenterBtn, 0, 1);
	rotateLayout->addWidget(m_topRightBtn, 0, 2);
	rotateLayout->addWidget(m_leftCenterBtn, 1, 0);
	rotateLayout->addWidget(m_centerBtn, 1, 1);
	rotateLayout->addWidget(m_rightCenterBtn, 1, 2);
	rotateLayout->addWidget(m_bottomLeftBtn, 2, 0);
	rotateLayout->addWidget(m_bottomCenterBtn, 2, 1);
	rotateLayout->addWidget(m_bottomRightBtn, 2, 2);
	QWidget* rotateWidget = new QWidget();
	rotateWidget->setStyleSheet(".QWidget{border-radius:2px;border:0.5px solid #d2d2d2;}");
	rotateWidget->setLayout(rotateLayout);
	m_propertyLayout->addWidget(rotateWidget,0, 9, 2, 2);
	//rotate
	QLabel* rotateLabel = new QLabel("Rotate");
	m_rotateBox = new LaserDoubleSpinBox();
	m_rotateBox->setMinimum(-360.0);
	m_rotateBox->setMaximum(360.0);
	m_rotateBox->setDecimals(1);
	m_mmOrIn = new QToolButton();
	m_ui->actionUnitChange->setText("mm");
	m_mmOrIn->setDefaultAction(m_ui->actionUnitChange);
	m_mmOrIn->setEnabled(false);

	m_propertyLayout->addWidget(rotateLabel, 0, 11, 2, 1);
	m_propertyLayout->addWidget(m_rotateBox, 0, 12, 2, 1);
	m_propertyLayout->addWidget(m_mmOrIn, 0, 13, 2, 1);

	m_propertyWidget = new QWidget();
	m_propertyWidget->setLayout(m_propertyLayout);
	m_ui->properties->addWidget(m_propertyWidget);
	m_propertyWidget->setEnabled(false);
	m_centerBtn->setChecked(true);
	//arrange
	/*m_mirrorHWidget = new QWidget();
	m_mirrorVWidget = new QWidget();
	m_ui->actionMirrorHorizontal->addWidget(m_mirrorHWidget);
	m_ui->actionMirrorVertical->addWidget(m_mirrorVWidget);*/
	
	connect(m_viewer->undoStack(), &QUndoStack::canUndoChanged,this, &LaserControllerWindow::onCanUndoChanged);
	connect(m_viewer->undoStack(), &QUndoStack::canRedoChanged, this, &LaserControllerWindow::onCanRedoChanged);
	connect(m_ui->actionUndo, &QAction::triggered, this, &LaserControllerWindow::onActionUndo);
	connect(m_ui->actionRedo, &QAction::triggered, this, &LaserControllerWindow::onActionRedo);
	//connect(m_ui->actionRedo, &QAction::triggered, this, &LaserControllerWindow::m_redoAction);
    connect(m_ui->actionImport, &QAction::triggered, this, &LaserControllerWindow::onActionImport);
    connect(m_ui->actionImportCorelDraw, &QAction::triggered, this, &LaserControllerWindow::onActionImportCorelDraw);
    connect(m_ui->actionRemoveLayer, &QAction::triggered, this, &LaserControllerWindow::onActionRemoveLayer);
    connect(m_ui->actionExportJSON, &QAction::triggered, this, &LaserControllerWindow::onActionExportJson);
    connect(m_ui->actionLoadJson, &QAction::triggered, this, &LaserControllerWindow::onActionLoadJson);
    connect(m_ui->actionMachining, &QAction::triggered, this, &LaserControllerWindow::onActionMachining);
    connect(m_ui->actionPause, &QAction::triggered, this, &LaserControllerWindow::onActionPauseMechining);
    connect(m_ui->actionStop, &QAction::triggered, this, &LaserControllerWindow::onActionStopMechining);
    connect(m_ui->actionLaserSpotShot, &QAction::triggered, this, &LaserControllerWindow::onActionLaserSpotShot);
    connect(m_ui->actionBounding, &QAction::triggered, this, &LaserControllerWindow::onActionLaserCut);
    connect(m_ui->actionLaserMove, &QAction::triggered, this, &LaserControllerWindow::onActionLaserMove);
	connect(m_ui->actionNew, &QAction::triggered, this, &LaserControllerWindow::onActionNew);
	connect(m_ui->actionSave, &QAction::triggered, this, &LaserControllerWindow::onActionSave);
	connect(m_ui->actionSaveAs, &QAction::triggered, this, &LaserControllerWindow::onActionSaveAs);
	connect(m_ui->actionOpen, &QAction::triggered, this, &LaserControllerWindow::onActionOpen);
	connect(m_ui->actionZoomIn, &QAction::triggered, this, &LaserControllerWindow::onActionZoomIn);
	connect(m_ui->actionZoomOut, &QAction::triggered, this, &LaserControllerWindow::onActionZoomOut);
	connect(m_ui->actionZoomToPage, &QAction::triggered, this, &LaserControllerWindow::onActionZoomToPage);
	connect(m_ui->actionZoomToSelection, &QAction::triggered, this, &LaserControllerWindow::onActionZoomToSelection);

    connect(m_ui->actionConnect, &QAction::triggered, this, &LaserControllerWindow::onActionConnect);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &LaserControllerWindow::onActionDisconnect);
    connect(m_ui->actionLoadMotor, &QAction::triggered, this, &LaserControllerWindow::onActionLoadMotor);
    connect(m_ui->actionUnloadMotor, &QAction::triggered, this, &LaserControllerWindow::onActionUnloadMotor);
    connect(m_ui->actionDownload, &QAction::triggered, this, &LaserControllerWindow::onActionDownload);
    connect(m_ui->actionWorkState, &QAction::triggered, this, &LaserControllerWindow::onActionWorkState);
    connect(m_ui->actionHalfTone, &QAction::triggered, this, &LaserControllerWindow::onActionHalfTone);
    connect(m_ui->actionDeletePrimitive, &QAction::triggered, this, &LaserControllerWindow::onActionDeletePrimitive);
	connect(m_ui->actionCopy, &QAction::triggered, this, &LaserControllerWindow::onActionCopy);
	connect(m_ui->actionPaste, &QAction::triggered, this, &LaserControllerWindow::onActionPaste);
	connect(m_ui->actionPasteInLine, &QAction::triggered, this, &LaserControllerWindow::onActionPasteInLine);
	connect(m_ui->actionCut, &QAction::triggered, this, &LaserControllerWindow::onActionCut);
	connect(m_ui->actionDuplication, &QAction::triggered, this, &LaserControllerWindow::onActionDuplication);
	connect(m_ui->actionGroup, &QAction::triggered, this, &LaserControllerWindow::onActionGroup);
	connect(m_ui->actionUngroup, &QAction::triggered, this, &LaserControllerWindow::onActionUngroup);
	
    connect(m_ui->actionCloseDocument, &QAction::triggered, this, &LaserControllerWindow::onActionCloseDocument);
    connect(m_ui->actionMoveLayerUp, &QAction::triggered, this, &LaserControllerWindow::onActionMoveLayerUp);
    connect(m_ui->actionMoveLayerDown, &QAction::triggered, this, &LaserControllerWindow::onActionMoveLayerDown);
    connect(m_ui->actionSettings, &QAction::triggered, this, &LaserControllerWindow::onActionSettings);
    connect(m_ui->actionPathOptimization, &QAction::triggered, this, &LaserControllerWindow::onActionPathOptimization);

	connect(m_ui->actionMirrorHorizontal, &QAction::triggered, this, &LaserControllerWindow::onActionMirrorHorizontal);
	connect(m_ui->actionMirrorVertical, &QAction::triggered, this, &LaserControllerWindow::onActionMirrorVertical);

    connect(m_ui->actionMoveTop, &QAction::triggered, this, &LaserControllerWindow::onActionMoveTop);
    connect(m_ui->actionMoveBottom, &QAction::triggered, this, &LaserControllerWindow::onActionMoveBottom);
    connect(m_ui->actionMoveLeft, &QAction::triggered, this, &LaserControllerWindow::onActionMoveLeft);
    connect(m_ui->actionMoveRight, &QAction::triggered, this, &LaserControllerWindow::onActionMoveRight);
    connect(m_ui->actionMoveTopLeft, &QAction::triggered, this, &LaserControllerWindow::onActionMoveTopLeft);
    connect(m_ui->actionMoveTopRight, &QAction::triggered, this, &LaserControllerWindow::onActionMoveTopRight);
    connect(m_ui->actionMoveBottomLeft, &QAction::triggered, this, &LaserControllerWindow::onActionMoveBottomLeft);
    connect(m_ui->actionMoveBottomRight, &QAction::triggered, this, &LaserControllerWindow::onActionMoveBottomRight);
    connect(m_ui->actionMoveUp, &QAction::triggered, this, &LaserControllerWindow::onActionMoveUp);
    connect(m_ui->actionMoveDown, &QAction::triggered, this, &LaserControllerWindow::onActionMoveDown);

	connect(m_ui->actionLineTool, &QAction::triggered, this, &LaserControllerWindow::onActionLine);
	connect(m_ui->actionRectangleTool, &QAction::triggered, this, &LaserControllerWindow::onActionRectangle);
	connect(m_ui->actionEllipseTool, &QAction::triggered, this, &LaserControllerWindow::onActionEllipse);
	connect(m_ui->actionSelectionTool, &QAction::triggered, this, &LaserControllerWindow::onActionSelectionTool);
	connect(m_ui->actionDragView, &QAction::triggered, this, &LaserControllerWindow::onActionViewDrag);
	connect(m_ui->actionPolygonTool, &QAction::triggered, this, &LaserControllerWindow::onActionPolygon);
	connect(m_ui->actionSplineTool, &QAction::triggered, this, &LaserControllerWindow::onActionSpline);
	connect(m_ui->actionEditSplineTool, &QAction::triggered, this, &LaserControllerWindow::onActionSplineEdit);
	connect(m_ui->actionTextTool, &QAction::triggered, this, &LaserControllerWindow::onActionText);
	connect(m_ui->actionBitmapTool, &QAction::triggered, this, &LaserControllerWindow::onActionBitmap);
    connect(m_ui->actionUpdate, &QAction::triggered, this, &LaserControllerWindow::onActionUpdate);

	connect(m_ui->actionShowMainCardInfo, &QAction::triggered, this, &LaserControllerWindow::onActionShowMainCardInfo);
	connect(m_ui->actionTemporaryLicense, &QAction::triggered, this, &LaserControllerWindow::onActionTemporaryLicense);
	connect(m_ui->actionAbout, &QAction::triggered, this, &LaserControllerWindow::onActionAbout);

    connect(m_ui->actionReset, &QAction::triggered, this, &LaserControllerWindow::laserResetToOriginalPoint);
    connect(m_ui->actionMoveToOrigin, &QAction::triggered, this, &LaserControllerWindow::laserBackToMachiningOriginalPoint);

    connect(m_ui->actionUpdateOutline, &QAction::triggered, this, &LaserControllerWindow::onActionUpdateOutline);

    connect(m_scene, &LaserScene::selectionChanged, this, &LaserControllerWindow::onLaserSceneSelectedChanged);
    connect(m_viewer, &LaserViewer::mouseMoved, this, &LaserControllerWindow::onLaserViewerMouseMoved);
    connect(m_viewer, &LaserViewer::scaleChanged, this, &LaserControllerWindow::onLaserViewerScaleChanged);
    connect(m_comboBoxScale, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LaserControllerWindow::onComboBoxSxaleIndexChanged);
    connect(m_comboBoxScale, &QComboBox::currentTextChanged, this, &LaserControllerWindow::onComboBoxSxaleTextChanged);

    connect(LaserApplication::device, &LaserDevice::comPortsFetched, this, &LaserControllerWindow::onDeviceComPortsFetched);
    connect(LaserApplication::device, &LaserDevice::connected, this, &LaserControllerWindow::onDeviceConnected);
    connect(LaserApplication::device, &LaserDevice::disconnected, this, &LaserControllerWindow::onDeviceDisconnected);
    connect(LaserApplication::device, &LaserDevice::mainCardRegistered, this, &LaserControllerWindow::onMainCardRegistered);
    connect(LaserApplication::device, &LaserDevice::mainCardActivated, this, &LaserControllerWindow::onMainCardActivated);
    connect(&LaserDriver::instance(), &LaserDriver::workStateUpdated, this, &LaserControllerWindow::onLaserReturnWorkState);

    //connect(this, &LaserControllerWindow::windowCreated, this, &LaserControllerWindow::onWindowCreated);
    connect(StateController::instance().deviceUnconnectedState(), &QState::entered, this, &LaserControllerWindow::onEnterDeviceUnconnectedState);
    connect(StateController::instance().deviceConnectedState(), &QState::entered, this, &LaserControllerWindow::onEnterDeviceConnectedState);
	connect(StateController::instance().documentPrimitiveSplineState(), &QState::exited, this, &LaserControllerWindow::onCreatSpline);
	connect(StateController::instance().documentIdleState(), &QState::entered, m_viewer, &LaserViewer::onDocumentIdle);
    connect(StateController::instance().documentPrimitiveTextState(), &QState::exited, this, [=] {
        //m_viewer->setAttribute(Qt::WA_InputMethodEnabled, false);
    });
	
	connect(StateController::instance().documentPrimitiveState(), &QState::entered, m_viewer, [=] {
		m_viewer->viewport()->repaint();
	});
	connect(StateController::instance().documentSelectedState(), &QState::entered, m_viewer, [=] {
		m_viewer->viewport()->repaint();
	});
	
	//selected properties
	//connect(m_scene, &LaserScene::selectionChanged,this, &LaserControllerWindow::selectionChange);
	connect(m_viewer, &LaserViewer::selectedChange, this, &LaserControllerWindow::selectedChange);
	connect(m_posXBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_MOVE;
		if (m_posXBox->value() > 9000) {
			m_posXBox->setValue(9000);
		}
		if (m_posXBox->value() < -9000) {
			m_posXBox->setValue(-9000);
		}
		selectionPropertyBoxChange();
	});
	connect(m_posYBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_MOVE;
		if (m_posYBox->value() > 9000) {
			m_posYBox->setValue(9000);
		}
		if (m_posYBox->value() < -9000) {
			m_posYBox->setValue(-9000);
		}
		selectionPropertyBoxChange();
	});
	connect(m_widthBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_RESIZE;
		if (m_widthBox->value() > 20000) {
			m_widthBox->setValue(20000);
		}
		/*if (m_widthBox->value() < -20000) {
			m_widthBox->setValue(-20000);
		}*/
		if (m_widthBox->value() <= 0) {
			m_widthBox->setValue(0.001);
		}
		selectionPropertyBoxChange();
	});
	connect(m_heightBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_RESIZE;
		if (m_heightBox->value() > 20000) {
			m_heightBox->setValue(20000);
		}
		/*if (m_heightBox->value() < -20000) {
			m_heightBox->setValue(-20000);
		}*/
		if (m_heightBox->value() <= 0) {
			m_heightBox->setValue(0.001);
		}
		
		selectionPropertyBoxChange();
	});
	connect(m_xRateBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_SCALE;
		if (m_xRateBox->value() <= 0) {
			m_xRateBox->setValue(0.001);
		}
		selectionPropertyBoxChange();
	});
	connect(m_yRateBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_SCALE;
		if (m_yRateBox->value() <= 0) {
			m_yRateBox->setValue(0.001);
		}
		selectionPropertyBoxChange();
	});
	//rotate
	connect(m_rotateBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_ROTATE;
		selectionPropertyBoxChange();
	});
	//selection original raido button
	connect(m_topLeftBtn, &QRadioButton::toggled, this, [=] {
		if (m_topLeftBtn->isChecked()) {
			m_selectionOriginalState = SelectionOriginalTopLeft;
			selectedChange();
		}
	});
	connect(m_topCenterBtn, &QRadioButton::toggled, this, [=] {
		if (m_topCenterBtn->isChecked()) {
			m_selectionOriginalState = SelectionOriginalTopCenter;
			selectedChange();
		}
	});
	connect(m_topRightBtn, &QRadioButton::toggled, this, [=] {
		if (m_topRightBtn->isChecked()) {
			m_selectionOriginalState = SelectionOriginalTopRight;
			selectedChange();
		}
	});
	connect(m_leftCenterBtn, &QRadioButton::toggled, this, [=] {
		if (m_leftCenterBtn->isChecked()) {
			m_selectionOriginalState = SelectionOriginalLeftCenter;
			selectedChange();
		}
	});
	connect(m_centerBtn, &QRadioButton::toggled, this, [=] {
		if (m_centerBtn->isChecked()) {
			m_selectionOriginalState = SelectionOriginalCenter;
			selectedChange();
		}
	});
	connect(m_rightCenterBtn, &QRadioButton::toggled, this, [=] {
		if (m_rightCenterBtn->isChecked()) {
			m_selectionOriginalState = SelectionOriginalRightCenter;
			selectedChange();
		}
	});
	connect(m_bottomLeftBtn, &QRadioButton::toggled, this, [=] {
		if (m_bottomLeftBtn->isChecked()) {
			m_selectionOriginalState = SelectionOriginalLeftBottom;
			selectedChange();
		}
	});
	connect(m_bottomCenterBtn, &QRadioButton::toggled, this, [=] {
		if (m_bottomCenterBtn->isChecked()) {
			m_selectionOriginalState = SelectionOriginalBottomCenter;
			selectedChange();
		}
	});
	connect(m_bottomRightBtn, &QRadioButton::toggled, this, [=] {
		if (m_bottomRightBtn->isChecked()) {
			m_selectionOriginalState = SelectionOriginalBottomRight;
			selectedChange();
		}
	});
	

    ADD_TRANSITION(initState, workingState, this, SIGNAL(windowCreated()));

    ADD_TRANSITION(documentIdleState, documentPrimitiveRectState, this, SIGNAL(readyRectangle()));
    ADD_TRANSITION(documentSelectionState, documentPrimitiveRectState, this, SIGNAL(readyRectangle()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveRectState, this, SIGNAL(readyRectangle()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveRectState, this, SIGNAL(readyRectangle()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveRectState, this, SIGNAL(readyRectangle()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitiveRectState, this, SIGNAL(readyRectangle()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitiveRectState, this, SIGNAL(readyRectangle()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitiveRectState, this, SIGNAL(readyRectangle()));
	ADD_TRANSITION(documentViewDragState, documentPrimitiveRectState, this, SIGNAL(readyRectangle()));

	ADD_TRANSITION(documentIdleState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentViewDragState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));

	ADD_TRANSITION(documentIdleState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentViewDragState, documentPrimitiveLineState, this, SIGNAL(readyLine()));

	ADD_TRANSITION(documentIdleState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentSelectionState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentViewDragState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));

	ADD_TRANSITION(documentIdleState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentViewDragState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));

	ADD_TRANSITION(documentIdleState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentViewDragState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));

	ADD_TRANSITION(documentIdleState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentViewDragState, documentPrimitiveTextState, this, SIGNAL(readyText()));

	ADD_TRANSITION(documentIdleState, documentViewDragState, this, SIGNAL(readyViewDrag()));
	ADD_TRANSITION(documentSelectionState, documentViewDragState, this, SIGNAL(readyViewDrag()));
	ADD_TRANSITION(documentPrimitiveRectState, documentViewDragState, this, SIGNAL(readyViewDrag()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentViewDragState, this, SIGNAL(readyViewDrag()));
	ADD_TRANSITION(documentPrimitiveLineState, documentViewDragState, this, SIGNAL(readyViewDrag()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentViewDragState, this, SIGNAL(readyViewDrag()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentViewDragState, this, SIGNAL(readyViewDrag()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentViewDragState, this, SIGNAL(readyViewDrag()));

    ADD_TRANSITION(documentPrimitiveState, documentIdleState, this, SIGNAL(isIdle()));
	ADD_TRANSITION(documentViewDragState, documentIdleState, this, SIGNAL(isIdle()));

    bindWidgetsProperties();

    // check tmp folder
    QDir appDir(QCoreApplication::applicationDirPath());
    m_tmpDir = QDir(QCoreApplication::applicationDirPath() + "/tmp");
    if (!m_tmpDir.exists())
    {
        appDir.mkpath("tmp");
    }

    //updatePostEventWidgets(m_ui->comboBoxPostEvent->currentIndex());
    qLogD << "main window initialized";
}

LaserControllerWindow::~LaserControllerWindow()
{
	m_propertyWidget = nullptr;
	if (m_viewer->undoStack()) {
		delete m_viewer->undoStack();
	}
}

void LaserControllerWindow::moveLaser(const QVector3D& delta, bool relative, const QVector3D& abstractDest)
{
    if (!LaserDriver::instance().isConnected())
    {
        QMessageBox::warning(this, tr("Operate failure"), tr("Laser device is not connected!"));
        return;
    }

    QVariant value;

    // Get Quadrant;
    /*if (!LaserDriver::instance().getRegister(LaserDriver::RT_WORKING_QUADRANT, value))
    {
        QMessageBox::warning(this, tr("Operate failure"), tr("Getting register value failure!"));
        return;
    }*/
    QUADRANT quad = static_cast<QUADRANT>(value.toInt());

    // Get registor #5
    /*if (!LaserDriver::instance().getRegister(LaserDriver::RT_MOVE_FAST_SPEED, value))
    {
        QMessageBox::warning(this, tr("Operate failure"), tr("Getting register value failure!"));
        return;
    }*/
    int moveFastSpeed = value.toInt();

    // Get registor #40
    /*if (!LaserDriver::instance().getRegister(LaserDriver::RT_MOVE_FAST_LAUNCHING_SPEED, value))
    {
        QMessageBox::warning(this, tr("Operate failure"), tr("Getting register value failure!"));
        return;
    }*/
    int moveFastLaunchingSpeed = value.toInt();

    // Get layout size
    float layoutWidth = 0;
    float layoutHeight = 0;
    if (!LaserDriver::instance().getLayout(layoutWidth, layoutHeight))
    {
        QMessageBox::warning(this, tr("Operate failure"), tr("Getting register value failure!"));
        return;
    }
    
    // Get current pos;
    QVector3D dest = utils::putToQuadrant(abstractDest, quad);
    if (relative)
    {
        QVector3D pos = utils::putToQuadrant(LaserDriver::instance().GetCurrentLaserPos(), quad);
        dest = pos + delta;
    }
    utils::limitToLayout(dest, quad, layoutWidth, layoutHeight);
    char xyzStyle = 0;
    if (dest.z() != 0.f)
        xyzStyle = 1;
    LaserDriver::instance().lPenQuickMoveTo(xyzStyle, true, dest.x(), dest.y(), dest.z(), moveFastSpeed, moveFastLaunchingSpeed);
}

FinishRun LaserControllerWindow::finishRun()
{
    FinishRun value;
    if (m_comboBoxPostEvent->currentIndex() < 3)
    {
        value.setAction(m_comboBoxPostEvent->currentIndex());
    }
    else
    {
        if (m_radioButtonMachiningOrigin1->isChecked())
        {
            value.setAction(3);
        }
        else if (m_radioButtonMachiningOrigin2->isChecked())
        {
            value.setAction(4);
        }
        else if (m_radioButtonMachiningOrigin3->isChecked())
        {
            value.setAction(5);
        }
    }
    //value.setRelay(0, m_ui->checkBoxRelay1->isChecked());
    //value.setRelay(1, m_ui->checkBoxRelay2->isChecked());
    //value.setRelay(2, m_ui->checkBoxRelay3->isChecked());
    qDebug() << value.toString();
    return value;
}

void LaserControllerWindow::handleSecurityException(int code, const QString& message)
{
    switch (code)
    {
    case E_MainCardInactivated:
    {
        LaserApplication::device->activateMainCard(
            "name",
            "address",
            "18688886666",
            "12341234",
            "wx_66886688",
            "happy@ever.net",
            "China",
            "Unknown",
            "Unknown",
            "Default"
        );
        break;
    }
    }
}

void LaserControllerWindow::createCentralDockPanel()
{
    QWidget* topLeftRuler = new QWidget;

    m_viewer = new LaserViewer(this);
    m_viewer->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_viewer->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //m_viewer->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_scene = qobject_cast<LaserScene*>(m_viewer->scene());

     // 初始化缩放列表控件
    m_comboBoxScale = new QComboBox;
    m_comboBoxScale->setEditable(true);
    m_comboBoxScale->addItem("10%");
    m_comboBoxScale->addItem("25%");
    m_comboBoxScale->addItem("50%");
    m_comboBoxScale->addItem("75%");
    m_comboBoxScale->addItem("100%");
    m_comboBoxScale->addItem("150%");
    m_comboBoxScale->addItem("200%");
    m_comboBoxScale->addItem("300%");
    m_comboBoxScale->addItem("400%");
    m_comboBoxScale->addItem("500%");
    m_comboBoxScale->addItem("1000%");
    m_comboBoxScale->setCurrentText("100%");
    QRegularExpression percentageRE("^[0-9]+%$");
    QValidator* percentageValidator = new QRegularExpressionValidator(percentageRE, m_comboBoxScale);
    m_comboBoxScale->setValidator(percentageValidator);

    QBoxLayout* viewHoriBottomLayout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
    viewHoriBottomLayout->setSpacing(0);
    viewHoriBottomLayout->setMargin(0);
    viewHoriBottomLayout->addWidget(m_comboBoxScale);
    viewHoriBottomLayout->setStretch(0, 0);
    viewHoriBottomLayout->addStretch(1);

    m_hRuler = new RulerWidget;
	m_hRuler->setViewer(m_viewer);
	m_viewer->setHorizontalRuler(m_hRuler);
	m_hRuler->refresh();
	connect(m_viewer, &LaserViewer::zoomChanged, m_hRuler, &RulerWidget::viewZoomChanged);
	connect(StateControllerInst.documentWorkingState(), &QState::initialStateChanged, m_hRuler, [=] {
		qDebug() << "ruler_h";
	});

    m_vRuler = new RulerWidget;
    m_vRuler->setViewer(m_viewer);
	m_vRuler->setIsVertical(true);
	m_viewer->setVerticalRuler(m_vRuler);
	m_vRuler->refresh();
	connect(m_viewer, &LaserViewer::zoomChanged, m_vRuler, &RulerWidget::viewZoomChanged);
	connect(StateControllerInst.documentState(), &QState::initialStateChanged, m_vRuler, [=] {
		qDebug() << "ruler_v";
	});

	// 初始化整个工作区。这是一个网格布局的9宫格。
    QGridLayout* centralGridLayout = new QGridLayout;
    centralGridLayout->addWidget(topLeftRuler, 0, 0);
    centralGridLayout->addWidget(m_hRuler, 0, 1);
    centralGridLayout->addWidget(m_vRuler, 1, 0);
    centralGridLayout->addWidget(m_viewer, 1, 1);
	centralGridLayout->setSpacing(0);
    centralGridLayout->setMargin(0);
    centralGridLayout->addLayout(viewHoriBottomLayout, 2, 0, 1, 2);

    // create center widget
    QWidget* centralWidget = new QWidget;
    centralWidget->setLayout(centralGridLayout);

    CDockWidget* centralDockWidget = new CDockWidget(tr("work space"));
    centralDockWidget->setWidget(centralWidget);
    CDockAreaWidget* centralDockArea = m_dockManager->setCentralWidget(centralDockWidget);
    centralDockArea->setAllowedAreas(DockWidgetArea::OuterDockAreas);
}

void LaserControllerWindow::createLayersDockPanel()
{
    m_tableWidgetLayers = new LaserLayerTableWidget;
    connect(m_tableWidgetLayers, &QTableWidget::cellDoubleClicked, this, &LaserControllerWindow::onTableWidgetLayersCellDoubleClicked);
    connect(m_tableWidgetLayers, &QTableWidget::itemSelectionChanged, this, &LaserControllerWindow::onTableWidgetItemSelectionChanged);

    m_buttonMoveLayerUp = new QToolButton;
    m_buttonMoveLayerUp->setDefaultAction(m_ui->actionMoveLayerUp);

    m_buttonMoveLayerDown = new QToolButton;
    m_buttonMoveLayerDown->setDefaultAction(m_ui->actionMoveLayerDown);

    m_buttonRemoveLayer = new QToolButton;
    m_buttonRemoveLayer->setDefaultAction(m_ui->actionRemoveLayer);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->setMargin(0);
    buttonsLayout->addStretch(0);
    buttonsLayout->addWidget(m_buttonMoveLayerUp);
    buttonsLayout->addWidget(m_buttonMoveLayerDown);
    buttonsLayout->addWidget(m_buttonRemoveLayer);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(m_tableWidgetLayers, 1);
    layout->addLayout(buttonsLayout, 0);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Layers"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockAreaLayers = m_dockManager->addDockWidget(RightDockWidgetArea, dockWidget);
}

void LaserControllerWindow::createCameraDockPanel()
{
    QLabel* labelCameras = new QLabel(tr("Cameras"));

    m_comboBoxCameras = new QComboBox;
    m_comboBoxCameras->addItem(tr("None"));

    m_buttonCameraUpdateOverlay = new QToolButton;
    m_buttonCameraUpdateOverlay->setDefaultAction(m_ui->actionCameraUpdateOverlay);
    m_buttonCameraUpdateOverlay->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonCameraTrace = new QToolButton;
    m_buttonCameraTrace->setDefaultAction(m_ui->actionCameraTrace);
    m_buttonCameraTrace->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonCameraSaveSettings = new QToolButton;
    m_buttonCameraSaveSettings->setDefaultAction(m_ui->actionCameraSaveSettings);
    m_buttonCameraSaveSettings->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_checkBoxCameraFade = new QCheckBox(tr("Fade"));
    QLabel* labelFadeWidth = new QLabel(tr("Width"));
    m_doubleSpinBoxCameraFadeWidth = new QDoubleSpinBox;
    m_doubleSpinBoxCameraFadeWidth->setDecimals(1);
    m_doubleSpinBoxCameraFadeWidth->setValue(0.0);
    QLabel* labelFadeHeight = new QLabel(tr("Height"));
    m_doubleSpinBoxCameraFadeHeight = new QDoubleSpinBox;
    m_doubleSpinBoxCameraFadeHeight->setDecimals(1);
    m_doubleSpinBoxCameraFadeHeight->setValue(0.0);

    m_checkBoxCameraShow = new QCheckBox(tr("Show"));
    QLabel* labelXShift = new QLabel(tr("X Shift"));
    m_doubleSpinBoxCameraXShift = new QDoubleSpinBox;
    m_doubleSpinBoxCameraXShift->setDecimals(1);
    m_doubleSpinBoxCameraXShift->setValue(0.0);
    QLabel* labelYShift = new QLabel(tr("Y Shift"));
    m_doubleSpinBoxCameraYShift = new QDoubleSpinBox;
    m_doubleSpinBoxCameraYShift->setDecimals(1);
    m_doubleSpinBoxCameraYShift->setValue(0.0);

    QGridLayout* layout = new QGridLayout;
    layout->setMargin(0);
    layout->addWidget(labelCameras, 0, 0);
    layout->addWidget(m_comboBoxCameras, 0, 1, 1, 4);
    layout->addWidget(m_buttonCameraUpdateOverlay, 1, 0);
    layout->addWidget(m_buttonCameraTrace, 1, 1, 1, 2);
    layout->addWidget(m_buttonCameraSaveSettings, 1, 3, 1, 2);
    layout->addWidget(m_checkBoxCameraFade, 2, 0);
    layout->addWidget(labelFadeWidth, 2, 1);
    layout->addWidget(m_doubleSpinBoxCameraFadeWidth, 2, 2);
    layout->addWidget(labelFadeHeight, 2, 3);
    layout->addWidget(m_doubleSpinBoxCameraFadeHeight, 2, 4);
    layout->addWidget(m_checkBoxCameraShow, 3, 0);
    layout->addWidget(labelXShift, 3, 1);
    layout->addWidget(m_doubleSpinBoxCameraXShift, 3, 2);
    layout->addWidget(labelYShift, 3, 3);
    layout->addWidget(m_doubleSpinBoxCameraYShift, 3, 4);
    layout->setRowStretch(6, 1);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Camera"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockAreaCameras = m_dockManager->addDockWidget(CenterDockWidgetArea, dockWidget, m_dockAreaLayers);
}

void LaserControllerWindow::createOperationsDockPanel()
{
    m_buttonOperationStart = new QToolButton;
    m_buttonOperationStart->setDefaultAction(m_ui->actionMachining);
    m_buttonOperationStart->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonOperationPause = new QToolButton;
    m_buttonOperationPause->setDefaultAction(m_ui->actionPause);
    m_ui->actionPause->setCheckable(true);
    m_buttonOperationPause->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonOperationStop = new QToolButton;
    m_buttonOperationStop->setDefaultAction(m_ui->actionStop);
    m_buttonOperationStop->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonOperationBounding = new QToolButton;
    m_buttonOperationBounding->setDefaultAction(m_ui->actionBounding);
    m_buttonOperationBounding->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonOperationSpotShot = new QToolButton;
    m_buttonOperationSpotShot->setDefaultAction(m_ui->actionLaserSpotShot);
    m_buttonOperationSpotShot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(m_buttonOperationSpotShot, &QToolButton::pressed, this, &LaserControllerWindow::lightOnLaser);
    connect(m_buttonOperationSpotShot, &QToolButton::released, this, &LaserControllerWindow::lightOffLaser);

    m_buttonOperationReset = new QToolButton;
    m_buttonOperationReset->setDefaultAction(m_ui->actionReset);
    m_buttonOperationReset->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonOperationOrigin = new QToolButton;
    m_buttonOperationOrigin->setDefaultAction(m_ui->actionMoveToOrigin);
    m_buttonOperationOrigin->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonOperationOptimize = new QToolButton;
    m_buttonOperationOptimize->setDefaultAction(m_ui->actionPathOptimization);
    m_buttonOperationOptimize->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    //QLabel* labelStartPosition = new QLabel(tr("Start Position"));
    m_comboBoxStartPosition = new QComboBox;
    m_comboBoxStartPosition->addItem(tr("Current Position"));
    m_comboBoxStartPosition->addItem(tr("Custom Origin"));
    m_comboBoxStartPosition->addItem(tr("Mechnical Origin"));

    //QLabel* labelLaserPower = new QLabel(tr("Laser Power"));
    m_floatEditSliderLaserPower = new FloatEditSlider;
    m_floatEditSliderLaserPower->setMinimum(0);
    m_floatEditSliderLaserPower->setMaximum(100);
    m_floatEditSliderLaserPower->setStep(0.1);
    m_floatEditSliderLaserPower->setPage(10);
    m_floatEditSliderLaserPower->setTextTemplate("%1%");
    m_floatEditSliderLaserPower->setMaximumLineEditWidth(40);
    connect(m_floatEditSliderLaserPower, &FloatEditSlider::valueChanged, this, &LaserControllerWindow::onFloatEditSliderLaserPower);

    //QLabel* labelLaserRange = new QLabel(tr("Laser Range"));
    m_floatEditDualSliderLaserRange = new FloatEditDualSlider;
    m_floatEditDualSliderLaserRange->setMinimum(0);
    m_floatEditDualSliderLaserRange->setMaximum(100);
    m_floatEditDualSliderLaserRange->setStep(0.1);
    m_floatEditDualSliderLaserRange->setTextTemplate("%1%");
    m_floatEditDualSliderLaserRange->setEditMaxWidth(40);
    m_floatEditDualSliderLaserRange->setLowerValue(Config::SystemRegister::laserMinPower());
    m_floatEditDualSliderLaserRange->setHigherValue(Config::SystemRegister::laserMaxPower());
    connect(m_floatEditDualSliderLaserRange, &FloatEditDualSlider::lowerValueChanged, this, &LaserControllerWindow::onFloatDualEditSliderLowerValueChanged);
    connect(m_floatEditDualSliderLaserRange, &FloatEditDualSlider::higherValueChanged, this, &LaserControllerWindow::onFloatDualEditSliderHigherValueChanged);
    connect(Config::SystemRegister::laserMinPowerItem(), &ConfigItem::valueChanged, this, &LaserControllerWindow::onLaserMinPowerChanged);
    connect(Config::SystemRegister::laserMaxPowerItem(), &ConfigItem::valueChanged, this, &LaserControllerWindow::onLaserMaxPowerChanged);

    QLabel* labelDevices = new QLabel(tr("Devices"));
    m_comboBoxDevices = new QComboBox;
    m_comboBoxDevices->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonConnect = new QToolButton;
    m_buttonConnect->setDefaultAction(m_ui->actionConnect);

    m_buttonRefresh = new QToolButton;
    m_buttonRefresh->setDefaultAction(m_ui->actionRefresh);

    QHBoxLayout* firstRow = new QHBoxLayout;
    firstRow->setMargin(0);
    firstRow->addWidget(m_buttonOperationStart);
    firstRow->addWidget(m_buttonOperationPause);
    firstRow->addWidget(m_buttonOperationStop);

    QHBoxLayout* secondRow = new QHBoxLayout;
    secondRow->setMargin(0);
    secondRow->addWidget(m_buttonOperationBounding);
    secondRow->addWidget(m_buttonOperationSpotShot);
    secondRow->addWidget(m_buttonOperationReset);
    secondRow->addWidget(m_buttonOperationOrigin);
    secondRow->addWidget(m_buttonOperationOptimize);

    QFormLayout* thirdRow = new QFormLayout;
    thirdRow->setMargin(0);
    thirdRow->addRow(tr("Start Position"), m_comboBoxStartPosition);
    thirdRow->addRow(tr("Laser Power"), m_floatEditSliderLaserPower);
    thirdRow->addRow(tr("Laser Range"), m_floatEditDualSliderLaserRange);

    QHBoxLayout* fourthRow = new QHBoxLayout;
    fourthRow->setMargin(0);
    fourthRow->addWidget(labelDevices);
    fourthRow->addWidget(m_comboBoxDevices);
    fourthRow->addWidget(m_buttonConnect);
    fourthRow->addWidget(m_buttonRefresh);
    fourthRow->setStretch(0, 0);
    fourthRow->setStretch(1, 1);
    fourthRow->setStretch(2, 0);
    fourthRow->setStretch(3, 0);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addLayout(firstRow);
    layout->addLayout(secondRow);
    layout->addLayout(thirdRow);
    layout->addLayout(fourthRow);
    layout->addStretch(1);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Operations"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockAreaOperations = m_dockManager->addDockWidget(BottomDockWidgetArea, dockWidget, m_dockAreaLayers);
}

void LaserControllerWindow::createOutlineDockPanel()
{
    m_treeWidgetOutline = new QTreeWidget;
    m_treeWidgetOutline->setHeaderHidden(true);

    QHBoxLayout* toolsLayout = new QHBoxLayout;
    toolsLayout->setMargin(0);

    QToolButton* updateButton = new QToolButton;
    updateButton->setDefaultAction(m_ui->actionUpdateOutline);
    QToolButton* showPathOptimizationOpt = new QToolButton;
    showPathOptimizationOpt->setDefaultAction(m_ui->actionPathOptimization);
    toolsLayout->addStretch(1);
    toolsLayout->addWidget(showPathOptimizationOpt);
    toolsLayout->addWidget(updateButton);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(m_treeWidgetOutline);
    layout->addLayout(toolsLayout);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Outline"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockAreaOutline = m_dockManager->addDockWidget(CenterDockWidgetArea, dockWidget, m_dockAreaLayers);
}

void LaserControllerWindow::createMovementDockPanel()
{
    m_lineEditCoordinatesX = new QLineEdit;
    m_lineEditCoordinatesX->setReadOnly(true);
    m_lineEditCoordinatesX->setText(QString::number(0.0));

    m_lineEditCoordinatesY = new QLineEdit;
    m_lineEditCoordinatesY->setReadOnly(true);
    m_lineEditCoordinatesY->setText(QString::number(0.0));

    m_lineEditCoordinatesZ = new QLineEdit;
    m_lineEditCoordinatesZ->setReadOnly(true);
    m_lineEditCoordinatesZ->setText(QString::number(0.0));

    m_doubleSpinBoxDistanceX = new QDoubleSpinBox;
    m_doubleSpinBoxDistanceX->setDecimals(1);
    m_doubleSpinBoxDistanceX->setValue(10.0);

    m_doubleSpinBoxDistanceY = new QDoubleSpinBox;
    m_doubleSpinBoxDistanceY->setDecimals(1);
    m_doubleSpinBoxDistanceY->setValue(10.0);

    m_doubleSpinBoxDistanceZ = new QDoubleSpinBox;
    m_doubleSpinBoxDistanceZ->setDecimals(1);
    m_doubleSpinBoxDistanceZ->setValue(10.0);

    QGridLayout* firstRow = new QGridLayout;
    firstRow->setMargin(0);
    firstRow->addWidget(new QLabel(tr("Coordinates")), 0, 0);
    firstRow->addWidget(new QLabel(tr("X")), 0, 1);
    firstRow->addWidget(m_lineEditCoordinatesX, 0, 2);
    firstRow->addWidget(new QLabel(tr("Y")), 0, 3);
    firstRow->addWidget(m_lineEditCoordinatesY, 0, 4);
    firstRow->addWidget(new QLabel(tr("Z")), 0, 5);
    firstRow->addWidget(m_lineEditCoordinatesZ, 0, 6);
    firstRow->addWidget(new QLabel(tr("Distance(mm)")), 1, 0);
    firstRow->addWidget(new QLabel(tr("X")), 1, 1);
    firstRow->addWidget(m_doubleSpinBoxDistanceX, 1, 2);
    firstRow->addWidget(new QLabel(tr("Y")), 1, 3);
    firstRow->addWidget(m_doubleSpinBoxDistanceY, 1, 4);
    firstRow->addWidget(new QLabel(tr("Z")), 1, 5);
    firstRow->addWidget(m_doubleSpinBoxDistanceZ, 1, 6);
    firstRow->setColumnStretch(0, 1);
    firstRow->setColumnStretch(1, 0);
    firstRow->setColumnStretch(2, 1);
    firstRow->setColumnStretch(3, 0);
    firstRow->setColumnStretch(4, 1);
    firstRow->setColumnStretch(5, 0);
    firstRow->setColumnStretch(6, 1);

    m_buttonMoveTopLeft = new QToolButton;
    m_buttonMoveTopLeft->setDefaultAction(m_ui->actionMoveTopLeft);

    m_buttonMoveTop= new QToolButton;
    m_buttonMoveTop->setDefaultAction(m_ui->actionMoveTop);

    m_buttonMoveTopRight = new QToolButton;
    m_buttonMoveTopRight->setDefaultAction(m_ui->actionMoveTopRight);

    m_buttonMoveLeft = new QToolButton;
    m_buttonMoveLeft->setDefaultAction(m_ui->actionMoveLeft);

    m_buttonMoveToOrigin= new QToolButton;
    m_buttonMoveToOrigin->setDefaultAction(m_ui->actionMoveToOrigin);

    m_buttonMoveRight = new QToolButton;
    m_buttonMoveRight->setDefaultAction(m_ui->actionMoveRight);

    m_buttonMoveBottomLeft = new QToolButton;
    m_buttonMoveBottomLeft->setDefaultAction(m_ui->actionMoveBottomLeft);

    m_buttonMoveBottom = new QToolButton;
    m_buttonMoveBottom->setDefaultAction(m_ui->actionMoveBottom);

    m_buttonMoveBottomRight = new QToolButton;
    m_buttonMoveBottomRight->setDefaultAction(m_ui->actionMoveBottomRight);

    m_buttonMoveUp = new QToolButton;
    m_buttonMoveUp->setDefaultAction(m_ui->actionMoveUp);

    m_buttonMoveDown = new QToolButton;
    m_buttonMoveDown->setDefaultAction(m_ui->actionMoveDown);

    QGridLayout* secondRow = new QGridLayout;
    secondRow->setMargin(0);
    secondRow->addWidget(m_buttonMoveTopLeft, 0, 0);
    secondRow->addWidget(m_buttonMoveTop, 0, 1);
    secondRow->addWidget(m_buttonMoveTopRight, 0, 2);
    secondRow->addWidget(m_buttonMoveUp, 0, 3);
    secondRow->addWidget(m_buttonMoveLeft, 1, 0);
    secondRow->addWidget(m_buttonMoveToOrigin, 1, 1);
    secondRow->addWidget(m_buttonMoveRight, 1, 2);
    secondRow->addWidget(m_buttonMoveBottomLeft, 2, 0);
    secondRow->addWidget(m_buttonMoveBottom, 2, 1);
    secondRow->addWidget(m_buttonMoveBottomRight, 2, 2);
    secondRow->addWidget(m_buttonMoveDown, 2, 3);

    m_comboBoxPostEvent = new QComboBox;
    m_comboBoxPostEvent->addItem(tr("Stop at current position"));
    m_comboBoxPostEvent->addItem(tr("Unload motor"));
    m_comboBoxPostEvent->addItem(tr("Back to mechnical origin"));
    m_comboBoxPostEvent->addItem(tr("Back to machining origin"));

    m_radioButtonMachiningOrigin1 = new QRadioButton(tr("Origin 1"));
    m_radioButtonMachiningOrigin2 = new QRadioButton(tr("Origin 1"));
    m_radioButtonMachiningOrigin3 = new QRadioButton(tr("Origin 1"));
    m_radioButtonMachiningOrigin1->setChecked(true);

    m_doubleSpinBoxOrigin1X = new QDoubleSpinBox;
    m_doubleSpinBoxOrigin1X->setDecimals(2);
    m_doubleSpinBoxOrigin1X->setValue(0.00);

    m_doubleSpinBoxOrigin1Y = new QDoubleSpinBox;
    m_doubleSpinBoxOrigin1Y->setDecimals(2);
    m_doubleSpinBoxOrigin1Y->setValue(0.00);

    m_doubleSpinBoxOrigin2X = new QDoubleSpinBox;
    m_doubleSpinBoxOrigin2X->setDecimals(2);
    m_doubleSpinBoxOrigin2X->setValue(0.00);

    m_doubleSpinBoxOrigin2Y = new QDoubleSpinBox;
    m_doubleSpinBoxOrigin2Y->setDecimals(2);
    m_doubleSpinBoxOrigin2Y->setValue(0.00);

    m_doubleSpinBoxOrigin3X = new QDoubleSpinBox;
    m_doubleSpinBoxOrigin3X->setDecimals(2);
    m_doubleSpinBoxOrigin3X->setValue(0.00);

    m_doubleSpinBoxOrigin3Y = new QDoubleSpinBox;
    m_doubleSpinBoxOrigin3Y->setDecimals(2);
    m_doubleSpinBoxOrigin3Y->setValue(0.00);

    QGridLayout* thirdRow = new QGridLayout;
    thirdRow->setMargin(0);
    thirdRow->addWidget(new QLabel(tr("Post Event")), 0, 0);
    thirdRow->addWidget(m_comboBoxPostEvent, 0, 1, 1, 4);
    thirdRow->addWidget(m_radioButtonMachiningOrigin1, 1, 0);
    thirdRow->addWidget(new QLabel(tr("X")), 1, 1);
    thirdRow->addWidget(m_doubleSpinBoxOrigin1X, 1, 2);
    thirdRow->addWidget(new QLabel(tr("Y")), 1, 3);
    thirdRow->addWidget(m_doubleSpinBoxOrigin1Y, 1, 4);
    thirdRow->addWidget(m_radioButtonMachiningOrigin2, 2, 0);
    thirdRow->addWidget(new QLabel(tr("X")), 2, 1);
    thirdRow->addWidget(m_doubleSpinBoxOrigin2X, 2, 2);
    thirdRow->addWidget(new QLabel(tr("Y")), 2, 3);
    thirdRow->addWidget(m_doubleSpinBoxOrigin2Y, 2, 4);
    thirdRow->addWidget(m_radioButtonMachiningOrigin3, 3, 0);
    thirdRow->addWidget(new QLabel(tr("X")), 3, 1);
    thirdRow->addWidget(m_doubleSpinBoxOrigin3X, 3, 2);
    thirdRow->addWidget(new QLabel(tr("Y")), 3, 3);
    thirdRow->addWidget(m_doubleSpinBoxOrigin3Y, 3, 4);
    thirdRow->setColumnStretch(0, 1);
    thirdRow->setColumnStretch(1, 0);
    thirdRow->setColumnStretch(2, 1);
    thirdRow->setColumnStretch(3, 0);
    thirdRow->setColumnStretch(4, 1);

    m_buttonReadOrigins = new QToolButton;
    m_buttonReadOrigins->setDefaultAction(m_ui->actionReadOrigins);
    m_buttonReadOrigins->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(m_buttonReadOrigins, &QToolButton::clicked, this, &LaserControllerWindow::readMachiningOrigins);

    m_buttonWriteOrigins = new QToolButton;
    m_buttonWriteOrigins->setDefaultAction(m_ui->actionWriteOrigins);
    m_buttonWriteOrigins->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(m_buttonWriteOrigins, &QToolButton::clicked, this, &LaserControllerWindow::writeMachiningOrigins);

    QHBoxLayout* fourthRow = new QHBoxLayout;
    fourthRow->setMargin(0);
    fourthRow->addWidget(m_buttonReadOrigins);
    fourthRow->addWidget(m_buttonWriteOrigins);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addLayout(firstRow);
    layout->addLayout(secondRow);
    layout->addLayout(thirdRow);
    layout->addLayout(fourthRow);
    layout->addStretch(1);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Movement"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockAreaMovement = m_dockManager->addDockWidget(CenterDockWidgetArea, dockWidget, m_dockAreaLayers);
}

void LaserControllerWindow::createUpdateDockPanel(int winId)
{
    QWindow* externalWindow = QWindow::fromWinId(winId);
    //externalWindow->showMaximized();
    QWidget* externalWidget = createWindowContainer(externalWindow);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(externalWidget);
    //layout->addStretch(1);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Update"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockAreaUpdate = m_dockManager->addDockWidget(CenterDockWidgetArea, dockWidget, m_dockAreaLayers);
}

void LaserControllerWindow::keyPressEvent(QKeyEvent * event)
{
	
	QWidget::keyPressEvent(event);
	if (!m_viewer) {
		return;
	}
	if (!m_scene || !m_scene->document()) {
		return;
	}
	
	switch (event->key())
	{
		case Qt::Key_Space: {
            if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {
                return;
            }
			if (!event->isAutoRepeat()) {
  				m_lastState = nullptr;
				m_lastState = m_viewer->currentState();
				this->onActionViewDrag(true);

			}
			break;
		}
	
	}
	
}

void LaserControllerWindow::keyReleaseEvent(QKeyEvent * event)
{
	QWidget::keyReleaseEvent(event);
	if (!m_viewer) {
		return;
	}
	if (!m_scene || !m_scene->document()) {
		return;
	}
	switch (event->key())
	{
		case Qt::Key_Space: {
			if (!event->isAutoRepeat()) {
				//this->onActionViewDrag(false);
				if (m_lastState == StateControllerInst.documentPrimitiveRectState()) {
					emit readyRectangle();
				}
				else if (m_lastState == StateControllerInst.documentPrimitiveEllipseState()) {
					emit readyEllipse();
				}
				else if (m_lastState == StateControllerInst.documentPrimitiveLineState()) {
					emit readyLine();
				}
				else if (m_lastState == StateControllerInst.documentPrimitivePolygonState()) {
					emit readyPolygon();
				}
				else if (m_lastState == StateControllerInst.documentPrimitiveSplineState()) {
					emit readySpline();
				}
				else if (m_lastState == StateControllerInst.documentPrimitiveSplineEditState()) {
					emit readySplineEdit();
				}
				else if (m_lastState == StateControllerInst.documentPrimitiveTextState()) {
					emit readyText();
				}
				else if (m_lastState == StateControllerInst.documentIdleState()) {
					emit isIdle();
				}
				else if (m_lastState == StateControllerInst.documentIdleState()) {
					emit isIdle();
				}
				m_lastState = nullptr;
				m_viewer->viewport()->repaint();
			}
			else
			break;
		}

	}
	
}
void LaserControllerWindow::contextMenuEvent(QContextMenuEvent * event)
{
	if (StateControllerInst.isInState(StateControllerInst.documentIdleState()) ||
		StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
		QMenu Context;
		Context.addAction(m_ui->actionCopy);
		Context.addAction(m_ui->actionCut);
		Context.addAction(m_ui->actionPaste);
		Context.addAction(m_ui->actionDuplication);
		Context.addSeparator();
		Context.addAction(m_ui->actionGroup);
		Context.addAction(m_ui->actionUngroup);
		Context.exec(QCursor::pos());
	}
	

}
void LaserControllerWindow::onActionUndo(bool checked) {
	int index = m_viewer->undoStack()->index();
	if (index <= 0) {
		return;
	}
	m_viewer->undoStack()->setIndex(index - 1);
}
void LaserControllerWindow::onActionRedo(bool checked) {
	int index = m_viewer->undoStack()->index();	
	m_viewer->undoStack()->setIndex(index + 1);
}
void LaserControllerWindow::onActionImport(bool checked)
{
    qLogD << "onActionImport";
    //QStringList filters;
    //filters << "image/svg+xml" << "image/svg+xml-compressed"
        //<< "application/dxf";
    QString filters = tr("SVG (*.svg);;CAD (*.dxf)");
    QString filename = getFilename(tr("Open Supported File"), filters);
    qLogD << "importing filename is " << filename;
    if (filename.isEmpty())
        return;

    QFileInfo file(filename);
    QSharedPointer<Importer> importer = Importer::getImporter(this, file.suffix());
    if (!importer.isNull())
    {
        LaserDocument* doc = importer->import(filename, m_scene);
        initDocument(doc);
    }
}

void LaserControllerWindow::onActionNew(bool checked)
{
	
	LaserDocument* doc = m_scene->document();
	if (doc) {
		if (!onActionCloseDocument()) {
			return;
		}
	}
	this->setWindowTitle("<Untitled> - " + m_windowTitle);
	createNewDocument();
	
	
}

bool LaserControllerWindow::onActionSave(bool checked)
{
	if (m_fileDirection.isEmpty()) {
		if (!onActionSaveAs()) {
			return false;
		}

	}
	else {
		m_scene->document()->save(m_fileDirection, this);
	}
	return true;
}

bool LaserControllerWindow::onActionSaveAs(bool checked)
{
	QString name = QFileDialog::getSaveFileName(nullptr, "save file", ".", "File(*lc)");
	
	
	if (name == "") {
		return false;
	}

	if (!name.endsWith(".lc")) {
		name += ".lc";
	}
	qDebug() << name;
	m_fileDirection = name;
	setWindowTitle(getCurrentFileName() + " - " + m_windowTitle);
	m_scene->document()->save(name, this);
	return true;
}

void LaserControllerWindow::onActionOpen(bool checked)
{
	LaserDocument* doc = m_scene->document();
	if (doc) {
		if (!onActionCloseDocument()) {
			return;
		}
	}
	//create title
	QString name = QFileDialog::getOpenFileName(nullptr, "open file", ".", "File(*lc)");
	m_fileDirection = name;
	if (name == "") {
		return;
	}
	setWindowTitle(getCurrentFileName() + " - " + m_windowTitle);
	//创建document
	createNewDocument();
	m_scene->document()->load(name, this);
}

void LaserControllerWindow::onActionZoomIn(bool checked)
{
	m_viewer->zoomIn();
}

void LaserControllerWindow::onActionZoomOut(bool checked)
{
	m_viewer->zoomOut();
}

void LaserControllerWindow::onActionZoomToPage(bool checked)
{
	m_viewer->resetZoom();
}

void LaserControllerWindow::onActionZoomToSelection(bool checked)
{
	m_viewer->zoomToSelection();
}


void LaserControllerWindow::onActionImportCorelDraw(bool checked)
{
    QSharedPointer<Importer> importer = Importer::getImporter(this, Importer::CORELDRAW);
    QVariantMap params;
    params["parent_winid"] = winId();
    params["parent_win"] = QVariant::fromValue<QMainWindow*>(this);
    LaserDocument* doc = importer->import("", m_scene, params);
    initDocument(doc);
}

void LaserControllerWindow::onActionRemoveLayer(bool checked)
{
    qDebug() << "removing layer.";
    QTableWidgetItem* item = m_tableWidgetLayers->currentItem();
	if (!item)
	{
		return;
	}

    item = m_tableWidgetLayers->item(item->row(), 0);
	int index = item->data(Qt::UserRole).toInt();
	LaserLayer* layer = m_scene->document()->layers()[index];
	if (layer->isDefault())
	{
		QMessageBox::warning(this, tr("Remove Layer"), tr("You can not remove default layer. (Note: The first two layers are default layers, one for cutting and another for engraving.)"));
		return;
	}

	QMessageBox msgBox(this);
	msgBox.setWindowTitle(tr("Remove Layer"));
	msgBox.setText(tr("You are about deleting selected layer. Do you want to delete all primitives belonged to this layer or move them to default layer?"));
	msgBox.setInformativeText(tr("If you click 'Delete', all primitives in this layer will be deleted. \nIf you click 'Move', they will be moved to the default layer. \nIf you click 'Cancel', do nothing."));
	msgBox.setIcon(QMessageBox::Icon::Question);
	QPushButton* deleteButton = msgBox.addButton(tr("Delete"), QMessageBox::ActionRole);
	QPushButton* moveButton = msgBox.addButton(tr("Move"), QMessageBox::ActionRole);
	QPushButton* cancelButton = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
	msgBox.exec();
	if (msgBox.clickedButton() == deleteButton)
	{
        for (LaserPrimitive* primitive : layer->primitives())
        {
            m_scene->document()->removePrimitive(primitive);
        }
	}
	else if (msgBox.clickedButton() == moveButton)
	{
		if (layer->type() == LLT_ENGRAVING)
		{
			for (LaserPrimitive* primitive : layer->primitives())
			{
				m_scene->document()->addPrimitive(primitive, m_scene->document()->defaultEngravingLayer());
			}
		}
		else if (layer->type() == LLT_CUTTING)
		{
			for (LaserPrimitive* primitive : layer->primitives())
			{
				m_scene->document()->addPrimitive(primitive, m_scene->document()->defaultCuttingLayer());
			}
		}
	}
	m_tableWidgetLayers->updateItems();
}

void LaserControllerWindow::onTableWidgetLayersCellDoubleClicked(int row, int column)
{
    QTableWidgetItem* item = m_tableWidgetLayers->item(row, 0);
    int index = item->data(Qt::UserRole).toInt();

    LaserLayer* layer = m_scene->document()->layers()[index];

    LaserLayerDialog dialog(layer);
    if (dialog.exec() == QDialog::Accepted)
    {
        m_tableWidgetLayers->updateItems();
    }
}

void LaserControllerWindow::onTableWidgetItemSelectionChanged()
{
    QList<QTableWidgetItem*> items = m_tableWidgetLayers->selectedItems();
    if (items.isEmpty())
        return;

    int row = items[0]->row();
    QTableWidgetItem* item = m_tableWidgetLayers->item(row, 0);
    int index = item->data(Qt::UserRole).toInt();

    LaserLayer* layer = m_scene->document()->layers()[index];
    m_scene->blockSignals(true);
    //m_scene->clearSelection();
    for (LaserPrimitive* primitive : layer->primitives())
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
    dialog.selectFile(m_scene->document()->name());
    if (dialog.exec() == QFileDialog::Accepted)
    {
        QString filename = dialog.selectedFiles().constFirst();
        if (!filename.isEmpty() && !filename.isNull())
        {
            m_scene->document()->outline();
            m_scene->document()->setFinishRun(finishRun());
            m_scene->document()->exportJSON(filename);
        }
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

        m_scene->document()->setFinishRun(finishRun());
        qDebug() << "exporting to temporary json file:" << filename;
        m_scene->document()->exportJSON(filename);
        qDebug() << "export temp json file for machining" << filename;
        LaserDriver::instance().loadDataFromFile(filename);
        LaserDriver::instance().startMachining(m_comboBoxStartPosition->currentIndex() == 3);
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

void LaserControllerWindow::onActionPathOptimization(bool checked)
{
    showConfigDialog(tr("Path Optimization"));
}

void LaserControllerWindow::onActionConnect(bool checked)
{
    if (m_comboBoxDevices->count() == 0)
        return;
    QString comName = m_comboBoxDevices->currentText();
    LaserDriver::instance().initComPort(comName);
}

void LaserControllerWindow::onActionDisconnect(bool checked)
{
    LaserDriver::instance().uninitComPort();
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

void LaserControllerWindow::onActionMoveTop(bool checked)
{
    QVector3D delta(0, m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveBottom(bool checked)
{
    QVector3D delta(0, -m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveLeft(bool checked)
{
    QVector3D delta(-m_doubleSpinBoxDistanceX->value(), 0, 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveRight(bool checked)
{
    QVector3D delta(m_doubleSpinBoxDistanceX->value(), 0, 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveTopLeft(bool checked)
{
    QVector3D delta(-m_doubleSpinBoxDistanceX->value(), m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveTopRight(bool checked)
{
    QVector3D delta(m_doubleSpinBoxDistanceX->value(), m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveBottomLeft(bool checked)
{
    QVector3D delta(-m_doubleSpinBoxDistanceX->value(), -m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveBottomRight(bool checked)
{
    QVector3D delta(m_doubleSpinBoxDistanceX->value(), -m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveUp(bool checked)
{
    QVector3D delta(0, 0, m_doubleSpinBoxDistanceZ->value());
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveDown(bool checked)
{
    QVector3D delta(0, 0, -m_doubleSpinBoxDistanceZ->value());
    moveLaser(delta);
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
    /*if (QMessageBox::Apply == QMessageBox::question(this, tr("Delete primitives?"), tr("Do you want to delete primitives selected?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {        
    }*/
	if (!m_scene) {
		return;
	}
	QGraphicsView* qv = m_scene->views()[0];
	LaserViewer* viewer = qobject_cast<LaserViewer*>(qv);
	if (!viewer) {
		return;
	}

	if (!viewer->group() || viewer->group()->isEmpty()) {
		return;
	}
	for each(QGraphicsItem* itemed in m_scene->selectedPrimitives()) {
		if (!m_viewer->group()->isAncestorOf(itemed)) {
			qLogW << "Exit many items have selected but not in group When Delete,Please check.";
			return;
		}
	}
	for (QGraphicsItem* item : viewer->group()->childItems())
	{
		if (!item->isSelected()) {
			qLogW << "Exit many items selected are false When Delete,Please check.";
			return;
		}
		
	}
	//undo 创建完后先执行redo
	AddDelUndoCommand* delCmd = new AddDelUndoCommand(m_scene, viewer->group()->childItems(), true);
	m_viewer->undoStack()->push(delCmd);
	m_tableWidgetLayers->updateItems();
	//change state
	m_viewer->onCancelSelected();
	//由被选中状态切换到Idle
	emit m_viewer->selectionToIdle();
	m_viewer->viewport()->repaint();
}

void LaserControllerWindow::onActionCopy(bool checked)
{
	if (!m_viewer) {
		return;
	}
	//QList<LaserPrimitive>* list =  m_viewer->copyedList();
	
	m_viewer->copyedList().clear();
	for each(QGraphicsItem* item in m_viewer->group()->childItems()) {
		LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
		m_viewer->copyedList().insert(p, p->sceneTransform());
	}
}

void LaserControllerWindow::onActionPaste(bool checked)
{
	if (!m_viewer || m_viewer->copyedList().isEmpty()) {
		return;
	}
	PasteCommand* cmd = new PasteCommand(m_viewer);
	m_viewer->undoStack()->push(cmd);
}

void LaserControllerWindow::onActionPasteInLine(bool checked)
{
	if (!m_viewer || m_viewer->copyedList().isEmpty()) {
		return;
	}
	PasteCommand* cmd = new PasteCommand(m_viewer, true);
	m_viewer->undoStack()->push(cmd);
}

void LaserControllerWindow::onActionCut(bool checked)
{
	onActionCopy();
	onActionDeletePrimitive();
}

void LaserControllerWindow::onActionDuplication(bool checked) {
	if (!m_viewer || m_viewer->group()->isEmpty()) {
		return;
	}
	PasteCommand* cmd = new PasteCommand(m_viewer, false, true);
	m_viewer->undoStack()->push(cmd);
}

void LaserControllerWindow::onActionGroup(bool checked)
{

}

void LaserControllerWindow::onActionUngroup(bool checked)
{
}

bool LaserControllerWindow::onActionCloseDocument(bool checked)
{
	//documentClose();
	/*QMessageBox msgBox;
	msgBox.setText(tr("Close document?"));
	msgBox.setInformativeText(tr("Do you want to save current document?"));
	msgBox.setStandardButtons(QMessageBox::Save
		| QMessageBox::Close
		| QMessageBox::Cancel);*/
	QMessageBox msgBox(QMessageBox::NoIcon,
		"Close document?", "Do you want to save current document?",
		QMessageBox::Save | QMessageBox::Close | QMessageBox::Cancel, NULL);
	int result = msgBox.exec();
	switch (result) {
		case QMessageBox::StandardButton::Save: {
			if (!onActionSave()) {
				return false;
			}
			documentClose();
			return true;
		}
		case QMessageBox::StandardButton::Close: {
			documentClose();
			return true;
		}
		default:
			return false;
	}
}

void LaserControllerWindow::onActionSettings(bool checked)
{
    showConfigDialog();
}

void LaserControllerWindow::onActionSelectionTool(bool checked)
{
    if (checked)
    {
        emit isIdle();
    }
    else
    {
        m_ui->actionSelectionTool->setChecked(true);
    }
}

void LaserControllerWindow::onActionViewDrag(bool checked)
{
	if (checked)
	{
		emit readyViewDrag();
		if (m_viewer != nullptr && m_viewer->viewport() != nullptr) {
			m_viewer->viewport()->repaint();
		}
		
	}
	else
	{
		m_ui->actionDragView->setChecked(true);
	}
}

void LaserControllerWindow::onActionRectangle(bool checked)
{
	if (checked) {
        emit readyRectangle();
	}
    else
    {
        m_ui->actionRectangleTool->setChecked(true);
    }
}

void LaserControllerWindow::onActionEllipse(bool checked)
{
	if (checked) {
		emit readyEllipse();
	}
	else
	{
		m_ui->actionEllipseTool->setChecked(true);
	}
}

void LaserControllerWindow::onActionLine(bool checked)
{
	if (checked) {
		emit readyLine();
	}
	else
	{
		m_ui->actionLineTool->setChecked(true);
		
	}
}

void LaserControllerWindow::onActionPolygon(bool checked)
{
	if (checked) {
		emit readyPolygon();
	}
	else
	{
		m_ui->actionPolygonTool->setChecked(true);
	}
}

void LaserControllerWindow::onActionSpline(bool checked)
{
	if (checked) {
		emit readySpline();
	}
	else
	{
		m_ui->actionSplineTool->setChecked(true);
	}
}

void LaserControllerWindow::onActionSplineEdit(bool checked)
{
	if (checked) {
		
		emit readySplineEdit();
	}
	else
	{
		m_ui->actionEditSplineTool->setChecked(true);
	}
}

void LaserControllerWindow::onActionText(bool checked)
{
	if (checked) {

		emit readyText();
	}
	else
	{
		m_ui->actionTextTool->setChecked(true);
	}
}

void LaserControllerWindow::onActionShowMainCardInfo(bool checked)
{
    MainCardInfoDialog dialog;
    dialog.exec();
}

void LaserControllerWindow::onActionTemporaryLicense(bool checked)
{
    if (LaserApplication::device->requestTemporaryLicense())
    {
        QMessageBox::information(this, tr("Request successful"), tr("Your application for temporary license is successful. Please restart your program."));
    }
    else
    {
        QMessageBox::information(this, tr("Request failure"), tr("Your application for temporary license is failure."));
    }
}

void LaserControllerWindow::onActionAbout(bool checked)
{
    LaserApplication::device->showLibraryVersion();
}

void LaserControllerWindow::onActionUpdateOutline(bool checked)
{
    m_scene->document()->outline();
    updateOutlineTree();
}

void LaserControllerWindow::onActionBitmap(bool checked)
{
	QString name = QFileDialog::getOpenFileName(nullptr, "open image", ".", "Images (*.jpg *.jpeg *.tif *.bmp *.png)");
	//qDebug() <<"name: "<< name;
	if (name == "") {
		return;
	}
	QImage image(name);
	qreal width = image.size().width();
	qreal height = image.size().height();
	LaserBitmap* bitmap = new LaserBitmap(image, QRectF(0, 0, width, height), m_scene->document());
	//undo 创建完后会执行redo
	QList<QGraphicsItem*> list;
	list.append(bitmap);
	AddDelUndoCommand* addCmd = new AddDelUndoCommand(m_scene, list);
	m_viewer->undoStack()->push(addCmd);
	//m_scene->addLaserPrimitive(bitmap);
	m_viewer->onReplaceGroup(bitmap);
    
}

void LaserControllerWindow::onActionUpdate(bool checked)
{
    LaserApplication::device->checkVersionUpdate(false, "{4A5F9C85-8735-414D-BCA7-E9DD111B23A8}", 0, "update_info.json");
}

void LaserControllerWindow::onActionMirrorHorizontal(bool checked)
{
	if (!m_viewer || !m_viewer->group()) {
		return;
	}
	MirrorHCommand* cmd = new MirrorHCommand(m_viewer);
	m_viewer->undoStack()->push(cmd);
}

void LaserControllerWindow::onActionMirrorVertical(bool checked)
{
	if (!m_viewer || !m_viewer->group()) {
		return;
	}
	MirrorVCommand* cmd = new MirrorVCommand(m_viewer);
	m_viewer->undoStack()->push(cmd);
}

void LaserControllerWindow::onDeviceComPortsFetched(const QStringList & ports)
{
    for (int i = 0; i < ports.size(); i++)
    {
        m_comboBoxDevices->addItem(ports[i], utils::parsePortName(ports[i]));
    }

    if (!ports.isEmpty())
    {
        LaserDriver::instance().initComPort(ports[0]);
    }
}

void LaserControllerWindow::onDeviceConnected()
{
    m_statusBarStatus->setText(tr("Connected"));
}

void LaserControllerWindow::onDeviceDisconnected()
{
    m_statusBarStatus->setText(tr("Disconnected"));
}

void LaserControllerWindow::onMainCardRegistered()
{
    m_statusBarRegister->setText(tr("Registered"));
}

void LaserControllerWindow::onMainCardActivated()
{
    m_statusBarActivation->setText(tr("Activated"));
}

void LaserControllerWindow::onWindowCreated()
{
}

void LaserControllerWindow::closeEvent(QCloseEvent* event)
{
    //m_dockManager->deleteLater();
    QMainWindow::closeEvent(event);
}

void LaserControllerWindow::onEnterDeviceUnconnectedState()
{
    m_buttonConnect->setDefaultAction(m_ui->actionConnect);
}

void LaserControllerWindow::onEnterDeviceConnectedState()
{
    m_buttonConnect->setDefaultAction(m_ui->actionDisconnect);
}

void LaserControllerWindow::onActionMoveLayerUp(bool checked)
{
    QList<QTableWidgetItem*> selectedItems = m_tableWidgetLayers->selectedItems();
    if (selectedItems.isEmpty())
        return;

    int row = selectedItems[0]->row();
    if (row == 0)
        return;

    QTableWidgetItem* current = m_tableWidgetLayers->item(row, 0);
    QTableWidgetItem* target = m_tableWidgetLayers->item(row - 1, 0);
    m_scene->document()->swapLayers(target->data(Qt::UserRole).toInt(), current->data(Qt::UserRole).toInt());
    m_tableWidgetLayers->selectRow(row - 1);
}

void LaserControllerWindow::onActionMoveLayerDown(bool checked)
{
    QList<QTableWidgetItem*> selectedItems = m_tableWidgetLayers->selectedItems();
    if (selectedItems.isEmpty())
        return;

    int row = selectedItems[0]->row();
    if (row >= m_tableWidgetLayers->rowCount() - 1)
        return;

    QTableWidgetItem* current = m_tableWidgetLayers->item(row, 0);
    QTableWidgetItem* target = m_tableWidgetLayers->item(row + 1, 0);
    m_scene->document()->swapLayers(target->data(Qt::UserRole).toInt(), current->data(Qt::UserRole).toInt());
    m_tableWidgetLayers->selectRow(row + 1);
}

void LaserControllerWindow::onLaserSceneSelectedChanged()
{
    QList<LaserPrimitive*> items = m_scene->selectedPrimitives();
	if (items.length() == 0) {
		if (m_propertyWidget) {
			m_propertyWidget->setEnabled(false);
		}
		
		m_ui->actionMirrorHorizontal->setEnabled(false);
		m_ui->actionMirrorVertical->setEnabled(false);
		m_ui->actionCopy->setEnabled(false);
		//m_ui->actionPaste->setEnabled(false);
		m_ui->actionCut->setEnabled(false);
		m_ui->actionDuplication->setEnabled(false);
		m_ui->actionDeletePrimitive->setEnabled(false);
		m_ui->actionGroup->setEnabled(false);
		m_ui->actionUngroup->setEnabled(false);
		return;
	}
	else if (items.length() > 0) {
		if (m_propertyWidget) {
			m_propertyWidget->setEnabled(true);
			selectedChange();
		}	
		m_ui->actionMirrorHorizontal->setEnabled(true);
		m_ui->actionMirrorVertical->setEnabled(true);
		m_ui->actionCopy->setEnabled(true);
		//m_ui->actionPaste->setEnabled(true);
		m_ui->actionCut->setEnabled(true);
		m_ui->actionDuplication->setEnabled(true);
		m_ui->actionDeletePrimitive->setEnabled(true);
		//m_ui->actionGroup->setEnabled(true);
		//m_ui->actionUngroup->setEnabled(true);
		if (items.length() > 1) {
			if (m_viewer->selectedGroupedList().isEmpty()) {
				m_ui->actionUngroup->setEnabled(false);
				m_ui->actionGroup->setEnabled(true);
			}
			else {
				m_ui->actionGroup->setEnabled(true);
				m_ui->actionUngroup->setEnabled(false);
			}
		}
		else {
			m_ui->actionGroup->setEnabled(false);
			m_ui->actionUngroup->setEnabled(false);
		}
		
	}
	

    
}

void LaserControllerWindow::onLaserViewerMouseMoved(const QPointF & pos)
{
    qreal x = Global::convertToMM(SU_PX, pos.x());
    qreal y = Global::convertToMM(SU_PX, pos.y());
    QString posStr = QString("%1mm,%2mm | %3px,%4px").arg(x).arg(y).arg(qFloor(pos.x())).arg(qFloor(pos.y()));
    m_statusBarLocation->setText(posStr);
}

void LaserControllerWindow::onLaserViewerScaleChanged(qreal factor)
{
    QString value = QString("%1%").arg(factor * 100, 0, 'f', 0);
    m_comboBoxScale->blockSignals(true);
    m_comboBoxScale->setCurrentText(value);
    m_comboBoxScale->blockSignals(false);
}

void LaserControllerWindow::onComboBoxSxaleIndexChanged(int index)
{
    //m_viewer->setZoomValue(m_comboBoxScale->currentData().toReal());
}

void LaserControllerWindow::onComboBoxSxaleTextChanged(const QString& text)
{
    // 使用正则表达式检查输入的内容，并获取数字部分的字符串。
    QRegularExpression percentageRE("^([0-9]+)%$");
    QRegularExpressionMatch match = percentageRE.match(text);
    if (match.hasMatch())
    {
        QString number = percentageRE.match(text).captured(1);
        qreal zoom = number.toDouble() / 100;
        m_viewer->setZoomValue(zoom);
    }
}

void LaserControllerWindow::onLaserReturnWorkState(LaserState state)
{
    //m_ui->labelCoordinates->setText(QString("X = %1, Y = %2, Z = %3").arg(state.x, 0, 'g').arg(state.y, 0, 'g').arg(state.z, 0, 'g'));
    m_lineEditCoordinatesX->setText(QString::number(state.x, 'g'));
    m_lineEditCoordinatesY->setText(QString::number(state.y, 'g'));
    m_lineEditCoordinatesZ->setText(QString::number(state.z, 'g'));
}

void LaserControllerWindow::onFloatEditSliderLaserPower(qreal value)
{
    //qLogD << "real time laser power: " << value;
}

void LaserControllerWindow::onFloatDualEditSliderLowerValueChanged(qreal value)
{
    Config::SystemRegister::laserMinPowerItem()->setValue(value);
}

void LaserControllerWindow::onFloatDualEditSliderHigherValueChanged(qreal value)
{
    Config::SystemRegister::laserMaxPowerItem()->setValue(value);
}

void LaserControllerWindow::onLaserMinPowerChanged(const QVariant& value)
{
    bool ok;
    qreal lower = value.toReal(&ok);
    if (!ok)
        return;
    m_floatEditDualSliderLaserRange->blockSignals(true);
    m_floatEditDualSliderLaserRange->setLowerValue(lower);
    m_floatEditDualSliderLaserRange->blockSignals(false);
}

void LaserControllerWindow::onLaserMaxPowerChanged(const QVariant& value)
{
    bool ok;
    qreal higher = value.toReal(&ok);
    if (!ok)
        return;
    m_floatEditDualSliderLaserRange->blockSignals(true);
    m_floatEditDualSliderLaserRange->setHigherValue(higher);
    m_floatEditDualSliderLaserRange->blockSignals(false);
}

void LaserControllerWindow::onCreatSpline()
{
	m_viewer->createSpline();
}

void LaserControllerWindow::lightOnLaser()
{
    LaserDriver::instance().testLaserLight(true);
}

void LaserControllerWindow::lightOffLaser()
{
    LaserDriver::instance().testLaserLight(false);
}

void LaserControllerWindow::readMachiningOrigins(bool checked)
{
    /*LaserDriver::instance().readSysParamFromCard(QList<int>() 
        << LaserDriver::RT_CUSTOM_1_X
        << LaserDriver::RT_CUSTOM_1_Y
        << LaserDriver::RT_CUSTOM_2_X
        << LaserDriver::RT_CUSTOM_2_Y
        << LaserDriver::RT_CUSTOM_3_X
        << LaserDriver::RT_CUSTOM_3_Y
    );*/
}

void LaserControllerWindow::writeMachiningOrigins(bool checked)
{
    /*LaserDriver::RegistersMap values;
    values[LaserDriver::RT_CUSTOM_1_X] = m_ui->doubleSpinBoxOrigin1X->value();
    values[LaserDriver::RT_CUSTOM_1_Y] = m_ui->doubleSpinBoxOrigin1Y->value();
    values[LaserDriver::RT_CUSTOM_2_X] = m_ui->doubleSpinBoxOrigin2X->value();
    values[LaserDriver::RT_CUSTOM_2_Y] = m_ui->doubleSpinBoxOrigin2Y->value();
    values[LaserDriver::RT_CUSTOM_3_X] = m_ui->doubleSpinBoxOrigin3X->value();
    values[LaserDriver::RT_CUSTOM_3_Y] = m_ui->doubleSpinBoxOrigin3Y->value();
    LaserDriver::instance().writeSysParamToCard(values);*/
}

void LaserControllerWindow::updatePostEventWidgets(int index)
{
    if (index == 3)
    {
        m_radioButtonMachiningOrigin1->setEnabled(true);
        m_radioButtonMachiningOrigin2->setEnabled(true);
        m_radioButtonMachiningOrigin3->setEnabled(true);

        m_doubleSpinBoxOrigin1X->setEnabled(true);
        m_doubleSpinBoxOrigin1Y->setEnabled(true);
        m_doubleSpinBoxOrigin2X->setEnabled(true);
        m_doubleSpinBoxOrigin2Y->setEnabled(true);
        m_doubleSpinBoxOrigin3X->setEnabled(true);
        m_doubleSpinBoxOrigin3Y->setEnabled(true);
    }
    else
    {
        m_radioButtonMachiningOrigin1->setEnabled(false);
        m_radioButtonMachiningOrigin2->setEnabled(false);
        m_radioButtonMachiningOrigin3->setEnabled(false);

        m_doubleSpinBoxOrigin1X->setEnabled(false);
        m_doubleSpinBoxOrigin1Y->setEnabled(false);
        m_doubleSpinBoxOrigin2X->setEnabled(false);
        m_doubleSpinBoxOrigin2Y->setEnabled(false);
        m_doubleSpinBoxOrigin3X->setEnabled(false);
        m_doubleSpinBoxOrigin3Y->setEnabled(false);
    }
}

void LaserControllerWindow::laserBackToMachiningOriginalPoint(bool checked)
{
    QVector3D dest;
    if (m_radioButtonMachiningOrigin1->isChecked())
    {
        dest = QVector3D(m_doubleSpinBoxOrigin1X->value(), m_doubleSpinBoxOrigin1Y->value(), 0.f);
    }
    else if (m_radioButtonMachiningOrigin2->isChecked())
    {
        dest = QVector3D(m_doubleSpinBoxOrigin2X->value(), m_doubleSpinBoxOrigin2Y->value(), 0.f);
    }
    else if (m_radioButtonMachiningOrigin3->isChecked())
    {
        dest = QVector3D(m_doubleSpinBoxOrigin3X->value(), m_doubleSpinBoxOrigin3Y->value(), 0.f);
    }
    moveLaser(QVector3D(), false, dest);
}

void LaserControllerWindow::laserResetToOriginalPoint(bool checked)
{
    LaserApplication::device->moveToOrigin();
    /*QVariant value;
    if (!LaserDriver::instance().getRegister(LaserDriver::RT_MOVE_TO_ORI_SPEED,value))
    {
        QMessageBox::warning(this, tr("Read registers failure"), tr("Cannot read registers value!"));
        return;
    }
    LaserDriver::instance().lPenMoveToOriginalPoint(value.toDouble());*/
}

void LaserControllerWindow::updateOutlineTree()
{
    if (!m_scene->document())
        return;

    m_treeWidgetOutline->clear();

    QStack<OptimizeNode*> stack;
    stack.push(m_scene->document()->optimizeNode());

    QMap<OptimizeNode*, QTreeWidgetItem*> nodeItemMap;
    while (!stack.isEmpty())
    {
        OptimizeNode* node = stack.pop();

        QTreeWidgetItem* parentItem = nullptr;
        if (node->parentNode() != nullptr && nodeItemMap.contains(node->parentNode()))
        {
            parentItem = nodeItemMap[node->parentNode()];
        }
        QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
        nodeItemMap.insert(node, item);
        item->setText(0, node->nodeName());
        if (parentItem == nullptr)
        {
            m_treeWidgetOutline->addTopLevelItem(item);
        }

        for (OptimizeNode* childNode : node->childNodes())
        {
            stack.push(childNode);
        }
    }

    m_treeWidgetOutline->expandAll();
}

void LaserControllerWindow::initDocument(LaserDocument* doc)
{
    if (doc)
    {
        connect(m_ui->actionAnalysisDocument, &QAction::triggered, doc, &LaserDocument::analysis);
        connect(doc, &LaserDocument::outlineUpdated, this, &LaserControllerWindow::updateOutlineTree);
        doc->bindLayerButtons(m_layerButtons);
		m_layerButtons[m_viewer->curLayerIndex()]->setCheckedTrue();
        m_scene->updateDocument(doc);
        doc->outline();
        m_tableWidgetLayers->setDocument(doc);
        m_tableWidgetLayers->updateItems();
        //undo
        m_viewer->undoStack()->clear();
        //m_ui->actionUndo->setEnabled(false);
        //m_ui->actionRedo->setEnabled(false);
        LaserViewer* viewer = qobject_cast<LaserViewer*>(m_scene->views()[0]);
        if (m_viewer) {
            //m_viewer->setZoomValue(1.0);
            //m_viewer->centerOn(0, 0);
            QRectF rect = m_scene->document()->pageBounds();

            //m_scene->setSceneRect(rect);
            m_scene->setSceneRect(QRectF(QPointF(-5000000, -5000000), QPointF(5000000, 5000000)));
            //m_viewer->centerOn(rect.center());
            m_viewer->setTransformationAnchor(QGraphicsView::NoAnchor);
            m_viewer->setAnchorPoint(m_viewer->mapFromScene(QPointF(0, 0)));//NoAnchor以scene的(0, 0)点为坐标原点

            //m_viewer->setResizeAnchor(QGraphicsView::AnchorViewCenter);

            //m_viewer->setTransform(QTransform());
            //m_viewer->centerOn(rect.center());
        }
        //初始化缩放输入
        m_comboBoxScale->setCurrentText("100%");
        //创建网格
        LaserBackgroundItem* backgroundItem = m_scene->backgroundItem();
        if (backgroundItem) {
            backgroundItem->onChangeGrids();
        }

        doc->open();
    }
}
void LaserControllerWindow::showConfigDialog(const QString& title)
{
    ConfigDialog dialog;
    if (!title.isEmpty() && !title.isNull())
        dialog.setCurrentPanel(title);
    dialog.exec();
	//关闭窗口
	if (m_scene) {
		LaserBackgroundItem* backgroudItem = m_scene->backgroundItem();
		if (backgroudItem) {
			backgroudItem->onChangeGrids();
		}
	}
}
//selected items change
void LaserControllerWindow::selectedChange()
{
	//QRectF rect = m_viewer->group()->sceneBoundingRect();

	/*m_posXBox->setValue(rect.left());
	m_posYBox->setValue(rect.top());*/
	
	int size = m_scene->selectedPrimitives().length();
	if (size > 0) {
		QRectF rect = m_viewer->selectedItemsSceneBoundingRect();
		if (rect.width() == 0 && rect.height() == 0) {
			return;
		}
		LaserBackgroundItem* backgroudItem = m_scene->backgroundItem();
		if (!backgroudItem) {
			return;
		}
		QRectF rectReal = QRectF(backgroudItem->mapFromScene(rect.topLeft()), m_scene->backgroundItem()->mapFromScene(rect.bottomRight()));
		qDebug() << rectReal.top();
		qreal x = 0, y = 0, width = 0, height = 0; 
		if (m_unitIsMM) {
			width = Global::pixelsF2mmX(rectReal.width());
			height = Global::pixelsF2mmY(rectReal.height());
		}

		switch (m_selectionOriginalState) {
			case SelectionOriginalTopLeft:{
				if (m_unitIsMM) {
					x = Global::pixelsF2mmX(rectReal.topLeft().x());
					y = Global::pixelsF2mmY(rectReal.topLeft().y());
				}
				break;
			}
			case SelectionOriginalTopCenter: {
				if (m_unitIsMM) {
					x = Global::pixelsF2mmX(rectReal.center().x());
					y = Global::pixelsF2mmY(rectReal.topLeft().y());
				}
				break;
			}
			case SelectionOriginalTopRight: {
				if (m_unitIsMM) {
					x = Global::pixelsF2mmX(rectReal.bottomRight().x());
					y = Global::pixelsF2mmY(rectReal.topLeft().y());
				}
				break;
			}
			
			case SelectionOriginalLeftCenter: {
				if (m_unitIsMM) {
					x = Global::pixelsF2mmX(rectReal.topLeft().x());
					y = Global::pixelsF2mmY(rectReal.center().y());
				}
				break;
			}
			case SelectionOriginalCenter: {
				if (m_unitIsMM) {
					x = Global::pixelsF2mmX(rectReal.center().x());
					y = Global::pixelsF2mmY(rectReal.center().y());
				}
				
				break;
			}
			case SelectionOriginalRightCenter: {
				if (m_unitIsMM) {
					x = Global::pixelsF2mmX(rectReal.bottomRight().x());
					y = Global::pixelsF2mmY(rectReal.center().y());
				}
				break;
			}
			case SelectionOriginalLeftBottom: {
				if (m_unitIsMM) {
					x = Global::pixelsF2mmX(rectReal.topLeft().x());
					y = Global::pixelsF2mmY(rectReal.bottomRight().y());
				}
				break;
			}
			case SelectionOriginalBottomCenter: {
				if (m_unitIsMM) {
					x = Global::pixelsF2mmX(rectReal.center().x());
					y = Global::pixelsF2mmY(rectReal.bottomRight().y());
				}
				break;
			}
			case SelectionOriginalBottomRight: {
				if (m_unitIsMM) {
					x = Global::pixelsF2mmX(rectReal.bottomRight().x());
					y = Global::pixelsF2mmY(rectReal.bottomRight().y());
				}
				break;
			}
											   
		}
		m_posXBox->setValue(x);
		m_posYBox->setValue(y);
		m_widthBox->setValue(width);
		m_heightBox->setValue(height);
		
	}
}
void LaserControllerWindow::selectionPropertyBoxChange()
{
	qreal x = m_posXBox->value();
	qreal y = m_posYBox->value();
	qreal width = m_widthBox->value();
	qreal height = m_heightBox->value();
	
	qreal xScale = m_xRateBox->value() * 0.01;
	qreal yScale = m_yRateBox->value() * 0.01;

	qreal rotate = m_rotateBox->value() / 180 * M_PI;

	if (m_unitIsMM) {
		x = Global::mm2PixelsXF(x);
		y = Global::mm2PixelsYF(y);
		width = Global::mm2PixelsXF(width);
		height = Global::mm2PixelsYF(height);
	}
	
	//repaint 
	m_viewer->resetSelectedItemsGroupRect(QRectF(x, y, width, height), xScale, yScale, rotate, m_selectionOriginalState, m_selectionTranformState);
	m_xRateBox->setValue(100);
	m_yRateBox->setValue(100);
	m_rotateBox->setValue(0);
	m_viewer->viewport()->repaint();
}
void LaserControllerWindow::onSelectionOriginalClicked(bool clicked)
{
	
}
//
void LaserControllerWindow::onUndoStackCleanChanged(bool clean)
{
	/*if (!m_viewer) {
		return;
	}
	QUndoStack* stack = m_viewer->undoStack();
	if (!stack) {
		return;
	}
	if (stack->canUndo()) {
		m_ui->actionUndo->setEnabled(true);
	}
	else {
		m_ui->actionUndo->setEnabled(false);
	}
	
	if (m_viewer->undoStack()->canRedo()) {
		m_ui->actionRedo->setEnabled(true);
	}
	else {
		m_ui->actionRedo->setEnabled(false);
	}*/

}

void LaserControllerWindow::onCanUndoChanged(bool can)
{
	if (m_ui && m_ui->actionUndo) {
		m_ui->actionUndo->setEnabled(can);
	}
	
}

void LaserControllerWindow::onCanRedoChanged(bool can)
{
	if (m_ui && m_ui->actionRedo) {
		m_ui->actionRedo->setEnabled(can);
	}
	
}

void LaserControllerWindow::bindWidgetsProperties()
{
    // actionOpen
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentWorkingState);
    // end actionOpen

    // actionImport
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", false, documentWorkingState);
    // end actionImportSVG

	// actionNew
	BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", true, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", true, documentWorkingState);
	// end actionNew

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

	// actionUndo
	BIND_PROP_TO_STATE(m_ui->actionUndo, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionUndo, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionUndo, "enabled", false, documentWorkingState);
	// end actionUndo

	// actionRedo
	BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentWorkingState);
	// end actionRedo

	// actionZoomIn
	BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", true, documentWorkingState);
	// end actionZoomIn

	// actionZoomOut
	BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", true, documentWorkingState);
	// end actionZoomOut

	// actionZoomToPage
	BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", true, documentWorkingState);
	// end actionZoomToPage

	// actionZoomToSelection
	BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", true, documentWorkingState);
	// end actionZoomToSelection

	// ZoomInput comboBoxScale
	BIND_PROP_TO_STATE(m_comboBoxScale, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_comboBoxScale, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_comboBoxScale, "enabled", true, documentWorkingState);
	// end ZoomInput comboBoxScale

    // actionCloseDocument
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentWorkingState);
    // end actionCloseDocument

	// actionDeletePrimitive
	BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentEmptyState);
	// end actionDeletePrimitive

	// actionCopy
	BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentEmptyState);
	// end actionCopy

	// actionGroup
	BIND_PROP_TO_STATE(m_ui->actionGroup, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionGroup, "enabled", false, documentEmptyState);
	// end actionGroup

	// actionUngroup
	BIND_PROP_TO_STATE(m_ui->actionUngroup, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionUngroup, "enabled", false, documentEmptyState);
	// end actionUngroup

	// actionPaste
	BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", true, documentWorkingState);
	// end actionPaste

	// actionPasteInLine
	BIND_PROP_TO_STATE(m_ui->actionPasteInLine, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionPasteInLine, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionPasteInLine, "enabled", true, documentWorkingState);
	// end actionPasteInLine

	// actionCut
	BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentEmptyState);
	// end actionCut

	// actionDuplication
	BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentEmptyState);
	// end actionDuplication

	// actionMirrorHorizontal
	BIND_PROP_TO_STATE(m_ui->actionMirrorHorizontal, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionMirrorHorizontal, "enabled", false, documentEmptyState);
	//BIND_PROP_TO_STATE(m_ui->actionMirrorHorizontal, "enabled", true, documentSelectionState);
	// end actionMirrorHorizontal

	// actionMirrorVertical
	BIND_PROP_TO_STATE(m_ui->actionMirrorVertical, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionMirrorVertical, "enabled", false, documentEmptyState);
	//BIND_PROP_TO_STATE(m_ui->actionMirrorHorizontal, "enabled", true, documentSelectionState);
	// end actionMirrorVertical

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
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, devicePausedState);
    // end actionConnect

    // actionDisconnect
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, devicePausedState);
    // end actionDisconnect

    // actionMachining
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, devicePausedState);
    // end actionMachining

    // actionPause
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, devicePausedState);
    // end actionPause

    // actionStop
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, devicePausedState);
    // end actionStop

    // actionLaserSpotShot
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, devicePausedState);
    // end actionLaserSpotShot

    // actionLaserCut
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", true, devicePausedState);
    // end actionLaserCut

    // actionLaserMove
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, devicePausedState);
    // end actionLaserMove

    // actionReset
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", false, devicePausedState);
    // end actionReset

    // actionMoveTop
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", false, devicePausedState);
    // end actionMoveTop

    // actionMoveBottom
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", false, devicePausedState);
    // end actionMoveBottom

    // actionMoveLeft
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", false, devicePausedState);
    // end actionMoveLeft

    // actionMoveRight
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", false, devicePausedState);
    // end actionMoveRight

    // actionMoveTopRight
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", false, devicePausedState);
    // end actionMoveTopRight

    // actionMoveTopLeft
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", false, devicePausedState);
    // end actionMoveTopLeft

    // actionMoveBottomLeft
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", false, devicePausedState);
    // end actionMoveBottomLeft

    // actionMoveBottomRight
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", false, devicePausedState);
    // end actionMoveBottomRight

    // actionMoveUp
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", false, devicePausedState);
    // end actionMoveUp

    // actionMoveDown
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", false, devicePausedState);
    // end actionMoveDown

    // actionMoveToOrigin
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", false, devicePausedState);
    // end actionMoveToOrigin

    // toolButtonReadOrigins
    BIND_PROP_TO_STATE(m_buttonReadOrigins, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_buttonReadOrigins, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_buttonReadOrigins, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_buttonReadOrigins, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_buttonReadOrigins, "enabled", false, devicePausedState);
    // end toolButtonReadOrigins

    // toolButtonWriteOrigins
    BIND_PROP_TO_STATE(m_buttonWriteOrigins, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_buttonWriteOrigins, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_buttonWriteOrigins, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_buttonWriteOrigins, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_buttonWriteOrigins, "enabled", false, devicePausedState);
    // end toolButtonWriteOrigins

    // actionSelectionTool
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentWorkingState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", true, documentIdleState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", true, documentSelectionState);
	
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, initState);	
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveEllipseState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveLineState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitivePolygonState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveSplineState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveSplineEditState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveTextState);
	BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentViewDragState);
    // end 
	// actionViewDragTool
	BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", true, documentWorkingState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentIdleState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", true, documentViewDragState);

	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentSelectionState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveEllipseState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveLineState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitivePolygonState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveSplineState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveSplineEditState);
	BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveTextState);
	// end 

    // actionRectangleTool
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentWorkingState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentIdleState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentSelectionState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitiveEllipseState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitiveLineState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", true, documentPrimitiveRectState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitivePolygonState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitiveSplineState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitiveSplineEditState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitiveTextState);
	BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentViewDragState);
    // end actionRectangleTool

	// actionElippseTool
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentWorkingState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentIdleState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentSelectionState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitiveRectState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitiveLineState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", true, documentPrimitiveEllipseState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitivePolygonState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitiveSplineState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitiveSplineEditState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitiveTextState);
	BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentViewDragState);
	// end actionElippseTool

	// actionLineTool
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentWorkingState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentIdleState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentSelectionState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitiveRectState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitiveEllipseState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", true, documentPrimitiveLineState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitivePolygonState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitiveSplineState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitiveSplineEditState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitiveTextState);
	BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentViewDragState);
	// end actionLineTool

	// actionPolygonTool
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentWorkingState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentIdleState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentSelectionState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveRectState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveEllipseState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveLineState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveSplineState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveSplineEditState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", true, documentPrimitivePolygonState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveTextState);
	BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentViewDragState);
	// end actionPolygonTool

	// actionSplineTool
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentWorkingState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentIdleState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentSelectionState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitiveRectState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitiveEllipseState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitiveLineState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitivePolygonState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitiveSplineEditState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", true, documentPrimitiveSplineState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitiveTextState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentViewDragState);
	// end actionSplineTool

	// actionSplineEditTool
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", false, documentWorkingState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentIdleState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentSelectionState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveRectState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveEllipseState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveLineState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitivePolygonState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveSplineState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", true, documentPrimitiveSplineEditState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveTextState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentViewDragState);
	// end actionSplineEditTool

	// actionTextTool
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentWorkingState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentIdleState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentSelectionState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitiveRectState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitiveEllipseState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitiveLineState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitivePolygonState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitiveSplineState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitiveSplineEditState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", true, documentPrimitiveTextState);
	BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentViewDragState);
	// end actionTextTool

	// actionBitmapTool
	BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentWorkingState);
	// end actionBitmapTool

    // actionRemoveLayer
	BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentIdleState);
    // end actionRemoveLayer

    // actionMoveLayerUp
	BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", true, documentIdleState);
    // end actionMoveLayerUp

    // actionMoveLayerDown
	BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", true, documentIdleState);
    // end actionMoveLayerDown

    // actionAnalysisDocument
	BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", true, documentIdleState);
    // end actionAnalysisDocument

    // actionPathOptimization
    //BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, initState);
    //BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentEmptyState);
    //BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", true, documentIdleState);
    // end actionPathOptimization
}

void LaserControllerWindow::showEvent(QShowEvent * event)
{
    if (!m_created)
    {
        m_created = true;
        QTimer::singleShot(100, this, &LaserControllerWindow::windowCreated);
    }
}

void LaserControllerWindow::createNewDocument()
{
	LaserDocument* doc = new LaserDocument(m_scene);
	PageInformation page;
	page.setWidth(Global::mm2PixelsXF(LaserApplication::device->layoutWidth()));
	page.setHeight(Global::mm2PixelsYF(LaserApplication::device->layoutHeight()));
	doc->setPageInformation(page);
	initDocument(doc);
	
	doc->open();
	
}

QString LaserControllerWindow::getCurrentFileName()
{
	QString name = "";
	if (m_fileDirection.isEmpty()) {
		return name;
	}
	QStringList list = m_fileDirection.split("/");
	name = list[list.size() - 1];
	return name;
}

void LaserControllerWindow::documentClose()
{
	m_scene->clearDocument(true);
	m_tableWidgetLayers->updateItems();
	this->m_fileDirection = "";
	this->m_fileName = "";
	this->setWindowTitle(m_windowTitle);
	//layer color buttons not enabled
	for each(LayerButton* button in m_layerButtons) {
		button->setEnabled(false);
		button->setChecked(false);
	}
	
}

QString LaserControllerWindow::getFilename(const QString& title, const QString& filters)
{
    qLogD << "getFilename:" << title << filters;
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    //dialog.setMimeTypeFilters(mime);
    dialog.setNameFilter(filters);
    dialog.setWindowTitle(title);
    if (dialog.exec() == QDialog::Accepted)
        return dialog.selectedFiles().constFirst();
    else
        return "";
}


