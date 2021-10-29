#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"
#include "widget/UndoCommand.h"

#include <DockAreaTabBar.h>
#include <DockAreaTitleBar.h>
#include <DockAreaWidget.h>
#include <DockComponentsFactory.h>
#include <DockContainerWidget.h>
#include <DockSplitter.h>
#include <DockWidgetTab.h>
#include <QFileDialog> 
#include <FloatingDockContainer.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QErrorMessage>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
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
#include <QFontComboBox>
#include <QSize>
#include <QUndoStack>
#include <QPainter>
#include <QtConcurrent>
#include <QDialogButtonBox>

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
#include "scene/LaserPrimitive.h"
#include "state/StateController.h"
#include "task/ProgressModel.h"
#include "ui/ConfigDialog.h"
#include "ui/HalftoneDialog.h"
#include "ui/LaserLayerDialog.h"
#include "ui/MainCardInfoDialog.h"
#include "ui/PreviewWindow.h"
#include "ui/UpdateDialog.h"
#include "ui/ActivationDialog.h"
#include "ui/RegisteDialog.h"
#include "ui/UserInfoDialog.h"
#include "util/ImageUtils.h"
#include "util/Utils.h"
#include "widget/FloatEditDualSlider.h"
#include "widget/FloatEditSlider.h"
#include "widget/LaserLayerTableWidget.h"
#include "widget/LaserViewer.h"
#include "widget/LayerButton.h"
#include "widget/PropertiesHelperManager.h"
#include "widget/RulerWidget.h"
#include "widget/ProgressBar.h"
#include "widget/Vector2DWidget.h"
#include "widget/PressedToolButton.h"
#include "widget/RadioButtonGroup.h"
#include "widget/PointPairTableWidget.h"
#include "widget/Label.h"

#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace ads;

LaserControllerWindow::LaserControllerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::LaserControllerWindow)
    , m_created(false)
    , m_useLoadedJson(false)
	, m_unitIsMM(true)
	, m_selectionOriginalState(SelectionOriginalCenter)
    , m_textFontWidget(nullptr)
    , m_propertyWidget(nullptr)
    , m_lastLockedState(Qt::Unchecked)
    , m_lockEqualRatio(false)
    , m_updateDialog(nullptr)
{
    m_ui->setupUi(this);

    // initialize Dock Manager
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, false);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    m_dockManager = new CDockManager(this);

    createCentralDockPanel();
    createLayersDockPanel();
    createCameraDockPanel();
    createOperationsDockPanel();
    createOutlineDockPanel();
    createMovementDockPanel();
    createShapePropertyDockPanel();
    createLaserPowerDockPanel();
    createPrintAndCutPanel();

    m_dockAreaLayers->setCurrentIndex(0);
    m_dockAreaOperations->setCurrentIndex(0);
    // 更改分割条的粗细
    internal::findParent<QSplitter*>(m_centralDockArea)->setHandleWidth(Config::Ui::splitterHandleWidth());
    internal::findParent<QSplitter*>(m_dockAreaLayers)->setHandleWidth(Config::Ui::splitterHandleWidth());
    internal::findParent<QSplitter*>(m_dockAreaCameras)->setHandleWidth(Config::Ui::splitterHandleWidth());
    internal::findParent<QSplitter*>(m_dockAreaOperations)->setHandleWidth(Config::Ui::splitterHandleWidth());
    internal::findParent<QSplitter*>(m_dockAreaOutline)->setHandleWidth(Config::Ui::splitterHandleWidth());
    internal::findParent<QSplitter*>(m_dockAreaMovement)->setHandleWidth(Config::Ui::splitterHandleWidth());
    internal::findParent<QSplitter*>(m_dockAreaProperty)->setHandleWidth(Config::Ui::splitterHandleWidth());
    for (CDockContainerWidget* container : m_dockManager->dockContainers())
    {
        connect(container, &CDockContainerWidget::dockAreasAdded,
            [=]() {
                qLogD << container;
                for (int i = 0; i < container->dockAreaCount(); i++)
                {
                    internal::findParent<QSplitter*>(container->dockArea(i))->setHandleWidth(Config::Ui::splitterHandleWidth());
                }
            }
        );
    }
    
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
		//button->setEnabled(false);
        m_ui->layoutLayerButtons->addWidget(button);
        m_layerButtons.append(button);

        connect(button, &LayerButton::colorUpdated, m_tableWidgetLayers, &LaserLayerTableWidget::updateItems);
    }
    m_ui->layoutLayerButtons->addStretch();
    //m_ui->actionMirrorAcrossLine->setVisible(false);

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

    // init status bar
    m_statusBarStatus = new QLabel;
    m_statusBarStatus->setText(ltr("Tips"));
    m_statusBarStatus->setMinimumWidth(60);
    m_statusBarStatus->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarStatus);

    m_statusBarRegister = new Label;
    m_statusBarRegister->setText(ltr("Unregistered"));
    m_statusBarRegister->setMinimumWidth(60);
    m_statusBarRegister->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarRegister);
    connect(m_statusBarRegister, &Label::clicked, this, &LaserControllerWindow::onStatusBarRegisterClicked);

    m_statusBarActivation = new Label;
    m_statusBarActivation->setText(ltr("Inactivated"));
    m_statusBarActivation->setMinimumWidth(60);
    m_statusBarActivation->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarActivation);
    connect(m_statusBarActivation, &Label::clicked, this, &LaserControllerWindow::onStatusBarActivationClicked);

    m_statusBarTips = new QLabel;
    m_statusBarTips->setText(ltr("Welcome!"));
    m_statusBarTips->setMinimumWidth(120);
    m_statusBarTips->setAlignment(Qt::AlignHCenter);

    m_statusBarCoordinate = new QLabel;
    m_statusBarCoordinate->setText(tr("0,0"));
    m_statusBarCoordinate->setMinimumWidth(45);
    m_statusBarCoordinate->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarCoordinate);
    
    m_statusBarLocation = new QLabel;
    m_statusBarLocation->setText(ltr("Top Left"));
    m_statusBarLocation->setMinimumWidth(300);
    m_statusBarLocation->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarLocation);

    m_statusBarPageInfo = new QLabel;
    m_statusBarPageInfo->setText(ltr("Page Size(mm): %1x%2")
        .arg(LaserApplication::device->layoutWidth())
        .arg(LaserApplication::device->layoutHeight()));
    m_statusBarPageInfo->setMinimumWidth(150);
    m_statusBarPageInfo->setAlignment(Qt::AlignHCenter);
    m_ui->statusbar->addWidget(m_statusBarPageInfo);

    m_statusBarProgress = new ProgressBar;
    m_statusBarProgress->setMinimum(0);
    m_statusBarProgress->setMaximum(100);
    m_ui->statusbar->addPermanentWidget(m_statusBarProgress);
    connect(m_statusBarProgress, &ProgressBar::clicked, this, &LaserControllerWindow::onProgressBarClicked);

    m_statusBarCopyright = new QLabel;
    m_statusBarCopyright->setText(LaserApplication::applicationName());
    m_statusBarCopyright->setMinimumWidth(80);
    m_statusBarCopyright->setAlignment(Qt::AlignHCenter);
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
	m_posXLabel = new QLabel(tr("X Pos"));
	m_posYLabel = new QLabel(tr("Y Pos"));
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
    m_lockEqualRatio = true;
    m_ui->actionLock->setChecked(true);
    m_ui->actionLock->setIcon(QIcon(":/ui/icons/images/lock.png"));
    connect(m_ui->actionLock, &QAction::triggered, this, [=] {
        if (m_lockOrUnlock->isChecked()) {
            m_lockEqualRatio = true;
            m_ui->actionLock->setIcon(QIcon(":/ui/icons/images/lock.png"));
        }
        else {
            m_lockEqualRatio = false;
            m_ui->actionLock->setIcon(QIcon(":/ui/icons/images/unlock.png"));
        }
        
    });

	m_propertyLayout->addWidget(m_posXLabel, 0, 0);
	m_propertyLayout->addWidget(m_posYLabel, 1, 0);
	m_propertyLayout->addWidget(m_posXBox, 0, 1);
	m_propertyLayout->addWidget(m_posYBox, 1, 1);
	m_propertyLayout->addWidget(m_posXUnit, 0, 2);
	m_propertyLayout->addWidget(m_posYUnit, 1, 2);
	m_propertyLayout->addWidget(m_lockOrUnlock, 0, 3, 2, 1);

	//m_width, m_height
	m_propertyWidthLabel = new QLabel(tr("Width"));
	m_propertyHeightLabel = new QLabel(tr("Height"));
	m_widthBox = new LaserDoubleSpinBox();
	m_widthBox->setMinimum(DBL_MIN);
	m_widthBox->setMaximum(DBL_MAX);
    m_widthBox->setDecimals(3);
	m_heightBox = new LaserDoubleSpinBox();
	m_heightBox->setMinimum(DBL_MIN);
	m_heightBox->setMaximum(DBL_MAX);
	//m_heightBox->setDecimals(3);
	m_heightBox->setDecimals(3);
	m_widthUnit = new QLabel("mm");
	m_heightUnit = new QLabel("mm");

	m_propertyLayout->addWidget(m_propertyWidthLabel, 0, 4);
	m_propertyLayout->addWidget(m_propertyHeightLabel, 1, 4);
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
	m_rotateLabel = new QLabel(tr("Rotate"));
	m_rotateBox = new LaserDoubleSpinBox();
	m_rotateBox->setMinimum(-360.0);
	m_rotateBox->setMaximum(360.0);
	m_rotateBox->setDecimals(1);
	m_mmOrIn = new QToolButton();
	m_ui->actionUnitChange->setText("mm");
	m_mmOrIn->setDefaultAction(m_ui->actionUnitChange);
	m_mmOrIn->setEnabled(false);

	m_propertyLayout->addWidget(m_rotateLabel, 0, 11, 2, 1);
	m_propertyLayout->addWidget(m_rotateBox, 0, 12, 2, 1);
	m_propertyLayout->addWidget(m_mmOrIn, 0, 13, 2, 1);

	m_propertyWidget = new QWidget();
	m_propertyWidget->setLayout(m_propertyLayout);
	m_ui->properties->addWidget(m_propertyWidget);
	m_propertyWidget->setEnabled(false);
	m_centerBtn->setChecked(true);
	//text
    m_textLayout = new QGridLayout();
    m_textLayout->setMargin(0);
    m_textLayout->setSpacing(3);
    m_textFontWidget = new QWidget(this);
    //family m_height
    m_textFontWidget->setLayout(m_textLayout);
    m_ui->textFontBar->addWidget(m_textFontWidget);
    m_fontFamily = new LaserFontComboBox();
    m_fontFamily->setEditable(false);
    m_fontFamily->setCurrentText("Times New Roman");
    //m_fontFamily->setWritingSystem(QFontDatabase::SimplifiedChinese);
    m_fontHeight = new LaserDoubleSpinBox();
    m_fontHeight->setValue(20);
    m_fontHeight->setMinimum(0.01);
    m_fontHeight->setMaximum(2000);
    //align
    m_fontAlignX = new QComboBox(this);
    m_fontAlignY = new QComboBox(this);
    m_fontAlignX->addItem(tr("Left"));
    m_fontAlignX->addItem(tr("Middle"));
    m_fontAlignX->addItem(tr("Right"));
    m_fontAlignX->setCurrentIndex(0);
    m_fontAlignY->addItem(tr("Top"));
    m_fontAlignY->addItem(tr("Middle"));
    m_fontAlignY->addItem(tr("Bottom"));
    m_fontAlignY->setCurrentIndex(1);
    //bold Italic upper
    m_fontBold = new QCheckBox(this);
    m_fontItalic = new QCheckBox(this);
    m_fontUpper = new QCheckBox(this);
    //sapcex , spacey
    m_fontSpaceX = new LaserDoubleSpinBox(this);
    m_fontSpaceY = new LaserDoubleSpinBox(this);
    //layot
    m_textLayout->addWidget(new QLabel(tr("Font")), 0, 0);
    m_textLayout->addWidget(m_fontFamily, 0, 1);
    m_textLayout->addWidget(new QLabel(tr("Height")), 1, 0);
    m_textLayout->addWidget(m_fontHeight, 1, 1);
    m_textLayout->addWidget(new QLabel(tr("Align X")), 0, 2);
    m_textLayout->addWidget(m_fontAlignX, 0, 3);
    m_textLayout->addWidget(new QLabel(tr("Align Y")), 1, 2);
    m_textLayout->addWidget(m_fontAlignY, 1, 3);
    m_textLayout->addWidget(new QLabel(tr("Spacing X")), 0, 4);
    m_textLayout->addWidget(m_fontSpaceX, 0, 5);
    m_textLayout->addWidget(new QLabel(tr("Spacing Y")), 1, 4);
    m_textLayout->addWidget(m_fontSpaceY, 1, 5);
    m_textLayout->addWidget(new QLabel(tr("Bold")), 0, 6);
    m_textLayout->addWidget(m_fontBold, 0, 7);
    m_textLayout->addWidget(new QLabel(tr("Italic")), 0, 8);
    m_textLayout->addWidget(m_fontItalic, 0, 9);
    m_textLayout->addWidget(new QLabel(tr("Upper Case")), 1, 6, 1, 2);
    m_textLayout->addWidget(m_fontUpper, 1, 8);
    //text family
    connect(m_fontFamily, QOverload<int>::of(&QComboBox::highlighted), this, &LaserControllerWindow::onFontComboBoxHighLighted);
    connect(m_viewer, &LaserViewer::creatingText, this, &LaserControllerWindow::onChangeFontComboBoxByEditingText);
    connect(m_fontFamily, QOverload<int>::of(&QComboBox::activated), this, &LaserControllerWindow::onFontComboBoxActived);
    connect(m_fontFamily, &LaserFontComboBox::hidePopupSignal, this, &LaserControllerWindow::onFontComboBoxHidePopup);
    //text m_height
    connect(m_fontHeight, &LaserDoubleSpinBox::enterOrLostFocus, this, &LaserControllerWindow::onFontHeightBoxEnterOrLostFocus);
    //bold italic upper
    connect(m_fontBold, &QCheckBox::stateChanged, this, &LaserControllerWindow::onFontBoldStateChanged);
    connect(m_fontItalic, &QCheckBox::stateChanged, this, &LaserControllerWindow::onFontItalicStateChanged);
    connect(m_fontUpper, &QCheckBox::stateChanged, this, &LaserControllerWindow::onFontUpperStateChanged);
    //space
    connect(m_fontSpaceX, &LaserDoubleSpinBox::enterOrLostFocus, this, &LaserControllerWindow::onFontSpaceXEnterOrLostFocus);
    connect(m_fontSpaceY, &LaserDoubleSpinBox::enterOrLostFocus, this, &LaserControllerWindow::onFontSpaceYEnterOrLostFocus);
    //text widget
    connect(StateController::instance().documentPrimitiveTextState(), &QState::exited, this, [=] {
        
        if (m_textFontWidget) {
            m_textFontWidget->setEnabled(false);
        }
        
    });
    connect(StateController::instance().documentPrimitiveTextState(), &QState::entered, this, [=] {
        if (m_textFontWidget) {
            m_textFontWidget->setEnabled(true);
        }
    });
    //text align
    connect(m_fontAlignX, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LaserControllerWindow::onAlignHBoxChanged);
    connect(m_fontAlignY, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LaserControllerWindow::onAlignVBoxChanged);

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
	connect(m_ui->actionPasteInPlace, &QAction::triggered, this, &LaserControllerWindow::onActionPasteInPlace);
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
    connect(m_ui->actionMirrorAcrossLine, &QAction::triggered, this, &LaserControllerWindow::onActionMirrorACrossLine);

    connect(m_buttonMoveTop, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveTop);
    connect(m_buttonMoveBottom, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveBottom);
    connect(m_buttonMoveLeft, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveLeft);
    connect(m_buttonMoveRight, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveRight);
    connect(m_buttonMoveTopLeft, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveTopLeft);
    connect(m_buttonMoveTopRight, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveTopRight);
    connect(m_buttonMoveBottomLeft, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveBottomLeft);
    connect(m_buttonMoveBottomRight, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveBottomRight);
    connect(m_buttonMoveUp, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveUp);
    connect(m_buttonMoveDown, &PressedToolButton::pressed, this, &LaserControllerWindow::onActionMoveDown);
    connect(m_buttonMoveTop, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);
    connect(m_buttonMoveBottom, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);
    connect(m_buttonMoveLeft, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);
    connect(m_buttonMoveRight, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);
    connect(m_buttonMoveTopLeft, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);
    connect(m_buttonMoveTopRight, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);
    connect(m_buttonMoveBottomLeft, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);
    connect(m_buttonMoveBottomRight, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);
    connect(m_buttonMoveUp, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);
    connect(m_buttonMoveDown, &PressedToolButton::released, this, &LaserControllerWindow::onMovementButtonReleased);

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
    connect(m_ui->actionActivate, &QAction::triggered, this, &LaserControllerWindow::onActionActivate);
    connect(m_ui->actionRegiste, &QAction::triggered, this, &LaserControllerWindow::onActionRegiste);
    connect(m_ui->actionUpdate, &QAction::triggered, this, &LaserControllerWindow::onActionUpdate);
    connect(m_ui->actionLaserPosition, &QAction::triggered, this, &LaserControllerWindow::onActionLaserPosition);

	connect(m_ui->actionMainCardInfo, &QAction::triggered, this, &LaserControllerWindow::onActionMainCardInfo);
	connect(m_ui->actionTemporaryLicense, &QAction::triggered, this, &LaserControllerWindow::onActionTemporaryLicense);
	connect(m_ui->actionUserInfo, &QAction::triggered, this, &LaserControllerWindow::onActionUserInfo);
	connect(m_ui->actionAbout, &QAction::triggered, this, &LaserControllerWindow::onActionAbout);
	connect(m_ui->actionOfficialWebsite, &QAction::triggered, this, &LaserControllerWindow::onActionAbout);
	connect(m_ui->actionOnlineHelp, &QAction::triggered, this, &LaserControllerWindow::onActionAbout);
	connect(m_ui->actionContactUs, &QAction::triggered, this, &LaserControllerWindow::onActionAbout);

    connect(m_ui->actionReset, &QAction::triggered, this, &LaserControllerWindow::laserResetToOriginalPoint);
    connect(m_ui->actionMoveToOrigin, &QAction::triggered, this, &LaserControllerWindow::laserBackToMachiningOriginalPoint);

    connect(m_ui->actionUpdateOutline, &QAction::triggered, this, &LaserControllerWindow::onActionUpdateOutline);
    connect(m_ui->actionFetchToUserOrigin, &QAction::triggered, this, &LaserControllerWindow::onActionFetchToUserOrigin);

    connect(m_ui->actionPrintAndCutNew, &QAction::triggered, this, &LaserControllerWindow::onActionPrintAndCutNew);
    connect(m_ui->actionPrintAndCutFetchLaser, &QAction::triggered, this, &LaserControllerWindow::onActionPrintAndCutFetchLaser);
    connect(m_ui->actionPrintAndCutFetchCanvas, &QAction::triggered, this, &LaserControllerWindow::onActionPrintAndCutFetchCanvas);
    connect(m_ui->actionPrintAndCutRemove, &QAction::triggered, this, &LaserControllerWindow::onActionPrintAndCutRemove);
    connect(m_ui->actionPrintAndCutClear, &QAction::triggered, this, &LaserControllerWindow::onActionPrintAndCutClear);
    connect(m_ui->actionPrintAndCutAlign, &QAction::triggered, this, &LaserControllerWindow::onActionPrintAndCutAlign);
    connect(m_ui->actionPrintAndCutRestore, &QAction::triggered, this, &LaserControllerWindow::onActionPrintAndCutRestore);

    connect(m_scene, &LaserScene::selectionChanged, this, &LaserControllerWindow::onLaserSceneSelectedChanged);
    
    connect(m_scene, QOverload<QGraphicsItem *, QGraphicsItem *, Qt::FocusReason>::of(&LaserScene::focusItemChanged), 
        this, &LaserControllerWindow::onLaserSceneFocusItemChanged);
    connect(m_viewer, &LaserViewer::mouseMoved, this, &LaserControllerWindow::onLaserViewerMouseMoved);
    connect(m_viewer, &LaserViewer::scaleChanged, this, &LaserControllerWindow::onLaserViewerScaleChanged);
    connect(m_comboBoxScale, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LaserControllerWindow::onComboBoxSxaleIndexChanged);
    connect(m_comboBoxScale, &QComboBox::currentTextChanged, this, &LaserControllerWindow::onComboBoxSxaleTextChanged);

    connect(LaserApplication::device, &LaserDevice::comPortsFetched, this, &LaserControllerWindow::onDeviceComPortsFetched);
    connect(LaserApplication::device, &LaserDevice::connected, this, &LaserControllerWindow::onDeviceConnected);
    connect(LaserApplication::device, &LaserDevice::disconnected, this, &LaserControllerWindow::onDeviceDisconnected);
    connect(LaserApplication::device, &LaserDevice::mainCardRegistrationChanged, this, &LaserControllerWindow::onMainCardRegistrationChanged);
    connect(LaserApplication::device, &LaserDevice::mainCardActivationChanged, this, &LaserControllerWindow::onMainCardActivationChanged);
    connect(LaserApplication::device, &LaserDevice::workStateUpdated, this, &LaserControllerWindow::onLaserReturnWorkState);
    connect(LaserApplication::device, &LaserDevice::layoutChanged, this, &LaserControllerWindow::onLayoutChanged);

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
		selectionPropertyBoxChange(PrimitiveProperty::PP_PosX);
	});
	connect(m_posYBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_MOVE;
		if (m_posYBox->value() > 9000) {
			m_posYBox->setValue(9000);
		}
		if (m_posYBox->value() < -9000) {
			m_posYBox->setValue(-9000);
		}
		selectionPropertyBoxChange(PrimitiveProperty::PP_PosY);
	});
	connect(m_widthBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
        if (m_viewer->selectedItemsSceneBoundingRect().width() == 0) {
            m_widthBox->setValue(0);
            return;
        }
		m_selectionTranformState = SelectionTransformType::Transform_RESIZE;
		if (m_widthBox->value() > 20000) {
			m_widthBox->setValue(20000);
		}
		if (m_widthBox->value() <= 0) {
			m_widthBox->setValue(0.001);
		}
        
		selectionPropertyBoxChange(PrimitiveProperty::PP_Width);
	});
	connect(m_heightBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
        if (m_viewer->selectedItemsSceneBoundingRect().height() == 0) {
            m_heightBox->setValue(0);
            return;
        }
		m_selectionTranformState = SelectionTransformType::Transform_RESIZE;
		if (m_heightBox->value() > 20000) {
			m_heightBox->setValue(20000);
		}
		if (m_heightBox->value() <= 0) {
			m_heightBox->setValue(0.001);
		}
        
		selectionPropertyBoxChange(PrimitiveProperty::PP_Height);
	});
	connect(m_xRateBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_SCALE;
		if (m_xRateBox->value() <= 0) {
			m_xRateBox->setValue(0.001);
		}
		selectionPropertyBoxChange(PP_ScaleX);
	});
	connect(m_yRateBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_SCALE;
		if (m_yRateBox->value() <= 0) {
			m_yRateBox->setValue(0.001);
		}
		selectionPropertyBoxChange(PrimitiveProperty::PP_ScaleY);
	});
	//rotate
	connect(m_rotateBox, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
		m_selectionTranformState = SelectionTransformType::Transform_ROTATE;
		selectionPropertyBoxChange(PP_Other);
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
	
    // config items
    connect(Config::Ui::autoRepeatDelayItem(), &ConfigItem::valueChanged, this, &LaserControllerWindow::updateAutoRepeatDelayChanged);

    connect(LaserApplication::progressModel, &ProgressModel::progressUpdated, m_statusBarProgress, QOverload<qreal>::of(&ProgressBar::setValue));
    connect(LaserApplication::app, &LaserApplication::languageChanged, this, &LaserControllerWindow::retranslate);

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
    ADD_TRANSITION(documentPrimitiveTextState, documentViewDragState, this, SIGNAL(readyViewDrag()));

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
    m_textFontWidget = nullptr;
    
	if (m_viewer->undoStack()) {
		delete m_viewer->undoStack();
	}
    m_viewer = nullptr;

    SAFE_DELETE(m_updateDialog)
}

void LaserControllerWindow::moveLaser(const QVector3D& delta, bool relative, const QVector3D& abstractDest)
{
    if (!LaserApplication::driver)
    {
        QMessageBox::warning(this, tr("Operate failure"), tr("Laser device is not connected!"));
        return;
    }

    QVariant value(0);

    QUADRANT quad = static_cast<QUADRANT>(value.toInt());

    qLogD << "move by: " << delta;

    if (relative)
    {
        LaserApplication::device->moveBy(delta, Config::Device::xEnabled(), Config::Device::yEnabled(), Config::Device::zEnabled());
    }
    else
    {
        // Get current pos;
        QVector3D dest = utils::putToQuadrant(abstractDest, quad);
        QVector3D pos = utils::putToQuadrant(LaserApplication::driver->getCurrentLaserPos(), quad);
        dest = pos + delta;
        LaserApplication::device->moveTo(dest, quad, Config::Device::xEnabled(), Config::Device::yEnabled(), Config::Device::zEnabled());
    }
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
        if (m_radioButtonUserOrigin1->isChecked())
        {
            value.setAction(3);
        }
        else if (m_radioButtonUserOrigin2->isChecked())
        {
            value.setAction(4);
        }
        else if (m_radioButtonUserOrigin3->isChecked())
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
        break;
    }
    }
}

void LaserControllerWindow::onFontComboBoxHighLighted(int index)
{
    if (m_viewer) {
        QString family = m_fontFamily->itemText(index);
        m_viewer->textFont()->setFamily(family);
        qDebug() << m_fontFamily->itemData(index);
        LaserText* text = m_viewer->editingText();
        if (text) {
            QFont font(text->font());
            font.setFamily(family);
            text->setFont(font);
            
        }
        m_fontComboxLightedIndex = index;
        m_viewer->modifyTextCursor();
        m_viewer->viewport()->repaint();
    }
}

void LaserControllerWindow::onFontComboBoxActived(int index)
{
    m_viewer->setFocus();
    if (m_fontComboxLightedIndex != index) {
        onFontComboBoxHighLighted(index);
    }
    
    //m_fontFamily->setIsChangedItem(true);
}

void LaserControllerWindow::onAlignHBoxChanged(int index)
{
    int align;
    switch (index) {
        case 0: {
            align = Qt::AlignLeft;
            break;
        }
        case 1: {
            align = Qt::AlignHCenter;
            break;
        }
        
        case 2: {
            align = Qt::AlignRight;
            break;
        }

    }
    m_viewer->setTextAlignH(align);
    if (m_viewer->editingText()) {
        m_viewer->editingText()->setAlignH(align);
        m_viewer->editingText()->modifyPathList();
    }
    m_viewer->modifyTextCursor();
    m_viewer->viewport()->repaint();
    m_viewer->setFocus();
}

void LaserControllerWindow::onAlignVBoxChanged(int index)
{
    int align;
    switch (index) {
    case 0: {
        align = Qt::AlignTop;
        break;
    }
    case 1: {
        align = Qt::AlignVCenter;
        break;
    }

    case 2: {
        align = Qt::AlignBottom;
        break;
    }

    }
    m_viewer->setTextAlignV(align);
    if (m_viewer->editingText()) {
        m_viewer->editingText()->setAlignV(align);
        m_viewer->editingText()->modifyPathList();
    }
    m_viewer->modifyTextCursor();
    m_viewer->viewport()->repaint();
    m_viewer->setFocus();
}

void LaserControllerWindow::onChangeFontComboBoxByEditingText()
{
    if (m_viewer->editingText()) {
        LaserText* text = m_viewer->editingText();
        //family
        QFont font = text->font();
        qreal t = font.pixelSize();
        m_fontFamily->setCurrentText(font.family());
        //m_height
        m_fontHeight->setValue(Global::pixelsF2mmY(font.pixelSize()));
        //align
        int aV = text->alignV();
        int aH = text->alignH();
        switch (aV) {
            case Qt::AlignTop: {
                m_fontAlignY->setCurrentIndex(0);
                break;
            }
            case Qt::AlignVCenter: {
                m_fontAlignY->setCurrentIndex(1);
                break;
            }
            case Qt::AlignBottom: {
                m_fontAlignY->setCurrentIndex(2);
                break;
            }
        }
        switch (aH) {
            case Qt::AlignLeft: {
                m_fontAlignX->setCurrentIndex(0);
                break;
            }
            case Qt::AlignHCenter: {
                m_fontAlignX->setCurrentIndex(1);
                break;
            }
            case Qt::AlignRight: {
                m_fontAlignX->setCurrentIndex(2);
                break;
            }
        }
        //bold italic  upper
        bool bold = font.bold();
        m_fontBold->setChecked(bold);
        bool italic = font.italic();
        m_fontItalic->setChecked(italic);
        QFont::Capitalization capitalization = font.capitalization();
        if (capitalization == QFont::AllUppercase) {
            m_fontUpper->setChecked(true);
        }
        else if (capitalization == QFont::MixedCase) {
            m_fontUpper->setChecked(false);
        }  
        //space
        qreal spaceX = font.letterSpacing();
        qreal spaceY = font.wordSpacing();
        m_fontSpaceX->setValue(spaceX);
        m_fontSpaceY->setValue(spaceY);

        m_viewer->textFont()->setFamily(font.family());
        m_viewer->textFont()->setPixelSize(font.pixelSize());
        m_viewer->textFont()->setBold(bold);
        m_viewer->textFont()->setItalic(italic);
        m_viewer->textFont()->setCapitalization(capitalization);
        m_viewer->textFont()->setLetterSpacing(QFont::AbsoluteSpacing,  spaceX);
        m_viewer->textFont()->setWordSpacing(spaceY);
        m_viewer->setTextAlignH(aH);
        m_viewer->setTextAlignV(aV);
    }
}

void LaserControllerWindow::onFontComboBoxHidePopup()
{
    //qDebug() << m_fontFamily->currentText();

    onFontComboBoxHighLighted(m_fontFamily->currentIndex());
    
    
    /*if (m_viewer) {
        QString family = m_fontFamily->currentText();
        m_viewer->textFont()->setFamily(family);
        LaserText* text = m_viewer->editingText();
        if (text) {
            QFont font(text->font());
            font.setFamily(family);
            text->setFont(font);
            m_viewer->modifyTextCursor();
        }

        m_viewer->viewport()->repaint();
    }*/
}

void LaserControllerWindow::onFontHeightBoxEnterOrLostFocus()
{
    if (m_viewer) {
        qreal size = Global::mm2PixelsYF(m_fontHeight->value());
        m_viewer->textFont()->setPixelSize(size);
        LaserText* text = m_viewer->editingText();
        if (text) {
            QFont font(text->font());
            //qreal q = m_fontHeight->value();
            font.setPixelSize(size);
            m_viewer->editingText()->setFont(font);
        }
        m_viewer->modifyTextCursor();
        m_viewer->setFocus();
        m_viewer->viewport()->repaint();
    }
}

void LaserControllerWindow::onFontBoldStateChanged()
{
    if (!m_viewer) {
        return;
    }
    LaserText* text = m_viewer->editingText();
    QFont font(text->font());
    if (m_fontBold->isChecked()) {  
        if (text) {
            font.setBold(true);
            m_viewer->editingText()->setFont(font);
            
        }     
        m_viewer->textFont()->setBold(true);
    }
    else {
        if (text) {
            font.setBold(false);
            m_viewer->editingText()->setFont(font);
        }
        m_viewer->textFont()->setBold(false);
    }
    m_viewer->setFocus();
    m_viewer->modifyTextCursor();
    m_viewer->viewport()->repaint();
}

void LaserControllerWindow::onFontItalicStateChanged()
{
    if (!m_viewer) {
        return;
    }
    LaserText* text = m_viewer->editingText();
    QFont font(text->font());
    if (m_fontItalic->isChecked()) {
        if (text) {
            font.setItalic(true);
            m_viewer->editingText()->setFont(font);

        }
        m_viewer->textFont()->setItalic(true);
    }
    else {
        if (text) {
            font.setItalic(false);
            m_viewer->editingText()->setFont(font);
        }
        m_viewer->textFont()->setItalic(false);
    }
    m_viewer->setFocus();
    m_viewer->modifyTextCursor();
    m_viewer->viewport()->repaint();
}

void LaserControllerWindow::onFontUpperStateChanged()
{
    if (!m_viewer) {
        return;
    }
    LaserText* text = m_viewer->editingText();
    QFont font(text->font());
    if (m_fontUpper->isChecked()) {
        if (text) {
            font.setCapitalization(QFont::AllUppercase);
            m_viewer->editingText()->setFont(font);

        }
        m_viewer->textFont()->setCapitalization(QFont::AllUppercase);
    }
    else {
        if (text) {
            font.setCapitalization(QFont::MixedCase);
            m_viewer->editingText()->setFont(font);
        }
        m_viewer->textFont()->setCapitalization(QFont::MixedCase);
    }
    m_viewer->setFocus();
    m_viewer->modifyTextCursor();
    m_viewer->viewport()->repaint();
}

void LaserControllerWindow::onFontSpaceXEnterOrLostFocus()
{
    QFont font = m_viewer->editingText()->font();
    font.setLetterSpacing(QFont::AbsoluteSpacing, m_fontSpaceX->value());
    m_viewer->editingText()->setFont(QFont(font));
    m_viewer->textFont()->setLetterSpacing(QFont::AbsoluteSpacing, m_fontSpaceX->value());

    m_viewer->setFocus();
    m_viewer->modifyTextCursor();
    m_viewer->viewport()->repaint();
}

void LaserControllerWindow::onFontSpaceYEnterOrLostFocus()
{
    if (!m_viewer->editingText()) {
        return;
    }
    //QFont font = m_viewer->editingText()->font();
    //font.setWordSpacing(m_fontSpaceY->value());
    qreal space = m_fontSpaceY->value();
    m_viewer->editingText()->setSpacceY(space);
    m_viewer->editingText()->modifyPathList();
    //m_viewer->textFont()->setWordSpacing(m_fontSpaceY->value());

    m_viewer->setFocus();
    m_viewer->modifyTextCursor();
    m_viewer->viewport()->repaint();
}

void LaserControllerWindow::createCentralDockPanel()
{
    QWidget* topLeftRuler = new QWidget;

    m_viewer = new LaserViewer(this);
    m_viewer->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_viewer->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //设置初始值
    m_viewer->textFont()->setFamily("Times New Roman");
    m_viewer->textFont()->setPixelSize(Global::mm2PixelsYF(20.0));
    m_viewer->setTextAlignH(Qt::AlignLeft);
    m_viewer->setTextAlignV(Qt::AlignVCenter);
    m_viewer->textFont()->setBold(false);
    m_viewer->textFont()->setLetterSpacing(QFont::SpacingType::AbsoluteSpacing, 0);
    m_viewer->textFont()->setWordSpacing(0);
    
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
		//qDebug() << "ruler_h";
	});

    m_vRuler = new RulerWidget;
    m_vRuler->setViewer(m_viewer);
	m_vRuler->setIsVertical(true);
	m_viewer->setVerticalRuler(m_vRuler);
	m_vRuler->refresh();
	connect(m_viewer, &LaserViewer::zoomChanged, m_vRuler, &RulerWidget::viewZoomChanged);
	connect(StateControllerInst.documentState(), &QState::initialStateChanged, m_vRuler, [=] {
		//qDebug() << "ruler_v";
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
    m_centralDockArea = m_dockManager->setCentralWidget(centralDockWidget);
    m_centralDockArea->setAllowedAreas(DockWidgetArea::OuterDockAreas);
}

void LaserControllerWindow::createLayersDockPanel()
{
    m_tableWidgetLayers = new LaserLayerTableWidget;
    connect(m_tableWidgetLayers, &QTableWidget::cellDoubleClicked, this, &LaserControllerWindow::onTableWidgetLayersCellDoubleClicked);
    connect(m_tableWidgetLayers, &QTableWidget::itemSelectionChanged, this, &LaserControllerWindow::onTableWidgetItemSelectionChanged);

    m_buttonMoveLayerUp = new QToolButton;
    m_buttonMoveLayerUp->setDefaultAction(m_ui->actionMoveLayerUp);
    m_buttonMoveLayerUp->setAutoRepeat(true);
    m_buttonMoveLayerUp->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonMoveLayerDown = new QToolButton;
    m_buttonMoveLayerDown->setDefaultAction(m_ui->actionMoveLayerDown);
    m_buttonMoveLayerDown->setAutoRepeat(true);
    m_buttonMoveLayerDown->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonRemoveLayer = new QToolButton;
    m_buttonRemoveLayer->setDefaultAction(m_ui->actionRemoveLayer);
    m_buttonRemoveLayer->setAutoRepeat(true);
    m_buttonRemoveLayer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->setMargin(0);
    buttonsLayout->addStretch(0);
    buttonsLayout->addWidget(m_buttonMoveLayerUp, Qt::AlignCenter);
    buttonsLayout->addWidget(m_buttonMoveLayerDown, Qt::AlignCenter);
    buttonsLayout->addWidget(m_buttonRemoveLayer, Qt::AlignCenter);
    buttonsLayout->setAlignment(Qt::AlignHCenter);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(3);
    layout->addWidget(m_tableWidgetLayers, 1);
    layout->addLayout(buttonsLayout, 0);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Layers"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockLayers = dockWidget;
    m_dockAreaLayers = m_dockManager->addDockWidget(RightDockWidgetArea, dockWidget);
    dockPanelOnlyShowIcon(dockWidget, QPixmap(":/ui/icons/images/layer.png"), "Layers");
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
    layout->setMargin(3);
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
    CDockWidget* dockWidget = new CDockWidget(tr("Cameras"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());
    m_dockCameras = dockWidget;
    m_dockAreaCameras = m_dockManager->addDockWidget(CenterDockWidgetArea, dockWidget, m_dockAreaLayers);
    //只显示tab中icon
    dockPanelOnlyShowIcon(dockWidget, QPixmap(":/ui/icons/images/camera.png"), "Cameras");
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

    m_comboBoxStartPosition = InputWidgetWrapper::createWidget<QComboBox*>(Config::Device::startFromItem());
    Config::Device::startFromItem()->bindWidget(m_comboBoxStartPosition);

    m_radioButtonGroupJobOrigin = InputWidgetWrapper::createWidget<RadioButtonGroup*>(Config::Device::jobOriginItem());
    Config::Device::jobOriginItem()->bindWidget(m_radioButtonGroupJobOrigin);
    int index = Config::Device::startFrom();
    switch (index)
    {
    case SFT_CurrentPosition:
    case SFT_UserOrigin:
        m_radioButtonGroupJobOrigin->setEnabled(true);
        m_radioButtonGroupJobOrigin->setRowsCols(3, 3);
        break;
    case SFT_AbsoluteCoords:
        m_radioButtonGroupJobOrigin->setEnabled(false);
        break;
    }

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

    m_comboBoxPostEvent = new QComboBox;
    m_comboBoxPostEvent->addItem(tr("Stop at current position"));
    m_comboBoxPostEvent->addItem(tr("Unload motor"));
    m_comboBoxPostEvent->addItem(tr("Back to mechnical origin"));

    QFormLayout* fifthRow = new QFormLayout;
    fifthRow->setMargin(0);
    fifthRow->addRow(Config::Device::startFromItem()->title(), m_comboBoxStartPosition);
    fifthRow->addRow(Config::Device::jobOriginItem()->title(), m_radioButtonGroupJobOrigin);
    fifthRow->addRow(tr("Post Event"), m_comboBoxPostEvent);

    QHBoxLayout* sixthRow = new QHBoxLayout;
    sixthRow->setMargin(0);
    sixthRow->addWidget(labelDevices);
    sixthRow->addWidget(m_comboBoxDevices);
    sixthRow->addWidget(m_buttonConnect);
    sixthRow->addWidget(m_buttonRefresh);
    sixthRow->setStretch(0, 0);
    sixthRow->setStretch(1, 1);
    sixthRow->setStretch(2, 0);
    sixthRow->setStretch(3, 0);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(3);
    layout->addLayout(firstRow);
    layout->addLayout(secondRow);
    layout->addLayout(fifthRow);
    layout->addLayout(sixthRow);
    layout->addStretch(1);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Operations"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockOperations = dockWidget;
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
    layout->setMargin(3);
    layout->addWidget(m_treeWidgetOutline);
    layout->addLayout(toolsLayout);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Outline"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockOutline = dockWidget;
    m_dockAreaOutline = m_dockManager->addDockWidget(CenterDockWidgetArea, dockWidget, m_dockAreaLayers);
    dockPanelOnlyShowIcon(dockWidget, QPixmap(":/ui/icons/images/outline.png"), "Outline");
}

void LaserControllerWindow::createMovementDockPanel()
{
    m_checkBoxXEnabled = new QCheckBox;
    m_checkBoxXEnabled->setText(tr("X Enabled"));
    Config::Device::xEnabledItem()->bindWidget(m_checkBoxXEnabled);

    m_checkBoxYEnabled = new QCheckBox;
    m_checkBoxYEnabled->setText(tr("Y Enabled"));
    Config::Device::yEnabledItem()->bindWidget(m_checkBoxYEnabled);

    m_checkBoxZEnabled = new QCheckBox;
    m_checkBoxZEnabled->setText(tr("Z Enabled"));
    Config::Device::zEnabledItem()->bindWidget(m_checkBoxZEnabled);

    m_lineEditCoordinatesX = new QLineEdit;
    m_lineEditCoordinatesX->setReadOnly(true);
    m_lineEditCoordinatesX->setText(QString::number(0.0, 'f', 2));

    m_lineEditCoordinatesY = new QLineEdit;
    m_lineEditCoordinatesY->setReadOnly(true);
    m_lineEditCoordinatesY->setText(QString::number(0.0, 'f', 2));

    m_lineEditCoordinatesZ = new QLineEdit;
    m_lineEditCoordinatesZ->setReadOnly(true);
    m_lineEditCoordinatesZ->setText(QString::number(0.0, 'f', 2));

    m_doubleSpinBoxDistanceX = new QDoubleSpinBox;
    m_doubleSpinBoxDistanceX->setDecimals(1);
    m_doubleSpinBoxDistanceX->setValue(10.0);
    m_doubleSpinBoxDistanceX->setMinimum(0);
    m_doubleSpinBoxDistanceX->setMaximum(1000);

    m_doubleSpinBoxDistanceY = new QDoubleSpinBox;
    m_doubleSpinBoxDistanceY->setDecimals(1);
    m_doubleSpinBoxDistanceY->setValue(10.0);
    m_doubleSpinBoxDistanceY->setMinimum(0);
    m_doubleSpinBoxDistanceY->setMaximum(1000);

    m_doubleSpinBoxDistanceZ = new QDoubleSpinBox;
    m_doubleSpinBoxDistanceZ->setDecimals(1);
    m_doubleSpinBoxDistanceZ->setValue(10.0);
    m_doubleSpinBoxDistanceZ->setMinimum(0);
    m_doubleSpinBoxDistanceZ->setMaximum(1000);

    QGridLayout* firstRow = new QGridLayout;
    firstRow->setMargin(0);
    firstRow->addWidget(m_checkBoxXEnabled, 0, 1, 1, 2);
    firstRow->addWidget(m_checkBoxYEnabled, 0, 3, 1, 2);
    firstRow->addWidget(m_checkBoxZEnabled, 0, 5, 1, 2);
    firstRow->addWidget(new QLabel(tr("Coordinates")), 1, 0);
    firstRow->addWidget(new QLabel(tr("X")), 1, 1);
    firstRow->addWidget(m_lineEditCoordinatesX, 1, 2);
    firstRow->addWidget(new QLabel(tr("Y")), 1, 3);
    firstRow->addWidget(m_lineEditCoordinatesY, 1, 4);
    firstRow->addWidget(new QLabel(tr("Z")), 1, 5);
    firstRow->addWidget(m_lineEditCoordinatesZ, 1, 6);
    firstRow->addWidget(new QLabel(tr("Distance(mm)")), 2, 0);
    firstRow->addWidget(new QLabel(tr("X")), 2, 1);
    firstRow->addWidget(m_doubleSpinBoxDistanceX, 2, 2);
    firstRow->addWidget(new QLabel(tr("Y")), 2, 3);
    firstRow->addWidget(m_doubleSpinBoxDistanceY, 2, 4);
    firstRow->addWidget(new QLabel(tr("Z")), 2, 5);
    firstRow->addWidget(m_doubleSpinBoxDistanceZ, 2, 6);
    firstRow->setColumnStretch(0, 1);
    firstRow->setColumnStretch(1, 0);
    firstRow->setColumnStretch(2, 1);
    firstRow->setColumnStretch(3, 0);
    firstRow->setColumnStretch(4, 1);
    firstRow->setColumnStretch(5, 0);
    firstRow->setColumnStretch(6, 1);

    m_buttonMoveTopLeft = new PressedToolButton;
    m_buttonMoveTopLeft->setDefaultAction(m_ui->actionMoveTopLeft);

    m_buttonMoveTop = new PressedToolButton;
    m_buttonMoveTop->setDefaultAction(m_ui->actionMoveTop);

    m_buttonMoveTopRight = new PressedToolButton;
    m_buttonMoveTopRight->setDefaultAction(m_ui->actionMoveTopRight);

    m_buttonMoveLeft = new PressedToolButton;
    m_buttonMoveLeft->setDefaultAction(m_ui->actionMoveLeft);

    m_buttonMoveToOrigin = new QToolButton;
    m_buttonMoveToOrigin->setDefaultAction(m_ui->actionMoveToOrigin);

    m_buttonMoveRight = new PressedToolButton;
    m_buttonMoveRight->setDefaultAction(m_ui->actionMoveRight);

    m_buttonMoveBottomLeft = new PressedToolButton;
    m_buttonMoveBottomLeft->setDefaultAction(m_ui->actionMoveBottomLeft);

    m_buttonMoveBottom = new PressedToolButton;
    m_buttonMoveBottom->setDefaultAction(m_ui->actionMoveBottom);

    m_buttonMoveBottomRight = new PressedToolButton;
    m_buttonMoveBottomRight->setDefaultAction(m_ui->actionMoveBottomRight);

    m_buttonMoveUp = new PressedToolButton;
    m_buttonMoveUp->setAutoRepeat(true);
    m_buttonMoveUp->setDefaultAction(m_ui->actionMoveUp);

    m_buttonMoveDown = new PressedToolButton;
    m_buttonMoveDown->setAutoRepeat(true);
    m_buttonMoveDown->setDefaultAction(m_ui->actionMoveDown);

    m_buttonLaserPosition = new QToolButton;
    m_buttonLaserPosition->setDefaultAction(m_ui->actionLaserPosition);

    updateAutoRepeatDelayChanged(Config::Ui::autoRepeatDelay(), MB_Manual);
    //updateAutoRepeatIntervalChanged(Config::Ui::autoRepeatInterval(), MB_Manual);

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
    secondRow->addWidget(m_buttonLaserPosition, 3, 0, 1, 4);

    m_radioButtonUserOrigin1 = new QRadioButton(tr("User Origin 1"));
    m_radioButtonUserOrigin2 = new QRadioButton(tr("User Origin 2"));
    m_radioButtonUserOrigin3 = new QRadioButton(tr("User Origin 3"));
    m_radioButtonUserOrigin1->setProperty("origin", 0);
    m_radioButtonUserOrigin2->setProperty("origin", 1);
    m_radioButtonUserOrigin3->setProperty("origin", 2);
    connect(m_radioButtonUserOrigin1, &QRadioButton::toggled, this, &LaserControllerWindow::onUserOriginRadioButtonChanged);
    connect(m_radioButtonUserOrigin2, &QRadioButton::toggled, this, &LaserControllerWindow::onUserOriginRadioButtonChanged);
    connect(m_radioButtonUserOrigin3, &QRadioButton::toggled, this, &LaserControllerWindow::onUserOriginRadioButtonChanged);

    updateUserOriginSelection(Config::Device::userOriginSelected());

    m_userOrigin1 = InputWidgetWrapper::createWidget<Vector2DWidget*>(Config::Device::userOrigin1Item());
    m_userOrigin2 = InputWidgetWrapper::createWidget<Vector2DWidget*>(Config::Device::userOrigin2Item());
    m_userOrigin3 = InputWidgetWrapper::createWidget<Vector2DWidget*>(Config::Device::userOrigin3Item());

    Config::Device::userOrigin1Item()->bindWidget(m_userOrigin1);
    Config::Device::userOrigin2Item()->bindWidget(m_userOrigin2);
    Config::Device::userOrigin3Item()->bindWidget(m_userOrigin3);

    QGridLayout* thirdRow = new QGridLayout;
    thirdRow->setMargin(0);
    thirdRow->addWidget(m_radioButtonUserOrigin1, 0, 0);
    thirdRow->addWidget(m_radioButtonUserOrigin2, 1, 0);
    thirdRow->addWidget(m_radioButtonUserOrigin3, 2, 0);
    thirdRow->addWidget(m_userOrigin1, 0, 1);
    thirdRow->addWidget(m_userOrigin2, 1, 1);
    thirdRow->addWidget(m_userOrigin3, 2, 1);
    thirdRow->setColumnStretch(0, 0);
    thirdRow->setColumnStretch(2, 1);

    m_buttonFetchToUserOrigin = new QToolButton;
    m_buttonFetchToUserOrigin->setDefaultAction(m_ui->actionFetchToUserOrigin);
    QHBoxLayout* fourthRow = new QHBoxLayout;
    fourthRow->setMargin(3);
    fourthRow->addWidget(m_buttonFetchToUserOrigin);

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

    m_dockMovement = dockWidget;
    m_dockAreaMovement = m_dockManager->addDockWidget(CenterDockWidgetArea, dockWidget, m_dockAreaLayers);
    dockPanelOnlyShowIcon(dockWidget, QPixmap(":/ui/icons/images/movement.png"), "Movement");
}

void LaserControllerWindow::createLaserPowerDockPanel()
{
    m_floatEditSliderScanLaserPower = InputWidgetWrapper::createWidget<FloatEditSlider*>(Config::UserRegister::scanLaserPowerItem());
    Config::UserRegister::scanLaserPowerItem()->bindWidget(m_floatEditSliderScanLaserPower);

    m_editSliderScanMaxGray = InputWidgetWrapper::createWidget<EditSlider*>(Config::UserRegister::maxScanGrayRatioItem());
    Config::UserRegister::maxScanGrayRatioItem()->bindWidget(m_editSliderScanMaxGray);

    m_editSliderScanMinGray = InputWidgetWrapper::createWidget<EditSlider*>(Config::UserRegister::minScanGrayRatioItem());
    Config::UserRegister::minScanGrayRatioItem()->bindWidget(m_editSliderScanMinGray);

    m_floatEditSliderCuttingMaxPower = InputWidgetWrapper::createWidget<FloatEditSlider*>(Config::UserRegister::defaultMaxCuttingPowerItem());
    Config::UserRegister::defaultMaxCuttingPowerItem()->bindWidget(m_floatEditSliderCuttingMaxPower);

    m_floatEditSliderCuttingMinPower = InputWidgetWrapper::createWidget<FloatEditSlider*>(Config::UserRegister::defaultMinCuttingPowerItem());
    Config::UserRegister::defaultMinCuttingPowerItem()->bindWidget(m_floatEditSliderCuttingMinPower);

    m_floatEditSliderSpotShotPower = InputWidgetWrapper::createWidget<FloatEditSlider*>(Config::UserRegister::spotShotPowerItem());
    Config::UserRegister::spotShotPowerItem()->bindWidget(m_floatEditSliderSpotShotPower);
    QFormLayout* layout = new QFormLayout;
    layout->addRow(Config::UserRegister::scanLaserPowerItem()->title(), m_floatEditSliderScanLaserPower);
    layout->addRow(Config::UserRegister::maxScanGrayRatioItem()->title(), m_editSliderScanMaxGray);
    layout->addRow(Config::UserRegister::minScanGrayRatioItem()->title(), m_editSliderScanMinGray);
    layout->addRow(Config::UserRegister::defaultMaxCuttingPowerItem()->title(), m_floatEditSliderCuttingMaxPower);
    layout->addRow(Config::UserRegister::defaultMinCuttingPowerItem()->title(), m_floatEditSliderCuttingMinPower);
    layout->addRow(Config::UserRegister::spotShotPowerItem()->title(), m_floatEditSliderSpotShotPower);
    m_formLayoutLaserPower = layout;

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Laser Power"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockLaserPower = dockWidget;
    m_dockAreaLaserPower = m_dockManager->addDockWidget(CenterDockWidgetArea, dockWidget, m_dockAreaOperations);
    //dockPanelOnlyShowIcon(dockWidget, QPixmap(":/ui/icons/images/laser_power.png"), "Movement");
}

void LaserControllerWindow::createPrintAndCutPanel()
{
    m_groupBoxPrintAndCutPoints = new QGroupBox;
    m_groupBoxPrintAndCutPoints->setTitle(tr("Points"));
    QVBoxLayout* pointsLayout = new QVBoxLayout;
    pointsLayout->setMargin(2);
    m_tablePrintAndCutPoints = new PointPairTableWidget;
    pointsLayout->addWidget(m_tablePrintAndCutPoints);

    QHBoxLayout* offsetLayout = new QHBoxLayout;
    m_labelPrintAndCutOffset = new QLabel(tr("Laser Offset: "));
    m_doubleSpinBoxPrintAndCutOffsetX = new QDoubleSpinBox;
    m_doubleSpinBoxPrintAndCutOffsetX->setMinimum(-100);
    m_doubleSpinBoxPrintAndCutOffsetX->setMaximum(100);
    m_doubleSpinBoxPrintAndCutOffsetX->setDecimals(1);
    m_doubleSpinBoxPrintAndCutOffsetY = new QDoubleSpinBox;
    m_doubleSpinBoxPrintAndCutOffsetY->setMinimum(-100);
    m_doubleSpinBoxPrintAndCutOffsetY->setMaximum(100);
    m_doubleSpinBoxPrintAndCutOffsetY->setDecimals(1);
    offsetLayout->addWidget(m_labelPrintAndCutOffset);
    offsetLayout->addWidget(new QLabel(" X"));
    offsetLayout->addWidget(m_doubleSpinBoxPrintAndCutOffsetX);
    offsetLayout->addWidget(new QLabel(" Y"));
    offsetLayout->addWidget(m_doubleSpinBoxPrintAndCutOffsetY);
    pointsLayout->addLayout(offsetLayout);
    offsetLayout->setStretch(0, 0);
    offsetLayout->setStretch(1, 0.1);
    offsetLayout->setStretch(2, 1);
    offsetLayout->setStretch(3, 0.1);
    offsetLayout->setStretch(4, 1);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    QToolButton* buttonNewLine = new QToolButton;
    buttonNewLine->setDefaultAction(m_ui->actionPrintAndCutNew);
    buttonsLayout->addWidget(buttonNewLine);
    QToolButton* buttonFetchLaser = new QToolButton;
    buttonFetchLaser->setDefaultAction(m_ui->actionPrintAndCutFetchLaser);
    buttonsLayout->addWidget(buttonFetchLaser);
    QToolButton* buttonFetchCanvas = new QToolButton;
    buttonFetchCanvas->setDefaultAction(m_ui->actionPrintAndCutFetchCanvas);
    buttonsLayout->addWidget(buttonFetchCanvas);
    QToolButton* buttonRemove = new QToolButton;
    buttonRemove->setDefaultAction(m_ui->actionPrintAndCutRemove);
    buttonsLayout->addWidget(buttonRemove);
    QToolButton* buttonClear = new QToolButton;
    buttonClear->setDefaultAction(m_ui->actionPrintAndCutClear);
    buttonsLayout->addWidget(buttonClear);
    pointsLayout->addLayout(buttonsLayout);
    m_groupBoxPrintAndCutPoints->setLayout(pointsLayout);

    m_groupBoxPrintAndCutResult = new QGroupBox;
    m_groupBoxPrintAndCutResult->setTitle(tr("Result"));
    QVBoxLayout* resultLayout = new QVBoxLayout;
    resultLayout->setMargin(2);
    m_labelPrintAndCutTranslationResult = new QLabel(tr("Translation"));
    m_labelPrintAndCutRotationResult = new QLabel(tr("Rotation"));
    m_labelPrintAndCutTranslation= new QLabel(tr("0.000, 0.000"));
    m_labelPrintAndCutRotation= new QLabel(tr("0.00 degrees"));
    QFormLayout* resultFormLayout = new QFormLayout;
    resultFormLayout->setLabelAlignment(Qt::AlignmentFlag::AlignRight);
    resultFormLayout->addRow(m_labelPrintAndCutTranslationResult, m_labelPrintAndCutTranslation);
    resultFormLayout->addRow(m_labelPrintAndCutRotationResult, m_labelPrintAndCutRotation);
    resultLayout->addLayout(resultFormLayout);

    buttonsLayout = new QHBoxLayout;
    QToolButton* buttonAlign = new QToolButton;
    buttonAlign->setDefaultAction(m_ui->actionPrintAndCutAlign);
    buttonsLayout->addWidget(buttonAlign);
    QToolButton* buttonRestore = new QToolButton;
    buttonRestore->setDefaultAction(m_ui->actionPrintAndCutRestore);
    buttonsLayout->addWidget(buttonRestore);
    resultLayout->addLayout(buttonsLayout);

    m_groupBoxPrintAndCutResult->setLayout(resultLayout);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(m_groupBoxPrintAndCutPoints);
    layout->addWidget(m_groupBoxPrintAndCutResult);

    QWidget* panelWidget = new QWidget;
    panelWidget->setLayout(layout);

    CDockWidget* dockWidget = new CDockWidget(tr("Print and Cut"));
    dockWidget->setWidget(panelWidget);
    m_ui->menuWindow->addAction(dockWidget->toggleViewAction());

    m_dockPrintAndCut = dockWidget;
    m_dockAreaPrintAndCut = m_dockManager->addDockWidget(CenterDockWidgetArea, dockWidget, m_dockAreaOperations);
    //m_dockPrintAndCut->setFloating();
    //m_dockAreaPrintAndCut->setBaseSize(QSize(300, 500));
}

void LaserControllerWindow::createShapePropertyDockPanel()
{
    m_cutOrderPriority = new LaserDoubleSpinBox();
    m_cutOrderPriorityLabel = new QLabel("Cut Order Priority");
    m_powerScale = new LaserDoubleSpinBox();
    m_powerScaleLabel = new QLabel("Power Scale");
    m_width = new LaserDoubleSpinBox();
    m_widthLabel = new QLabel("Width");
    m_height = new LaserDoubleSpinBox();
    m_heightLabel = new QLabel("Height");
    m_maxWidth = new LaserDoubleSpinBox();
    m_maxWidthLabel = new QLabel("Max Width");
    m_cornerRadius = new LaserDoubleSpinBox();
    m_cornerRadius->setMinimum(0);
    m_cornerRadius->setMaximum(DBL_MAX);
    m_cornerRadiusLabel = new QLabel("Corner Radius");
    m_locked = new QCheckBox();
    m_lockedLabel = new QLabel("Locked");

    m_cornerRadius->connect(m_cornerRadius, &LaserDoubleSpinBox::enterOrLostFocus, this, [=] {
        LaserViewer* view = qobject_cast<LaserViewer*>(m_scene->views()[0]);
        QList<LaserPrimitive*>list = view->scene()->selectedPrimitives();
        if (list.count() != 1) {
            return;
        }
        LaserRect* rect = nullptr;
        rect = qgraphicsitem_cast<LaserRect*>(list[0]);

        if (rect) {
            CornerRadiusCommand* cmd = new CornerRadiusCommand(m_viewer, rect, m_cornerRadius, m_cornerRadius->value());
            view->undoStack()->push(cmd);            
        }        
    });
    
    m_locked->connect(m_locked, &QCheckBox::clicked, this, [=] {
        qDebug() << "state:" << m_locked->checkState();
        LaserViewer* view = qobject_cast<LaserViewer*>(m_scene->views()[0]);
        QList<LaserPrimitive*>list = view->scene()->selectedPrimitives();
        QList<LaserPrimitive*>lockedList;
        for (LaserPrimitive* p : list) {
            if (p->isLocked()) {
                lockedList.append(p);
            }
            
        }
        LockedCommand* cmd = new LockedCommand(view, m_locked, m_lastLockedState, lockedList);
        view->undoStack()->push(cmd);
        m_lastLockedState = m_locked->checkState();

    });
    m_locked->connect(m_locked, &QCheckBox::toggled, this, [=] {
        //qDebug() << "state:" << m_locked->checkState();
    });

    m_propertyPanelWidget = new QWidget();
    m_propertyDockWidget = new CDockWidget(tr("Movement"));
    
    m_ui->menuWindow->addAction(m_propertyDockWidget->toggleViewAction());

    m_dockAreaProperty = m_dockManager->addDockWidget(CenterDockWidgetArea, m_propertyDockWidget, m_dockAreaLayers);
    dockPanelOnlyShowIcon(m_propertyDockWidget, QPixmap(":/ui/icons/images/shape.png"), "Shape Properties");
    createPrimitivePropertiesPanel();
}

void LaserControllerWindow::showShapePropertyPanel()
{
    LaserPrimitiveType type = LaserPrimitiveType::LPT_NULL;
    QList<LaserPrimitive*> list = m_scene->selectedPrimitives();
    bool isLocked = false;
    for (int i = 0; i < list.length(); i ++) {
        if (i == 0) {
            type = list[0]->primitiveType();
            isLocked = list[0]->isLocked();
            if (isLocked) {
                m_locked->setCheckState(Qt::Checked);
            }
            else {
                m_locked->setCheckState(Qt::Unchecked);
            }
        }
        else {
            LaserPrimitiveType curType = list[i]->primitiveType();
            bool curIsLocked = list[i]->isLocked();
            if (curType != type) {
                type = LaserPrimitiveType::LPT_UNKNOWN;
            }
            if (isLocked != curIsLocked) {
                m_locked->setCheckState(Qt::PartiallyChecked);
            }
        }

    }
    m_lastLockedState = m_locked->checkState();
    switch(type) {
        case LPT_UNKNOWN: {
            m_mixturePropertyLayout->setMargin(10);
            m_mixturePropertyLayout->setSpacing(10);
            m_mixturePropertyLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);  

            m_mixturePropertyLayout->addWidget(m_cutOrderPriorityLabel, 0, 0, Qt::AlignRight);
            m_mixturePropertyLayout->addWidget(m_cutOrderPriority, 0, 1);
            m_mixturePropertyLayout->addWidget(m_powerScaleLabel, 1, 0, Qt::AlignRight);
            m_mixturePropertyLayout->addWidget(m_powerScale, 1, 1);
            m_mixturePropertyLayout->addWidget(m_lockedLabel, 2, 0, Qt::AlignRight);
            m_mixturePropertyLayout->addWidget(m_locked, 2, 1);
            m_mixturePropertyWidget->setLayout(m_mixturePropertyLayout);


            m_propertyDockWidget->setWidget(m_mixturePropertyWidget);
            //m_lockedLabel->setText("LPT_UNKNOWN");
            break;
        }
        case LPT_NULL: {
            m_nullPropertyWidget->setLayout(m_nullPropertyLayout);
            m_propertyDockWidget->setWidget(m_nullPropertyWidget);
            break;
        }
        case LPT_LINE: {
            m_linePropertyLayout->setMargin(10);
            m_linePropertyLayout->setSpacing(10);
            m_linePropertyLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

            m_linePropertyLayout->addWidget(m_cutOrderPriorityLabel, 0, 0, Qt::AlignRight);
            m_linePropertyLayout->addWidget(m_cutOrderPriority, 0, 1);
            m_linePropertyLayout->addWidget(m_powerScaleLabel, 1, 0, Qt::AlignRight);
            m_linePropertyLayout->addWidget(m_powerScale, 1, 1);
            m_linePropertyLayout->addWidget(m_lockedLabel, 2, 0, Qt::AlignRight);
            m_linePropertyLayout->addWidget(m_locked, 2, 1);

            m_linePropertyWidget->setLayout(m_linePropertyLayout);
            m_propertyDockWidget->setWidget(m_linePropertyWidget);
            //m_lockedLabel->setText("line");
            break;
        }
        case LPT_RECT: {
            m_rectPropertyLayout->setMargin(0);
            m_rectPropertyLayout->setSpacing(10);
            m_rectPropertyLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

            m_rectPropertyLayout->addWidget(m_cutOrderPriorityLabel, 0, 0, Qt::AlignRight);
            m_rectPropertyLayout->addWidget(m_cutOrderPriority, 0, 1);
            m_rectPropertyLayout->addWidget(m_powerScaleLabel, 1, 0, Qt::AlignRight);
            m_rectPropertyLayout->addWidget(m_powerScale, 1, 1);
            m_rectPropertyLayout->addWidget(m_lockedLabel, 2, 0, Qt::AlignRight);
            m_rectPropertyLayout->addWidget(m_locked, 2, 1);
            m_rectPropertyLayout->addWidget(m_widthLabel, 3, 0, Qt::AlignRight);
            m_rectPropertyLayout->addWidget(m_width, 3, 1);
            m_rectPropertyLayout->addWidget(m_heightLabel, 4, 0, Qt::AlignRight);
            m_rectPropertyLayout->addWidget(m_height, 4, 1);
            m_rectPropertyLayout->addWidget(m_cornerRadiusLabel, 5, 0, Qt::AlignRight);
            m_rectPropertyLayout->addWidget(m_cornerRadius, 5, 1);

            m_rectPropertyWidget->setLayout(m_rectPropertyLayout);
            m_propertyDockWidget->setWidget(m_rectPropertyWidget);
            LaserRect* rect = nullptr;
            rect = qgraphicsitem_cast<LaserRect*>(list[0]);
            m_cornerRadius->setValue(rect->cornerRadius());
            m_lastCornerRadiusValue = rect->cornerRadius();
            break;
        }
        case LPT_ELLIPSE: {
            m_ellipsePropertyLayout->setMargin(0);
            m_ellipsePropertyLayout->setSpacing(10);
            m_ellipsePropertyLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            m_ellipsePropertyLayout->addWidget(m_cutOrderPriorityLabel, 0, 0, Qt::AlignRight);
            m_ellipsePropertyLayout->addWidget(m_cutOrderPriority, 0, 1);
            m_ellipsePropertyLayout->addWidget(m_powerScaleLabel, 1, 0, Qt::AlignRight);
            m_ellipsePropertyLayout->addWidget(m_powerScale, 1, 1);
            m_ellipsePropertyLayout->addWidget(m_lockedLabel, 2, 0, Qt::AlignRight);
            m_ellipsePropertyLayout->addWidget(m_locked, 2, 1);
            m_ellipsePropertyLayout->addWidget(m_widthLabel, 3, 0, Qt::AlignRight);
            m_ellipsePropertyLayout->addWidget(m_width, 3, 1);
            m_ellipsePropertyLayout->addWidget(m_heightLabel, 4, 0, Qt::AlignRight);
            m_ellipsePropertyLayout->addWidget(m_height, 4, 1);
            m_ellipsePropertyWidget->setLayout(m_ellipsePropertyLayout);
            m_propertyDockWidget->setWidget(m_ellipsePropertyWidget);
            //m_lockedLabel->setText("LPT_ELLIPSE");
            break;
        }
        case LPT_POLYLINE: {
            m_polylinePropertyLayout->setMargin(0);
            m_polylinePropertyLayout->setSpacing(10);
            m_polylinePropertyLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

            m_polylinePropertyLayout->addWidget(m_cutOrderPriorityLabel, 0, 0, Qt::AlignRight);
            m_polylinePropertyLayout->addWidget(m_cutOrderPriority, 0, 1);
            m_polylinePropertyLayout->addWidget(m_powerScaleLabel, 1, 0, Qt::AlignRight);
            m_polylinePropertyLayout->addWidget(m_powerScale, 1, 1);
            m_polylinePropertyLayout->addWidget(m_lockedLabel, 2, 0, Qt::AlignRight);
            m_polylinePropertyLayout->addWidget(m_locked, 2, 1);
            m_polylinePropertyWidget->setLayout(m_polylinePropertyLayout);
            m_propertyDockWidget->setWidget(m_polylinePropertyWidget);
            //m_lockedLabel->setText("LPT_POLYLINE");
            break;
        }
        case LPT_POLYGON: {
            m_polygonPropertyLayout->setMargin(0);
            m_polygonPropertyLayout->setSpacing(10);
            m_polygonPropertyLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

            m_polygonPropertyLayout->addWidget(m_cutOrderPriorityLabel, 0, 0, Qt::AlignRight);
            m_polygonPropertyLayout->addWidget(m_cutOrderPriority, 0, 1);
            m_polygonPropertyLayout->addWidget(m_powerScaleLabel, 1, 0, Qt::AlignRight);
            m_polygonPropertyLayout->addWidget(m_powerScale, 1, 1);
            m_polygonPropertyLayout->addWidget(m_lockedLabel, 2, 0, Qt::AlignRight);
            m_polygonPropertyLayout->addWidget(m_locked, 2, 1);
            m_polygonPropertyWidget->setLayout(m_polygonPropertyLayout);
            m_propertyDockWidget->setWidget(m_polygonPropertyWidget);
            //m_lockedLabel->setText("LPT_POLYGON");
            break;
        }
        case LPT_TEXT: {
            m_textPropertyLayout->setMargin(0);
            m_textPropertyLayout->setSpacing(10);
            m_textPropertyLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

            m_textPropertyLayout->addWidget(m_cutOrderPriorityLabel, 0, 0, Qt::AlignRight);
            m_textPropertyLayout->addWidget(m_cutOrderPriority, 0, 1);
            m_textPropertyLayout->addWidget(m_powerScaleLabel, 1, 0, Qt::AlignRight);
            m_textPropertyLayout->addWidget(m_powerScale, 1, 1);
            m_textPropertyLayout->addWidget(m_lockedLabel, 2, 0, Qt::AlignRight);
            m_textPropertyLayout->addWidget(m_locked, 2, 1);
            m_textPropertyLayout->addWidget(m_maxWidthLabel, 3, 0, Qt::AlignRight);
            m_textPropertyLayout->addWidget(m_maxWidth, 3, 1);
            m_textPropertyWidget->setLayout(m_textPropertyLayout);
            m_propertyDockWidget->setWidget(m_textPropertyWidget);
            //m_lockedLabel->setText("LPT_TEXT");
            break;
        }
        case LPT_NURBS: {
            m_nurbsPropertyLayout->setMargin(0);
            m_nurbsPropertyLayout->setSpacing(10);
            m_nurbsPropertyLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

            m_nurbsPropertyLayout->addWidget(m_cutOrderPriorityLabel, 0, 0, Qt::AlignRight);
            m_nurbsPropertyLayout->addWidget(m_cutOrderPriority, 0, 1);
            m_nurbsPropertyLayout->addWidget(m_powerScaleLabel, 1, 0, Qt::AlignRight);
            m_nurbsPropertyLayout->addWidget(m_powerScale, 1, 1);
            m_nurbsPropertyLayout->addWidget(m_lockedLabel, 2, 0, Qt::AlignRight);
            m_nurbsPropertyLayout->addWidget(m_locked, 2, 1);
            m_nurbsPropertyWidget->setLayout(m_nurbsPropertyLayout);
            m_propertyDockWidget->setWidget(m_nurbsPropertyWidget);
            //m_lockedLabel->setText("LPT_NURBS");
            break;
        }
        case LPT_BITMAP: {
            m_bitmapPropertyLayout->setMargin(0);
            m_bitmapPropertyLayout->setSpacing(10);
            m_bitmapPropertyLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

            m_bitmapPropertyLayout->addWidget(m_cutOrderPriorityLabel, 0, 0, Qt::AlignRight);
            m_bitmapPropertyLayout->addWidget(m_cutOrderPriority, 0, 1);
            m_bitmapPropertyLayout->addWidget(m_powerScaleLabel, 1, 0, Qt::AlignRight);
            m_bitmapPropertyLayout->addWidget(m_powerScale, 1, 1);
            m_bitmapPropertyLayout->addWidget(m_lockedLabel, 2, 0, Qt::AlignRight);
            m_bitmapPropertyLayout->addWidget(m_locked, 2, 1);
            m_bitmapPropertyWidget->setLayout(m_bitmapPropertyLayout);
            m_propertyDockWidget->setWidget(m_bitmapPropertyWidget);
            //m_lockedLabel->setText("LPT_BITMAP");
            break;
        }
    }
    

}

void LaserControllerWindow::createPrimitivePropertiesPanel()
{
    //rect
    m_rectPropertyLayout = new QGridLayout();
    m_rectPropertyWidget = new QWidget();
    
    //line
    m_linePropertyLayout = new QGridLayout();
    m_linePropertyWidget = new QWidget();
    
    //ellipse
    m_ellipsePropertyLayout = new QGridLayout();
    m_ellipsePropertyWidget = new QWidget();
    
    //ploygon
    m_polygonPropertyLayout = new QGridLayout();
    m_polygonPropertyWidget = new QWidget();
    
    //ployline
    m_polylinePropertyLayout = new QGridLayout();
    m_polylinePropertyWidget = new QWidget();
    
    //bitmap
    m_bitmapPropertyLayout = new QGridLayout();
    m_bitmapPropertyWidget = new QWidget();
    
    //text
    m_textPropertyLayout = new QGridLayout();
    m_textPropertyWidget = new QWidget();
    
    //mix
    m_mixturePropertyLayout = new QGridLayout();
    m_mixturePropertyWidget = new QWidget();
    
    //null
    m_nullPropertyLayout = new QGridLayout();
    m_nullPropertyWidget = new QWidget();
    
}

void LaserControllerWindow::createPrimitiveLinePropertyPanel()
{
    QGridLayout* layout = new QGridLayout();
    
    layout->setMargin(0);
    layout->setSpacing(10);
    layout->addWidget(m_lockedLabel, 0, 0, Qt::AlignRight);
    layout->addWidget(m_locked, 0, 1);
    
    m_propertyPanelWidget->setLayout(layout);
    m_propertyDockWidget->setWidget(m_propertyPanelWidget);
}

void LaserControllerWindow::createPrimitiveRectPropertyPanel()
{
}

void LaserControllerWindow::createPrimitiveEllipsePropertyPanel()
{
}

void LaserControllerWindow::createPrimitivePloygonPropertyPanel()
{
}

void LaserControllerWindow::createPrimitivePloylinePropertyPanel()
{
}

void LaserControllerWindow::createPrimitiveTextPropertyPanel()
{
}

void LaserControllerWindow::createPrimitiveBitmapPropertyPanel()
{
}

void LaserControllerWindow::createEmptyPropertyPanel()
{
}

void LaserControllerWindow::createMixturePropertyPanel()
{
}

void LaserControllerWindow::dockPanelOnlyShowIcon(CDockWidget* dockWidget, QPixmap icon, char* text)
{
    CDockWidgetTab* tab = dockWidget->tabWidget();
    tab->setMinimumWidth(0);
    tab->setFixedWidth(35);
    tab->setElideMode(Qt::TextElideMode::ElideNone);
    tab->setToolTip(tr(text));
    tab->setIcon(icon);
    tab->setTitleLabelVisible(false);
    connect(tab, &CDockWidgetTab::activeTabChanged, this, [=] {
        tab->closeButton()->setVisible(false);
    });
}

LaserDocument* LaserControllerWindow::currentDocument() const
{
    return m_viewer->scene()->document();
}

bool LaserControllerWindow::lockEqualRatio()
{
    return m_lockEqualRatio;
}

LaserDoubleSpinBox * LaserControllerWindow::widthBox()
{
    return m_widthBox;
}

LaserDoubleSpinBox * LaserControllerWindow::heightBox()
{
    return m_heightBox;
}

void LaserControllerWindow::setLastCornerRadiusValue(qreal val)
{
    m_lastCornerRadiusValue = val;
}

qreal LaserControllerWindow::lastCornerRadiusValue()
{
    return m_lastCornerRadiusValue;
}

LaserDoubleSpinBox * LaserControllerWindow::fontSpaceYDoubleSpinBox()
{
    return m_fontSpaceY;
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
        case Qt::Key_Delete: {
            if (!StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {
                onActionDeletePrimitive();
            }
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
            else {

            }
			break;
		}

	}
	
}

void LaserControllerWindow::contextMenuEvent(QContextMenuEvent * event)
{
	if (StateControllerInst.isInState(StateControllerInst.documentIdleState()) ||
		StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
        //across mirror Line
        m_ui->actionMirrorAcrossLine->setEnabled(false);
        if (m_viewer->mirrorLine()) {
            m_ui->actionMirrorAcrossLine->setEnabled(true);
        }
        
		QMenu Context;
        
        Context.addAction(m_ui->actionMirrorAcrossLine);
        Context.addSeparator();
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

    askMergeOrNew();

    QString filters = tr("SVG (*.svg);;CAD (*.dxf)");
    QString filename = getFilename(tr("Open Supported File"), filters);
    qLogD << "importing filename is " << filename;
    if (filename.isEmpty())
        return;

    QFileInfo file(filename);
    QSharedPointer<Importer> importer = Importer::getImporter(this, file.suffix());
    if (!importer.isNull())
    {
        LaserApplication::progressModel->clear();
        LaserApplication::showProgressWindow();
        ProgressItem* progress = LaserApplication::progressModel->createComplexItem("Importing", nullptr);
        importer->import(filename, m_scene, progress);
    }
}

void LaserControllerWindow::onActionImportCorelDraw(bool checked)
{
    askMergeOrNew();

    QSharedPointer<Importer> importer = Importer::getImporter(this, Importer::CORELDRAW);
    QVariantMap params;
    params["parent_winid"] = winId();
    params["parent_win"] = QVariant::fromValue<QMainWindow*>(this);
    LaserApplication::progressModel->clear();
    LaserApplication::showProgressWindow();
    ProgressItem* progress = LaserApplication::progressModel->createComplexItem("Importing", nullptr);
    importer->import("", m_scene, progress, params);
}

void LaserControllerWindow::onActionNew(bool checked)
{
	
	LaserDocument* doc = m_scene->document();
	if (doc) {
		if (!onActionCloseDocument()) {
			return;
		}
	}
	this->setWindowTitle(tr("Untitled - "));
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
	setWindowTitle(getCurrentFileName() + " - ");
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
	QString name = QFileDialog::getOpenFileName(nullptr, tr("Open File"), ".", "File(*.lc)");
	m_fileDirection = name;
	if (name == "") {
		return;
	}
	setWindowTitle(getCurrentFileName() + " - ");
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
    LaserDocument* doc = m_scene->document();
    if (!doc) {
        return;
    }
    QList<LaserLayer*>list = doc->layers();
    if (list.isEmpty()) {
        return;
    }
    LaserLayer* layer = list[index];
    m_scene->blockSignals(true);
    //m_scene->clearSelection();
    //清理之前的选区
    LaserViewer* view = qobject_cast<LaserViewer*>( m_scene->views()[0]);
    if (!view) {
        return;
    }
    view->clearGroupSelection();
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
            LaserApplication::progressModel->clear();
            LaserApplication::showProgressWindow();
            ProgressItem* progress = LaserApplication::progressModel->createComplexItem("Exporting", nullptr);
            QtConcurrent::run([=]()
                {
                    m_scene->document()->outline(progress);
                    m_scene->document()->setFinishRun(finishRun());
                    m_prepareMachining = false;
                    m_scene->document()->exportJSON(filename, progress);
                }
            );
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
    LaserApplication::previewWindow->reset();
    //LaserApplication::showProgressWindow();

    if (m_useLoadedJson)
    {
        qDebug() << "export temp json file for machining" << m_currentJson;
        LaserApplication::driver->loadDataFromFile(m_currentJson);
    }
    else
    {
        if (m_scene->document() == nullptr)
        {
            QMessageBox::warning(this, tr("Alert"), tr("No active document. Please open or import a document to mechining"));
            return;
        }
        QString filename = QDir::current().absoluteFilePath("tmp/export.json");
        //QTemporaryFile file;
        //if (file.open())
        //{
            //QString filename = file.fileName();

        LaserApplication::progressModel->clear();
        LaserApplication::showProgressWindow();
        ProgressItem* progress = LaserApplication::progressModel->createComplexItem("Exporting", nullptr);
        QtConcurrent::run([=]()
            {
                m_scene->document()->outline(progress);
                m_scene->document()->setFinishRun(finishRun());
                qDebug() << "exporting to temporary json file:" << filename;
                m_prepareMachining = true;
                qDebug() << "export temp json file for machining" << filename;
                m_scene->document()->exportJSON(filename, progress);
            }
        );
        //}
    }
    m_useLoadedJson = false;
}

void LaserControllerWindow::onActionPauseMechining(bool checked)
{
    //LaserDriver::instance().pauseContinueMachining(!checked);
    int result = LaserApplication::driver->pauseContinueMachining(!checked);
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
    LaserApplication::driver->stopMachining();
    m_ui->actionPause->setChecked(false);
}

void LaserControllerWindow::onActionLaserSpotShot(bool checked)
{
    int result = LaserApplication::driver->testLaserLight(checked);
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
    LaserApplication::driver->initComPort(comName);
}

void LaserControllerWindow::onActionDisconnect(bool checked)
{
    LaserApplication::driver->uninitComPort();
}

void LaserControllerWindow::onActionDownload(bool checked)
{
	QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open File"), ".", "JSON (*.json)");
    filename = m_tmpDir.absoluteFilePath(filename);
    m_prepareMachining = false;
    LaserApplication::driver->loadDataFromFile(filename);
}

void LaserControllerWindow::onActionLoadMotor(bool checked)
{
    LaserApplication::driver->controlMotor(false);
}

void LaserControllerWindow::onActionUnloadMotor(bool checked)
{
    if (QMessageBox::Apply == QMessageBox::question(this, tr("Unload motor"), tr("Do you want to unload motor?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {
        LaserApplication::driver->controlMotor(false);
    }
}

void LaserControllerWindow::onActionWorkState(bool checked)
{
    LaserApplication::driver->getDeviceWorkState();
}

void LaserControllerWindow::onActionMoveTop()
{
    QVector3D delta(0, -m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveBottom()
{
    QVector3D delta(0, m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveLeft()
{
    QVector3D delta(-m_doubleSpinBoxDistanceX->value(), 0, 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveRight()
{
    QVector3D delta(m_doubleSpinBoxDistanceX->value(), 0, 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveTopLeft()
{
    QVector3D delta(-m_doubleSpinBoxDistanceX->value(), -m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveTopRight()
{
    QVector3D delta(m_doubleSpinBoxDistanceX->value(), -m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveBottomLeft()
{
    QVector3D delta(-m_doubleSpinBoxDistanceX->value(), m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveBottomRight()
{
    QVector3D delta(m_doubleSpinBoxDistanceX->value(), m_doubleSpinBoxDistanceY->value(), 0);
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveUp()
{
    QVector3D delta(0, 0, m_doubleSpinBoxDistanceZ->value());
    moveLaser(delta);
}

void LaserControllerWindow::onActionMoveDown()
{
    QVector3D delta(0, 0, -m_doubleSpinBoxDistanceZ->value());
    moveLaser(delta);
}

void LaserControllerWindow::onMovementButtonReleased()
{
    LaserApplication::driver->startMoveLaserMotors();
}

void LaserControllerWindow::onActionHalfTone(bool checked)
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), QString(), tr("Images (*.png *.bmp *.jpg)"));
    if (!filename.isEmpty() && !filename.isNull())
    {
        QImage image(filename);
        image = image.convertToFormat(QImage::Format_Grayscale8);
        cv::Mat src(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());

        int gridSize = qRound(Config::EngravingLayer::DPI() * 1.0 / Config::EngravingLayer::LPI());
        
        switch (Config::Export::halfToneStyle())
        {
        case 0:
            imageUtils::halftone4(src, Config::EngravingLayer::halftoneAngles(), gridSize);
            break;
        case 1:
            imageUtils::halftone5(src, Config::EngravingLayer::halftoneAngles(), gridSize);
            break;
        case 2:
            imageUtils::halftone6(src, Config::EngravingLayer::halftoneAngles(), gridSize);
            break;
        }
        QFileInfo tmpFile("tmp/dst.bmp");
        QUrl url = QUrl::fromLocalFile(tmpFile.absolutePath());
        QDesktopServices::openUrl(url);
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

void LaserControllerWindow::onActionPasteInPlace(bool checked)
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
	QMessageBox msgBox(QMessageBox::NoIcon,
		tr("Close document?"), tr("Do you want to save current document?"),
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
    qDebug()<<"";
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

void LaserControllerWindow::onActionMainCardInfo(bool checked)
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

void LaserControllerWindow::onActionUserInfo(bool checked)
{
    UserInfoDialog dlg;
    dlg.exec();
}

void LaserControllerWindow::onActionAbout(bool checked)
{
    LaserApplication::device->showLibraryVersion();
}

void LaserControllerWindow::onActionUpdateOutline(bool checked)
{
    LaserApplication::progressModel->clear();
    LaserApplication::showProgressWindow();
    ProgressItem* progress = LaserApplication::progressModel->createComplexItem("Outlining", nullptr);
    QtConcurrent::run(
        [=]() {
            m_scene->document()->outline(progress);
        }
    );
}

void LaserControllerWindow::onActionFetchToUserOrigin(bool checked)
{
    QPointF laserPos = LaserApplication::device->getCurrentLaserPos().toPointF();
    switch (Config::Device::userOriginSelected())
    {
    case 0:
        Config::Device::userOrigin1Item()->setValue(laserPos);
        break;
    case 1:
        Config::Device::userOrigin2Item()->setValue(laserPos);
        break;
    case 2:
        Config::Device::userOrigin3Item()->setValue(laserPos);
        break;
    }
}

void LaserControllerWindow::onActionBitmap(bool checked)
{
	QString name = QFileDialog::getOpenFileName(nullptr, "open image", ".", "Images (*.jpg *.jpeg *.tif *.bmp *.png*.svg*.ico)");
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

void LaserControllerWindow::onActionRegiste(bool checked)
{
    RegisteDialog dlg;
    dlg.exec();
}

void LaserControllerWindow::onStatusBarRegisterClicked(bool checked)
{
    RegisteDialog dlg;
    dlg.exec();
}

void LaserControllerWindow::onActionActivate(bool checked)
{
    ActivationDialog dlg;
    dlg.exec();
}

void LaserControllerWindow::onStatusBarActivationClicked(bool checked)
{
    ActivationDialog dlg;
    dlg.exec();
}

void LaserControllerWindow::onActionUpdate(bool checked)
{
    if (!m_updateDialog)
    {
        m_updateDialog = new UpdateDialog;
    }
    m_updateDialog->setModal(true);
    m_updateDialog->show();
}

void LaserControllerWindow::onActionLaserPosition(bool checked)
{
    LaserApplication::device->getCurrentLaserPos();
}

void LaserControllerWindow::onProgressBarClicked()
{
    LaserApplication::showProgressWindow();
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

void LaserControllerWindow::onActionMirrorACrossLine(bool checked)
{
    if (!m_viewer || !m_viewer->group()) {
        return;
    }
    MirrorACommand* cmd = new MirrorACommand(m_viewer);
    m_viewer->undoStack()->push(cmd);
}

void LaserControllerWindow::onActionPrintAndCutNew(bool checked)
{
    m_tablePrintAndCutPoints->addNewLine();
}

void LaserControllerWindow::onActionPrintAndCutFetchLaser(bool checked)
{
    m_tablePrintAndCutPoints->setLaserPoint(LaserApplication::device->getCurrentLaserPos().toPointF());
}

void LaserControllerWindow::onActionPrintAndCutFetchCanvas(bool checked)
{
    if (m_scene->selectedPrimitives().count() != 1)
        return;

    LaserPrimitive* primitive = m_scene->selectedPrimitives().first();
    LaserRect* laserRect = dynamic_cast<LaserRect*>(primitive);
    if (!laserRect)
        return;

    QRectF bounding = laserRect->sceneBoundingRect();
    QRectF boundingViewer = m_viewer->mapFromScene(bounding).boundingRect();

    //Config::Ui::visualGridSpacing();
    int gridContrast = Config::Ui::gridContrast();
    bool showDocBounding = Config::Ui::showDocumentBoundingRect();

    Config::Ui::gridContrastItem()->setValue(0);
    m_scene->removeLaserPrimitive(primitive);
    m_viewer->update();
    QPixmap pixmap = m_viewer->grab(boundingViewer.toRect());
    pixmap.save("tmp/printAndCut.bmp");
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_Grayscale8);
    cv::Mat src(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
    std::vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack(src, corners, 5, 0.01, 10, cv::Mat());
    cv::Mat featuresImg;
    cv::cvtColor(src, featuresImg, cv::COLOR_GRAY2BGR);
    cv::Rect2f validRect(cv::Point2f(qRound(src.cols * 0.05), qRound(src.rows * 0.05)), 
        cv::Size2f(qRound(src.cols * 0.9), qRound(src.rows * 0.9)));
    QList<cv::Point2f> features;
    for (int i = 0; i < corners.size(); i++)
    {
        cv::Point2f pt = corners[i];
        if (validRect.contains(pt))
        {
            cv::circle(featuresImg, pt, 2, cv::Scalar(0, 255, 0), 2);
            features.append(pt);
        }
    }
    cv::imwrite("tmp/features.bmp", featuresImg);
    
    if (features.size() == 0)
    {
        QMessageBox::warning(this, tr("No Feature points found"), tr("There's no features point found. Please move or scale your selection rect primitive."));
    }
    else if (features.size() > 1)
    {
        QMessageBox::warning(this, tr("Too many feature points"), tr("Too many reature points found. Please move or scale your selection rect primitive to confirm that only one feature point to be found."));
    }
    else if (features.size() == 1)
    {
        cv::Point2f cvPt = features[0];
        QPointF candidatePt(cvPt.x * bounding.width() / src.cols, cvPt.y * bounding.height() / src.rows);
        candidatePt += bounding.topLeft();
        qLogD << "candidate point: " << candidatePt;
        QPointF canvasPoint(Global::pixels2mmX(candidatePt.x()), Global::pixels2mmY(candidatePt.y()));
        qLogD << "canvas point: " << canvasPoint;
        m_tablePrintAndCutPoints->setCanvasPoint(canvasPoint);
    }

    Config::Ui::gridContrastItem()->setValue(gridContrast);
    Config::Ui::showDocumentBoundingRectItem()->setValue(showDocBounding);
    m_viewer->update();
}

void LaserControllerWindow::onActionPrintAndCutRemove(bool checked)
{
    m_tablePrintAndCutPoints->removeSelected();
}

void LaserControllerWindow::onActionPrintAndCutClear(bool checked)
{
    m_tablePrintAndCutPoints->clearContents();
}

void LaserControllerWindow::onActionPrintAndCutAlign(bool checked)
{
    if (!m_scene->document())
        return;
    PointPairList pointPairs = m_tablePrintAndCutPoints->pointPairs();
    qLogD << "Point pairs count: " << pointPairs.length();

    //PointPairList pointPairs;
    //pointPairs << PointPair(QPointF(56.88, 8.93), QPointF(31.22, 20.11))
        //<< PointPair(QPointF(223.01, 154.01), QPointF(234.95, 103.98));
    //<< PointPair(QPointF(70.01, 80.54), QPointF(65.88, 83.87));

    if (pointPairs.length() < 2)
        return;

    QTransform t = utils::leastSquare4d(pointPairs, QPointF(m_doubleSpinBoxPrintAndCutOffsetX->value(), m_doubleSpinBoxPrintAndCutOffsetY->value()));

    QLineF l1(0, 0, 1, 0);
    QLineF l2 = t.map(l1);
    qreal angle = l2.angleTo(l1);
    qLogD << "angle: " << angle;
    QPointF diff(t.dx(), t.dy());

    t = QTransform(t.m11(), t.m12(), t.m21(), t.m22(), t.dx() * 1000, t.dy() * 1000);
    m_labelPrintAndCutRotation->setText(tr("%1 degrees").arg(angle));
    m_labelPrintAndCutTranslation->setText(tr("%1, %2").arg(diff.x()).arg(diff.y()));

    m_scene->document()->setEnablePrintAndCut(true);
    m_scene->document()->setPrintAndCutTransform(t);
    m_scene->document()->setPrintAndCutPointPairs(pointPairs);
}

void LaserControllerWindow::onActionPrintAndCutRestore(bool checked)
{
    m_scene->document()->setEnablePrintAndCut(false);
    m_scene->document()->setPrintAndCutTransform(QTransform());
}

void LaserControllerWindow::onDeviceComPortsFetched(const QStringList & ports)
{
    for (int i = 0; i < ports.size(); i++)
    {
        m_comboBoxDevices->addItem(ports[i], utils::parsePortName(ports[i]));
    }

    if (!ports.isEmpty())
    {
        LaserApplication::driver->initComPort(ports[0]);
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

void LaserControllerWindow::onMainCardRegistrationChanged(bool registered)
{
    if (registered)
        m_statusBarRegister->setText(tr("Registered"));
    else
        m_statusBarRegister->setText(tr("Unregistered"));
}

void LaserControllerWindow::onMainCardActivationChanged(bool activated)
{
    if (activated)
        m_statusBarActivation->setText(tr("Activated"));
    else
        m_statusBarActivation->setText(tr("Inactivated"));
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
    if (!m_viewer) {
        return;
    }
	if (items.length() == 0) {
        QAction* mirrorH = m_ui->actionMirrorHorizontal;		
		m_ui->actionMirrorHorizontal->setEnabled(false);
		m_ui->actionMirrorVertical->setEnabled(false);
        m_ui->actionMirrorAcrossLine->setEnabled(false);
		m_ui->actionCopy->setEnabled(false);
		//m_ui->actionPaste->setEnabled(false);
		m_ui->actionCut->setEnabled(false);
		m_ui->actionDuplication->setEnabled(false);
		m_ui->actionDeletePrimitive->setEnabled(false);
		m_ui->actionGroup->setEnabled(false);
		m_ui->actionUngroup->setEnabled(false);
        m_viewer->setMirrorLine(nullptr);
	}
	else if (items.length() > 0) {
		m_ui->actionMirrorHorizontal->setEnabled(true);
		m_ui->actionMirrorVertical->setEnabled(true);
        m_ui->actionMirrorAcrossLine->setEnabled(false);
		m_ui->actionCopy->setEnabled(true);
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
    //判断显示哪个属性面板，shape properties panel
    showShapePropertyPanel();
}

void LaserControllerWindow::onLaserPrimitiveGroupItemChanged()
{
    //qDebug() << "onLaserPrimitiveGroupItemChanged";
    if (!m_viewer) {
        return;
    }
    LaserPrimitiveGroup* group = m_viewer->group();
    if (!group || !m_propertyWidget) {
        return;
    }
    int i = group->childItems().size();
    if (i == 0) {
        m_propertyWidget->setEnabled(false);
    }
    else if (i > 0) {
        m_propertyWidget->setEnabled(true);
        selectedChange();
    }
}

void LaserControllerWindow::retranslate()
{
    m_ui->retranslateUi(this);

    m_dockOperations->setWindowTitle(tr("Operations"));
    m_dockLaserPower->setWindowTitle(tr("Laser Power"));


    qobject_cast<QLabel*>(m_formLayoutLaserPower->itemAt(0, QFormLayout::LabelRole)->widget())->setText(Config::UserRegister::scanLaserPowerItem()->title());
    qobject_cast<QLabel*>(m_formLayoutLaserPower->itemAt(1, QFormLayout::LabelRole)->widget())->setText(Config::UserRegister::maxScanGrayRatioItem()->title());
    qobject_cast<QLabel*>(m_formLayoutLaserPower->itemAt(2, QFormLayout::LabelRole)->widget())->setText(Config::UserRegister::minScanGrayRatioItem()->title());
    qobject_cast<QLabel*>(m_formLayoutLaserPower->itemAt(3, QFormLayout::LabelRole)->widget())->setText(Config::UserRegister::defaultMaxCuttingPowerItem()->title());
    qobject_cast<QLabel*>(m_formLayoutLaserPower->itemAt(4, QFormLayout::LabelRole)->widget())->setText(Config::UserRegister::defaultMinCuttingPowerItem()->title());
    qobject_cast<QLabel*>(m_formLayoutLaserPower->itemAt(5, QFormLayout::LabelRole)->widget())->setText(Config::UserRegister::spotShotPowerItem()->title());

    m_posXLabel->setText(tr("Pos X"));
    m_posXLabel->setText(tr("Pos Y"));
    m_propertyWidthLabel->setText(tr("Width"));
    m_propertyHeightLabel->setText(tr("Height"));
    m_rotateLabel->setText(tr("Rotate"));
    m_fontAlignX->setItemText(0, tr("Left"));
    m_fontAlignX->setItemText(1, tr("Middle"));
    m_fontAlignX->setItemText(2, tr("Right"));
    m_fontAlignY->setItemText(0, tr("Top"));
    m_fontAlignY->setItemText(1, tr("Middle"));
    m_fontAlignY->setItemText(2, tr("Bottom"));
    qobject_cast<QLabel*>(m_textLayout->itemAtPosition(0, 0)->widget())->setText(tr("Font"));
    qobject_cast<QLabel*>(m_textLayout->itemAtPosition(1, 0)->widget())->setText(tr("Height"));
    qobject_cast<QLabel*>(m_textLayout->itemAtPosition(0, 2)->widget())->setText(tr("Align X"));
    qobject_cast<QLabel*>(m_textLayout->itemAtPosition(1, 2)->widget())->setText(tr("Align Y"));
    qobject_cast<QLabel*>(m_textLayout->itemAtPosition(0, 4)->widget())->setText(tr("Spacing X"));
    qobject_cast<QLabel*>(m_textLayout->itemAtPosition(1, 4)->widget())->setText(tr("Spacing Y"));
    qobject_cast<QLabel*>(m_textLayout->itemAtPosition(0, 6)->widget())->setText(tr("Bold"));
    qobject_cast<QLabel*>(m_textLayout->itemAtPosition(0, 8)->widget())->setText(tr("Italic"));
    qobject_cast<QLabel*>(m_textLayout->itemAtPosition(1, 6)->widget())->setText(tr("Upper Case"));
}

void LaserControllerWindow::onLaserSceneFocusItemChanged(QGraphicsItem *, QGraphicsItem *, Qt::FocusReason)
{
    qDebug() << "";
}

void LaserControllerWindow::onLaserViewerMouseMoved(const QPointF & pos)
{
    qreal x = Global::convertToMM(SU_PX, pos.x());
    qreal y = Global::convertToMM(SU_PX, pos.y());
    QPointF posMM = QPointF(x, y);
    posMM = LaserApplication::device->transform().map(posMM);
    QString posStr = QString("%1mm,%2mm | %3px,%4px")
        .arg(posMM.x(), 8, 'f', 3).arg(posMM.y(), 8, 'f', 3)
        .arg(qFloor(pos.x()), 5).arg(qFloor(pos.y()), 5);
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
    m_lineEditCoordinatesX->setText(QString::number(state.pos.x() / 1000, 'f', 2));
    m_lineEditCoordinatesY->setText(QString::number(state.pos.y() / 1000, 'f', 2));
    m_lineEditCoordinatesZ->setText(QString::number(state.pos.z() / 1000, 'f', 2));
}

void LaserControllerWindow::onLayoutChanged(const QSizeF& size)
{
    m_statusBarPageInfo->setText(tr("Page Size(mm): %1x%2")
        .arg(LaserApplication::device->layoutWidth())
        .arg(LaserApplication::device->layoutHeight()));
}

void LaserControllerWindow::onFloatEditSliderLaserPower(qreal value)
{
    //qLogD << "real time laser power: " << value;
}

void LaserControllerWindow::onFloatDualEditSliderLowerValueChanged(qreal value)
{
    Config::SystemRegister::laserMinPowerItem()->setValue(value, MB_Widget);
}

void LaserControllerWindow::onFloatDualEditSliderHigherValueChanged(qreal value)
{
    Config::SystemRegister::laserMaxPowerItem()->setValue(value, MB_Widget);
}

void LaserControllerWindow::onUserOriginRadioButtonChanged(bool checked)
{
    if (!checked)
        return;

    QRadioButton* rb = qobject_cast<QRadioButton*>(sender());
    if (rb)
    {
        int originIndex = rb->property("origin").toInt();
        Config::Device::userOriginSelectedItem()->setValue(originIndex, MB_Widget);
    }
}

void LaserControllerWindow::onCreatSpline()
{
	m_viewer->createSpline();
}

void LaserControllerWindow::onDocumentExportFinished(const QString& filename)
{
    if (!m_prepareMachining)
        return;

    QFileInfo fileInfo(filename);
    QString filePath = fileInfo.absoluteFilePath();
#ifdef Q_OS_WIN
    filePath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    //filePath = fileInfo.absoluteFilePath().replace("/", "\\");
#endif
    LaserApplication::driver->loadDataFromFile(filePath);
    //LaserApplication::driver->startMachining(0);
}

void LaserControllerWindow::onPreviewWindowProgressUpdated(qreal progress)
{
    m_statusBarProgress->setValue(qRound(progress * 100));
}

void LaserControllerWindow::onUserOriginConfigValueChanged(const QVariant& index, ModifiedBy modifiedBy)
{
    updateUserOriginSelection(index);
}

void LaserControllerWindow::updateUserOriginSelection(const QVariant& index)
{
    int originIndex = index.toInt();
    QRadioButton* dest = nullptr;
    switch (originIndex)
    {
    case 0:
        dest = m_radioButtonUserOrigin1;
        break;
    case 1:
        dest = m_radioButtonUserOrigin2;
        break;
    case 2:
        dest = m_radioButtonUserOrigin3;
        break;
    }
    if (dest && !dest->isChecked())
    {
        dest->blockSignals(true);
        dest->setChecked(true);
        dest->blockSignals(false);
    }
}

void LaserControllerWindow::lightOnLaser()
{
    LaserApplication::driver->testLaserLight(true);
}

void LaserControllerWindow::lightOffLaser()
{
    LaserApplication::driver->testLaserLight(false);
}

void LaserControllerWindow::updatePostEventWidgets(int index)
{
    if (index == 3)
    {
        m_radioButtonUserOrigin1->setEnabled(true);
        m_radioButtonUserOrigin2->setEnabled(true);
        m_radioButtonUserOrigin3->setEnabled(true);

        /*m_doubleSpinBoxOrigin1X->setEnabled(true);
        m_doubleSpinBoxOrigin1Y->setEnabled(true);
        m_doubleSpinBoxOrigin2X->setEnabled(true);
        m_doubleSpinBoxOrigin2Y->setEnabled(true);
        m_doubleSpinBoxOrigin3X->setEnabled(true);
        m_doubleSpinBoxOrigin3Y->setEnabled(true);*/
    }
    else
    {
        m_radioButtonUserOrigin1->setEnabled(false);
        m_radioButtonUserOrigin2->setEnabled(false);
        m_radioButtonUserOrigin3->setEnabled(false);

        /*m_doubleSpinBoxOrigin1X->setEnabled(false);
        m_doubleSpinBoxOrigin1Y->setEnabled(false);
        m_doubleSpinBoxOrigin2X->setEnabled(false);
        m_doubleSpinBoxOrigin2Y->setEnabled(false);
        m_doubleSpinBoxOrigin3X->setEnabled(false);
        m_doubleSpinBoxOrigin3Y->setEnabled(false);*/
    }
}

void LaserControllerWindow::laserBackToMachiningOriginalPoint(bool checked)
{
    QVector3D dest;
    if (m_radioButtonUserOrigin1->isChecked())
    {
        //dest = QVector3D(m_doubleSpinBoxOrigin1X->value(), m_doubleSpinBoxOrigin1Y->value(), 0.f);
    }
    else if (m_radioButtonUserOrigin2->isChecked())
    {
        //dest = QVector3D(m_doubleSpinBoxOrigin2X->value(), m_doubleSpinBoxOrigin2Y->value(), 0.f);
    }
    else if (m_radioButtonUserOrigin3->isChecked())
    {
        //dest = QVector3D(m_doubleSpinBoxOrigin3X->value(), m_doubleSpinBoxOrigin3Y->value(), 0.f);
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
    m_treeWidgetOutline->clear();

    if (!m_scene->document())
        return;

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
        connect(doc, &LaserDocument::outlineUpdated, this, &LaserControllerWindow::updateOutlineTree);
        connect(doc, &LaserDocument::exportFinished, this, &LaserControllerWindow::onDocumentExportFinished);

        doc->bindLayerButtons(m_layerButtons);
		m_layerButtons[m_viewer->curLayerIndex()]->setCheckedTrue();
        m_tableWidgetLayers->setDocument(doc);
        m_tableWidgetLayers->updateItems();
        setWindowTitle(doc->name());
        //undo
        m_viewer->undoStack()->clear();
        LaserViewer* viewer = qobject_cast<LaserViewer*>(m_scene->views()[0]);
        if (m_viewer) {
            QRectF rect = LaserApplication::device->boundingRect();

            m_scene->setSceneRect(QRectF(QPointF(-5000000, -5000000), QPointF(5000000, 5000000)));
            m_viewer->setTransformationAnchor(QGraphicsView::NoAnchor);
            m_viewer->setAnchorPoint(m_viewer->mapFromScene(QPointF(0, 0)));//NoAnchor以scene的(0, 0)点为坐标原点进行变换
            LaserBackgroundItem* backgroundItem = m_scene->backgroundItem();
            if (backgroundItem) {
                backgroundItem->onChangeGrids();
            }
            m_comboBoxScale->setCurrentText("100%");
            //初始化缩放输入
            QString str = QString::number(qFloor(m_viewer->adapterViewScale() * 100)) + "%";
            m_comboBoxScale->setCurrentText(str);
            doc->open();
        }
    }
}
void LaserControllerWindow::showConfigDialog(const QString& title)
{
    qreal lastValidMaxRegion = Config::Ui::validMaxRegion();
    ConfigDialog dialog;
    if (!title.isEmpty() && !title.isNull())
        dialog.setCurrentPanel(title);
    int result = dialog.exec();   
    qreal validMaxRegion = Config::Ui::validMaxRegion();
	//关闭窗口
    if (result == QDialog::Accepted) {
        if (m_scene) {
            //改变网格
            LaserBackgroundItem* backgroudItem = m_scene->backgroundItem();
            if (backgroudItem) {
                backgroudItem->onChangeGrids();
            }
            //更新树
            if (lastValidMaxRegion != validMaxRegion) {
                m_scene->updateTree();
            }
        }
    }
    else if (result == QDialog::Rejected) {
        if (lastValidMaxRegion != validMaxRegion) {
            Config::Ui::validMaxRegionItem()->setValue(lastValidMaxRegion);
            //updata region
            m_scene->updateValidMaxRegionRect();
        }
    }
	
}
//selected items change,被选中的物体，发生移动，缩放，旋转,以线镜像变换时
void LaserControllerWindow::selectedChange()
{
	//int size = m_scene->selectedPrimitives().length();
    LaserViewer* view = qobject_cast<LaserViewer*> (m_scene->views()[0]);
    LaserPrimitiveGroup* group =  view->group();
    if (!group) {
        return;
    }
    m_propertyWidget->setEnabled(true);
    int size = view->group()->childItems().size();
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
		//qDebug() << rectReal.top();
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
void LaserControllerWindow::selectionPropertyBoxChange(int state)
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

    /*if (m_lockEqualRatio) {
        QRectF bounds = m_viewer->selectedItemsSceneBoundingRect();
        qreal widthRatio = 1;
        qreal heightRatio = 1;
        if (bounds.width() != 0) {
            widthRatio = width / bounds.width();
        }
        if (bounds.height() != 0) {
            heightRatio = height / bounds.height();
        }
        switch (state)
        {
        case PrimitiveProperty::PP_Height: {
            width *= widthRatio;
            break;
        }
        case PrimitiveProperty::PP_Width: {
            height *= heightRatio;
            break;
        }
        case PrimitiveProperty::PP_ScaleX: {
            break;
        }
        case PrimitiveProperty::PP_ScaleY: {
            break;
        }
            default:
                break;
        }
    }*/
    
	
	
	//repaint 
	m_viewer->resetSelectedItemsGroupRect(QRectF(x, y, width, height), xScale, yScale, rotate, m_selectionOriginalState, 
        m_selectionTranformState, state, m_unitIsMM);
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
    //text font bar widget
    BIND_PROP_TO_STATE(m_textFontWidget, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_textFontWidget, "enabled", false, documentEmptyState);
    //text font bar widget
    
    // actionOpen
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentWorkingState);
    // end actionOpen

    // actionImport
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentWorkingState);
    // end actionImportSVG

	// actionNew
	BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", true, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", true, documentWorkingState);
	// end actionNew

    // actionImportCorelDraw
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", true, documentWorkingState);
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
	BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, initState);
	BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, documentEmptyState);
	BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", true, documentWorkingState);
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

    // actionMirrorAcrossLine
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentEmptyState);
    // end actionMirrorAcrossLine

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

    // actionFetchToUserOrigin
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", false, devicePausedState);
    // end actionFetchToUserOrigin

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
        //LaserApplication::device->closeAboutWindow();
    }
}

void LaserControllerWindow::createNewDocument()
{
	LaserDocument* doc = new LaserDocument(m_scene);
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
	//layer color buttons not enabled
	for each(LayerButton* button in m_layerButtons) {
		//button->setEnabled(false);
		button->setChecked(false);
	}
    updateOutlineTree();
}

void LaserControllerWindow::updateAutoRepeatDelayChanged(const QVariant& value, ModifiedBy modifiedBy)
{
    m_buttonMoveTopLeft->setAutoRepeatDelay(value.toInt());
    m_buttonMoveTop->setAutoRepeatDelay(value.toInt());
    m_buttonMoveTopRight->setAutoRepeatDelay(value.toInt());
    m_buttonMoveLeft->setAutoRepeatDelay(value.toInt());
    m_buttonMoveRight->setAutoRepeatDelay(value.toInt());
    m_buttonMoveBottomLeft->setAutoRepeatDelay(value.toInt());
    m_buttonMoveBottom->setAutoRepeatDelay(value.toInt());
    m_buttonMoveBottomRight->setAutoRepeatDelay(value.toInt());
    m_buttonMoveUp->setAutoRepeatDelay(value.toInt());
    m_buttonMoveDown->setAutoRepeatDelay(value.toInt());
}

void LaserControllerWindow::askMergeOrNew()
{
    if (m_scene->document()) {
        // 询问关闭还是合并
        QMessageBox msgDlg;
        msgDlg.setText(tr("A document is opened."));
        msgDlg.setInformativeText(tr("Do you want to merge or create a new one?"));
        msgDlg.addButton(tr("Merge"), QMessageBox::ButtonRole::YesRole);
        msgDlg.addButton(tr("New"), QMessageBox::ButtonRole::NoRole);
        msgDlg.setDefaultButton(QMessageBox::Yes);
        int ret = msgDlg.exec();

        if (ret == 1)
        {
            if (onActionCloseDocument()) {
                createNewDocument();
            }
        }
	}
    else
    {
        createNewDocument();
    }
}

//void LaserControllerWindow::updateAutoRepeatIntervalChanged(const QVariant& value, ModifiedBy modifiedBy)
//{
//    m_buttonMoveTopLeft->setAutoRepeatInterval(value.toInt());
//    m_buttonMoveTop->setAutoRepeatInterval(value.toInt());
//    m_buttonMoveTopRight->setAutoRepeatInterval(value.toInt());
//    m_buttonMoveLeft->setAutoRepeatInterval(value.toInt());
//    m_buttonMoveRight->setAutoRepeatInterval(value.toInt());
//    m_buttonMoveBottomLeft->setAutoRepeatInterval(value.toInt());
//    m_buttonMoveBottom->setAutoRepeatInterval(value.toInt());
//    m_buttonMoveBottomRight->setAutoRepeatInterval(value.toInt());
//    m_buttonMoveUp->setAutoRepeatInterval(value.toInt());
//    m_buttonMoveDown->setAutoRepeatInterval(value.toInt());
//}

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


