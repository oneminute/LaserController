#include "CameraController.h"

#include "common/Config.h"
#include "LaserApplication.h"
#include "util/ImageUtils.h"
#include "ImageProcessor.h"
#include <QtMath>

#include <opencv2/videoio/videoio.hpp>

CameraController::CameraController(QObject* parent)
    : QThread(parent)
    , d_ptr(new CameraControllerPrivate(this))
    , m_videoCapture(new cv::VideoCapture)
    , m_status(CS_IDLE)
    , m_maxQueueLength(10)
{
    connect(d_ptr.data(), &CameraControllerPrivate::frameCaptured, this, &CameraController::onFrameCaptured);
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

void CameraController::registerSubscriber(QObject* subscriber)
{
    Q_D(CameraController);
    if (!d->subscribers.contains(subscriber))
    {
        d->subscribers.append(subscriber);
    }
}

void CameraController::unregisterSubscriber(QObject* subscriber)
{
    Q_D(CameraController);
    d->subscribers.removeOne(subscriber);
}

void CameraController::clearFrameCache()
{
    Q_D(CameraController);
    d->frameEvent = nullptr;
}

QList<int> CameraController::supportedCameras()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QList<int> supportedCameraIndices;
    int index = 0;
    for (const QCameraInfo& cameraInfo : cameras)
    {
        qLogD << cameraInfo;
        qLogD << cameraInfo.description();
        if (cameraInfo.description().toLower().contains("lightburn") ||
            cameraInfo.description().toLower().contains("wn") ||
            cameraInfo.description().toLower().contains("bba") ||
            cameraInfo.description().toLower().contains("EFS Camera"))
        {
            supportedCameraIndices.append(index);
        }
        index++;
    }
    return supportedCameraIndices;
}

void CameraController::run()
{
    Q_D(CameraController);
    cv::Mat frame;
    cv::Mat origin;
    cv::Mat processed;
    cv::Mat thumb;
    m_status = CS_STARTED;
    int errorCount = 0;
    m_lastTime = QDateTime::currentDateTime();
    QSize thumbSize = Config::Camera::thumbResolution();
    QSize originSize = Config::Camera::resolution();
    int index = -1;
    int idleCount = 0;
    while (m_status == CS_STARTED)
    {
        if (m_videoCapture->isOpened())
        {
            while (m_status == CS_STARTED)
            {
                QMutexLocker locker(&m_mutex);
                if (d->processing)
                {
                    idleCount++;
                    QThread::currentThread()->msleep(10);
                    continue;
                }
                bool ok = m_videoCapture->read(frame);
                QDateTime currTime = QDateTime::currentDateTime();
                qint64 duration = m_lastTime.msecsTo(currTime);
                m_lastTime = currTime;
                //qLogD << duration << "ms, idle count: " << idleCount;
                idleCount = 0;
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
                qreal xScale = thumbSize.width() * 1.0 / originSize.width();
                qreal yScale = thumbSize.height() * 1.0 / originSize.height();
                qreal scale = qMin(1.0, qMax(xScale, yScale));
                cv::resize(origin, thumb, cv::Size(), scale, scale);
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
                    d->frameEvent = new CameraFrameEvent(this);
                    d->frameEvent->setDuration(duration);
                    d->frameEvent->setFrameIndex(index);
                    d->frameEvent->setTimestamp(currTime);
                    d->frameEvent->setOriginImage(origin);
                    d->frameEvent->setThumbImage(thumb);
                    d->frameEvent->setProcessedImage(processed);
                    d->processing = true;
                    emit d->frameCaptured();
                }

                if (m_status != CS_STARTED)
                {
                    m_videoCapture->release();
                    emit disconnected();
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

void CameraController::onFrameCaptured()
{
    Q_D(CameraController);
    QMutexLocker locker(&m_mutex);
    QDateTime currTime = QDateTime::currentDateTime();
    for (QObject* subscriber : d->subscribers)
    {
        LaserApplication::sendEvent(subscriber, d->frameEvent);
    }
    SAFE_DELETE(d->frameEvent);
    d->processing = false;
    qint64 duration = m_lastTime.msecsTo(currTime);
    //qLogD << "sending events duration: " << duration;
}

void CameraController::postFrameCapturedEvents()
{
    Q_D(CameraController);
    for (QObject* subscriber : d->subscribers)
    {
        LaserApplication::postEvent(subscriber, d->frameEvent);
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
    this->wait();
    m_status = CS_IDLE;
}

bool CameraController::restart()
{
    stop();
    return start();
}

CameraFrameEvent::CameraFrameEvent(CameraController* controller)
    : QEvent(static_cast<QEvent::Type>(Event_CameraFrame))
    , m_controller(controller)
{
}

CameraFrameEvent::~CameraFrameEvent()
{
    //m_controller->clearFrameCache();
}
