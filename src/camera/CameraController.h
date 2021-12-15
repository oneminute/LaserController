#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QVideoFrame>
#include <QMutex>
#include <QCameraInfo>

class QCamera;
class VideoFrameGrabber;

class CameraController : public QObject
{
    Q_OBJECT
public:
    explicit CameraController(const QCameraInfo& info, QObject* parent = nullptr);
    ~CameraController();

    QCamera* camera() const;
    QCameraInfo cameraInfo() const;

    QImage image() const;

public slots:
    void start();
    void stop();
    void load();
    void unload();

signals:
    void started();
    void stopped();
    void frameCaptured();

private:
    QThread m_thread;
    QCamera* m_camera;
    QCameraInfo m_cameraInfo;
    VideoFrameGrabber* m_grabber;
    QMutex m_mutex;
    int m_maxFramesQueueLength;
};

#endif // CAMERACONTROLLER_H