#include "ImageProcessor.h"

ImageProcessor::ImageProcessor()
    : m_enabled(true)
{
}

ImageProcessor::~ImageProcessor()
{
}

bool ImageProcessor::enabled() const
{
    return m_enabled;
}

void ImageProcessor::setEnabled(bool value)
{
    m_enabled = value;
}
