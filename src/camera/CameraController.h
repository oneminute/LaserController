#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QImage>
#include <QMutex>
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

protected:
    void run() override;
    void addImage(const cv::Mat& mat);

public slots:
    bool start();
    void stop();
    bool load(int cameraIndex);

signals:
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
};

#endif // CAMERACONTROLLER_H