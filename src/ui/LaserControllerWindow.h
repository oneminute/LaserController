#ifndef LASERCONTROLLERWINDOW_H
#define LASERCONTROLLERWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QScopedPointer>

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
    void onToolButtonAddEngravingLayer(bool checked = false);
    void onToolButtonAddCuttingLayer(bool checked = false);
    void onTreeWidgetLayersItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onActionExportJson(bool checked = false);

    void updateLayers();

private:
    QString getFilename(const QString& title, const QStringList& mime);
    void fillLayersTree(QList<LaserLayer> &layers, const QString& type);

private:
    QScopedPointer<Ui::LaserControllerWindow> m_ui;
    LaserViewer* m_viewer;
    QScopedPointer<LaserScene> m_scene;
};

#endif // LASERCONTROLLERWINDOW_H