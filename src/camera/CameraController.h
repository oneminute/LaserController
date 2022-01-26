#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QDateTime>
#include <QEvent>
#include <QObject>
#include <QThread>
#include <QQueue>
#include <QImage>
#include <QMutex>
#include <QTimer>
#include <QCameraInfo>
#include <opencv2/opencv.hpp>

namespace cv
{
    class VideoCapture;
}

class ImageProcessor;
class CameraController;

class CameraFrameEvent : public QEvent
{
public:
    explicit CameraFrameEvent(CameraController* controller);
    ~CameraFrameEvent();

    int duration() { return m_duration; }
    void setDuration(int value) { m_duration = value; }

    int frameIndex() { return m_frameIndex; }
    void setFrameIndex(int value) { m_frameIndex = value; }

    QDateTime timestamp() { return m_timestamp; }
    void setTimestamp(const QDateTime& value) { m_timestamp = value; }

    cv::Mat thumbImage() { return m_thumbImage; }
    void setThumbImage(const cv::Mat& value) { m_thumbImage = value; }

    cv::Mat originImage() { return m_originImage; }
    void setOriginImage(const cv::Mat& value) { m_originImage = value; }

    cv::Mat processedImage() { return m_processedImage; }
    void setProcessedImage(const cv::Mat& value) { m_processedImage = value; }

private:
    int m_duration;
    int m_frameIndex;
    QDateTime m_timestamp;
    cv::Mat m_thumbImage;
    cv::Mat m_originImage;
    cv::Mat m_processedImage;
    CameraController* m_controller;
};

class CameraControllerPrivate;
class CameraController : public QThread
{
    Q_OBJECT
public:
    enum CameraStatus
    {
        CS_IDLE,
        CS_STARTED,
        CS_STOPPING
    };

    enum ErrorCode
    {
        Error_No_Cameras,
        Error_No_Data,
        Error_Processing,
        Error_Unknown
    };

    explicit CameraController(QObject* parent = nullptr);
    ~CameraController();

    bool setResolution(const QSize& size);
    QSize resolution() const;

    void installProcessor(ImageProcessor* processor);
    void uninstallProcessor(ImageProcessor* processor);

    void autoLoading() const;
    void setAutoLoading(bool value);

    void registerSubscriber(QObject* subscriber);
    void unregisterSubscriber(QObject* subscriber);

    void clearFrameCache();

    static QList<int> supportedCameras();

protected:
    void run() override;
    //void addImage(const cv::Mat& mat, const cv::Mat& origin);

    void onFrameCaptured();

    void postFrameCapturedEvents();

public slots:
    bool start();
    void stop();
    bool restart();

protected slots:

signals:
    void connected();
    void disconnected();
    void error(int code);

private:
    QScopedPointer<CameraControllerPrivate> d_ptr;
    QScopedPointer<cv::VideoCapture> m_videoCapture;
    int m_cameraIndex;
    QMutex m_mutex;
    int m_maxQueueLength;
    CameraStatus m_status;
    QList<ImageProcessor*> m_processors;
    bool m_autoLoading;
    QDateTime m_lastTime;

    Q_DECLARE_PRIVATE(CameraController)
    Q_DISABLE_COPY(CameraController)
};

class CameraControllerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(CameraController)
public:
    CameraControllerPrivate(CameraController* ptr)
        : q_ptr(ptr)
        , frameEvent(nullptr)
        , processing(false)
    {}

    CameraController* q_ptr;
    CameraFrameEvent* frameEvent;
    QList<QObject*> subscribers;
    bool processing;

signals:
    void frameCaptured();
};


#endif // CAMERACONTROLLER_H