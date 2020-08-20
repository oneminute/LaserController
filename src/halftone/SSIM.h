#ifndef SSIM_H
#define SSIM_H

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/core_c.h>

// will process float point input image
IplImage *ssim(IplImage *input1, IplImage *input2);

cv::Mat ssim(const cv::Mat &input1, const cv::Mat &input2);

#endif // SSIM_H
