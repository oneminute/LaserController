#include <opencv2/opencv.hpp>
#include <opencv2/flann.hpp>

#include <QtCore>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    cv::Mat samples(4, 2, CV_32F);

    samples.ptr<cv::Vec2f>(0)[0] = cv::Vec2f(-100, -100);
    samples.ptr<cv::Vec2f>(1)[0] = cv::Vec2f(-100, 100);
    samples.ptr<cv::Vec2f>(2)[0] = cv::Vec2f(100, 100);
    samples.ptr<cv::Vec2f>(3)[0] = cv::Vec2f(100, -100);

    cv::flann::Index flannIndex(samples, cv::flann::KDTreeIndexParams(1));

    cv::Mat query(1, 2, CV_32F);
    cv::Point2f& point = query.ptr<cv::Point2f>(0)[0];
    point.x = -200;
    point.y = 200;
    //query.ptr<float>(0)[0] = -200;
    //query.ptr<float>(0)[1] = 200;
    cv::Mat indices, dists;
    flannIndex.knnSearch(query, indices, dists, 1);

    int index = indices.ptr<int>(0)[0];
    float dist = std::sqrtf(dists.ptr<float>(0)[0]);
    qDebug() << index << dist;
    cv::Point2f target = samples.ptr<cv::Point2f>(index)[0];
    qDebug() << target.x << target.y;

    return 0;
}
