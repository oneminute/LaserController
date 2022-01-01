#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QDateTime>
#include <QObject>
#include <QThread>
#include <QQueue>
#include <QImage>
#include <QMutex>
#include <QTimer>
#include <QCameraInfo>
#include <opencv2/opencv.hpp>

class ImageProcessor;

struct FrameArgs
{
    int duration;
    int frameIndex;
    QDateTime timestamp;
};
Q_DECLARE_METATYPE(FrameArgs)

namespace cv
{
    class VideoCapture;
}

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

    static QList<int> supportedCameras();

protected:
    void run() override;
    //void addImage(const cv::Mat& mat, const cv::Mat& origin);

public slots:
    bool start();
    void stop();

protected slots:

signals:
    void connected();
    void disconnected();
    void frameCaptured(cv::Mat processed, cv::Mat origin, FrameArgs args);
    void error(int code);

private:
    QScopedPointer<cv::VideoCapture> m_videoCapture;
    int m_cameraIndex;
    QMutex m_mutex;
    int m_maxQueueLength;
    CameraStatus m_status;
    QList<ImageProcessor*> m_processors;
    bool m_autoLoading;
    QDateTime m_lastTime;
};

#endif // CAMERACONTROLLER_H