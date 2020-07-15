#ifndef LASERCONTROLLERWINDOW_H
#define LASERCONTROLLERWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class LaserControllerWindow; }
QT_END_NAMESPACE

class LaserViewer;
class LaserScene;

class LaserControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    LaserControllerWindow(QWidget* parent = nullptr);
    ~LaserControllerWindow();

public slots:
    void onActionImportSVG(bool checked = false);
    void onToolButtonAddEngravingLayer(bool checked = false);
    void onToolButtonAddCuttingLayer(bool checked = false);

    void updateLayers();

private:
    QString getFilename(const QString& title, const QStringList& mime);

private:
    QScopedPointer<Ui::LaserControllerWindow> m_ui;
    LaserViewer* m_viewer;
    QScopedPointer<LaserScene> m_scene;
};

#endif // LASERCONTROLLERWINDOW_H