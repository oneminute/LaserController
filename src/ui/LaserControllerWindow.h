#ifndef LASERCONTROLLERWINDOW_H
#define LASERCONTROLLERWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QScopedPointer>
#include <QDir>
#include "scene/LaserLayer.h"
#include "widget/LayerButton.h"
#include "laser/LaserDriver.h"
#include "widget/LaserDoubleSpinBox.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LaserControllerWindow; }
QT_END_NAMESPACE

class QWidget;
class LaserViewer;
class LaserScene;
class QComboBox;
class QLabel;
class QTreeWidgetItem;
class QPushButton;
class QGridLayout;
class QDoubleSpinBox;
class QToolButton;
class QRadioButton;

class LaserControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    LaserControllerWindow(QWidget* parent = nullptr);
    ~LaserControllerWindow();

    void moveLaser(const QVector3D& delta, bool relative = true, const QVector3D& abstractDest = QVector3D());
    FinishRun finishRun();
    void setFinishRun(const FinishRun& finishRun);

public slots:
    void handleSecurityException(int code, const QString& message);

protected slots:
    void onActionImportSVG(bool checked = false);
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
    void onActionConnect(bool checked = false);
    void onActionDisconnect(bool checked = false);
    void onActionDownload(bool checked = false);
    void onActionLoadMotor(bool checked = false);
    void onActionUnloadMotor(bool checked = false);
    void onActionWorkState(bool checked = false);
	void onActionNew(bool checked = false);

    void onActionMoveTop(bool checked = false);
    void onActionMoveBottom(bool checked = false);
    void onActionMoveLeft(bool checked = false);
    void onActionMoveRight(bool checked = false);
    void onActionMoveTopLeft(bool checked = false);
    void onActionMoveTopRight(bool checked = false);
    void onActionMoveBottomLeft(bool checked = false);
    void onActionMoveBottomRight(bool checked = false);
    void onActionMoveUp(bool checked = false);
    void onActionMoveDown(bool checked = false);

    void onActionHalfTone(bool checked = false);
    void onActionDeletePrimitive(bool checked = false);
    void onActionCloseDocument(bool checked = false);
    void onActionSettings(bool checked = false);
    void onActionDeviceSettings(bool checked = false);
    void onActionSelectionTool(bool checked = false);
	void onActionRectangle(bool checked = false);
	void onActionEllipse(bool checked = false);
	void onActionLine(bool checked = false);
	void onActionPolygon(bool checked = false);
	void onActionSpline(bool checked = false);
	void onActionSplineEdit(bool checked = false);
	void onActionText(bool checked = false);

    void onDeviceComPortsFetched(const QStringList& ports);
    void onDeviceConnected();
    void onDeviceDisconnected();
    void onMainCardRegistered();
    void onMainCardActivated();

    void onWindowCreated();

    void onEnterDeviceUnconnectedState();
    void onEnterDeviceConnectedState();
    void onActionMoveLayerUp(bool checked = false);
    void onActionMoveLayerDown(bool checked = false);

    
    void onLaserViewerMouseMoved(const QPointF& pos);
    void onLaserViewerScaleChanged(qreal factor);
    void onComboBoxSxaleIndexChanged(int index);
    void onComboBoxSxaleTextChanged(const QString& text);

    void onEditSliderLaserEngergyMinChanged(int value);
    void onEditSliderLaserEngergyMaxChanged(int value);

    void onLaserRegistersFetched(const LaserDriver::RegistersMap& registers);
    void onLaserReturnWorkState(LaserState state);

	void onCreatSpline();

    void lightOnLaser();
    void lightOffLaser();
    void readMachiningOrigins(bool checked = false);
    void writeMachiningOrigins(bool checked = false);
    void readMachiningPower(bool checked = false);
    void writeMachiningPower(bool checked = false);
    void updatePostEventWidgets(int index);
    void laserBackToMachiningOriginalPoint(bool checked = false);
    void laserResetToOriginalPoint(bool checked = false);
    void updateOutlineTree();
    void initDocument(LaserDocument* doc);
	//selection
	void onLaserSceneSelectedChanged();
	void selectedChange();//items
	void selectionPropertyBoxChange();//doubleSpinBox's enter or mouse lost focus

private:
    QString getFilename(const QString& title, const QStringList& mime);
    void bindWidgetsProperties();
    virtual void showEvent(QShowEvent *event);

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

private:
    QScopedPointer<Ui::LaserControllerWindow> m_ui;
    LaserViewer* m_viewer;
    LaserScene* m_scene;
    QComboBox* m_comboBoxScale;
    bool m_created;
    QDir m_tmpDir;
    QString m_currentJson;
    bool m_useLoadedJson;

    // widgets on status bar
    QLabel* m_statusBarStatus;
    QLabel* m_statusBarRegister;
    QLabel* m_statusBarActivation;
    QLabel* m_statusBarTips;
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
	QDoubleSpinBox* m_xRateBox;
	QDoubleSpinBox* m_yRateBox;
	QRadioButton* m_topLeftBtn;
	QRadioButton* m_topCenterBtn;
	QRadioButton* m_topRightBtn;
	QRadioButton* m_bottomLeftBtn;
	QRadioButton* m_bottomCenterBtn;
	QRadioButton* m_bottomRightBtn;
	QRadioButton* m_leftCenterBtn;
	QRadioButton* m_centerBtn;
	QRadioButton* m_rightCenterBtn;
	QDoubleSpinBox* m_rotateBox;
	QToolButton* m_mmOrIn;
};

#endif // LASERCONTROLLERWINDOW_H