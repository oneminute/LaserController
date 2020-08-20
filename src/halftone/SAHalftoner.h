#ifndef SAHALFTONER_H
#define SAHALFTONER_H

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/core_c.h>
#include <string>

class SAHer {
public:
    SAHer(cv::Mat &im8uc1);
    ~SAHer() {}

    void ComputeSAH(const cv::Mat &sal = cv::Mat());
    cv::Mat GetResult();

private:
    static const int IMG_TYPE;
    cv::Mat src_image_, halftone_image_;
    int w_, h_;
    void HalfToneInit();
    float Objective(const cv::Rect &roi);
};

#endif // SAHALFTONER_H
