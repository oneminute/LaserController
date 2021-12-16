#include "CameraController.h"
//#include "VideoFrameGrabber.h"

#include "common/common.h"
#include "LaserApplication.h"
#include "util/ImageUtils.h"

#include <opencv2/videoio/videoio.hpp>
//#include <QCamera>

CameraController::CameraController(QObject* parent)
    : QThread(parent)
    , m_videoCapture(new cv::VideoCapture)
    , m_status(CS_IDLE)
    , m_maxQueueLength(10)
{
}

CameraController::~CameraController()
{
    stop();
}

cv::Mat CameraController::image() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    if (m_images.empty())
        return cv::Mat();
    else
    {
        return m_images.last();
    }
}

bool CameraController::setResolution(const QSize& size)
{
    bool done = true;
    done = done && m_videoCapture->set(cv::CAP_PROP_FRAME_WIDTH, size.width());
    done = done && m_videoCapture->set(cv::CAP_PROP_FRAME_HEIGHT, size.height());
    return done;
}

QSize CameraController::resolution() const
{
    QSize size;
    size.setWidth(qRound(m_videoCapture->get(cv::CAP_PROP_FRAME_WIDTH)));
    size.setHeight(qRound(m_videoCapture->get(cv::CAP_PROP_FRAME_HEIGHT)));
    return size;
}

void CameraController::run()
{
    cv::Mat frame;
    m_status = CS_STARTED;
    while (m_status == CS_STARTED)
    {
        m_videoCapture->read(frame);
        if (frame.empty())
        {
            emit error();
            continue;
        }

        addImage(frame);
        emit frameCaptured();
    }
}

void CameraController::addImage(const cv::Mat& mat)
{
    QMutexLocker locker(&m_mutex);
    cv::Mat out;
    cv::cvtColor(mat, out, cv::COLOR_RGB2BGR);
    m_images.enqueue(out);
    if (m_images.length() > m_maxQueueLength)
    {
        m_images.dequeue();
    }
}

bool CameraController::start()
{
    if (m_status != CS_IDLE)
        return true;

    m_status = CS_STARTING;
    QThread::start();
    return true;
}

void CameraController::stop()
{
    if (m_status == CS_IDLE)
        return;

    m_status = CS_STOPPING;
    m_videoCapture->release();
    this->wait();
    m_status = CS_IDLE;
}

bool CameraController::load(int cameraIndex)
{
    if (m_status != CS_IDLE)
        return true;

    m_cameraIndex = cameraIndex;

    m_videoCapture->open(m_cameraIndex, cv::CAP_ANY);
    if (!m_videoCapture->isOpened())
    {
        qLogW << "Cannot open camera " << m_cameraIndex;
        m_status = CS_IDLE;
        return false;
    }

    return true;
}


