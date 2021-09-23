#ifndef LASERCONTROLLERWINDOW_H
#define LASERCONTROLLERWINDOW_H

#include <DockManager.h>
//#include <DockWidgetTab.h>
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

class FloatEditDualSlider;
class FloatEditSlider;
class LaserLayerTableWidget;
class LaserViewer;
class LaserScene;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QGridLayout;
class QRadioButton;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;
class RulerWidget;
class ProgressBar;
class PressedToolButton;

class LaserControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    LaserControllerWindow(QWidget* parent = nullptr);
    ~LaserControllerWindow();

    void moveLaser(const QVector3D& delta, bool relative = true, const QVector3D& abstractDest = QVector3D());
    FinishRun finishRun();

    void createUpdateDockPanel(int winId);

    LaserDocument* currentDocument() const;
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
	
protected:
    void createCentralDockPanel();
    void createLayersDockPanel();
    void createCameraDockPanel();
    void createOperationsDockPanel();
    void createOutlineDockPanel();
    void createMovementDockPanel();
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
    void onActionLaserSpotShot(bool checked = false);
    void onActionLaserCut(bool checked = false);
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

    /*void onButtonMoveTopLeft();
    void onButtonMoveTop();
    void onButtonMoveTopRight();
    void onButtonMoveBottomLeft();
    void onButtonMoveBottom();
    void onButtonMoveBottomRight();
    void onButtonMoveLeft();
    void onButtonMoveRight();
    void onButtonMoveUp();
    void onButtonMoveDown();*/

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
    void onActionUpdate(bool checked = false);
    void onActionLaserPosition(bool checked = false);

	void onActionMirrorHorizontal(bool checked = false);
	void onActionMirrorVertical(bool checked = false);
    void onActionMirrorACrossLine(bool checked = false);

    void onActionShowMainCardInfo(bool checked = false);
    void onActionTemporaryLicense(bool checked = false);
    void onActionAbout(bool checked = false);

    void onActionUpdateOutline(bool checked = false);

    void onDeviceComPortsFetched(const QStringList& ports);
    void onDeviceConnected();
    void onDeviceDisconnected();
    void onMainCardRegistered();
    void onMainCardActivated();

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
    void onFloatEditSliderLaserPower(qreal value);
    void onFloatDualEditSliderLowerValueChanged(qreal value);
    void onFloatDualEditSliderHigherValueChanged(qreal value);
    void onLaserMinPowerChanged(const QVariant& value, ModifiedBy modifiedBy);
    void onLaserMaxPowerChanged(const QVariant& value, ModifiedBy modifiedBy);

	void onCreatSpline();
    void onDocumentExportFinished(const QString& filename);

    void onPreviewWindowProgressUpdated(qreal progress);

    void lightOnLaser();
    void lightOffLaser();
    void readMachiningOrigins(bool checked = false);
    void writeMachiningOrigins(bool checked = false);
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
	void selectionPropertyBoxChange();//doubleSpinBox's enter or mouse lost focus
	void onSelectionOriginalClicked(bool clicked);
	//undo
	void onUndoStackCleanChanged(bool clean);
	void onCanUndoChanged(bool can);
	void onCanRedoChanged(bool can);

    // config items
    void updateAutoRepeatDelayChanged(const QVariant& value, ModifiedBy modifiedBy);
    //void updateAutoRepeatIntervalChanged(const QVariant& valu, ModifiedBy modifiedBye);

    

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
    ads::CDockAreaWidget* m_dockAreaCameras;

    // Operations Panel widgets
    QToolButton* m_buttonOperationStart;
    QToolButton* m_buttonOperationPause;
    QToolButton* m_buttonOperationStop;
    QToolButton* m_buttonOperationBounding;
    QToolButton* m_buttonOperationSpotShot;
    QToolButton* m_buttonOperationReset;
    QToolButton* m_buttonOperationOrigin;
    QToolButton* m_buttonOperationOptimize;
    QComboBox* m_comboBoxStartPosition;
    FloatEditSlider* m_floatEditSliderLaserPower;
    FloatEditDualSlider* m_floatEditDualSliderLaserRange;
    QComboBox* m_comboBoxDevices;
    QToolButton* m_buttonConnect;
    QToolButton* m_buttonRefresh;
    ads::CDockAreaWidget* m_dockAreaOperations;

    // Outline Panel widgets
    QTreeWidget* m_treeWidgetOutline;
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

    QCheckBox* m_checkBoxXEnabled;
    QCheckBox* m_checkBoxYEnabled;
    QCheckBox* m_checkBoxZEnabled;

    QToolButton* m_buttonLaserPosition;
    QComboBox* m_comboBoxPostEvent;
    QRadioButton* m_radioButtonMachiningOrigin1;
    QRadioButton* m_radioButtonMachiningOrigin2;
    QRadioButton* m_radioButtonMachiningOrigin3;
    QDoubleSpinBox* m_doubleSpinBoxOrigin1X;
    QDoubleSpinBox* m_doubleSpinBoxOrigin1Y;
    QDoubleSpinBox* m_doubleSpinBoxOrigin2X;
    QDoubleSpinBox* m_doubleSpinBoxOrigin2Y;
    QDoubleSpinBox* m_doubleSpinBoxOrigin3X;
    QDoubleSpinBox* m_doubleSpinBoxOrigin3Y;
    QToolButton* m_buttonReadOrigins;
    QToolButton* m_buttonWriteOrigins;
    ads::CDockAreaWidget* m_dockAreaMovement;

    // Update Panel widgets
    ads::CDockAreaWidget* m_dockAreaUpdate;

    bool m_created;
    QDir m_tmpDir;
    QString m_currentJson;
    bool m_useLoadedJson;
    bool m_prepareMachining;

    // widgets on status bar
    QLabel* m_statusBarStatus;
    QLabel* m_statusBarRegister;
    QLabel* m_statusBarActivation;
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
	LaserDoubleSpinBox* m_rotateBox;
	QToolButton* m_mmOrIn;
	bool m_unitIsMM;
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
    ads::CDockWidget* m_propertyDockWidget;

    friend class LaserApplication;
};

#endif // LASERCONTROLLERWINDOW_H