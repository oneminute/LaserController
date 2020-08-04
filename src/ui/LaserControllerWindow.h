#ifndef LASERCONTROLLERWINDOW_H
#define LASERCONTROLLERWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QScopedPointer>
#include <QDir>

#include "scene/LaserLayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LaserControllerWindow; }
QT_END_NAMESPACE

class LaserViewer;
class LaserScene;
class QTreeWidgetItem;

class LaserControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    LaserControllerWindow(QWidget* parent = nullptr);
    ~LaserControllerWindow();

protected slots:
    void onActionImportSVG(bool checked = false);
    void onActionImportCorelDraw(bool checked = false);
    void onActionAddEngravingLayer(bool checked = false);
    void onActionAddCuttingLayer(bool checked = false);
    void onActionRemoveLayer(bool checked = false);
    void onTreeWidgetLayersItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onActionExportJson(bool checked = false);
    void onActionMechining(bool checked = false);
    void onActionPauseMechining(bool checked = false);
    void onActionStopMechining(bool checked = false);
    void onActionConnect(bool checked = false);
    void onActionDisconnect(bool checked = false);
    void onActionLoadMotor(bool checked = false);
    void onActionUnloadMotor(bool checked = false);

private:
    QString getFilename(const QString& title, const QStringList& mime);
    void bindWidgetsProperties();
    virtual void showEvent(QShowEvent *event);

signals:
    void windowCreated();

private:
    QScopedPointer<Ui::LaserControllerWindow> m_ui;
    LaserViewer* m_viewer;
    QScopedPointer<LaserScene> m_scene;
    bool m_created;
    QDir m_tmpDir;
};

#endif // LASERCONTROLLERWINDOW_H