#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <opencv2/opencv.hpp>

class ImageProcessor
{
public:
    ImageProcessor();
    ~ImageProcessor();

    virtual bool process(cv::Mat& mat) = 0;
};

#endif // IMAGEPROCESSOR_H