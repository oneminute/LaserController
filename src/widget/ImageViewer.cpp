#include "ImageViewer.h"

#include "common/common.h"

#include <QScrollBar>

ImageViewer::ImageViewer(QWidget* parent)
    : QScrollArea(parent)
    , m_labelImage(new QLabel)
    , m_scaleFactor(1.0)
    , m_minScaleFactor(0.01)
    , m_maxScaleFactor(100)
{
    m_labelImage->setBackgroundRole(QPalette::Base);
    m_labelImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_labelImage->setScaledContents(true);

    setBackgroundRole(QPalette::Dark);
    setWidget(m_labelImage);
}

ImageViewer::~ImageViewer()
{
}

void ImageViewer::loadImage(const QString& filename)
{
}

void ImageViewer::setImage(const QImage& image)
{
    setImage(QPixmap::fromImage(image));
}

void ImageViewer::setImage(const QPixmap& image)
{
    m_image = image;
    m_labelImage->setPixmap(m_image);

    m_scaleFactor = 1.0;
    m_labelImage->adjustSize();
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::scaleImage(qreal factor)
{
    if (!m_labelImage->pixmap())
        return;

    m_scaleFactor *= factor;
    m_scaleFactor = qBound(m_minScaleFactor, m_scaleFactor, m_maxScaleFactor);
    m_labelImage->resize(m_scaleFactor * m_labelImage->pixmap()->size());

    adjustScrollBar(horizontalScrollBar(), factor);
    adjustScrollBar(verticalScrollBar(), factor);
}

qreal ImageViewer::scaleFactor() const
{
    return m_scaleFactor;
}

void ImageViewer::adjustScrollBar(QScrollBar* scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
        + ((factor - 1) * scrollBar->pageStep() / 2)));
}
