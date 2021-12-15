#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <QObject>
#include <QAbstractVideoSurface>
#include <QQueue>
#include <QMutex>

class QLabel;
class ImageViewer;

class VideoFrameGrabber : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    VideoFrameGrabber(QObject* parent = nullptr);
    ~VideoFrameGrabber();

    QImage image() const;

    // Inherited via QAbstractVideoSurface
    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const override;
    virtual bool present(const QVideoFrame& frame) override;

protected:
    void addImage(const QImage& image);

public slots:

signals:
    void imageCaptured();

private:
    QMutex m_mutex;
    QQueue<QImage> m_images;
    int m_maxQueueLength;
};



#endif // VIDEOFRAMEGRABBER_H