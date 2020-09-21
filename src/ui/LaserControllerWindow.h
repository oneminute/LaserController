#ifndef LASERCONTROLLERWINDOW_H
#define LASERCONTROLLERWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QScopedPointer>
#include <QDir>

#include "scene/LaserLayer.h"
#include "widget/LayerButton.h"

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

protected slots:
    void onActionImportSVG(bool checked = false);
    void onActionImportCorelDraw(bool checked = false);
    //void onActionAddEngravingLayer(bool checked = false);
    //void onActionAddCuttingLayer(bool checked = false);
    void onActionRemoveLayer(bool checked = false);
    void onTableWidgetLayersCellDoubleClicked(int row, int column);
    void onTableWidgetItemSelectionChanged();
    void onActionExportJson(bool checked = false);
    void onActionLoadJson(bool checked = false);
    void onActionMachining(bool checked = false);
    void onActionPauseMechining(bool checked = false);
    void onActionStopMechining(bool checked = false);
    void onActionConnect(bool checked = false);
    void onActionDisconnect(bool checked = false);
    void onActionDownload(bool checked = false);
    void onActionLoadMotor(bool checked = false);
    void onActionUnloadMotor(bool checked = false);
    void onActionWorkState(bool checked = false);
    void onActionMoveToOriginalPoint(bool checked = false);
    void onActionHalfTone(bool checked = false);
    void onActionDeletePrimitive(bool checked = false);
    void onActionCloseDocument(bool checked = false);

    void onDriverComPortsFetched(const QStringList& ports);
    void onDriverComPortConnected();
    void onDriverComPortDisconnected(bool isError = false, const QString& errorMsg = "");

    void onWindowCreated();

    void onEnterDeviceUnconnectedState();
    void onActionMoveLayerUp(bool checked = false);
    void onActionMoveLayerDown(bool checked = false);

    void onLaserSceneSelectedChanged();

private:
    QString getFilename(const QString& title, const QStringList& mime);
    void bindWidgetsProperties();
    virtual void showEvent(QShowEvent *event);

signals:
    void windowCreated();

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
    QLabel* m_statusBarTips;
    QLabel* m_statusBarCoordinate;
    QLabel* m_statusBarLocation;
    QLabel* m_statusBarPageInfo;
    QLabel* m_statusBarCopyright;

    QList<LayerButton*> m_layerButtons;

};

#endif // LASERCONTROLLERWINDOW_H