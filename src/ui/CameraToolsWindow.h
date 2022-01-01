#ifndef CAMERATOOLSWINDOW_H
#define CAMERATOOLSWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>

#include <DockManager.h>
#include "camera/CameraController.h"
#include "camera/DistortionCalibrator.h"

class QCameraImageCapture;
class QCameraViewfinder;
class QCamera;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QVBoxLayout;
class ImageViewer;
class Vector2DWidget;
class CameraController;
class ImageProcessor;

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
    void createCameraSettings();
    void createImageSettings();

    void initCameraSettings();

    virtual void closeEvent(QCloseEvent* event) override;

public slots:
    void retranslate();

protected slots:
    void onComboBoxCamerasIndexChanged(int index);

    void onActionRefreshCameras(bool checked);
    void onActionConnectCamera(bool checked);
    void onActionCapture(bool checked);
    void onActionApplyCameraSettings(bool checked);

    void onFrameCaptured(cv::Mat processed, cv::Mat origin);

signals:

private:
    // actions
    QAction* m_actionRefreshCameras;
    QAction* m_actionConnectCamera;
    QAction* m_actionCapture;
    QAction* m_actionApplyCameraSettings;

    // menu bar 
    QMenuBar* m_menuBar;

    // tool bar 
    QToolBar* m_toolBar;
    QComboBox* m_comboBoxCameras;

    // status bar
    QStatusBar* m_statusBar;
    QLabel* m_labelStatus;
    QLabel* m_labelImageSize;

    // dock panels
    ads::CDockManager* m_dockManager;
    ads::CDockAreaWidget* m_cameraDockArea;
    QCameraImageCapture* m_capture;
    ImageViewer* m_imageViewerCamera;

    ads::CDockAreaWidget* m_imageDockArea;
    ImageViewer* m_imageViewerCapture;

    ads::CDockAreaWidget* m_cameraSettingsDockArea;

    // camera variables
    QScopedPointer<CameraController> m_cameraController;
    DistortionCalibrator* m_calibrator;

    // camera general variables
    QComboBox* m_comboBoxFrameRateRange;
    QComboBox* m_comboBoxPixelFormat;

    // camera image processing variables
    QDoubleSpinBox* m_doubleSpinBoxCameraBrightness;
    QDoubleSpinBox* m_doubleSpinBoxCameraContrast;
    QDoubleSpinBox* m_doubleSpinBoxDenoisingLevel;
    QDoubleSpinBox* m_doubleSpinBoxWhiteBalanceRed;
    QDoubleSpinBox* m_doubleSpinBoxWhiteBalanceBlue;
    QDoubleSpinBox* m_doubleSpinBoxSaturation;
    QDoubleSpinBox* m_doubleSpinBoxSharpeningLevel;

    // camera exposure variables
    QDoubleSpinBox* m_doubleSpinBoxExposureCompensation;

    // camera focus variables
    QComboBox* m_comboBoxFocus;
    
    Q_DISABLE_COPY(CameraToolsWindow)
};


#endif //CAMERATOOLSWINDOW_H