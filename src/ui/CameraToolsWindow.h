#ifndef CAMERATOOLSWINDOW_H
#define CAMERATOOLSWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>

#include <DockManager.h>

class QCameraImageCapture;
class QCameraViewfinder;
class QCamera;
class ImageViewer;

class CameraToolsWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CameraToolsWindow(QWidget* parent = nullptr);
    ~CameraToolsWindow();

    void updateCameras();

protected:
    void createVideoPanel();
    void createImagePanel();

    virtual void closeEvent(QCloseEvent* event) override;

public slots:
    void retranslate();

protected slots:
    void onComnboBoxCamerasIndexChanged(int index);

    void onActionRefreshCameras(bool checked);
    void onActionConnectCamera(bool checked);
    void onActionCapture(bool checked);

signals:

private:
    // actions
    QAction* m_actionRefreshCameras;
    QAction* m_actionConnectCamera;
    QAction* m_actionCapture;

    // menu bar 
    QMenuBar* m_menuBar;

    // tool bar 
    QToolBar* m_toolBar;
    QComboBox* m_comboBoxCameras;

    // status bar
    QStatusBar* m_statusBar;

    // dock panels
    ads::CDockManager* m_dockManager;
    ads::CDockAreaWidget* m_cameraDockArea;
    QCameraImageCapture* m_capture;
    QCameraViewfinder* m_viewfinder;
    ads::CDockAreaWidget* m_imageDockArea;
    ImageViewer* m_imageViewerCapture;

    // other variables
    QScopedPointer<QCamera> m_camera;
    QScopedPointer<QCameraImageCapture> m_cameraCapture;

    Q_DISABLE_COPY(CameraToolsWindow)
};


#endif //CAMERATOOLSWINDOW_H