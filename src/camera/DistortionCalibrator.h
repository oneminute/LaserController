#ifndef DISTORTIONCALIBRATOR_H
#define DISTORTIONCALIBRATOR_H

#include <QObject>
#include "common/common.h"
#include "ImageProcessor.h"

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>

class DistortionCalibrator : public QObject, public ImageProcessor
{
    Q_OBJECT
public:
    explicit DistortionCalibrator(QObject* parent = nullptr);
    ~DistortionCalibrator();

    bool validate();

    virtual bool process(cv::Mat& mat) override;

    bool undistortImage(cv::Mat& inMat);

    bool calibration(cv::Size imageSize, cv::Mat& cameraMatrix, cv::Mat& distCoeffs,
        std::vector<std::vector<cv::Point2f>> imagePoints, float grid_width, bool release_object);

    double computeReprojectionErrors(const std::vector<std::vector<cv::Point3f> >& objectPoints,
        const std::vector<std::vector<cv::Point2f> >& imagePoints,
        const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs,
        const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs,
        std::vector<float>& perViewErrors, bool fisheye);

    void calcBoardCornerPositions(cv::Size boardSize, float squareSize, std::vector<cv::Point3f>& corners,
        CalibrationPattern patternType /*= Settings::CHESSBOARD*/);

protected:
    bool runCalibration(cv::Size& imageSize, cv::Mat& cameraMatrix, cv::Mat& distCoeffs,
        std::vector<std::vector<cv::Point2f> > imagePoints, std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs,
        std::vector<float>& reprojErrs, double& totalAvgErr, std::vector<cv::Point3f>& newObjPoints,
        float grid_width, bool release_object);

private:
    float aspectRatio;           // The aspect ratio
    int delay;                   // In case of a video input
    bool writePoints;            // Write detected feature points
    bool writeExtrinsics;        // Write extrinsic parameters
    bool writeGrid;              // Write refined 3D target grid points
    bool calibZeroTangentDist;   // Assume zero tangential distortion
    bool calibFixPrincipalPoint; // Fix the principal point at the center
    bool flipVertical;           // Flip the captured images around the horizontal axis
    bool showUndistorted;        // Show undistorted images after calibration
    bool useFisheye;             // use fisheye camera model for calibration
    bool fixK1;                  // fix K1 distortion coefficient
    bool fixK2;                  // fix K2 distortion coefficient
    bool fixK3;                  // fix K3 distortion coefficient
    bool fixK4;                  // fix K4 distortion coefficient
    bool fixK5;                  // fix K5 distortion coefficient
    int winSize;
    int flag;
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

    QList<cv::Mat> m_images;
};

#endif // DISTORTIONCALIBRATOR_H
