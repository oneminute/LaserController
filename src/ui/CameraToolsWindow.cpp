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
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

#include "widget/ImageViewer.h"
#include "widget/Vector2DWidget.h"
#include "util/WidgetUtils.h"
#include "util/Utils.h"

#include <opencv2/videoio/videoio.hpp>

using namespace ads;

Q_DECLARE_METATYPE(QCameraInfo)

CameraToolsWindow::CameraToolsWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_cameraController(new CameraController)
    , m_calibrator(nullptr)
{
    m_actionRefreshCameras = new QAction(QIcon(QStringLiteral(":/ui/icons/images/reset.png")), tr("Refresh"), this);
    connect(m_actionRefreshCameras, &QAction::triggered, this, &CameraToolsWindow::onActionRefreshCameras);

    m_actionConnectCamera = new QAction(QIcon(QStringLiteral(":/ui/icons/images/connect.png")), tr("Connect"), this);
    m_actionConnectCamera->setCheckable(true);
    connect(m_actionConnectCamera, &QAction::toggled, this, &CameraToolsWindow::onActionConnectCamera);

    m_actionCapture = new QAction(QIcon(QStringLiteral(":/ui/icons/images/camera_capture.png")), tr("Capture"), this);
    connect(m_actionCapture, &QAction::triggered, this, &CameraToolsWindow::onActionCapture);

    m_actionApplyCameraSettings = new QAction(tr("Apply"));
    connect(m_actionApplyCameraSettings, &QAction::triggered, this, &CameraToolsWindow::onActionApplyCameraSettings);

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

    m_labelStatus = new QLabel;
    m_labelStatus->setText("");
    m_statusBar->addWidget(m_labelStatus);
    
    m_labelImageSize = new QLabel;
    m_labelImageSize->setText("");
    m_statusBar->addWidget(m_labelImageSize);

    // initialize Dock Manager
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, false);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    m_dockManager = new CDockManager(this);

    createVideoPanel();
    createImagePanel();
    createCameraSettings();
    createImageSettings();

    //m_videoFrameGrabber.reset(new VideoFrameGrabber(m_imageViewerCamera));
    //m_videoFrameGrabber.reset(new VideoFrameGrabber(m_labelCamera));

    m_cameraDockArea->setCurrentIndex(0);
    m_cameraDockArea->resize(600, 800);
    m_cameraDockArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_cameraSettingsDockArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_cameraSettingsDockArea->resize(300, 800);

    resize(800, 600);

    updateCameras();

    initCameraSettings();

    connect(m_cameraController.data(), &CameraController::frameCaptured,
        this, &CameraToolsWindow::onFrameCaptured);
}

CameraToolsWindow::~CameraToolsWindow()
{
    if (!m_calibrator)
    {
        m_cameraController->uninstallProcessor(m_calibrator);
    }
}

void CameraToolsWindow::updateCameras()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    m_comboBoxCameras->clear();
    int prefered = -1;
    int index = -1;
    m_comboBoxCameras->blockSignals(true);
    for (const QCameraInfo& cameraInfo : cameras)
    {
        index++;
        qLogD << cameraInfo.deviceName();
        m_comboBoxCameras->addItem(cameraInfo.description(), QVariant::fromValue<QCameraInfo>(cameraInfo));
        if (cameraInfo.description().toLower().contains("lightburn"))
        {
            prefered = index;
        }
    }
    m_comboBoxCameras->blockSignals(false);
    if (prefered >= 0)
        m_comboBoxCameras->setCurrentIndex(prefered);
    else if (index >= 0)
        m_comboBoxCameras->setCurrentIndex(0);
}

void CameraToolsWindow::createVideoPanel()
{
    QVBoxLayout* layout = new QVBoxLayout;
    m_imageViewerCamera = new ImageViewer;
    layout->addWidget(m_imageViewerCamera);

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
    QLabel* labelGeneral = new QLabel(tr("General"));
    QFont font = labelGeneral->font();
    font.setBold(true);
    labelGeneral->setFont(font);
    layout->addRow(labelGeneral, line);
    layout->setAlignment(line, Qt::AlignBottom);

    m_comboBoxFrameRateRange = new QComboBox;
    m_comboBoxFrameRateRange->addItem("2", 2);
    m_comboBoxFrameRateRange->addItem("3", 3);
    m_comboBoxFrameRateRange->addItem("5", 5);
    m_comboBoxFrameRateRange->addItem("10", 10);
    m_comboBoxFrameRateRange->addItem("15", 15);
    m_comboBoxFrameRateRange->addItem("20", 20);
    m_comboBoxFrameRateRange->addItem("30", 30);
    m_comboBoxFrameRateRange->setCurrentText("30");
    layout->addRow(tr("Frame Rate Range"), m_comboBoxFrameRateRange);

    m_comboBoxPixelFormat = new QComboBox;
    m_comboBoxPixelFormat->addItem("MJPEG", "MJPG");
    m_comboBoxPixelFormat->addItem("YUY2", "YUY2");
    layout->addRow(tr("Pixel Format"), m_comboBoxPixelFormat);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    QLabel* labelImageProcessing = new QLabel(tr("Image Processing"));
    labelImageProcessing->setFont(font);
    layout->addRow(labelImageProcessing, line);
    layout->setAlignment(line, Qt::AlignBottom);

    m_doubleSpinBoxCameraBrightness = new QDoubleSpinBox;
    m_doubleSpinBoxCameraBrightness->setMinimum(-1.0);
    m_doubleSpinBoxCameraBrightness->setMaximum(1.0);
    m_doubleSpinBoxCameraBrightness->setSingleStep(0.01);
    layout->addRow(tr("Brightness"), m_doubleSpinBoxCameraBrightness);

    m_doubleSpinBoxCameraContrast = new QDoubleSpinBox;
    m_doubleSpinBoxCameraContrast->setMinimum(-1.0);
    m_doubleSpinBoxCameraContrast->setMaximum(1.0);
    m_doubleSpinBoxCameraContrast->setSingleStep(0.01);
    layout->addRow(tr("Contrast"), m_doubleSpinBoxCameraContrast);

    m_doubleSpinBoxDenoisingLevel = new QDoubleSpinBox;
    m_doubleSpinBoxDenoisingLevel->setMinimum(-1.0);
    m_doubleSpinBoxDenoisingLevel->setMaximum(1.0);
    m_doubleSpinBoxDenoisingLevel->setSingleStep(0.01);
    layout->addRow(tr("Denoising Level"), m_doubleSpinBoxDenoisingLevel);

    m_doubleSpinBoxWhiteBalanceRed = new QDoubleSpinBox;
    m_doubleSpinBoxWhiteBalanceRed->setMinimum(-1.0);
    m_doubleSpinBoxWhiteBalanceRed->setMaximum(1.0);
    m_doubleSpinBoxWhiteBalanceRed->setSingleStep(0.01);
    layout->addRow(tr("White Balance Red"), m_doubleSpinBoxWhiteBalanceRed);

    m_doubleSpinBoxWhiteBalanceBlue = new QDoubleSpinBox;
    m_doubleSpinBoxWhiteBalanceBlue->setMinimum(-1.0);
    m_doubleSpinBoxWhiteBalanceBlue->setMaximum(1.0);
    m_doubleSpinBoxWhiteBalanceBlue->setSingleStep(0.01);
    layout->addRow(tr("White Balance Blue"), m_doubleSpinBoxWhiteBalanceBlue);

    m_doubleSpinBoxSaturation = new QDoubleSpinBox;
    m_doubleSpinBoxSaturation->setMinimum(-1.0);
    m_doubleSpinBoxSaturation->setMaximum(1.0);
    m_doubleSpinBoxSaturation->setSingleStep(0.01);
    layout->addRow(tr("Saturation"), m_doubleSpinBoxSaturation);

    m_doubleSpinBoxSharpeningLevel = new QDoubleSpinBox;
    m_doubleSpinBoxSharpeningLevel->setMinimum(-1.0);
    m_doubleSpinBoxSharpeningLevel->setMaximum(1.0);
    m_doubleSpinBoxSharpeningLevel->setSingleStep(0.01);
    layout->addRow(tr("Sharpening Level"), m_doubleSpinBoxSharpeningLevel);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    QLabel* labelExposure = new QLabel(tr("Exposure"));
    labelExposure->setFont(font);
    layout->addRow(labelExposure, line);
    layout->setAlignment(line, Qt::AlignBottom);

    m_doubleSpinBoxExposureCompensation = new QDoubleSpinBox;
    m_doubleSpinBoxExposureCompensation->setMinimum(-1.0);
    m_doubleSpinBoxExposureCompensation->setMaximum(1.0);
    m_doubleSpinBoxExposureCompensation->setSingleStep(0.01);
    layout->addRow(tr("Compensation"), m_doubleSpinBoxExposureCompensation);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    QLabel* labelFocus = new QLabel(tr("Focus"));
    labelFocus->setFont(font);
    layout->addRow(labelFocus, line);
    layout->setAlignment(line, Qt::AlignBottom);

    QToolButton* buttonApply = new QToolButton;
    buttonApply->setDefaultAction(m_actionApplyCameraSettings);
    layout->addRow("", buttonApply);

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

void CameraToolsWindow::initCameraSettings()
{
}

void CameraToolsWindow::closeEvent(QCloseEvent* event)
{
    deleteLater();
}

void CameraToolsWindow::onActionRefreshCameras(bool checked)
{
    updateCameras();
}

void CameraToolsWindow::onActionConnectCamera(bool checked)
{
    if (checked)
    {
        m_calibrator = new DistortionCalibrator;
        m_calibrator->validate();
        m_cameraController->installProcessor(m_calibrator);
        m_cameraController->start();
        m_actionConnectCamera->setText(tr("Disconnect"));
        m_actionConnectCamera->setIcon(QIcon(QStringLiteral(":/ui/icons/images/disconnect.png")));
    }
    else
    {
        m_cameraController->stop();
        m_cameraController->uninstallProcessor(m_calibrator);
        SAFE_DELETE(m_calibrator);
        m_actionConnectCamera->setText(tr("Connect"));
        m_actionConnectCamera->setIcon(QIcon(QStringLiteral(":/ui/icons/images/connect.png")));
    }
}

void CameraToolsWindow::onActionCapture(bool checked)
{
    m_imageViewerCapture->setImage(m_imageViewerCamera->pixmap());
}

void CameraToolsWindow::onActionApplyCameraSettings(bool checked)
{
    //m_cameraController->load(m_comboBoxCameras->currentIndex());
}

void CameraToolsWindow::onFrameCaptured(cv::Mat processed, cv::Mat origin)
{
    QImage image(processed.data, processed.cols, processed.rows, processed.step, QImage::Format_RGB888);
    m_labelImageSize->setText(QString("%1 x %2").arg(image.width()).arg(image.height()));
    m_imageViewerCamera->setImage(image);
}

void CameraToolsWindow::retranslate()
{

}

void CameraToolsWindow::onComboBoxCamerasIndexChanged(int index)
{
    QCameraInfo& info = m_comboBoxCameras->currentData().value<QCameraInfo>();
    qLogD << info.description() << ", " << info.deviceName();
}
