#include "VideoFrameGrabber.h"

#include "widget/ImageViewer.h"
#include <QMutexLocker>

VideoFrameGrabber::VideoFrameGrabber(QObject* parent)
    : QAbstractVideoSurface(parent)
    , m_maxQueueLength(10)
{
}

VideoFrameGrabber::~VideoFrameGrabber()
{
}

QImage VideoFrameGrabber::image() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    if (m_images.empty())
        return QImage();
    else
    {
        return m_images.last();
    }
}

QList<QVideoFrame::PixelFormat> VideoFrameGrabber::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
    static QList<QVideoFrame::PixelFormat> formats = {
        QVideoFrame::Format_ARGB32,
        QVideoFrame::Format_ARGB32_Premultiplied,
        QVideoFrame::Format_RGB32,
        QVideoFrame::Format_RGB24,
        QVideoFrame::Format_RGB565,
        QVideoFrame::Format_RGB555,
        QVideoFrame::Format_ARGB8565_Premultiplied,
        QVideoFrame::Format_BGRA32,
        QVideoFrame::Format_BGRA32_Premultiplied,
        QVideoFrame::Format_BGR32,
        QVideoFrame::Format_BGR24,
        QVideoFrame::Format_BGR565,
        QVideoFrame::Format_BGR555,
        QVideoFrame::Format_BGRA5658_Premultiplied,
        QVideoFrame::Format_AYUV444,
        QVideoFrame::Format_AYUV444_Premultiplied,
        QVideoFrame::Format_YUV444,
        QVideoFrame::Format_YUV420P,
        QVideoFrame::Format_YV12,
        QVideoFrame::Format_UYVY,
        QVideoFrame::Format_CameraRaw,
        QVideoFrame::Format_AdobeDng
    };
    return formats;
}

bool VideoFrameGrabber::present(const QVideoFrame& frame)
{
    if (frame.isValid()) {
        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        const QImage img(cloneFrame.bits(),
            cloneFrame.width(),
            cloneFrame.height(),
            QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat()));

        addImage(img.mirrored(false, true));

        cloneFrame.unmap();
        emit imageCaptured();
        return true;
    }
    return false;
}

void VideoFrameGrabber::addImage(const QImage& image)
{
    QMutexLocker locker(&m_mutex);
    m_images.enqueue(image);
    if (m_images.length() > m_maxQueueLength)
    {
        m_images.dequeue();
    }
}
