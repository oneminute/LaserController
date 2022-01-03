#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <opencv2/opencv.hpp>

class ImageProcessor
{
public:
    ImageProcessor();
    ~ImageProcessor();

    virtual bool process(cv::Mat& processing, cv::Mat origin) = 0;

    bool enabled() const;
    void setEnabled(bool value);

private:
    bool m_enabled;
};

#endif // IMAGEPROCESSOR_H