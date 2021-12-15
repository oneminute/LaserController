#include "CameraController.h"
#include "VideoFrameGrabber.h"

#include "common/common.h"
#include "LaserApplication.h"

#include <QCamera>

CameraController::CameraController(const QCameraInfo& info, QObject* parent)
    : QObject(parent)
    , m_cameraInfo(info)
{
    m_camera = new QCamera(info);
    m_grabber = new VideoFrameGrabber;

    connect(m_grabber, &VideoFrameGrabber::imageCaptured, this, &CameraController::frameCaptured);

    m_camera->setViewfinder(m_grabber);
}

CameraController::~CameraController()
{
    m_camera->blockSignals(true);
    stop();
    unload();
}

QCamera* CameraController::camera() const
{
    return m_camera;
}

QCameraInfo CameraController::cameraInfo() const
{
    return m_cameraInfo;
}

QImage CameraController::image() const
{
    return m_grabber->image();
}

void CameraController::start()
{
    if (!m_camera)
        return;

    m_thread.start();
    m_camera->searchAndLock();
    m_camera->start();
}

void CameraController::stop()
{
    if (!m_camera)
        return;

    m_camera->stop();
    m_thread.quit();
    m_thread.wait();
}

void CameraController::load()
{
    if (!m_camera)
        return;

    moveToThread(&m_thread);
    m_camera->load();
}

void CameraController::unload()
{
    if (!m_camera)
        return;

    m_camera->unload();
    SAFE_DELETE(m_camera);
}

