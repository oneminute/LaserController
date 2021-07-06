#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"

#include <QErrorMessage>
#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStack>
#include <QtMath>
#include <QScrollBar>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QGridLayout>
#include <QFileDialog> 
#include "scene/LaserPrimitiveGroup.h"

#include "LaserApplication.h"
#include "common/common.h"
#include "common/Config.h"
#include "import/Importer.h"
#include "laser/LaserDevice.h"
#include "scene/LaserDocument.h"
#include "scene/LaserPrimitive.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "state/StateController.h"
#include "ui/ConfigDialog.h"
#include "ui/DeviceSettingsDialog.h"
#include "ui/HalftoneDialog.h"
#include "ui/LaserLayerDialog.h"
#include "ui/MainCardInfoDialog.h"
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
	, m_unitIsMM(true)
	, m_selectionOriginalState(SelectionOriginalCenter)
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

	// 初始化整个工作区。这是一个网格布局的9宫格。

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
    QBoxLayout* viewHoriBottomLayout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
    QRegularExpression percentageRE("^[0-9]+%$");
    QValidator* percentageValidator = new QRegularExpressionValidator(percentageRE, m_comboBoxScale);
    m_comboBoxScale->setValidator(percentageValidator);
    m_comboBoxScale->setMaximumHeight(16);

    viewHoriBottomLayout->setSpacing(0);
    viewHoriBottomLayout->setMargin(0);
    viewHoriBottomLayout->addWidget(m_comboBoxScale);
    viewHoriBottomLayout->addWidget(m_viewer->horizontalScrollBar());
    viewHoriBottomLayout->setStretch(0, 0);
    viewHoriBottomLayout->setStretch(1, 1);
	m_ui->gridLayout->setSpacing(0);
    m_ui->gridLayout->addWidget(const_cast<QScrollBar*>(m_viewer->verticalScrollBar()), 0, 2, 2, 1);
    m_ui->gridLayout->addLayout(viewHoriBottomLayout, 2, 0, 1, 2);
	m_ui->horizontal_ruler->setViewer(m_viewer);
	
	//m_ui->ruler_topleft->setStyleSheet("background:blue");
	m_viewer->setHorizontalRuler(m_ui->horizontal_ruler);
	m_ui->horizontal_ruler->refresh();
	connect(m_viewer, &LaserViewer::zoomChanged, m_ui->horizontal_ruler, &RulerWidget::viewZoomChanged);
	connect(StateControllerInst.documentWorkingState(), &QState::initialStateChanged, m_ui->horizontal_ruler, [=] {
		qDebug() << "ruler_h";
		//m_ui->horizontal_ruler->setScale(1.0);
		//m_ui->horizontal_ruler->refresh();
	});

	m_ui->vertical_ruler->setViewer(m_viewer);
	m_ui->vertical_ruler->setIsVertical(true);
	
	m_viewer->setVerticalRuler(m_ui->vertical_ruler);
	m_ui->vertical_ruler->refresh();
	connect(m_viewer, &LaserViewer::zoomChanged, m_ui->vertical_ruler, &RulerWidget::viewZoomChanged);
	connect(StateControllerInst.documentState(), &QState::initialStateChanged, m_ui->vertical_ruler, [=] {
		qDebug() << "ruler_v";
		//m_ui->vertical_ruler->setScale(1.0);
		//m_ui->vertical_ruler->refresh();
	});
	
    int colorTick = 360 / Config::Layers::maxLayersCount();
    for (int i = 0; i < Config::Layers::maxLayersCount(); i++)
    {
        LayerButton* button = new LayerButton;
        button->setMinimumWidth(Config::Ui::colorButtonWidth());
        button->setFixedHeight(Config::Ui::colorButtonHeight());
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
	removeDockWidget(m_ui->dockWidgetOutline);

    //addDockWidget(Qt::RightDockWidgetArea, m_ui->dockWidgetLayerButtons);
    addDockWidget(Qt::RightDockWidgetArea, m_ui->dockWidgetLayers);
    //splitDockWidget(m_ui->dockWidgetLayerButtons, m_ui->dockWidgetLayers, Qt::Horizontal);
    splitDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetLayerButtons, Qt::Horizontal);
    splitDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetOperations, Qt::Vertical);

    tabifyDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetMovement);
    tabifyDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetProperties);
    tabifyDockWidget(m_ui->dockWidgetLayers, m_ui->dockWidgetOutline);

    m_ui->dockWidgetLayerButtons->show();
    m_ui->dockWidgetLayers->show();
    m_ui->dockWidgetProperties->show();
    m_ui->dockWidgetOperations->show();
    m_ui->dockWidgetOutline->show();

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

    m_ui->toolButtonConnect->setDefaultAction(m_ui->actionConnect);
    m_ui->toolButtonStart->setDefaultAction(m_ui->actionMachining);
    m_ui->toolButtonPause->setDefaultAction(m_ui->actionPause);
    m_ui->actionPause->setCheckable(true);
    m_ui->toolButtonStop->setDefaultAction(m_ui->actionStop);
    m_ui->toolButtonSpotShot->setDefaultAction(m_ui->actionLaserSpotShot);
    m_ui->toolButtonCut->setDefaultAction(m_ui->actionLaserCut);
    m_ui->toolButtonReset->setDefaultAction(m_ui->actionReset);
    m_ui->toolButtonBackToOrigin->setDefaultAction(m_ui->actionMoveToOrigin);
    m_ui->toolButtonPathOptimization->setDefaultAction(m_ui->actionPathOptimization);

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
	m_ui->toolButtonRemoveLayer->setDefaultAction(m_ui->actionRemoveLayer);

    m_ui->editSliderLaserPower->setMinimum(0);
    m_ui->editSliderLaserPower->setMaximum(1000);
    m_ui->editSliderLaserEnergyMin->setMinimum(0);
    m_ui->editSliderLaserEnergyMin->setMaximum(1000);
    m_ui->editSliderLaserEnergyMax->setMinimum(0);
    m_ui->editSliderLaserEnergyMax->setMaximum(1000);

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

    m_ui->tableWidgetParameters->setColumnWidth(0, 100);
    m_ui->tableWidgetParameters->setColumnWidth(1, 300);
	//设置为可选择
    m_ui->actionSelectionTool->setCheckable(true);
	m_ui->actionRectangleTool->setCheckable(true);
	m_ui->actionEllipseTool->setCheckable(true);
	m_ui->actionLineTool->setCheckable(true);
	m_ui->actionPolygonTool->setCheckable(true);
	m_ui->actionSplineTool->setCheckable(true);
	m_ui->actionEditSplineTool->setCheckable(true);
	m_ui->actionTextTool->setCheckable(true);
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

	m_propertyLayout->addWidget(rotateLabel, 0, 11, 2, 1);
	m_propertyLayout->addWidget(m_rotateBox, 0, 12, 2, 1);
	m_propertyLayout->addWidget(m_mmOrIn, 0, 13, 2, 1);

	m_propertyWidget = new QWidget();
	m_propertyWidget->setLayout(m_propertyLayout);
	m_ui->properties->addWidget(m_propertyWidget);
	//m_ui->properties->addWidget(m_posXLabel);
	m_propertyWidget->setEnabled(false);
	//init selection original radio button
	m_centerBtn->setChecked(true);

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
	connect(m_ui->actionNew, &QAction::triggered, this, &LaserControllerWindow::onActionNew);
	connect(m_ui->actionSave, &QAction::triggered, this, &LaserControllerWindow::onActionSave);
	connect(m_ui->actionOpen, &QAction::triggered, this, &LaserControllerWindow::onActionOpen);

    connect(m_ui->actionConnect, &QAction::triggered, this, &LaserControllerWindow::onActionConnect);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &LaserControllerWindow::onActionDisconnect);
    connect(m_ui->actionLoadMotor, &QAction::triggered, this, &LaserControllerWindow::onActionLoadMotor);
    connect(m_ui->actionUnloadMotor, &QAction::triggered, this, &LaserControllerWindow::onActionUnloadMotor);
    connect(m_ui->actionDownload, &QAction::triggered, this, &LaserControllerWindow::onActionDownload);
    connect(m_ui->actionWorkState, &QAction::triggered, this, &LaserControllerWindow::onActionWorkState);
    connect(m_ui->actionHalfTone, &QAction::triggered, this, &LaserControllerWindow::onActionHalfTone);
    connect(m_ui->actionDeletePrimitive, &QAction::triggered, this, &LaserControllerWindow::onActionDeletePrimitive);
    connect(m_ui->actionCloseDocument, &QAction::triggered, this, &LaserControllerWindow::onActionCloseDocument);
    connect(m_ui->actionMoveLayerUp, &QAction::triggered, this, &LaserControllerWindow::onActionMoveLayerUp);
    connect(m_ui->actionMoveLayerDown, &QAction::triggered, this, &LaserControllerWindow::onActionMoveLayerDown);
    connect(m_ui->actionDeviceSettings, &QAction::triggered, this, &LaserControllerWindow::onActionDeviceSettings);
    connect(m_ui->actionSettings, &QAction::triggered, this, &LaserControllerWindow::onActionSettings);

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
	connect(m_ui->actionPolygonTool, &QAction::triggered, this, &LaserControllerWindow::onActionPolygon);
	connect(m_ui->actionSplineTool, &QAction::triggered, this, &LaserControllerWindow::onActionSpline);
	connect(m_ui->actionEditSplineTool, &QAction::triggered, this, &LaserControllerWindow::onActionSplineEdit);
	connect(m_ui->actionTextTool, &QAction::triggered, this, &LaserControllerWindow::onActionText);
	connect(m_ui->actionBitmapTool, &QAction::triggered, this, &LaserControllerWindow::onActionBitmap);

	connect(m_ui->actionShowMainCardInfo, &QAction::triggered, this, &LaserControllerWindow::onActionShowMainCardInfo);
	connect(m_ui->actionTemporaryLicense, &QAction::triggered, this, &LaserControllerWindow::onActionTemporaryLicense);

    connect(m_ui->toolButtonReadOrigins, &QToolButton::clicked, this, &LaserControllerWindow::readMachiningOrigins);
    connect(m_ui->toolButtonWriteOrigins, &QToolButton::clicked, this, &LaserControllerWindow::writeMachiningOrigins);
    connect(m_ui->toolButtonReadPower, &QToolButton::clicked, this, &LaserControllerWindow::readMachiningPower);
    connect(m_ui->toolButtonWritePower, &QToolButton::clicked, this, &LaserControllerWindow::writeMachiningPower);
    connect(m_ui->toolButtonSpotShot, &QToolButton::pressed, this, &LaserControllerWindow::lightOnLaser);
    connect(m_ui->toolButtonSpotShot, &QToolButton::released, this, &LaserControllerWindow::lightOffLaser);
    connect(m_ui->actionReset, &QAction::triggered, this, &LaserControllerWindow::laserResetToOriginalPoint);
    connect(m_ui->actionMoveToOrigin, &QAction::triggered, this, &LaserControllerWindow::laserBackToMachiningOriginalPoint);

    connect(m_ui->tableWidgetLayers, &QTableWidget::cellDoubleClicked, this, &LaserControllerWindow::onTableWidgetLayersCellDoubleClicked);
    connect(m_ui->tableWidgetLayers, &QTableWidget::itemSelectionChanged, this, &LaserControllerWindow::onTableWidgetItemSelectionChanged);
    connect(m_ui->editSliderLaserEnergyMin, &EditSlider::valueChanged, this, &LaserControllerWindow::onEditSliderLaserEngergyMinChanged);
    connect(m_ui->editSliderLaserEnergyMax, &EditSlider::valueChanged, this, &LaserControllerWindow::onEditSliderLaserEngergyMaxChanged);

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
    connect(&LaserDriver::instance(), &LaserDriver::registersFectched, this, &LaserControllerWindow::onLaserRegistersFetched);

    //connect(this, &LaserControllerWindow::windowCreated, this, &LaserControllerWindow::onWindowCreated);
    connect(StateController::instance().deviceUnconnectedState(), &QState::entered, this, &LaserControllerWindow::onEnterDeviceUnconnectedState);
    connect(StateController::instance().deviceConnectedState(), &QState::entered, this, &LaserControllerWindow::onEnterDeviceConnectedState);
	connect(StateController::instance().documentPrimitiveSplineState(), &QState::exited, this, &LaserControllerWindow::onCreatSpline);
	connect(StateController::instance().documentIdleState(), &QState::entered, m_viewer, &LaserViewer::onDocumentIdle);
	
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

	ADD_TRANSITION(documentIdleState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitiveEllipseState, this, SIGNAL(readyEllipse()));

	ADD_TRANSITION(documentIdleState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitiveLineState, this, SIGNAL(readyLine()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitiveLineState, this, SIGNAL(readyLine()));

	ADD_TRANSITION(documentIdleState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentSelectionState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitivePolygonState, this, SIGNAL(readyPolygon()));

	ADD_TRANSITION(documentIdleState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitiveSplineState, this, SIGNAL(readySpline()));

	ADD_TRANSITION(documentIdleState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));
	ADD_TRANSITION(documentPrimitiveTextState, documentPrimitiveSplineEditState, this, SIGNAL(readySplineEdit()));

	ADD_TRANSITION(documentIdleState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentSelectionState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitivePolygonState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveSplineState, documentPrimitiveTextState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentPrimitiveSplineEditState, documentPrimitiveTextState, this, SIGNAL(readyText()));

    ADD_TRANSITION(documentPrimitiveState, documentIdleState, this, SIGNAL(isIdle()));

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
    
    // Get Laser power;
    int power = m_ui->editSliderLaserPower->value();
    
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
    if (m_ui->comboBoxPostEvent->currentIndex() < 3)
    {
        value.setAction(m_ui->comboBoxPostEvent->currentIndex());
    }
    else
    {
        if (m_ui->radioButtonMachiningOrigin1->isChecked())
        {
            value.setAction(3);
        }
        else if (m_ui->radioButtonMachiningOrigin2->isChecked())
        {
            value.setAction(4);
        }
        else if (m_ui->radioButtonMachiningOrigin3->isChecked())
        {
            value.setAction(5);
        }
    }
    value.setRelay(0, m_ui->checkBoxRelay1->isChecked());
    value.setRelay(1, m_ui->checkBoxRelay2->isChecked());
    value.setRelay(2, m_ui->checkBoxRelay3->isChecked());
    qDebug() << value.toString();
    return value;
}

void LaserControllerWindow::setFinishRun(const FinishRun & finishRun)
{
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

void LaserControllerWindow::onActionImportSVG(bool checked)
{
    qLogD << "onActionImportSVG";
    QString filename = getFilename(tr("Open SVG File"), QStringList() << "image/svg+xml" << "image/svg+xml-compressed");
    qLogD << "importing filename is " << filename;
    if (filename.isEmpty())
        return;
    QSharedPointer<Importer> importer = Importer::getImporter(this, Importer::SVG);
    LaserDocument* doc = importer->import(filename, m_scene);
    initDocument(doc);
}
void LaserControllerWindow::onActionNew(bool checked)
{
	createNewDocument();
}

void LaserControllerWindow::onActionSave(bool checked)
{
	//QString name = QFileDialog::getOpenFileName(nullptr, "open image", ".", "Images (*.jpg *.jpeg *.tif *.bmp *.png)");
	QString name = QFileDialog::getSaveFileName(nullptr, "save file", ".", "File(*lc)");
	if (name == "") {
		return;
	}
	if (!name.endsWith(".lc")) {
		name += ".lc";
	}
	qDebug() << name;
	m_scene->document()->save(name);
	
}

void LaserControllerWindow::onActionOpen(bool checked)
{
	QString name = QFileDialog::getOpenFileName(nullptr, "open file", ".", "File(*lc)");
	if (name == "") {
		return;
	}
	//创建document
	createNewDocument();
	m_scene->document()->load(name);
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
    QTableWidgetItem* item = m_ui->tableWidgetLayers->currentItem();
	if (!item)
	{
		return;
	}

    item = m_ui->tableWidgetLayers->item(item->row(), 0);
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
	m_ui->tableWidgetLayers->updateItems();
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
    dialog.selectFile(m_scene->document()->nodeName());
    if (dialog.exec() == QFileDialog::Accepted)
    {
        QString filename = dialog.selectedFiles().constFirst();
        if (!filename.isEmpty() && !filename.isNull())
        {
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
        LaserDriver::instance().startMachining(m_ui->comboBoxStartPosition->currentIndex() == 3);
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
    if (m_ui->comboBoxDevices->count() == 0)
        return;
    QString comName = m_ui->comboBoxDevices->currentText();
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
    QVector3D delta(0, m_ui->doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveBottom(bool checked)
{
    QVector3D delta(0, -m_ui->doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveLeft(bool checked)
{
    QVector3D delta(-m_ui->doubleSpinBoxDistanceX->value(), 0, 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveRight(bool checked)
{
    QVector3D delta(m_ui->doubleSpinBoxDistanceX->value(), 0, 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveTopLeft(bool checked)
{
    QVector3D delta(-m_ui->doubleSpinBoxDistanceX->value(), m_ui->doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveTopRight(bool checked)
{
    QVector3D delta(m_ui->doubleSpinBoxDistanceX->value(), m_ui->doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveBottomLeft(bool checked)
{
    QVector3D delta(-m_ui->doubleSpinBoxDistanceX->value(), -m_ui->doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveBottomRight(bool checked)
{
    QVector3D delta(m_ui->doubleSpinBoxDistanceX->value(), -m_ui->doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveUp(bool checked)
{
    QVector3D delta(0, 0, m_ui->doubleSpinBoxDistanceZ->value());
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveDown(bool checked)
{
    QVector3D delta(0, 0, -m_ui->doubleSpinBoxDistanceZ->value());
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
    if (QMessageBox::Apply == QMessageBox::question(this, tr("Delete primitives?"), tr("Do you want to delete primitives selected?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {
        for (LaserPrimitive* primitive : m_scene->selectedPrimitives())
        {
            m_scene->document()->removePrimitive(primitive);
        }
		m_ui->tableWidgetLayers->updateItems();
		//change state
		m_viewer->onCancelSelected();
    }
}

void LaserControllerWindow::onActionCloseDocument(bool checked)
{
    if (QMessageBox::Apply == QMessageBox::question(this, tr("Close document?"), tr("Do you want to close current document?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {
        m_scene->clearDocument(true);
		m_ui->tableWidgetLayers->updateItems();
    }
}

void LaserControllerWindow::onActionSettings(bool checked)
{
    ConfigDialog dialog;
    dialog.exec();
}

void LaserControllerWindow::onActionDeviceSettings(bool checked)
{
    DeviceSettingsDialog dialog;
    dialog.exec();
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
	m_scene->addLaserPrimitive(bitmap);
	m_viewer->onReplaceGroup(bitmap);
}

void LaserControllerWindow::onDeviceComPortsFetched(const QStringList & ports)
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

void LaserControllerWindow::onEnterDeviceUnconnectedState()
{
    m_ui->toolButtonConnect->setDefaultAction(m_ui->actionConnect);
}

void LaserControllerWindow::onEnterDeviceConnectedState()
{
    m_ui->toolButtonConnect->setDefaultAction(m_ui->actionDisconnect);
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
	if (items.length() == 0) {
		m_propertyWidget->setEnabled(false);
		return;
	}
	else if (items.length() > 0) {
		m_propertyWidget->setEnabled(true);
		selectedChange();
	}
    qLogD << "selected items count: " << items.length();
	/*for (LaserPrimitive* item : items)
    {
		if (items[0]->layer() == nullptr)
			continue;
        int row = items[0]->layer()->row();
        if (row != -1)
            m_ui->tableWidgetLayers->selectRow(row);

        qDebug().nospace().nospace() << "selected primitive: " << item->nodeName();
    }*/
    
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

void LaserControllerWindow::onEditSliderLaserEngergyMinChanged(int value)
{
    if (m_ui->editSliderLaserEnergyMax->value() < value)
    {
        m_ui->editSliderLaserEnergyMax->blockSignals(true);
        m_ui->editSliderLaserEnergyMax->setValue(value);
        m_ui->editSliderLaserEnergyMax->blockSignals(false);
    }
}

void LaserControllerWindow::onEditSliderLaserEngergyMaxChanged(int value)
{
    if (m_ui->editSliderLaserEnergyMin->value() > value)
    {
        m_ui->editSliderLaserEnergyMin->blockSignals(true);
        m_ui->editSliderLaserEnergyMin->setValue(value);
        m_ui->editSliderLaserEnergyMin->blockSignals(false);
    }
}

void LaserControllerWindow::onLaserRegistersFetched(const LaserDriver::RegistersMap & registers)
{
    /*if (registers.contains(LaserDriver::RT_CUTTING_LASER_POWER))
    {
        m_ui->editSliderLaserPower->setValue(registers[LaserDriver::RT_CUTTING_LASER_POWER].toInt());
    }
    if (registers.contains(LaserDriver::RT_MAX_LASER_ENERGY))
    {
        m_ui->editSliderLaserEnergyMax->setValue(registers[LaserDriver::RT_MAX_LASER_ENERGY].toInt());
    }
    if (registers.contains(LaserDriver::RT_MIN_LASER_ENERGY))
    {
        m_ui->editSliderLaserEnergyMin->setValue(registers[LaserDriver::RT_MIN_LASER_ENERGY].toInt());
    }
    if (registers.contains(LaserDriver::RT_CUSTOM_1_X))
    {
        m_ui->doubleSpinBoxOrigin1X->setValue(registers[LaserDriver::RT_CUSTOM_1_X].toDouble());
    }
    if (registers.contains(LaserDriver::RT_CUSTOM_1_Y))
    {
        m_ui->doubleSpinBoxOrigin2Y->setValue(registers[LaserDriver::RT_CUSTOM_1_Y].toDouble());
    }
    if (registers.contains(LaserDriver::RT_CUSTOM_2_X))
    {
        m_ui->doubleSpinBoxOrigin2X->setValue(registers[LaserDriver::RT_CUSTOM_2_X].toDouble());
    }
    if (registers.contains(LaserDriver::RT_CUSTOM_2_Y))
    {
        m_ui->doubleSpinBoxOrigin2Y->setValue(registers[LaserDriver::RT_CUSTOM_2_Y].toDouble());
    }
    if (registers.contains(LaserDriver::RT_CUSTOM_3_X))
    {
        m_ui->doubleSpinBoxOrigin3X->setValue(registers[LaserDriver::RT_CUSTOM_3_X].toDouble());
    }
    if (registers.contains(LaserDriver::RT_CUSTOM_3_Y))
    {
        m_ui->doubleSpinBoxOrigin3Y->setValue(registers[LaserDriver::RT_CUSTOM_3_Y].toDouble());
    }
    if (registers.contains(LaserDriver::RT_LAYOUT_SIZE))
    {
        float width, height;
        LaserDriver::instance().getLayout(width, height);
        m_ui->doubleSpinBoxOrigin1X->setMaximum(width);
        m_ui->doubleSpinBoxOrigin2X->setMaximum(width);
        m_ui->doubleSpinBoxOrigin3X->setMaximum(width);
        m_ui->doubleSpinBoxOrigin1Y->setMaximum(height);
        m_ui->doubleSpinBoxOrigin2Y->setMaximum(height);
        m_ui->doubleSpinBoxOrigin3Y->setMaximum(height);
    }*/
}

void LaserControllerWindow::onLaserReturnWorkState(LaserState state)
{
    m_ui->labelCoordinates->setText(QString("X = %1, Y = %2, Z = %3").arg(state.x, 0, 'g').arg(state.y, 0, 'g').arg(state.z, 0, 'g'));
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

void LaserControllerWindow::readMachiningPower(bool checked)
{
    /*LaserDriver::instance().readSysParamFromCard(QList<int>() 
        << LaserDriver::RT_CUTTING_LASER_POWER
        << LaserDriver::RT_MIN_LASER_ENERGY
        << LaserDriver::RT_MAX_LASER_ENERGY
    );*/
}

void LaserControllerWindow::writeMachiningPower(bool checked)
{
   /* LaserDriver::RegistersMap values;
    values[LaserDriver::RT_CUTTING_LASER_POWER] = m_ui->editSliderLaserPower->value();
    values[LaserDriver::RT_MIN_LASER_ENERGY] = m_ui->editSliderLaserEnergyMin->value();
    values[LaserDriver::RT_MAX_LASER_ENERGY] = m_ui->editSliderLaserEnergyMax->value();
    LaserDriver::instance().writeSysParamToCard(values);*/
}

void LaserControllerWindow::updatePostEventWidgets(int index)
{
    if (index == 3)
    {
        m_ui->radioButtonMachiningOrigin1->setEnabled(true);
        m_ui->radioButtonMachiningOrigin2->setEnabled(true);
        m_ui->radioButtonMachiningOrigin3->setEnabled(true);

        m_ui->doubleSpinBoxOrigin1X->setEnabled(true);
        m_ui->doubleSpinBoxOrigin1Y->setEnabled(true);
        m_ui->doubleSpinBoxOrigin2X->setEnabled(true);
        m_ui->doubleSpinBoxOrigin2Y->setEnabled(true);
        m_ui->doubleSpinBoxOrigin3X->setEnabled(true);
        m_ui->doubleSpinBoxOrigin3Y->setEnabled(true);
    }
    else
    {
        m_ui->radioButtonMachiningOrigin1->setEnabled(false);
        m_ui->radioButtonMachiningOrigin2->setEnabled(false);
        m_ui->radioButtonMachiningOrigin3->setEnabled(false);

        m_ui->doubleSpinBoxOrigin1X->setEnabled(false);
        m_ui->doubleSpinBoxOrigin1Y->setEnabled(false);
        m_ui->doubleSpinBoxOrigin2X->setEnabled(false);
        m_ui->doubleSpinBoxOrigin2Y->setEnabled(false);
        m_ui->doubleSpinBoxOrigin3X->setEnabled(false);
        m_ui->doubleSpinBoxOrigin3Y->setEnabled(false);
    }
}

void LaserControllerWindow::laserBackToMachiningOriginalPoint(bool checked)
{
    QVector3D dest;
    if (m_ui->radioButtonMachiningOrigin1->isChecked())
    {
        dest = QVector3D(m_ui->doubleSpinBoxOrigin1X->value(), m_ui->doubleSpinBoxOrigin1Y->value(), 0.f);
    }
    else if (m_ui->radioButtonMachiningOrigin2->isChecked())
    {
        dest = QVector3D(m_ui->doubleSpinBoxOrigin2X->value(), m_ui->doubleSpinBoxOrigin2Y->value(), 0.f);
    }
    else if (m_ui->radioButtonMachiningOrigin3->isChecked())
    {
        dest = QVector3D(m_ui->doubleSpinBoxOrigin3X->value(), m_ui->doubleSpinBoxOrigin3Y->value(), 0.f);
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
    m_ui->treeWidgetOutline->clear();

    QStack<LaserNode*> stack;
    stack.push(m_scene->document());

    QMap<LaserNode*, QTreeWidgetItem*> nodeItemMap;
    while (!stack.isEmpty())
    {
        LaserNode* node = stack.pop();

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
            m_ui->treeWidgetOutline->addTopLevelItem(item);
        }

        for (LaserNode* childNode : node->childNodes())
        {
            stack.push(childNode);
        }
    }

    m_ui->treeWidgetOutline->expandAll();
}

void LaserControllerWindow::initDocument(LaserDocument* doc)
{
    if (doc)
    {
		connect(m_ui->actionAnalysisDocument, &QAction::triggered, doc, &LaserDocument::analysis);
        connect(doc, &LaserDocument::outlineUpdated, this, &LaserControllerWindow::updateOutlineTree);
        connect(m_ui->actionPathOptimization, &QAction::triggered, doc, &LaserDocument::optimize);
        doc->bindLayerButtons(m_layerButtons);
        m_scene->updateDocument(doc);
        doc->outline();
        m_ui->tableWidgetLayers->setDocument(doc);
        m_ui->tableWidgetLayers->updateItems();
		
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
		QRectF rectReal = QRectF(m_scene->backgroundItem()->mapFromScene(rect.topLeft()), m_scene->backgroundItem()->mapFromScene(rect.bottomRight()));
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
//selection areas change
/*void LaserControllerWindow::

{
	if (m_scene->selectedItems().size() == 0) {
		m_propertyWidget->setEnabled(false);
	}
	else if (m_scene->selectedItems().size() > 0) {
		m_propertyWidget->setEnabled(true);
		selectedChange();
	}
}*/

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

	// actionNew
	BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", true, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, documentWorkingState);
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

    // actionCloseDocument
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentWorkingState);
    // end actionCloseDocument

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
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionLaserCut, "enabled", true, devicePausedState);
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
    BIND_PROP_TO_STATE(m_ui->toolButtonReadOrigins, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->toolButtonReadOrigins, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonReadOrigins, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonReadOrigins, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->toolButtonReadOrigins, "enabled", false, devicePausedState);
    // end toolButtonReadOrigins

    // toolButtonWriteOrigins
    BIND_PROP_TO_STATE(m_ui->toolButtonWriteOrigins, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->toolButtonWriteOrigins, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonWriteOrigins, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonWriteOrigins, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->toolButtonWriteOrigins, "enabled", false, devicePausedState);
    // end toolButtonWriteOrigins

    // toolButtonReadPower
    BIND_PROP_TO_STATE(m_ui->toolButtonReadPower, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->toolButtonReadPower, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonReadPower, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonReadPower, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->toolButtonReadPower, "enabled", false, devicePausedState);
    // end toolButtonReadPower

    // toolButtonWritePower
    BIND_PROP_TO_STATE(m_ui->toolButtonWritePower, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->toolButtonWritePower, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonWritePower, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->toolButtonWritePower, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->toolButtonWritePower, "enabled", false, devicePausedState);
    // end toolButtonWritePower

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
	// end actionPolygonTool

	// actionSplineTool
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", true, documentWorkingState);
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
	// end actionSplineTool

	// actionSplineEditTool
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", true, documentWorkingState);
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
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", true, documentIdleState);
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
	doc->open();
	initDocument(doc);
}

QString LaserControllerWindow::getFilename(const QString& title, const QStringList & mime)
{
    qLogD << "getFilename:" << title << mime;
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters(mime);
    dialog.setWindowTitle(tr("Open SVG File"));
    qLogD << "preparing open dialog";
    if (dialog.exec() == QDialog::Accepted)
        return dialog.selectedFiles().constFirst();
    else
        return "";
}


