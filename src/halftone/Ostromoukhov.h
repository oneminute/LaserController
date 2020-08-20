#ifndef OSTROMOUKHOV_H
#define OSTROMOUKHOV_H

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/core_c.h>

// input type: 32fc1, output type: 8uc1
IplImage *OstromoukhovHalftone(IplImage *I);

cv::Mat OstromoukhovHalftone(cv::Mat I);


#endif // OSTROMOUKHOV_H
