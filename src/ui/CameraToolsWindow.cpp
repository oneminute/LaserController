#include "CameraToolsWindow.h"
#include "common/common.h"

#include <DockAreaTabBar.h>
#include <DockAreaTitleBar.h>
#include <DockAreaWidget.h>
#include <DockComponentsFactory.h>
#include <DockContainerWidget.h>
#include <DockSplitter.h>
#include <DockWidgetTab.h>

#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include "widget/ImageViewer.h"
#include "widget/Vector2DWidget.h"
#include "util/WidgetUtils.h"

using namespace ads;

Q_DECLARE_METATYPE(QCameraInfo)

CameraToolsWindow::CameraToolsWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_camera(nullptr)
    , m_cameraCapture(nullptr)
{
    m_actionRefreshCameras = new QAction(QIcon(QStringLiteral(":/ui/icons/images/reset.png")), tr("Refresh"), this);
    connect(m_actionRefreshCameras, &QAction::triggered, this, &CameraToolsWindow::onActionRefreshCameras);

    m_actionConnectCamera = new QAction(QIcon(QStringLiteral(":/ui/icons/images/connect.png")), tr("Connect"), this);
    m_actionConnectCamera->setCheckable(true);
    connect(m_actionConnectCamera, &QAction::toggled, this, &CameraToolsWindow::onActionConnectCamera);

    m_actionCapture = new QAction(QIcon(QStringLiteral(":/ui/icons/images/camera_capture.png")), tr("Capture"), this);
    connect(m_actionCapture, &QAction::triggered, this, &CameraToolsWindow::onActionCapture);

    m_menuBar = new QMenuBar(this);
    setMenuBar(m_menuBar);

    m_toolBar = new QToolBar(this);
    addToolBar(m_toolBar);
    m_comboBoxCameras = new QComboBox(this);
    connect(m_comboBoxCameras, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComboBoxCamerasIndexChanged);
    m_toolBar->addWidget(m_comboBoxCameras);
    m_toolBar->addAction(m_actionRefreshCameras);
    m_toolBar->addAction(m_actionConnectCamera);
    m_toolBar->addAction(m_actionCapture);

    m_statusBar = new QStatusBar(this);
    setStatusBar(m_statusBar);

    // initialize Dock Manager
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, false);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    m_dockManager = new CDockManager(this);

    // initialize variables
    m_cameraCapture.reset(new QCameraImageCapture(m_camera.data()));
    m_cameraCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);

    createVideoPanel();
    createImagePanel();
    createCameraSettings();
    createImageSettings();

    m_cameraDockArea->setCurrentIndex(0);
    m_cameraDockArea->resize(600, 800);
    m_cameraDockArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_cameraSettingsDockArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_cameraSettingsDockArea->resize(300, 800);

    resize(800, 600);

    updateCameras();
}

CameraToolsWindow::~CameraToolsWindow()
{
}

void CameraToolsWindow::updateCameras()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    m_comboBoxCameras->clear();
    for (const QCameraInfo& cameraInfo : cameras)
    {
        qLogD << cameraInfo.deviceName();
        m_comboBoxCameras->addItem(cameraInfo.description(), QVariant::fromValue<QCameraInfo>(cameraInfo));
    }
}

void CameraToolsWindow::createVideoPanel()
{
    QVBoxLayout* layout = new QVBoxLayout;
    m_viewfinder = new QCameraViewfinder;
    layout->addWidget(m_viewfinder);

    // create center widget
    QWidget* cameraWidget = new QWidget;
    cameraWidget->setLayout(layout);

    CDockWidget* cameraDockWidget = new CDockWidget(tr("camera"));
    cameraDockWidget->setWidget(cameraWidget);
    m_cameraDockArea = m_dockManager->addDockWidget(CenterDockWidgetArea, cameraDockWidget);
    m_cameraDockArea->setAllowedAreas(DockWidgetArea::AllDockAreas);
}

void CameraToolsWindow::createImagePanel()
{
    QVBoxLayout* layout = new QVBoxLayout;
    m_imageViewerCapture = new ImageViewer;
    layout->addWidget(m_imageViewerCapture);

    QWidget* widget = new QWidget;
    widget->setLayout(layout);

    CDockWidget* dock = new CDockWidget(tr("Image"));
    dock->setWidget(widget);
    m_imageDockArea = m_dockManager->addDockWidget(CenterDockWidgetArea, dock, m_cameraDockArea);
    m_imageDockArea->setAllowedAreas(DockWidgetArea::AllDockAreas);
}

void CameraToolsWindow::createCameraSettings()
{
    QFormLayout* layout = new QFormLayout;

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    QLabel* labelImageProcessing = new QLabel(tr("Image Processing"));
    QFont font = labelImageProcessing->font();
    font.setBold(true);
    labelImageProcessing->setFont(font);
    layout->addRow(labelImageProcessing, line);
    layout->setAlignment(line, Qt::AlignBottom);

    m_doubleSpinBoxCameraBrightness = new QDoubleSpinBox;
    m_doubleSpinBoxCameraBrightness->setMinimum(-1.0);
    m_doubleSpinBoxCameraBrightness->setMaximum(1.0);
    m_doubleSpinBoxCameraBrightness->setSingleStep(0.01);
    connect(m_doubleSpinBoxCameraBrightness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraToolsWindow::onDoubleSpinBoxBrightnessChanged);
    layout->addRow(tr("Brightness"), m_doubleSpinBoxCameraBrightness);

    m_comboBoxColorCameraFilter = new QComboBox;
    m_comboBoxColorCameraFilter->addItem(tr("None"), 0);
    connect(m_comboBoxColorCameraFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComboBoxFilterChanged);
    layout->addRow(tr("Color Filter"), m_comboBoxColorCameraFilter);

    m_doubleSpinBoxCameraContrast = new QDoubleSpinBox;
    m_doubleSpinBoxCameraContrast->setMinimum(-1.0);
    m_doubleSpinBoxCameraContrast->setMaximum(1.0);
    m_doubleSpinBoxCameraContrast->setSingleStep(0.01);
    connect(m_doubleSpinBoxCameraContrast, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraToolsWindow::onDoubleSpinBoxContrastChanged);
    layout->addRow(tr("Contrast"), m_doubleSpinBoxCameraContrast);

    m_doubleSpinBoxDenoisingLevel = new QDoubleSpinBox;
    m_doubleSpinBoxDenoisingLevel->setMinimum(-1.0);
    m_doubleSpinBoxDenoisingLevel->setMaximum(1.0);
    m_doubleSpinBoxDenoisingLevel->setSingleStep(0.01);
    connect(m_doubleSpinBoxDenoisingLevel, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraToolsWindow::onDoubleSpinBoxDenoisingLevelChanged);
    layout->addRow(tr("Denoising Level"), m_doubleSpinBoxDenoisingLevel);

    m_comboBoxWhiteBalanceMode = new QComboBox;
    m_comboBoxWhiteBalanceMode->addItem(tr("Auto"), 0);
    connect(m_comboBoxWhiteBalanceMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComboBoxWhiteBalanceModeChanged);
    layout->addRow(tr("White Balance Mode"), m_comboBoxWhiteBalanceMode);

    m_doubleSpinBoxWhiteBalance = new QDoubleSpinBox;
    m_doubleSpinBoxWhiteBalance->setMinimum(-1.0);
    m_doubleSpinBoxWhiteBalance->setMaximum(1.0);
    m_doubleSpinBoxWhiteBalance->setSingleStep(0.01);
    connect(m_doubleSpinBoxWhiteBalance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraToolsWindow::onDoubleSpinBoxWhiteBalanceChanged);
    layout->addRow(tr("White Balance"), m_doubleSpinBoxWhiteBalance);

    m_doubleSpinBoxSaturation = new QDoubleSpinBox;
    m_doubleSpinBoxSaturation->setMinimum(-1.0);
    m_doubleSpinBoxSaturation->setMaximum(1.0);
    m_doubleSpinBoxSaturation->setSingleStep(0.01);
    connect(m_doubleSpinBoxSaturation, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraToolsWindow::onDoubleSpinBoxSaturationChanged);
    layout->addRow(tr("Saturation"), m_doubleSpinBoxSaturation);

    m_doubleSpinBoxSharpeningLevel = new QDoubleSpinBox;
    m_doubleSpinBoxSharpeningLevel->setMinimum(-1.0);
    m_doubleSpinBoxSharpeningLevel->setMaximum(1.0);
    m_doubleSpinBoxSharpeningLevel->setSingleStep(0.01);
    connect(m_doubleSpinBoxSharpeningLevel, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraToolsWindow::onDoubleSpinBoxSharpeningLevelChanged);
    layout->addRow(tr("Sharpening Level"), m_doubleSpinBoxSharpeningLevel);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    QLabel* labelExposure = new QLabel(tr("Exposure"));
    labelExposure->setFont(font);
    layout->addRow(labelExposure, line);
    layout->setAlignment(line, Qt::AlignBottom);

    m_checkBoxAutoAperture = new QCheckBox;
    connect(m_checkBoxAutoAperture, &QCheckBox::stateChanged, this, &CameraToolsWindow::onCheckBoxAutoApertureStateChanged);
    layout->addRow(tr("Auto Aperture"), m_checkBoxAutoAperture);

    m_checkBoxAutoIsoSensitivity = new QCheckBox;
    connect(m_checkBoxAutoIsoSensitivity, &QCheckBox::stateChanged, this, &CameraToolsWindow::onCheckBoxAutoIsoSensitivityStateChanged);
    layout->addRow(tr("Auto IsoSensitivity"), m_checkBoxAutoIsoSensitivity);

    m_checkBoxAutoShutterSpeed = new QCheckBox;
    connect(m_checkBoxAutoShutterSpeed, &QCheckBox::stateChanged, this, &CameraToolsWindow::onCheckBoxAutoShutterSpeedStateChanged);
    layout->addRow(tr("Auto ShutterSpeed"), m_checkBoxAutoShutterSpeed);

    m_doubleSpinBoxExposureCompensation = new QDoubleSpinBox;
    m_doubleSpinBoxExposureCompensation->setMinimum(-1.0);
    m_doubleSpinBoxExposureCompensation->setMaximum(1.0);
    m_doubleSpinBoxExposureCompensation->setSingleStep(0.01);
    connect(m_doubleSpinBoxExposureCompensation, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraToolsWindow::onDoubleSpinBoxExposureCompensationChanged);
    layout->addRow(tr("Compensation"), m_doubleSpinBoxExposureCompensation);

    m_comboBoxExposureMode = new QComboBox;
    m_comboBoxExposureMode->addItem(tr("Auto"), 0);
    connect(m_comboBoxExposureMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComboBoxExposureModeChanged);
    layout->addRow(tr("Exposure Mode"), m_comboBoxExposureMode);

    m_layoutFlashMode = new QVBoxLayout;
    layout->addRow(tr("Flash Mode"), m_layoutFlashMode);

    m_comboBoxMeteringMode = new QComboBox;
    connect(m_comboBoxMeteringMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComboBoxMeteringModeChanged);
    layout->addRow(tr("Metering Mode"), m_comboBoxMeteringMode);

    m_comboBoxAperture = new QComboBox;
    connect(m_comboBoxAperture, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComboBoxApertureChanged);
    layout->addRow(tr("Aperture"), m_comboBoxAperture);

    m_comboBoxIsoSensitivity = new QComboBox;
    connect(m_comboBoxIsoSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComboBoxIsoSensitivityChanged);
    layout->addRow(tr("Iso Sensitivity"), m_comboBoxIsoSensitivity);

    m_comboBoxShutterSpeed = new QComboBox;
    connect(m_comboBoxShutterSpeed, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComboBoxShutterSpeedChanged);
    layout->addRow(tr("Shutter Speed"), m_comboBoxShutterSpeed);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    QLabel* labelFocus = new QLabel(tr("Focus"));
    labelFocus->setFont(font);
    layout->addRow(labelFocus, line);
    layout->setAlignment(line, Qt::AlignBottom);

    m_layoutFocusMode = new QVBoxLayout;
    layout->addRow(tr("Focus Mode"), m_layoutFocusMode);

    m_vector2DFocusPoint = new Vector2DWidget;
    m_vector2DFocusPoint->setXMinimum(0);
    m_vector2DFocusPoint->setYMinimum(0);
    m_vector2DFocusPoint->setXMaximum(1);
    m_vector2DFocusPoint->setYMaximum(1);
    m_vector2DFocusPoint->setValue(QPointF(0.5, 0.5));
    connect(m_vector2DFocusPoint, &Vector2DWidget::valueChanged, this, &CameraToolsWindow::onVector2DFocusPointChanged);
    layout->addRow(tr("Focus Point"), m_vector2DFocusPoint);

    m_comboBoxFocusPointMode = new QComboBox;
    connect(m_comboBoxFocusPointMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComboBoxFocusPointModeChanged);
    layout->addRow(tr("Focus Point Mode"), m_comboBoxFocusPointMode);

    m_doubleSpinBoxDigitalZoom = new QDoubleSpinBox;
    m_doubleSpinBoxDigitalZoom->setMinimum(0.01);
    m_doubleSpinBoxDigitalZoom->setMaximum(100.0);
    connect(m_doubleSpinBoxDigitalZoom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraToolsWindow::onDoubleSpinBoxExposureCompensationChanged);
    layout->addRow(tr("Digital Zoom"), m_doubleSpinBoxDigitalZoom);

    m_doubleSpinBoxOpticalZoom = new QDoubleSpinBox;
    m_doubleSpinBoxOpticalZoom->setMinimum(0.01);
    m_doubleSpinBoxOpticalZoom->setMaximum(100.0);
    connect(m_doubleSpinBoxOpticalZoom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraToolsWindow::onDoubleSpinBoxOpticalZoomChanged);
    layout->addRow(tr("Optical Zoom"), m_doubleSpinBoxOpticalZoom);

    m_lineEditMaximumDigitalZoom = new QLineEdit;
    m_lineEditMaximumDigitalZoom->setReadOnly(true);
    m_lineEditMaximumDigitalZoom->setText("0");
    layout->addRow(tr("Digital Zoom"), m_lineEditMaximumDigitalZoom);

    m_lineEditMaximumOpticalZoom = new QLineEdit;
    m_lineEditMaximumOpticalZoom->setReadOnly(true);
    m_lineEditMaximumOpticalZoom->setText("0");
    layout->addRow(tr("Optical Zoom"), m_lineEditMaximumOpticalZoom);

    QWidget* widget = new QWidget;
    widget->setLayout(layout);

    CDockWidget* dock = new CDockWidget(tr("Camera Settings"));
    dock->setWidget(widget);
    m_cameraSettingsDockArea = m_dockManager->addDockWidget(RightDockWidgetArea, dock);
    m_cameraSettingsDockArea->setAllowedAreas(DockWidgetArea::AllDockAreas);
}

void CameraToolsWindow::createImageSettings()
{
}

void CameraToolsWindow::closeEvent(QCloseEvent* event)
{
    deleteLater();
}

void CameraToolsWindow::onDoubleSpinBoxBrightnessChanged(qreal value)
{
    if (m_camera.isNull())
        return;

    m_camera->imageProcessing()->setBrightness(value);
}

void CameraToolsWindow::onComboBoxFilterChanged(int index)
{
    if (m_camera.isNull())
        return;

    m_camera->imageProcessing()->setColorFilter(static_cast<QCameraImageProcessing::ColorFilter>(
        m_comboBoxColorCameraFilter->currentData().toInt()));
}

void CameraToolsWindow::onDoubleSpinBoxContrastChanged(qreal value)
{
    if (m_camera.isNull())
        return;

    m_camera->imageProcessing()->setContrast(value);
}

void CameraToolsWindow::onDoubleSpinBoxDenoisingLevelChanged(qreal value)
{
    if (m_camera.isNull())
        return;

    m_camera->imageProcessing()->setDenoisingLevel(value);
}

void CameraToolsWindow::onComboBoxWhiteBalanceModeChanged(int index)
{
    if (m_camera.isNull())
        return;

    m_camera->imageProcessing()->setWhiteBalanceMode(static_cast<QCameraImageProcessing::WhiteBalanceMode>(
        m_comboBoxWhiteBalanceMode->currentData().toInt()));
    m_doubleSpinBoxWhiteBalance->setEnabled(m_camera->imageProcessing()->whiteBalanceMode()
        == QCameraImageProcessing::WhiteBalanceManual);
}

void CameraToolsWindow::onDoubleSpinBoxWhiteBalanceChanged(qreal value)
{
    if (m_camera.isNull())
        return;

    m_camera->imageProcessing()->setManualWhiteBalance(value);
}

void CameraToolsWindow::onDoubleSpinBoxSaturationChanged(qreal value)
{
    if (m_camera.isNull())
        return;

    m_camera->imageProcessing()->setSaturation(value);
}

void CameraToolsWindow::onDoubleSpinBoxSharpeningLevelChanged(qreal value)
{
    if (m_camera.isNull())
        return;

    m_camera->imageProcessing()->setSharpeningLevel(value);
}

void CameraToolsWindow::onDoubleSpinBoxExposureCompensationChanged(qreal value)
{
    if (m_camera.isNull())
        return;

    m_camera->exposure()->setExposureCompensation(value);
}

void CameraToolsWindow::onCheckBoxAutoApertureStateChanged(int state)
{
    if (m_camera.isNull())
        return;

    if (m_checkBoxAutoAperture->isChecked())
    {
        m_camera->exposure()->setAutoAperture();
        m_comboBoxAperture->setEnabled(false);
    }
    else
    {
        m_comboBoxAperture->setEnabled(true);
        onComboBoxApertureChanged(m_comboBoxAperture->currentData().toReal());
    }
}

void CameraToolsWindow::onCheckBoxAutoIsoSensitivityStateChanged(int state)
{
    if (m_camera.isNull())
        return;

    if (m_checkBoxAutoIsoSensitivity->isChecked())
    {
        m_camera->exposure()->setAutoIsoSensitivity();
        m_comboBoxIsoSensitivity->setEnabled(false);
    }
    else
    {
        m_comboBoxIsoSensitivity->setEnabled(true);
        onComboBoxIsoSensitivityChanged(m_comboBoxIsoSensitivity->currentData().toInt());
    }
}

void CameraToolsWindow::onCheckBoxAutoShutterSpeedStateChanged(int state)
{
    if (m_camera.isNull())
        return;

    if (m_checkBoxAutoShutterSpeed->isChecked())
    {
        m_camera->exposure()->setAutoShutterSpeed();
        m_comboBoxShutterSpeed->setEnabled(false);
    }
    else
    {
        m_comboBoxShutterSpeed->setEnabled(true);
        onComboBoxShutterSpeedChanged(m_comboBoxShutterSpeed->currentData().toReal());
    }
}

void CameraToolsWindow::onComboBoxExposureModeChanged(int index)
{
    if (m_camera.isNull())
        return;

    m_camera->exposure()->setExposureMode(
        m_comboBoxExposureMode->currentData().value<QCameraExposure::ExposureMode>());
    m_doubleSpinBoxExposureCompensation->setEnabled(
        m_comboBoxExposureMode->currentData().value<QCameraExposure::ExposureMode>() == 
        QCameraExposure::ExposureManual);
}

void CameraToolsWindow::onCheckBoxFlashModeChanged(int state)
{
    if (m_camera.isNull())
        return;

    QCheckBox* w = qobject_cast<QCheckBox*>(sender());
    QCameraExposure::FlashMode mode = static_cast<QCameraExposure::FlashMode>(w->property("flag").toInt());

    QCameraExposure::FlashModes modes = m_camera->exposure()->flashMode();
    modes.setFlag(mode, w->isChecked());
    m_camera->exposure()->setFlashMode(modes);
}

void CameraToolsWindow::onComboBoxMeteringModeChanged(int index)
{
    if (m_camera.isNull())
        return;

    m_camera->exposure()->setMeteringMode(m_comboBoxMeteringMode->currentData().value<QCameraExposure::MeteringMode>());
}

void CameraToolsWindow::onComboBoxApertureChanged(int index)
{
    if (m_camera.isNull())
        return;

    m_camera->exposure()->setManualAperture(m_comboBoxAperture->currentData().toReal());
}

void CameraToolsWindow::onComboBoxIsoSensitivityChanged(int value)
{
    if (m_camera.isNull())
        return;

    m_camera->exposure()->setManualIsoSensitivity(m_comboBoxIsoSensitivity->currentData().toInt());
}

void CameraToolsWindow::onComboBoxShutterSpeedChanged(int index)
{
    if (m_camera.isNull())
        return;

    m_camera->exposure()->setManualShutterSpeed(m_comboBoxShutterSpeed->currentData().toReal());
}

void CameraToolsWindow::onVector2DFocusPointChanged(qreal x, qreal y)
{
    if (m_camera.isNull())
        return;

    QPointF point = m_camera->focus()->customFocusPoint();
    m_camera->focus()->setCustomFocusPoint(point);
}

void CameraToolsWindow::onChecBoxFocusModeChanged(int state)
{
    if (m_camera.isNull())
        return;
    QCheckBox* w = qobject_cast<QCheckBox*>(sender());
    QCameraFocus::FocusMode mode = static_cast<QCameraFocus::FocusMode>(w->property("flag").toInt());

    QCameraFocus::FocusModes modes = m_camera->focus()->focusMode();
    modes.setFlag(mode, w->isChecked());
    m_camera->focus()->setFocusMode(modes);
}

void CameraToolsWindow::onComboBoxFocusPointModeChanged(int index)
{
    if (m_camera.isNull())
        return;

    m_camera->focus()->setFocusPointMode(m_comboBoxFocusPointMode->currentData().value<QCameraFocus::FocusPointMode>());
}

void CameraToolsWindow::onDoubleSpinBoxDigitalZoomChanged(qreal value)
{
    if (m_camera.isNull())
        return;

    qreal optZoom = m_camera->focus()->opticalZoom();
    m_camera->focus()->zoomTo(value, optZoom);
}

void CameraToolsWindow::onDoubleSpinBoxOpticalZoomChanged(qreal value)
{
    if (m_camera.isNull())
        return;

    qreal digZoom = m_camera->focus()->opticalZoom();
    m_camera->focus()->zoomTo(digZoom, value);
}

void CameraToolsWindow::onActionRefreshCameras(bool checked)
{
    updateCameras();
}

void CameraToolsWindow::onActionConnectCamera(bool checked)
{
    if (checked)
    {
        QCameraInfo& info = m_comboBoxCameras->currentData().value<QCameraInfo>();
        m_camera.reset(new QCamera(info));
        m_camera->setViewfinder(m_viewfinder);
        m_camera->setCaptureMode(QCamera::CaptureStillImage);
        m_camera->searchAndLock();

        QCameraImageProcessing* imageProcessing = m_camera->imageProcessing();

        m_doubleSpinBoxCameraBrightness->blockSignals(true);
        m_doubleSpinBoxCameraBrightness->setValue(imageProcessing->brightness());
        m_doubleSpinBoxCameraBrightness->blockSignals(false);

        m_comboBoxColorCameraFilter->blockSignals(true);
        m_comboBoxColorCameraFilter->clear();
        m_comboBoxColorCameraFilter->addItem(tr("None"), 0);
        if (imageProcessing->isColorFilterSupported(QCameraImageProcessing::ColorFilterGrayscale))
            m_comboBoxColorCameraFilter->addItem(tr("Grayscale"), 1);
        if (imageProcessing->isColorFilterSupported(QCameraImageProcessing::ColorFilterNegative))
            m_comboBoxColorCameraFilter->addItem(tr("Negative"), 2);
        if (imageProcessing->isColorFilterSupported(QCameraImageProcessing::ColorFilterSolarize))
            m_comboBoxColorCameraFilter->addItem(tr("Solarize"), 3);
        if (imageProcessing->isColorFilterSupported(QCameraImageProcessing::ColorFilterSepia))
            m_comboBoxColorCameraFilter->addItem(tr("Sepia"), 4);
        if (imageProcessing->isColorFilterSupported(QCameraImageProcessing::ColorFilterPosterize))
            m_comboBoxColorCameraFilter->addItem(tr("Posterize"), 5);
        if (imageProcessing->isColorFilterSupported(QCameraImageProcessing::ColorFilterWhiteboard))
            m_comboBoxColorCameraFilter->addItem(tr("Whiteboard"), 6);
        if (imageProcessing->isColorFilterSupported(QCameraImageProcessing::ColorFilterBlackboard))
            m_comboBoxColorCameraFilter->addItem(tr("Blackboard"), 7);
        if (imageProcessing->isColorFilterSupported(QCameraImageProcessing::ColorFilterAqua))
            m_comboBoxColorCameraFilter->addItem(tr("Aqua"), 8);
        int index = widgetUtils::findComboBoxIndexByValue(m_comboBoxColorCameraFilter, imageProcessing->colorFilter());
        m_comboBoxColorCameraFilter->setCurrentIndex(index);
        m_comboBoxColorCameraFilter->blockSignals(false);

        m_doubleSpinBoxCameraContrast->blockSignals(true);
        m_doubleSpinBoxCameraContrast->setValue(imageProcessing->contrast());
        m_doubleSpinBoxCameraContrast->blockSignals(false);

        m_doubleSpinBoxDenoisingLevel->blockSignals(true);
        m_doubleSpinBoxDenoisingLevel->setValue(imageProcessing->denoisingLevel());
        m_doubleSpinBoxDenoisingLevel->blockSignals(false);

        m_comboBoxWhiteBalanceMode->blockSignals(true);
        m_comboBoxWhiteBalanceMode->clear();
        m_comboBoxWhiteBalanceMode->addItem(tr("Auto"), 0);
        if (imageProcessing->isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceManual))
            m_comboBoxColorCameraFilter->addItem(tr("Manual"), 1);
        if (imageProcessing->isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceSunlight))
            m_comboBoxColorCameraFilter->addItem(tr("Sunlight"), 2);
        if (imageProcessing->isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceCloudy))
            m_comboBoxColorCameraFilter->addItem(tr("Cloudy"), 3);
        if (imageProcessing->isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceShade))
            m_comboBoxColorCameraFilter->addItem(tr("Shade"), 4);
        if (imageProcessing->isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceTungsten))
            m_comboBoxColorCameraFilter->addItem(tr("Tungsten"), 5);
        if (imageProcessing->isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceFluorescent))
            m_comboBoxColorCameraFilter->addItem(tr("Fluorescent"), 6);
        if (imageProcessing->isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceFlash))
            m_comboBoxColorCameraFilter->addItem(tr("Flash"), 7);
        if (imageProcessing->isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceSunset))
            m_comboBoxColorCameraFilter->addItem(tr("Sunset"), 8);
        index = widgetUtils::findComboBoxIndexByValue(m_comboBoxWhiteBalanceMode, imageProcessing->whiteBalanceMode());
        m_comboBoxWhiteBalanceMode->setCurrentIndex(index);
        m_doubleSpinBoxWhiteBalance->setEnabled(imageProcessing->whiteBalanceMode() == QCameraImageProcessing::WhiteBalanceManual);
        m_comboBoxWhiteBalanceMode->blockSignals(false);

        m_doubleSpinBoxWhiteBalance->blockSignals(true);
        m_doubleSpinBoxWhiteBalance->setValue(imageProcessing->manualWhiteBalance());
        m_doubleSpinBoxWhiteBalance->blockSignals(false);

        m_doubleSpinBoxSaturation->blockSignals(true);
        m_doubleSpinBoxSaturation->setValue(imageProcessing->saturation());
        m_doubleSpinBoxSaturation->blockSignals(false);

        m_doubleSpinBoxSharpeningLevel->blockSignals(true);
        m_doubleSpinBoxSharpeningLevel->setValue(imageProcessing->sharpeningLevel());
        m_doubleSpinBoxSharpeningLevel->blockSignals(false);

        QCameraExposure* exposure = m_camera->exposure();
        m_checkBoxAutoAperture->blockSignals(true);
        m_checkBoxAutoAperture->setChecked(false);
        m_checkBoxAutoAperture->blockSignals(false);

        m_checkBoxAutoIsoSensitivity->blockSignals(true);
        m_checkBoxAutoIsoSensitivity->setChecked(false);
        m_checkBoxAutoIsoSensitivity->blockSignals(false);

        m_checkBoxAutoShutterSpeed->blockSignals(true);
        m_checkBoxAutoShutterSpeed->setChecked(false);
        m_checkBoxAutoShutterSpeed->blockSignals(false);

        m_doubleSpinBoxExposureCompensation->blockSignals(true);
        m_doubleSpinBoxExposureCompensation->setValue(exposure->exposureCompensation());
        m_doubleSpinBoxExposureCompensation->blockSignals(false);

        m_comboBoxExposureMode->blockSignals(true);
        m_comboBoxExposureMode->clear();
        m_comboBoxExposureMode->addItem(tr("Auto"), 0);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureManual))
            m_comboBoxExposureMode->addItem(tr("Manual"), 1);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposurePortrait))
            m_comboBoxExposureMode->addItem(tr("Portrait"), 2);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureNight))
            m_comboBoxExposureMode->addItem(tr("Night"), 3);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureBacklight))
            m_comboBoxExposureMode->addItem(tr("Backlight"), 4);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureSpotlight))
            m_comboBoxExposureMode->addItem(tr("Spotlight"), 5);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureSports))
            m_comboBoxExposureMode->addItem(tr("Sports"), 6);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureSnow))
            m_comboBoxExposureMode->addItem(tr("Snow"), 7);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureBeach))
            m_comboBoxExposureMode->addItem(tr("Beach"), 8);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureLargeAperture))
            m_comboBoxExposureMode->addItem(tr("LargeAperture"), 9);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureSmallAperture))
            m_comboBoxExposureMode->addItem(tr("SmallAperture"), 10);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureAction))
            m_comboBoxExposureMode->addItem(tr("Action"), 11);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureLandscape))
            m_comboBoxExposureMode->addItem(tr("Landscape"), 12);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureNightPortrait))
            m_comboBoxExposureMode->addItem(tr("NightPortrait"), 13);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureTheatre))
            m_comboBoxExposureMode->addItem(tr("Theatre"), 14);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureSunset))
            m_comboBoxExposureMode->addItem(tr("Sunset"), 15);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureSteadyPhoto))
            m_comboBoxExposureMode->addItem(tr("SteadyPhoto"), 16);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureFireworks))
            m_comboBoxExposureMode->addItem(tr("Fireworks"), 17);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureParty))
            m_comboBoxExposureMode->addItem(tr("Party"), 18);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureCandlelight))
            m_comboBoxExposureMode->addItem(tr("Candlelight"), 19);
        if (exposure->isExposureModeSupported(QCameraExposure::ExposureBarcode))
            m_comboBoxExposureMode->addItem(tr("Barcode"), 20);
        index = widgetUtils::findComboBoxIndexByValue(m_comboBoxWhiteBalanceMode, exposure->exposureMode());
        m_comboBoxExposureMode->setCurrentIndex(index);
        m_doubleSpinBoxExposureCompensation->setEnabled(exposure->exposureMode() == QCameraExposure::ExposureManual);
        m_comboBoxExposureMode->blockSignals(false);

        for (QCheckBox* w : m_checkBoxFlashMode)
        {
            m_layoutFlashMode->removeWidget(w);
            delete w;
        }
        m_checkBoxFlashMode.clear();
        if (exposure->isFlashModeSupported(QCameraExposure::FlashAuto))
        {
            QCheckBox* w = new QCheckBox(tr("Auto"));
            w->setProperty("flag", 0x1);
            m_checkBoxFlashMode.append(w);
        }
        if (exposure->isFlashModeSupported(QCameraExposure::FlashOff))
        {
            QCheckBox* w = new QCheckBox(tr("Off"));
            w->setProperty("flag", 0x2);
            m_checkBoxFlashMode.append(w);
        }
        if (exposure->isFlashModeSupported(QCameraExposure::FlashOn))
        {
            QCheckBox* w = new QCheckBox(tr("On"));
            w->setProperty("flag", 0x4);
            m_checkBoxFlashMode.append(w);
        }
        if (exposure->isFlashModeSupported(QCameraExposure::FlashRedEyeReduction))
        {
            QCheckBox* w = new QCheckBox(tr("Red Eye Reduction"));
            w->setProperty("flag", 0x8);
            m_checkBoxFlashMode.append(w);
        }
        if (exposure->isFlashModeSupported(QCameraExposure::FlashFill))
        {
            QCheckBox* w = new QCheckBox(tr("Fill"));
            w->setProperty("flag", 0x10);
            m_checkBoxFlashMode.append(w);
        }
        if (exposure->isFlashModeSupported(QCameraExposure::FlashTorch))
        {
            QCheckBox* w = new QCheckBox(tr("Torch"));
            w->setProperty("flag", 0x20);
            m_checkBoxFlashMode.append(w);
        }
        if (exposure->isFlashModeSupported(QCameraExposure::FlashVideoLight))
        {
            QCheckBox* w = new QCheckBox(tr("Video Ligth"));
            w->setProperty("flag", 0x40);
            m_checkBoxFlashMode.append(w);
        }
        if (exposure->isFlashModeSupported(QCameraExposure::FlashSlowSyncFrontCurtain))
        {
            QCheckBox* w = new QCheckBox(tr("Slow Sync Front Curtain"));
            w->setProperty("flag", 0x80);
            m_checkBoxFlashMode.append(w);
        }
        if (exposure->isFlashModeSupported(QCameraExposure::FlashSlowSyncRearCurtain))
        {
            QCheckBox* w = new QCheckBox(tr("Slow Sync Rear Curtain"));
            w->setProperty("flag", 0x100);
            m_checkBoxFlashMode.append(w);
        }
        if (exposure->isFlashModeSupported(QCameraExposure::FlashManual))
        {
            QCheckBox* w = new QCheckBox(tr("Manual"));
            w->setProperty("flag", 0x200);
            m_checkBoxFlashMode.append(w);
        }
        for (QCheckBox* w : m_checkBoxFlashMode)
        {
            connect(w, &QCheckBox::stateChanged, this, &CameraToolsWindow::onCheckBoxFlashModeChanged);
        } 

        m_comboBoxMeteringMode->blockSignals(true);
        m_comboBoxMeteringMode->clear();
        if (exposure->isMeteringModeSupported(QCameraExposure::MeteringMatrix))
            m_comboBoxMeteringMode->addItem(tr("Matrix"), 1);
        if (exposure->isMeteringModeSupported(QCameraExposure::MeteringAverage))
            m_comboBoxMeteringMode->addItem(tr("Average"), 2);
        if (exposure->isMeteringModeSupported(QCameraExposure::MeteringSpot))
            m_comboBoxMeteringMode->addItem(tr("Spot"), 3);
        index = widgetUtils::findComboBoxIndexByValue(m_comboBoxMeteringMode, exposure->meteringMode());
        m_comboBoxMeteringMode->setCurrentIndex(index);
        m_comboBoxMeteringMode->blockSignals(false);

        bool ok;
        m_comboBoxAperture->blockSignals(true);
        QList<qreal> apertureValues = exposure->supportedApertures(&ok);
        if (ok)
        {
            for (qreal v : apertureValues)
            {
                m_comboBoxAperture->addItem(QString::number(v), v);
            }
            m_comboBoxAperture->setCurrentText(
                QString::number(exposure->requestedAperture())
            );
        }
        m_comboBoxAperture->blockSignals(false);

        m_comboBoxIsoSensitivity->blockSignals(true);
        QList<int> isoValues = exposure->supportedIsoSensitivities(&ok);
        if (ok)
        {
            for (int v : isoValues)
            {
                m_comboBoxIsoSensitivity->addItem(QString::number(v), v);
            }
            m_comboBoxIsoSensitivity->setCurrentText(
                QString::number(exposure->requestedAperture())
            );
        }
        m_comboBoxIsoSensitivity->blockSignals(false);

        m_comboBoxShutterSpeed->blockSignals(true);
        QList<qreal> speedValues = exposure->supportedShutterSpeeds(&ok);
        if (ok)
        {
            for (qreal v : speedValues)
            {
                m_comboBoxShutterSpeed->addItem(QString::number(v), v);
            }
            m_comboBoxShutterSpeed->setCurrentText(
                QString::number(exposure->requestedAperture())
            );
        }
        m_comboBoxShutterSpeed->blockSignals(false);

        QCameraFocus* focus = m_camera->focus();
        QPointF focusPoint = focus->customFocusPoint();
        m_vector2DFocusPoint->blockSignals(true);
        m_vector2DFocusPoint->setValue(focusPoint);
        m_vector2DFocusPoint->blockSignals(false);

        for (QCheckBox* w : m_checkBoxFocusMode)
        {
            m_layoutFocusMode->removeWidget(w);
            delete w;
        }
        m_checkBoxFocusMode.clear();
        if (focus->isFocusModeSupported(QCameraFocus::ManualFocus))
        {
            QCheckBox* w = new QCheckBox(tr("Manual"));
            w->setProperty("flag", 0x1);
            m_checkBoxFocusMode.append(w);
        }
        if (focus->isFocusModeSupported(QCameraFocus::HyperfocalFocus))
        {
            QCheckBox* w = new QCheckBox(tr("Hyperfocal"));
            w->setProperty("flag", 0x2);
            m_checkBoxFocusMode.append(w);
        }
        if (focus->isFocusModeSupported(QCameraFocus::InfinityFocus))
        {
            QCheckBox* w = new QCheckBox(tr("Infinity"));
            w->setProperty("flag", 0x4);
            m_checkBoxFocusMode.append(w);
        }
        if (focus->isFocusModeSupported(QCameraFocus::AutoFocus))
        {
            QCheckBox* w = new QCheckBox(tr("Auto"));
            w->setProperty("flag", 0x8);
            m_checkBoxFocusMode.append(w);
        }
        if (focus->isFocusModeSupported(QCameraFocus::ContinuousFocus))
        {
            QCheckBox* w = new QCheckBox(tr("Continuous"));
            w->setProperty("flag", 0x10);
            m_checkBoxFocusMode.append(w);
        }
        if (focus->isFocusModeSupported(QCameraFocus::MacroFocus))
        {
            QCheckBox* w = new QCheckBox(tr("Macro"));
            w->setProperty("flag", 0x20);
            m_checkBoxFocusMode.append(w);
        }
        for (QCheckBox* w : m_checkBoxFocusMode)
        {
            connect(w, &QCheckBox::stateChanged, this, &CameraToolsWindow::onChecBoxFocusModeChanged);
        }

        m_comboBoxFocusPointMode->blockSignals(true);
        m_comboBoxFocusPointMode->clear();
        if (focus->isFocusPointModeSupported(QCameraFocus::FocusPointAuto))
            m_comboBoxFocusPointMode->addItem(tr("Auto"), 0);
        if (focus->isFocusPointModeSupported(QCameraFocus::FocusPointCenter))
            m_comboBoxFocusPointMode->addItem(tr("Center"), 1);
        if (focus->isFocusPointModeSupported(QCameraFocus::FocusPointFaceDetection))
            m_comboBoxFocusPointMode->addItem(tr("Detection"), 2);
        if (focus->isFocusPointModeSupported(QCameraFocus::FocusPointCustom))
            m_comboBoxFocusPointMode->addItem(tr("Custom"), 3);
        m_vector2DFocusPoint->setEnabled(focus->focusPointMode() == QCameraFocus::FocusPointCustom);
        index = widgetUtils::findComboBoxIndexByValue(m_comboBoxFocusPointMode, focus->focusPointMode());
        m_comboBoxFocusPointMode->setCurrentIndex(index);
        m_comboBoxFocusPointMode->blockSignals(false);

        m_lineEditMaximumDigitalZoom->setText(QString::number(focus->maximumDigitalZoom()));
        m_lineEditMaximumOpticalZoom->setText(QString::number(focus->maximumOpticalZoom()));

        m_actionConnectCamera->setText(tr("Disconnect"));
        m_actionConnectCamera->setIcon(QIcon(QStringLiteral(":/ui/icons/images/disconnect.png")));
        m_camera->start();
    }
    else
    {
        m_camera->stop();
        m_cameraCapture->cancelCapture();
        m_actionConnectCamera->setText(tr("Connect"));
        m_actionConnectCamera->setIcon(QIcon(QStringLiteral(":/ui/icons/images/connect.png")));
    }
}

void CameraToolsWindow::onActionCapture(bool checked)
{
    QPixmap image = m_viewfinder->grab();
    m_imageViewerCapture->setImage(image);
}

void CameraToolsWindow::retranslate()
{

}

void CameraToolsWindow::onComboBoxCamerasIndexChanged(int index)
{
    QCameraInfo& info = m_comboBoxCameras->currentData().value<QCameraInfo>();
    qLogD << info.description() << ", " << info.deviceName();
}
