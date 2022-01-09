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
    qRegisterMetaType<FrameArgs>("FrameArgs");
}

CameraController::~CameraController()
{
    stop();
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
        if (cameraInfo.description().toLower().contains("lightburn") ||
            cameraInfo.description().toLower().contains("wn") ||
            cameraInfo.description().toLower().contains("bba"))
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
    cv::Mat origin;
    cv::Mat processed;
    m_status = CS_STARTED;
    int errorCount = 0;
    m_lastTime = QDateTime::currentDateTime();
    int index = -1;
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
                //qLogD << duration << "ms";
                if (!ok || frame.empty())
                {
                    emit error(Error_No_Data);
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
                cv::cvtColor(frame, origin, cv::COLOR_RGB2BGR);
                processed = origin.clone();
                index++;

                bool done = true;
                for (ImageProcessor* processor : m_processors)
                {
                    if (!processor->enabled())
                        continue;

                    if (!processor->process(processed, origin))
                    {
                        done = false;
                        emit error(Error_Processing);
                        break;
                    }
                }

                if (done)
                {
                    FrameArgs args;
                    args.duration = duration;
                    args.frameIndex = index;
                    args.timestamp = currTime;
                    emit frameCaptured(processed, origin, args);
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
            index = -1;
        }

        QThread::currentThread()->msleep(500);
    }
}

bool CameraController::start()
{
    if (m_status != CS_IDLE)
        return true;

    m_status = CS_STARTED;
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

