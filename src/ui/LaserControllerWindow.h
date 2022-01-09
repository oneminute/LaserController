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
#include "widget/LaserToolButton.h"
#include "ui/LaserMeuu.h"
#include "scene/LaserPrimitive.h"

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
class LaserMenu;

class LaserControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    LaserControllerWindow(QWidget* parent = nullptr);
    ~LaserControllerWindow();
    bool m_hasMessageBox;

    FinishRunType finishRun();

    LaserDocument* currentDocument() const;

    bool lockEqualRatio();
    LaserDoubleSpinBox* widthBox();
    LaserDoubleSpinBox* heightBox();
    //void setLastCornerRadiusValue(qreal val);
    //qreal lastCornerRadiusValue();
    LaserDoubleSpinBox* fontSpaceYDoubleSpinBox();

    PointPairList printAndCutPoints() const;
    bool hasPrintAndCutPoints() const;

    LaserViewer* viewer() const { return m_viewer; }
    LaserScene* scene() const { return m_scene; }

    void findPrintAndCutPoints(const QRect& bounding);
    QList<QPoint> printAndCutCandidatePoints() const
    {
        return m_printAndCutCandidatePoints;
    }
    void clearPrintAndCutCandidatePoints();
    int selectedPrintAndCutPoint() const
    {
        return m_selectedPrintAndCutPointIndex;
    }
    void setPrintAndCutPoint(const QPoint& pt);
    int hoveredPrintAndCutPoint(const QPoint& mousePos) const;
    LaserPrimitive* alignTarget();
    LaserDoubleSpinBox* textSpaceYSpinBox();
    //align
    void initAlignTarget();
    void changeAlignButtonsEnable();
    void tabAlignTarget();
    void setAlignTargetState(bool isAlignTarget);
    //shapes weld / tow shapes unite
    //void changeShapesWeldButtonsEnable();
    LaserPath* uniteTwoShapes(LaserPrimitive* p1, LaserPrimitive* p2, 
        LaserLayer* layer, QSet<LaserPrimitive*>* joinedGroup);
    //selection property
    bool unitIsMM();
    QLabel* labelPercentage();
    
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
    void onLaserPrimitiveGroupItemTransformChanged();

	void newDocument();
	void closeDocument();
    void startMachining();
    void updateLayers();

    void retranslate();
	
protected:
    //recent files
    void loadRecentFilesMenu();
    void addRecentFile(QString path);
    void deleteRecentFile(QString path);
    void updataRecentFilesActions();
    void updataRecentFilesFile();
    QString RecentFilesFilePath();
    //panel
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

    QList<QPoint> findCanvasPointsWithinRect(const QRect& bounding) const;

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
    void onActionMultiDuplication(bool checked = false);
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
    void onActionShowLaserPosition(bool checked = false);
    void onActionHideLaserPosition(bool checked = false);
    void onActionSaveZOrigin(bool checked = false);
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

    void onActionCameraTools(bool checked = false);
    void onActionCameraCalibration();
    void onActionGenerateCalibrationBoard();
    void onActionCameraAlignment();

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
    void onComboBoxSxaleTextChanged(const QString& text);

    void onLaserReturnWorkState(DeviceState state);
    void onLayoutChanged(const QSize& size);
    void onFloatEditSliderLaserPower(qreal value);

    void onUserOriginRadioButtonChanged(bool checked);

	void onCreatSpline();
    //void onDocumentExportFinished(const QString& filename);
    void onDocumentExportFinished(const QByteArray& data);

    void onPreviewWindowProgressUpdated(qreal progress);
    void onUserOriginConfigValueChanged(const QVariant& index, void* senderPtr);
    void updateUserOriginSelection(const QVariant& index);

    void onStateEntered(QAbstractState* state);

    void lightOnLaser();
    void lightOffLaser();
    void updatePostEventWidgets(int index);
    void laserBackToMachiningOriginalPoint(bool checked = false);
    void laserResetToOriginalPoint(bool checked = false);
    void moveToZOrigin(bool checked = false);
    void updateOutlineTree();
    void initDocument(LaserDocument* doc);
    void showConfigDialog(const QString& title = QString());

	//selection
	void onLaserSceneSelectedChanged();//scene emit
	void selectedChangedFromMouse();//items
	void selectionPropertyBoxChange(int state);//doubleSpinBox's enter or mouse lost focus
	void onSelectionOriginalClicked(bool clicked);
	//undo
	void onUndoStackCleanChanged(bool clean);
	void onCanUndoChanged(bool can);
	void onCanRedoChanged(bool can);

    // config items
    void deviceOriginChanged(const QVariant& value, void* senderPtr);
    void jobOriginChanged(const QVariant& value, void* senderPtr);
    void startFromChanged(const QVariant& value, void* senderPtr);
    void userOriginChanged(const QVariant& value, void* senderPtr);

    void askMergeOrNew();

    void applyJobOriginToDocument(const QVariant& value);
    //arrange align
    void onActionAlignCenter();
    void onActionAlignHorinzontalMiddle();
    void onActionAlignHorinzontalTop();
    void onActionAlignHorinzontalBottom();
    void onActionAlignVerticalMiddle();
    void onActionAlignVerticalLeft();
    void onActionAlignVerticalRight();
    void onActionDistributeVSpaced();
    void onActionDistributeVCentered();
    void onActionDistributeHSpaced();
    void onActionDistributeHCentered();
    void onActionSameWidth();
    void onActionSameHeight();
    void onActionMovePageToTopLeft();
    void onActionMovePageToTopRight();
    void onActionMovePageToBottomRight();
    void onActionMovePageToBottomLeft();
    void onActionMovePageToCenter();
    void onActionMovePageToTop();
    void onActionMovePageToBottom();
    void onActionMovePageToLeft();
    void onActionMovePageToRight();
    void onActionSelectAll();
    void onActionInvertSelect();
    void onActionTwoShapesUnite();

public slots:
    void onLaserPrimitiveGroupChildrenChanged();//group emit
    void onJoinedGroupChanged();
    void onLaserToolButtonShowMenu();
    void onClickedMmOrInch();

private:
    QString getFilename(const QString& title, const QString& filters = "");
    void bindWidgetsProperties();
    virtual void showEvent(QShowEvent *event);
	QString getCurrentFileName();

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
    void startPrintAndCutSelecting();
    void finishPrintAndCutSelecting();
    void startPrintAndCutAligning();
    void finishPrintAndCutAligning();
    void joinedGroupChanged();

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
    QLabel* m_labelPercentage;
    ads::CDockAreaWidget* m_centralDockArea;
    QRect m_layoutRect;

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
    QToolButton* m_buttonMoveToZOrigin;
    PressedToolButton* m_buttonMoveDown;

    //QCheckBox* m_checkBoxXEnabled;
    //QCheckBox* m_checkBoxYEnabled;
    //QCheckBox* m_checkBoxZEnabled;

    QToolButton* m_buttonShowLaserPosition;
    QToolButton* m_buttonHideLaserPosition;
    QToolButton* m_buttonSaveZOrigin;
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
    QGridLayout* m_pathPropertyLayout;
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
    QWidget* m_pathPropertyWidget;
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
    QToolButton* m_buttonPrintAndCutFetchCanvas;
    QToolButton* m_buttonPrintAndCutFinishFetchCanvas;
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
    QList<QPoint> m_printAndCutCandidatePoints;
    int m_selectedPrintAndCutPointIndex;

    bool m_created;
    QDir m_tmpDir;
    QString m_currentJson;
    bool m_useLoadedJson;
    bool m_prepareMachining;

    // widgets on status bar
    QLabel* m_statusBarDeviceStatus;
    QLabel* m_statusBarAppStatus;
    Label* m_statusBarRegister;
    Label* m_statusBarActivation;
    QLabel* m_statusBarTips;
    QLabel* m_statusSelectionCount;
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
	//QToolButton* m_mmOrIn;
    QPushButton* m_mmOrIn;
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

    //MultiDuplicationDialog
    int m_MultiDuplicationCopies;
    int m_MultiDuplicationHSettings;
    int m_MultiDuplicationVSettings;
    int m_MultiDuplicationHDirection;
    int m_MultiDuplicationVDirection;
    qreal m_MultiDuplicationHDistance;
    qreal m_MultiDuplicationVDistance;
    //recent files
    QMenu* m_recentFilesMenu;
    QList<QString> m_recentFileList;
    int m_maxRecentFilesSize;
    LaserToolButton* m_arrangeButtonAlignHorinzontal;
    LaserToolButton* m_arrangeButtonAlignVertical;
    LaserToolButton* m_arrangeButtonAlignCenter;
    LaserToolButton* m_arrangeButtonDistributeHorinzontal;
    LaserToolButton* m_arrangeButtonDistributeVertical;
    LaserToolButton* m_arrangeButtonSameWidth;
    LaserToolButton* m_arrangeButtonSameHeight;
    LaserToolButton* m_arrangeMoveToPage;
    int m_alignTargetIndex;
    LaserPrimitive* m_alignTarget;
    friend class LaserApplication;
};

#endif // LASERCONTROLLERWINDOW_H
