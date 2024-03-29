#ifndef DISTORTIONCALIBRATOR_H
#define DISTORTIONCALIBRATOR_H

#include <QObject>
#include <QMutex>
#include "common/common.h"

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>

class QGraphicsScene;
class QGraphicsPixmapItem;

struct CalibratorItem
{
    qreal confidence;
    cv::Mat3f transform;
    std::vector<cv::Point2f> points;
};

class DistortionCalibrator : public QObject
{
    Q_OBJECT
public:
    explicit DistortionCalibrator(QObject* parent = nullptr);
    ~DistortionCalibrator();

    bool validate();

    qreal captureSample(cv::Mat inMat, bool drawLines = false);

    bool undistortImage(cv::Mat& processing);
    bool perspective(const cv::Mat& inMat, cv::Mat& outMat);
    QGraphicsPixmapItem* alignToCanvas(const cv::Mat perspected, QGraphicsScene* scene);

    qreal calibrate();

    double computeReprojectionErrors(const std::vector<std::vector<cv::Point3f> >& objectPoints,
        const std::vector<std::vector<cv::Point2f> >& imagePoints,
        const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs,
        const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs,
        std::vector<float>& perViewErrors, bool fisheye);

    void calcBoardCornerPositions(cv::Size boardSize, float squareSize, std::vector<cv::Point3f>& corners,
        CalibrationPattern patternType /*= Settings::CHESSBOARD*/);

    QList<CalibratorItem> calibrationSamples() const;
    const CalibratorItem& currentItem() const;
    void removeCurrentItem();
    int calibrationSamplesCount() const;
    bool calculateHomography(const std::vector<cv::Point2f>& srcPoints,
        const std::vector<cv::Point2f>& dstPoints);
    bool isHomographyValid() const;

    void saveSamples();
    void loadSamples();

    bool saveCoeffs();
    bool loadCoeffs();

    static void generateCalibrationBoard();

protected:
    bool runCalibration(cv::Size& imageSize, cv::Mat& cameraMatrix, cv::Mat& distCoeffs,
        std::vector<std::vector<cv::Point2f> > imagePoints, std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs,
        std::vector<float>& reprojErrs, double& totalAvgErr, std::vector<cv::Point3f>& newObjPoints,
        float grid_width, bool release_object);

    static void generateCirclesBoard();
    static void generateACirclesBoard();
    static void generateChessBoard();

signals:
    //void sampleCaptured(cv::Mat mat, qreal error);

private:
    float aspectRatio;           // The aspect ratio
    int delay;                   // In case of a video input
    bool writePoints;            // Write detected feature points
    bool writeExtrinsics;        // Write extrinsic parameters
    bool writeGrid;              // Write refined 3D target grid points
    bool calibZeroTangentDist;   // Assume zero tangential distortion
    bool calibFixPrincipalPoint; // Fix the principal point at the center
    bool flipVertical;           // Flip the captured images around the horizontal axis
    int winSize;
    int flag;
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
    cv::Mat m_homography;

    QList<CalibratorItem> m_samples;
    std::vector<std::vector<cv::Point2f>> m_imagePoints;
};

#endif // DISTORTIONCALIBRATOR_H
