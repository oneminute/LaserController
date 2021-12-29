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
        CS_LOADED,
        CS_STARTING,
        CS_STARTED,
        CS_STOPPING
    };

    explicit CameraController(QObject* parent = nullptr);
    ~CameraController();

    cv::Mat image() const;

    bool setResolution(const QSize& size);
    QSize resolution() const;

    void installProcessor(ImageProcessor* processor);
    void uninstallProcessor(ImageProcessor* processor);

    void autoLoading() const;
    void setAutoLoading(bool value);

    static QList<int> supportedCameras();

protected:
    void run() override;
    void addImage(const cv::Mat& mat);

public slots:
    bool start();
    void stop();
    bool load(int cameraIndex);

protected slots:

signals:
    void connected();
    void disconnected();
    void frameCaptured();
    void error();

private:
    QScopedPointer<cv::VideoCapture> m_videoCapture;
    int m_cameraIndex;
    QMutex m_mutex;
    QQueue<cv::Mat> m_images;
    int m_maxQueueLength;
    CameraStatus m_status;
    QList<ImageProcessor*> m_processors;
    bool m_autoLoading;
    QDateTime m_lastTime;
};

#endif // CAMERACONTROLLER_H