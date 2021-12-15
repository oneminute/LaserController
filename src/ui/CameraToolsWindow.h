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

    void onComboBoxFrameRateRangeChanged(int index);
    void onComboBoxPixelFormatChanged(int index);
    void onComboBoxResolutionChanged(int index);

    void onDoubleSpinBoxBrightnessChanged(qreal value);
    void onComboBoxFilterChanged(int index);
    void onDoubleSpinBoxContrastChanged(qreal value);
    void onDoubleSpinBoxDenoisingLevelChanged(qreal value);
    void onComboBoxWhiteBalanceModeChanged(int index);
    void onDoubleSpinBoxWhiteBalanceChanged(qreal value);
    void onDoubleSpinBoxSaturationChanged(qreal value);
    void onDoubleSpinBoxSharpeningLevelChanged(qreal value);
    void onDoubleSpinBoxExposureCompensationChanged(qreal value);

    void onCheckBoxAutoApertureStateChanged(int state);
    void onCheckBoxAutoIsoSensitivityStateChanged(int state);
    void onCheckBoxAutoShutterSpeedStateChanged(int state);
    void onComboBoxExposureModeChanged(int index);
    void onCheckBoxFlashModeChanged(int state);
    void onComboBoxMeteringModeChanged(int index);
    void onComboBoxApertureChanged(int index);
    void onComboBoxIsoSensitivityChanged(int index);
    void onComboBoxShutterSpeedChanged(int index);

    void onVector2DFocusPointChanged(qreal x, qreal y);
    void onChecBoxFocusModeChanged(int state);
    void onComboBoxFocusPointModeChanged(int index);
    void onDoubleSpinBoxDigitalZoomChanged(qreal value);
    void onDoubleSpinBoxOpticalZoomChanged(qreal value);

    void onActionRefreshCameras(bool checked);
    void onActionConnectCamera(bool checked);
    void onActionCapture(bool checked);

    void onCameraStateChanged(QCamera::State state);
    void onCameraStatusChanged(QCamera::Status status);
    void onFrameCaptured();

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
    //QScopedPointer<QCamera> m_camera;
    //QScopedPointer<VideoFrameGrabber> m_videoFrameGrabber;
    QScopedPointer<CameraController> m_cameraController;

    // camera general variables
    QComboBox* m_comboBoxFrameRateRange;
    QComboBox* m_comboBoxPixelFormat;
    QComboBox* m_comboBoxResolution;

    // camera image processing variables
    QDoubleSpinBox* m_doubleSpinBoxCameraBrightness;
    QComboBox* m_comboBoxColorCameraFilter;
    QDoubleSpinBox* m_doubleSpinBoxCameraContrast;
    QDoubleSpinBox* m_doubleSpinBoxDenoisingLevel;
    QComboBox* m_comboBoxWhiteBalanceMode;
    QDoubleSpinBox* m_doubleSpinBoxWhiteBalance;
    QDoubleSpinBox* m_doubleSpinBoxSaturation;
    QDoubleSpinBox* m_doubleSpinBoxSharpeningLevel;

    // camera exposure variables
    QCheckBox* m_checkBoxAutoAperture;
    QCheckBox* m_checkBoxAutoIsoSensitivity;
    QCheckBox* m_checkBoxAutoShutterSpeed;
    QDoubleSpinBox* m_doubleSpinBoxExposureCompensation;
    QComboBox* m_comboBoxExposureMode;
    QList<QCheckBox*> m_checkBoxFlashMode;
    QVBoxLayout* m_layoutFlashMode;
    QComboBox* m_comboBoxMeteringMode;
    QComboBox* m_comboBoxAperture;
    QComboBox* m_comboBoxIsoSensitivity;
    QComboBox* m_comboBoxShutterSpeed;

    // camera focus variables
    Vector2DWidget* m_vector2DFocusPoint;
    QList<QCheckBox*> m_checkBoxFocusMode;
    QVBoxLayout* m_layoutFocusMode;
    QComboBox* m_comboBoxFocusPointMode;
    QDoubleSpinBox* m_doubleSpinBoxDigitalZoom;
    QDoubleSpinBox* m_doubleSpinBoxOpticalZoom;
    QLineEdit* m_lineEditMaximumDigitalZoom;
    QLineEdit* m_lineEditMaximumOpticalZoom;

    Q_DISABLE_COPY(CameraToolsWindow)
};


#endif //CAMERATOOLSWINDOW_H