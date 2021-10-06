#include <opencv2/opencv.hpp>
#include <opencv2/flann.hpp>

#include <QtCore>

#include <iostream>

using namespace std;
using namespace cv;


int g_d = 12;
int g_sigmaColor = 10;
int g_sigmaSpace = 50;

Mat image1;
Mat image2;

void on_Trackbar(int, void*)
{
    bilateralFilter(image1, image2, g_d, g_sigmaColor, g_sigmaSpace);
    imshow("output", image2);
}

int main(int argc, char** argv)
{
    //QCoreApplication app(argc, argv);

    //cv::Mat samples(4, 2, CV_32F);

    //samples.ptr<cv::Vec2f>(0)[0] = cv::Vec2f(-100, -100);
    //samples.ptr<cv::Vec2f>(1)[0] = cv::Vec2f(-100, 100);
    //samples.ptr<cv::Vec2f>(2)[0] = cv::Vec2f(100, 100);
    //samples.ptr<cv::Vec2f>(3)[0] = cv::Vec2f(100, -100);

    //cv::flann::Index flannIndex(samples, cv::flann::KDTreeIndexParams(1));

    //cv::Mat query(1, 2, CV_32F);
    //cv::Point2f& point = query.ptr<cv::Point2f>(0)[0];
    //point.x = -200;
    //point.y = 200;
    ////query.ptr<float>(0)[0] = -200;
    ////query.ptr<float>(0)[1] = 200;
    //cv::Mat indices, dists;
    //flannIndex.knnSearch(query, indices, dists, 1);

    //int index = indices.ptr<int>(0)[0];
    //float dist = std::sqrtf(dists.ptr<float>(0)[0]);
    //qDebug() << index << dist;
    //cv::Point2f target = samples.ptr<cv::Point2f>(index)[0];
    //qDebug() << target.x << target.y;

    //return 0;
    Mat image1 = imread("tmp/ain600dpi100mm.bmp");
    if (image1.empty())
    {
        cout << "Could not load image ... " << endl;
        return  -1;
    }

    image2 = Mat::zeros(image1.rows, image1.cols, image1.type());
    bilateralFilter(image1, image2, g_d, g_sigmaColor, g_sigmaSpace);

    namedWindow("output");

    createTrackbar("核直径", "output", &g_d, 50, on_Trackbar);
    createTrackbar("颜色空间方差", "output", &g_sigmaColor, 100, on_Trackbar);
    createTrackbar("坐标空间方差", "output", &g_sigmaSpace, 100, on_Trackbar);

    imshow("input", image1);
    imshow("output", image2);

    waitKey(0);
    return 0;
}
