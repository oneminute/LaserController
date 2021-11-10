#ifndef LASERCONTROLLERWINDOW_H
#define LASERCONTROLLERWINDOW_H

#include <DockManager.h>
//#include <DockWidgetTab.h>
#include <QtConcurrent>
#include <QObject>
#include <QMainWindow>
#include <QScopedPointer>
#include <QDir>
#include <QState>
#include <QFontComboBox>
#include "scene/LaserLayer.h"
#include "widget/LayerButton.h"
#include "laser/LaserDriver.h"
#include "widget/LaserDoubleSpinBox.h"
#include "widget/LaserFontComboBox.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LaserControllerWindow; }
QT_END_NAMESPACE

class EditSlider;
class FloatEditDualSlider;
class FloatEditSlider;
class QFormLayout;
class LaserLayerTableWidget;
class LaserViewer;
class LaserScene;
class Label;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QGridLayout;
class QRadioButton;
class QTableWidget;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;
class RulerWidget;
class ProgressBar;
class PressedToolButton;
class Vector2DWidget;
class RadioButtonGroup;
class PointPairTableWidget;
class UpdateDialog;

class LaserControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    LaserControllerWindow(QWidget* parent = nullptr);
    ~LaserControllerWindow();

    FinishRunType finishRun();

    LaserDocument* currentDocument() const;

    bool lockEqualRatio();
    LaserDoubleSpinBox* widthBox();
    LaserDoubleSpinBox* heightBox();
    void setLastCornerRadiusValue(qreal val);
    qreal lastCornerRadiusValue();
    LaserDoubleSpinBox* fontSpaceYDoubleSpinBox();

    PointPairList printAndCutPoints() const;
    
public slots:
    void handleSecurityException(int code, const QString& message);
    void onFontComboBoxHighLighted(int index);
    void onFontComboBoxActived(int index);
    void onAlignHBoxChanged(int index);
    void onAlignVBoxChanged(int index);
    void onChangeFontComboBoxByEditingText();
    void onFontComboBoxHidePopup();
    void onFontHeightBoxEnterOrLostFocus();
    void onFontBoldStateChanged();
    void onFontItalicStateChanged();
    void onFontUpperStateChanged();
    void onFontSpaceXEnterOrLostFocus();
    void onFontSpaceYEnterOrLostFocus();
    void onLaserPrimitiveGroupItemChanged();

    void retranslate();
	
protected:
    void createCentralDockPanel();
    void createLayersDockPanel();
    void createCameraDockPanel();
    void createOperationsDockPanel();
    void createOutlineDockPanel();
    void createMovementDockPanel();
    void createLaserPowerDockPanel();
    void createPrintAndCutPanel();
    //shape properties panel / dock panel
    void createShapePropertyDockPanel();
    //show shape properties
    void showShapePropertyPanel();
    void createPrimitivePropertiesPanel();
    //shape properties panel / dock panel
    void createPrimitiveLinePropertyPanel();
    void createPrimitiveRectPropertyPanel();
    void createPrimitiveEllipsePropertyPanel();
    void createPrimitivePloygonPropertyPanel();
    void createPrimitivePloylinePropertyPanel();
    void createPrimitiveTextPropertyPanel();
    void createPrimitiveBitmapPropertyPanel();
    void createEmptyPropertyPanel();
    void createMixturePropertyPanel();
    //OnlyShowIcon
    void dockPanelOnlyShowIcon(ads::CDockWidget* dockWidget, QPixmap icon, char* text);

	//key
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void contextMenuEvent(QContextMenuEvent *event) override;

protected slots:
	void onActionUndo(bool checked = false);
	void onActionRedo(bool checked);
    void onActionImport(bool checked = false);
    void onActionImportCorelDraw(bool checked = false);
    void onActionRemoveLayer(bool checked = false);
    void onTableWidgetLayersCellDoubleClicked(int row, int column);
    void onTableWidgetItemSelectionChanged();
    void onActionExportJson(bool checked = false);
    void onActionLoadJson(bool checked = false);
    void onActionMachining(bool checked = false);
    void onActionPauseMechining(bool checked = false);
    void onActionStopMechining(bool checked = false);
    void onActionBounding(bool checked = false);
    void onActionLaserSpotShot(bool checked = false);
    void onActionLaserMove(bool checked = false);
    void onActionPathOptimization(bool checked = false);
    void onActionConnect(bool checked = false);
    void onActionDisconnect(bool checked = false);
    void onActionDownload(bool checked = false);
    void onActionLoadMotor(bool checked = false);
    void onActionUnloadMotor(bool checked = false);
    void onActionWorkState(bool checked = false);
	void onActionNew(bool checked = false);
	bool onActionSave(bool checked = false);
	bool onActionSaveAs(bool checked = false);
	void onActionOpen(bool checked = false);
	void onActionZoomIn(bool checked = false);
	void onActionZoomOut(bool checked = false);
	void onActionZoomToPage(bool checked = false);
	void onActionZoomToSelection(bool checked = false);

    void onActionMoveTop();
    void onActionMoveBottom();
    void onActionMoveLeft();
    void onActionMoveRight();
    void onActionMoveTopLeft();
    void onActionMoveTopRight();
    void onActionMoveBottomLeft();
    void onActionMoveBottomRight();
    void onActionMoveUp();
    void onActionMoveDown();

    void onMovementButtonReleased();

    void onActionHalfTone(bool checked = false);
    void onActionDeletePrimitive(bool checked = false);
	void onActionCopy(bool checked = false);
	void onActionPaste(bool checked = false);
	void onActionPasteInPlace(bool checked = false);
	void onActionCut(bool checked = false);
	void onActionDuplication(bool checked = false);
	void onActionGroup(bool checked = false);
	void onActionUngroup(bool checked = false);
	bool onActionCloseDocument(bool checked = false);
    void onActionSettings(bool checked = false);
    void onActionSelectionTool(bool checked = false);
	void onActionViewDrag(bool checked = false);
	void onActionRectangle(bool checked = false);
	void onActionEllipse(bool checked = false);
	void onActionLine(bool checked = false);
	void onActionPolygon(bool checked = false);
	void onActionSpline(bool checked = false);
	void onActionSplineEdit(bool checked = false);
	void onActionText(bool checked = false);
	void onActionBitmap(bool checked = false);
    void onActionRegiste(bool checked = false);
    void onStatusBarRegisterClicked(bool checked = false);
    void onActionActivate(bool checked = false);
    void onStatusBarActivationClicked(bool checked = false);
    void onActionUpdateSoftware(bool checked = false);
    void onActionUpdateFirmware(bool checked = false);
    void onActionLaserPosition(bool checked = false);
    void onProgressBarClicked();

	void onActionMirrorHorizontal(bool checked = false);
	void onActionMirrorVertical(bool checked = false);
    void onActionMirrorACrossLine(bool checked = false);

    void onActionMainCardInfo(bool checked = false);
    void onActionTemporaryLicense(bool checked = false);
    void onActionUserInfo(bool checked = false);
    void onActionAbout(bool checked = false);

    void onActionUpdateOutline(bool checked = false);
    void onActionFetchToUserOrigin(bool checked = false);
    void onActionMoveToUserOrigin(bool checked = false);

    void onActionPrintAndCutNew(bool checked = false);
    void onActionPrintAndCutFetchLaser(bool checked = false);
    void onActionPrintAndCutFetchCanvas(bool checked = false);
    void onActionPrintAndCutRemove(bool checked = false);
    void onActionPrintAndCutClear(bool checked = false);
    void onActionPrintAndCutAlign(bool checked = false);
    void onActionPrintAndCutRestore(bool checked = false);
    void onActionPrintAndCutSelectPoint(bool checked = false);
    void onActionPrintAndCutEndSelect(bool checked = false);
    void onActionRedLightAlignmentStart(bool checked = false);
    void onActionRedLightAlignmentFinish(bool checked = false);

    void onDeviceComPortsFetched(const QStringList& ports);
    void onDeviceConnected();
    void onDeviceDisconnected();
    void onMainCardRegistrationChanged(bool registered);
    void onMainCardActivationChanged(bool activated);

    void onWindowCreated();
    virtual void closeEvent(QCloseEvent* event) override;

    void onEnterDeviceUnconnectedState();
    void onEnterDeviceConnectedState();
    void onActionMoveLayerUp(bool checked = false);
    void onActionMoveLayerDown(bool checked = false);

    void onLaserViewerMouseMoved(const QPointF& pos);
    void onLaserViewerScaleChanged(qreal factor);
    void onComboBoxSxaleIndexChanged(int index);
    void onComboBoxSxaleTextChanged(const QString& text);

    void onLaserReturnWorkState(LaserState state);
    void onLayoutChanged(const QSizeF& size);
    void onFloatEditSliderLaserPower(qreal value);
    void onFloatDualEditSliderLowerValueChanged(qreal value);
    void onFloatDualEditSliderHigherValueChanged(qreal value);

    void onUserOriginRadioButtonChanged(bool checked);

	void onCreatSpline();
    void onDocumentExportFinished(const QString& filename);

    void onPreviewWindowProgressUpdated(qreal progress);
    void onUserOriginConfigValueChanged(const QVariant& index, ModifiedBy modifiedBy);
    void updateUserOriginSelection(const QVariant& index);

    void lightOnLaser();
    void lightOffLaser();
    void updatePostEventWidgets(int index);
    void laserBackToMachiningOriginalPoint(bool checked = false);
    void laserResetToOriginalPoint(bool checked = false);
    void updateOutlineTree();
    void initDocument(LaserDocument* doc);
    void showConfigDialog(const QString& title = QString());

	//selection
	void onLaserSceneSelectedChanged();
    
    void onLaserSceneFocusItemChanged(QGraphicsItem *, QGraphicsItem *, Qt::FocusReason);
	void selectedChange();//items
	void selectionPropertyBoxChange(int state);//doubleSpinBox's enter or mouse lost focus
	void onSelectionOriginalClicked(bool clicked);
	//undo
	void onUndoStackCleanChanged(bool clean);
	void onCanUndoChanged(bool can);
	void onCanRedoChanged(bool can);

    // config items
    void updateAutoRepeatDelayChanged(const QVariant& value, ModifiedBy modifiedBy);
    //void updateAutoRepeatIntervalChanged(const QVariant& valu, ModifiedBy modifiedBye);

    void askMergeOrNew();

    void applyJobOriginToDocument(const QVariant& value);

private:
    QString getFilename(const QString& title, const QString& filters = "");
    void bindWidgetsProperties();
    virtual void showEvent(QShowEvent *event);
	void createNewDocument();
	QString getCurrentFileName();
	void documentClose();

signals:
    void windowCreated();
    void isIdle();
    void readyRectangle();
	void readyEllipse();
	void readyLine();
	void readyPolygon();
	void readySpline();
	void readySplineEdit();
	void readyText();
	void readyViewDrag();
    void startPrintAndCut();
    void finishPrintAndCut();

private:
    QScopedPointer<Ui::LaserControllerWindow> m_ui;
	QState* m_lastState;
    // Central Panel widgets
    ads::CDockManager* m_dockManager;
    LaserViewer* m_viewer;
    LaserScene* m_scene;
    RulerWidget* m_hRuler;
    RulerWidget* m_vRuler;
    QComboBox* m_comboBoxScale;
    ads::CDockAreaWidget* m_centralDockArea;

    // Layers Panel widgets
    LaserLayerTableWidget* m_tableWidgetLayers;
    QToolButton* m_buttonMoveLayerUp;
    QToolButton* m_buttonMoveLayerDown;
    QToolButton* m_buttonRemoveLayer;
    ads::CDockWidget* m_dockLayers;
    ads::CDockAreaWidget* m_dockAreaLayers;

    // Camera Panel widgets
    QComboBox* m_comboBoxCameras;
    QToolButton* m_buttonCameraUpdateOverlay;
    QToolButton* m_buttonCameraTrace;
    QToolButton* m_buttonCameraSaveSettings;
    QCheckBox* m_checkBoxCameraFade;
    QDoubleSpinBox* m_doubleSpinBoxCameraFadeWidth;
    QDoubleSpinBox* m_doubleSpinBoxCameraFadeHeight;
    QCheckBox* m_checkBoxCameraShow;
    QDoubleSpinBox* m_doubleSpinBoxCameraXShift;
    QDoubleSpinBox* m_doubleSpinBoxCameraYShift;
    ads::CDockWidget* m_dockCameras;
    ads::CDockAreaWidget* m_dockAreaCameras;

    // Operations Panel widgets
    QToolButton* m_buttonOperationStart;
    QToolButton* m_buttonOperationPause;
    QToolButton* m_buttonOperationStop;
    QToolButton* m_buttonOperationBounding;
    QToolButton* m_buttonOperationSpotShot;
    QToolButton* m_buttonOperationReset;
    QToolButton* m_buttonOperationOrigin;
    QComboBox* m_comboBoxStartPosition;
    RadioButtonGroup* m_radioButtonGroupJobOrigin;
    QComboBox* m_comboBoxDevices;
    QToolButton* m_buttonConnect;
    QToolButton* m_buttonRefresh;
    ads::CDockWidget* m_dockOperations;
    ads::CDockAreaWidget* m_dockAreaOperations;

    // Outline Panel widgets
    QTreeWidget* m_treeWidgetOutline;
    ads::CDockWidget* m_dockOutline;
    ads::CDockAreaWidget* m_dockAreaOutline;

    // Movement Panel widgets
    QLineEdit* m_lineEditCoordinatesX;
    QLineEdit* m_lineEditCoordinatesY;
    QLineEdit* m_lineEditCoordinatesZ;
    QDoubleSpinBox* m_doubleSpinBoxDistanceX;
    QDoubleSpinBox* m_doubleSpinBoxDistanceY;
    QDoubleSpinBox* m_doubleSpinBoxDistanceZ;

    PressedToolButton* m_buttonMoveTopLeft;
    PressedToolButton* m_buttonMoveTop;
    PressedToolButton* m_buttonMoveTopRight;
    PressedToolButton* m_buttonMoveLeft;
    QToolButton* m_buttonMoveToOrigin;
    PressedToolButton* m_buttonMoveRight;
    PressedToolButton* m_buttonMoveBottomLeft;
    PressedToolButton* m_buttonMoveBottom;
    PressedToolButton* m_buttonMoveBottomRight;
    PressedToolButton* m_buttonMoveUp;
    PressedToolButton* m_buttonMoveDown;

    //QCheckBox* m_checkBoxXEnabled;
    //QCheckBox* m_checkBoxYEnabled;
    //QCheckBox* m_checkBoxZEnabled;

    QToolButton* m_buttonLaserPosition;
    QComboBox* m_comboBoxPostEvent;
    QRadioButton* m_radioButtonUserOrigin1;
    QRadioButton* m_radioButtonUserOrigin2;
    QRadioButton* m_radioButtonUserOrigin3;
    Vector2DWidget* m_userOrigin1;
    Vector2DWidget* m_userOrigin2;
    Vector2DWidget* m_userOrigin3;
    QToolButton* m_buttonFetchToUserOrigin;
    QToolButton* m_buttonMoveToUserOrigin;
    ads::CDockWidget* m_dockMovement;
    ads::CDockAreaWidget* m_dockAreaMovement;

    // Property panel
    LaserDoubleSpinBox* m_cutOrderPriority;
    QLabel* m_cutOrderPriorityLabel;
    LaserDoubleSpinBox* m_powerScale;
    QLabel* m_powerScaleLabel;
    LaserDoubleSpinBox* m_width;
    QLabel* m_widthLabel;
    LaserDoubleSpinBox* m_height;
    QLabel* m_heightLabel;
    LaserDoubleSpinBox* m_maxWidth;
    QLabel* m_maxWidthLabel;
    LaserDoubleSpinBox* m_cornerRadius;
    qreal m_lastCornerRadiusValue;
    QLabel* m_cornerRadiusLabel;
    QCheckBox* m_locked;
    QLabel* m_lockedLabel;
    LaserDoubleSpinBox* m_gamma;
    QLabel* m_gammaLabel;
    LaserDoubleSpinBox* m_garmma;
    QLabel* m_garmmaLabel;
    LaserDoubleSpinBox* m_brightness;
    QLabel* m_brightnessLabel;
    LaserDoubleSpinBox* m_contrast;
    QLabel* m_contrastLabel;
    LaserDoubleSpinBox* m_enhanceRadius;
    QLabel* m_enhanceRadiusLabel;
    LaserDoubleSpinBox* m_enhanceAmout;
    QLabel* m_enhanceAmoutLabel;
    LaserDoubleSpinBox* m_enhanceDenoise;
    QLabel* m_enhanceDenoiseLabel;
    QGridLayout* m_rectPropertyLayout;
    QGridLayout* m_linePropertyLayout;
    QGridLayout* m_ellipsePropertyLayout;
    QGridLayout* m_polygonPropertyLayout;
    QGridLayout* m_polylinePropertyLayout;
    QGridLayout* m_bitmapPropertyLayout;
    QGridLayout* m_textPropertyLayout;
    QGridLayout* m_nurbsPropertyLayout;
    QGridLayout* m_mixturePropertyLayout;
    QGridLayout* m_nullPropertyLayout;
    QWidget* m_propertyPanelWidget;
    QWidget* m_rectPropertyWidget;
    QWidget* m_linePropertyWidget;
    QWidget* m_ellipsePropertyWidget;
    QWidget* m_polygonPropertyWidget;
    QWidget* m_polylinePropertyWidget;
    QWidget* m_bitmapPropertyWidget;
    QWidget* m_textPropertyWidget;
    QWidget* m_nurbsPropertyWidget;
    QWidget* m_mixturePropertyWidget;
    QWidget* m_nullPropertyWidget;
    Qt::CheckState m_lastLockedState;
    ads::CDockWidget* m_propertyDockWidget;
    ads::CDockAreaWidget* m_dockAreaProperty;

    // Laser Power Panel
    FloatEditSlider* m_floatEditSliderScanLaserPower;
    EditSlider* m_editSliderScanMaxGray;
    EditSlider* m_editSliderScanMinGray;
    FloatEditSlider* m_floatEditSliderCuttingMaxPower;
    FloatEditSlider* m_floatEditSliderCuttingMinPower;
    FloatEditSlider* m_floatEditSliderSpotShotPower;
    QFormLayout* m_formLayoutLaserPower;
    ads::CDockWidget* m_dockLaserPower;
    ads::CDockAreaWidget* m_dockAreaLaserPower;

    // Print and Cut Panel
    QGroupBox* m_groupBoxPrintAndCutPoints;
    PointPairTableWidget* m_tablePrintAndCutPoints;
    QGroupBox* m_groupBoxRedLightAlignment;
    QLabel* m_labelRedLightAlignmentFirst;
    QLabel* m_labelRedLightAlignmentSecond;
    QLabel* m_labelPrintAndCutOffset;
    QToolButton* m_buttonRedLightAlignmentStart;
    QToolButton* m_buttonRedLightAlignmentFinish;
    QGroupBox* m_groupBoxPrintAndCutResult;
    QLabel* m_labelPrintAndCutTranslationResult;
    QLabel* m_labelPrintAndCutRotationResult;
    QLabel* m_labelPrintAndCutTranslation;
    QLabel* m_labelPrintAndCutRotation;
    ads::CDockWidget* m_dockPrintAndCut;
    ads::CDockAreaWidget* m_dockAreaPrintAndCut;

    bool m_created;
    QDir m_tmpDir;
    QString m_currentJson;
    bool m_useLoadedJson;
    bool m_prepareMachining;

    // widgets on status bar
    QLabel* m_statusBarStatus;
    Label* m_statusBarRegister;
    Label* m_statusBarActivation;
    QLabel* m_statusBarTips;
    ProgressBar* m_statusBarProgress;
    QLabel* m_statusBarCoordinate;
    QLabel* m_statusBarLocation;
    QLabel* m_statusBarPageInfo;
    QLabel* m_statusBarCopyright;

    QList<LayerButton*> m_layerButtons;
	//selected items properties
	QGridLayout* m_propertyLayout;
	QWidget* m_propertyWidget;
	QLabel* m_posXLabel;
	QLabel* m_posYLabel;
	LaserDoubleSpinBox* m_posXBox;
	LaserDoubleSpinBox* m_posYBox;
	QToolButton* m_lockOrUnlock;
	QLabel* m_posXUnit;
	QLabel* m_posYUnit;
    QLabel* m_propertyWidthLabel;
    QLabel* m_propertyHeightLabel;
	LaserDoubleSpinBox* m_widthBox;
	LaserDoubleSpinBox* m_heightBox;
	QLabel* m_widthUnit;
	QLabel* m_heightUnit;
	LaserDoubleSpinBox* m_xRateBox;
	LaserDoubleSpinBox* m_yRateBox;
	QRadioButton* m_topLeftBtn;
	QRadioButton* m_topCenterBtn;
	QRadioButton* m_topRightBtn;
	QRadioButton* m_bottomLeftBtn;
	QRadioButton* m_bottomCenterBtn;
	QRadioButton* m_bottomRightBtn;
	QRadioButton* m_leftCenterBtn;
	QRadioButton* m_centerBtn;
	QRadioButton* m_rightCenterBtn;
    QLabel* m_rotateLabel;
	LaserDoubleSpinBox* m_rotateBox;
	QToolButton* m_mmOrIn;
	bool m_unitIsMM;
    bool m_lockEqualRatio;
	//selection tool bar
	int m_selectionOriginalState;
	int m_selectionTranformState;

	QString m_fileDirection;
	QString m_fileName;
	//QString m_windowTitle;
	//Text 
    QGridLayout* m_textLayout;
    QWidget * m_textFontWidget;
    LaserFontComboBox* m_fontFamily;
    LaserDoubleSpinBox* m_fontHeight;
    QComboBox* m_fontAlignX;
    QComboBox* m_fontAlignY;
    QCheckBox* m_fontBold;
    QCheckBox* m_fontItalic;
    QCheckBox* m_fontUpper;
    LaserDoubleSpinBox* m_fontSpaceX;
    LaserDoubleSpinBox* m_fontSpaceY;
    int m_fontComboxLightedIndex;
    //dock panel/ shape properties panel
    
    UpdateDialog* m_updateDialog;

    QFutureWatcher<LaserDocument*> m_watcher;

    QPointF m_redLightAlignment1stPt;
    QPointF m_redLightAlignment2ndPt;

    friend class LaserApplication;
};

#endif // LASERCONTROLLERWINDOW_H
