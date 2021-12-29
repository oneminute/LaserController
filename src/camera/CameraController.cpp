#include "CameraController.h"

#include "common/common.h"
#include "common/Config.h"
#include "LaserApplication.h"
#include "util/ImageUtils.h"
#include "ImageProcessor.h"

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

void CameraController::installProcessor(ImageProcessor* processor)
{
    if (!processor)
        return;

    QMutexLocker locker(&m_mutex);
    if (m_processors.contains(processor))
        return;
    m_processors.append(processor);
}

void CameraController::uninstallProcessor(ImageProcessor* processor)
{
    if (!processor)
        return;

    QMutexLocker locker(&m_mutex);
    m_processors.removeOne(processor);
}

void CameraController::autoLoading() const
{
}

void CameraController::setAutoLoading(bool value)
{
}

QList<int> CameraController::supportedCameras()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QList<int> supportedCameraIndices;
    int index = 0;
    for (const QCameraInfo& cameraInfo : cameras)
    {
        if (cameraInfo.description().toLower().contains("lightburn"))
        {
            supportedCameraIndices.append(index);
        }
        index++;
    }
    return supportedCameraIndices;
}

void CameraController::run()
{
    cv::Mat frame;
    m_status = CS_STARTED;
    int errorCount = 0;
    m_lastTime = QDateTime::currentDateTime();
    while (m_status == CS_STARTED)
    {
        if (m_videoCapture->isOpened())
        {
            while (m_status == CS_STARTED)
            {
                bool ok = m_videoCapture->read(frame);
                QDateTime currTime = QDateTime::currentDateTime();
                qint64 duration = m_lastTime.msecsTo(currTime);
                m_lastTime = currTime;
                qLogD << duration << "ms";
                if (!ok || frame.empty())
                {
                    emit error();
                    errorCount++;
                    if (errorCount >= 1)
                    {
                        qLogW << "Camera disconnected!";
                        m_videoCapture->release();
                        emit disconnected();
                        break;
                    }
                    continue;
                }

                bool done = true;
                for (ImageProcessor* processor : m_processors)
                {
                    if (!processor->enabled())
                        continue;

                    if (!processor->process(frame))
                    {
                        done = false;
                        emit error();
                        break;
                    }
                }

                if (done)
                {
                    addImage(frame);
                    emit frameCaptured();
                }
            }
        }
        else
        {
            QList<int> cameraIndices = supportedCameras();
            if (!cameraIndices.empty())
            {
                if (m_videoCapture->open(cameraIndices.first(), cv::CAP_ANY))
                {
                    setResolution(Config::Camera::resolution());
                    emit connected();
                }
            }
        }

        QThread::currentThread()->msleep(500);
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

    /*m_cameraIndex = cameraIndex;

    m_videoCapture->open(m_cameraIndex, cv::CAP_ANY);
    if (!m_videoCapture->isOpened())
    {
        qLogW << "Cannot open camera " << m_cameraIndex;
        m_status = CS_IDLE;
        return false;
    }*/

    //setResolution(Config::Camera::resolution());
    m_status = CS_LOADED;

    return true;
}

