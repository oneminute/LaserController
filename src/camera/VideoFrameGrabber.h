#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <QObject>
#include <QAbstractVideoSurface>

class QLabel;
class ImageViewer;

class VideoFrameGrabber : public QObject, public QAbstractVideoSurface
{
    Q_OBJECT
public:
    //VideoFrameGrabber(QLabel* viewer);
    VideoFrameGrabber(ImageViewer* viewer);
    ~VideoFrameGrabber();

    QImage image() const;

    // Inherited via QAbstractVideoSurface
    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const override;
    virtual bool present(const QVideoFrame& frame) override;

public slots:
    void onImageCaptured();

signals:
    void imageCaptured();

private:
    //QLabel* m_label;
    ImageViewer* m_viewer;
    QImage m_currentImage;
};



#endif // VIDEOFRAMEGRABBER_H