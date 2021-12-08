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
#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>

#include "widget/ImageViewer.h"

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
    connect(m_comboBoxCameras, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraToolsWindow::onComnboBoxCamerasIndexChanged);
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

    // create center widget
    QWidget* widget = new QWidget;
    widget->setLayout(layout);

    CDockWidget* dock = new CDockWidget(tr("Image"));
    dock->setWidget(widget);
    m_imageDockArea = m_dockManager->addDockWidget(RightDockWidgetArea, dock, m_cameraDockArea);
    m_imageDockArea->setAllowedAreas(DockWidgetArea::AllDockAreas);
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
        QCameraInfo& info = m_comboBoxCameras->currentData().value<QCameraInfo>();
        m_camera.reset(new QCamera(info));
        m_camera->setViewfinder(m_viewfinder);
        m_camera->setCaptureMode(QCamera::CaptureStillImage);
        m_camera->searchAndLock();

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

void CameraToolsWindow::onComnboBoxCamerasIndexChanged(int index)
{
    QCameraInfo& info = m_comboBoxCameras->currentData().value<QCameraInfo>();
    qLogD << info.description() << ", " << info.deviceName();
}
