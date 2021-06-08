#ifndef LASERCONTROLLERWINDOW_H
#define LASERCONTROLLERWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QScopedPointer>
#include <QDir>

#include "scene/LaserLayer.h"
#include "widget/LayerButton.h"
#include "laser/LaserDriver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LaserControllerWindow; }
QT_END_NAMESPACE

class LaserViewer;
class LaserScene;
class QLabel;
class QTreeWidgetItem;
class QPushButton;

class LaserControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    LaserControllerWindow(QWidget* parent = nullptr);
    ~LaserControllerWindow();

    void moveLaser(const QVector3D& delta, bool relative = true, const QVector3D& abstractDest = QVector3D());
    FinishRun finishRun();
    void setFinishRun(const FinishRun& finishRun);

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

    void onDriverComPortsFetched(const QStringList& ports);
    void onDriverComPortConnected();
    void onDriverComPortDisconnected(bool isError = false, const QString& errorMsg = "");

    void onWindowCreated();

    void onEnterDeviceUnconnectedState();
    void onEnterDeviceConnectedState();
    void onActionMoveLayerUp(bool checked = false);
    void onActionMoveLayerDown(bool checked = false);

    void onLaserSceneSelectedChanged();
    void onLaserViewerMouseMoved(const QPointF& pos);
    void onLaserViewerScaleChanged(qreal factor);

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
    bool m_created;
    QDir m_tmpDir;
    QString m_currentJson;
    bool m_useLoadedJson;

    // widgets on status bar
    QLabel* m_statusBarStatus;
    QLabel* m_statusBarScale;
    QLabel* m_statusBarTips;
    QLabel* m_statusBarCoordinate;
    QLabel* m_statusBarLocation;
    QLabel* m_statusBarPageInfo;
    QLabel* m_statusBarCopyright;

    QList<LayerButton*> m_layerButtons;

};

#endif // LASERCONTROLLERWINDOW_H