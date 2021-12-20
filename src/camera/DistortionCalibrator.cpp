#include "DistortionCalibrator.h"

#include "common/common.h"
#include "common/Config.h"

DistortionCalibrator::DistortionCalibrator(QObject* parent)
    : QObject(parent)
    , winSize(11)
{

}

DistortionCalibrator::~DistortionCalibrator()
{
}

bool DistortionCalibrator::validate()
{
    bool goodInput = true;

    flag = 0;
    if (calibFixPrincipalPoint) flag |= cv::CALIB_FIX_PRINCIPAL_POINT;
    if (calibZeroTangentDist)   flag |= cv::CALIB_ZERO_TANGENT_DIST;
    if (aspectRatio)            flag |= cv::CALIB_FIX_ASPECT_RATIO;
    if (fixK1)                  flag |= cv::CALIB_FIX_K1;
    if (fixK2)                  flag |= cv::CALIB_FIX_K2;
    if (fixK3)                  flag |= cv::CALIB_FIX_K3;
    if (fixK4)                  flag |= cv::CALIB_FIX_K4;
    if (fixK5)                  flag |= cv::CALIB_FIX_K5;

    if (useFisheye) {
        // the fisheye model has its own enum, so overwrite the flags
        flag = cv::fisheye::CALIB_FIX_SKEW | cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC;
        if (fixK1)                    flag |= cv::fisheye::CALIB_FIX_K1;
        if (fixK2)                    flag |= cv::fisheye::CALIB_FIX_K2;
        if (fixK3)                    flag |= cv::fisheye::CALIB_FIX_K3;
        if (fixK4)                    flag |= cv::fisheye::CALIB_FIX_K4;
        if (calibFixPrincipalPoint)   flag |= cv::fisheye::CALIB_FIX_PRINCIPAL_POINT;
    }

    return goodInput;
}

bool DistortionCalibrator::process(cv::Mat& mat)
{
    cv::Mat view;
    bool blinkOutput = false;
    validate();

    if (mat.empty())
        return false;

    view = mat;

    cv::Size imageSize = view.size();  // Format input image.

    //! [find_pattern]
    std::vector<cv::Point2f> pointBuf;

    bool found;

    int chessBoardFlags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE;

    if (!useFisheye) {
        // fast check erroneously fails with high distortions like fisheye
        chessBoardFlags |= cv::CALIB_CB_FAST_CHECK;
    }

    cv::Size boardSize(Config::Camera::hCornersCount(), Config::Camera::vCornersCount());
    switch (Config::Camera::calibrationPattern()) // Find feature points on the input format
    {
    case CP_CHESSBOARD:
        found = cv::findChessboardCorners(view, boardSize, pointBuf, chessBoardFlags);
        break;
    case CP_CIRCLES_GRID:
        found = cv::findCirclesGrid(view, boardSize, pointBuf);
        break;
    case CP_ASYMMETRIC_CIRCLES_GRID:
        found = cv::findCirclesGrid(view, boardSize, pointBuf, cv::CALIB_CB_ASYMMETRIC_GRID);
        break;
    case CP_CHARUCO_BOARD:
    default:
        found = false;
        break;
    }
    //! [find_pattern]
    //! [pattern_found]
    if (found)                // If done with success,
    {
        // improve the found corners' coordinate accuracy for chessboard
        if (Config::Camera::calibrationPattern() == CP_CHESSBOARD)
        {
            cv::Mat viewGray;
            cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);
            cornerSubPix(viewGray, pointBuf, cv::Size(winSize, winSize),
                cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.0001));
        }

        // Draw the corners.
        drawChessboardCorners(view, boardSize, cv::Mat(pointBuf), found);
    }
    //! [pattern_found]
    
    return true;
}

bool DistortionCalibrator::undistortImage(cv::Mat& inMat)
{
    cv::Mat temp = inMat.clone();
    if (useFisheye)
    {
        cv::Mat newCamMat;
        cv::fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, inMat.size(),
            cv::Matx33d::eye(), newCamMat, 1);
        cv::fisheye::undistortImage(temp, inMat, cameraMatrix, distCoeffs, newCamMat);
    }
    else
        undistort(temp, inMat, cameraMatrix, distCoeffs);
    return false;
}

bool DistortionCalibrator::calibration(cv::Size imageSize, cv::Mat& cameraMatrix, cv::Mat& distCoeffs, std::vector<std::vector<cv::Point2f>> imagePoints, float grid_width, bool release_object)
{
    std::vector<cv::Mat> rvecs, tvecs;
    std::vector<float> reprojErrs;
    double totalAvgErr = 0;
    std::vector<cv::Point3f> newObjPoints;

    bool ok = false;
    ok = runCalibration(imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs, reprojErrs,
        totalAvgErr, newObjPoints, grid_width, release_object);
    qLogD << (ok ? "Calibration succeeded" : "Calibration failed") << ". avg re projection error = " << totalAvgErr << endl;

    this->cameraMatrix = cameraMatrix;
    this->distCoeffs = distCoeffs;
    /*if (ok)
        saveCameraParams(s, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, reprojErrs, imagePoints,
            totalAvgErr, newObjPoints);*/

    return ok;
}

bool DistortionCalibrator::runCalibration(cv::Size& imageSize, cv::Mat& cameraMatrix, cv::Mat& distCoeffs, std::vector<std::vector<cv::Point2f>> imagePoints, std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs, std::vector<float>& reprojErrs, double& totalAvgErr, std::vector<cv::Point3f>& newObjPoints, float grid_width, bool release_object)
{
    bool ok = false;
    //! [fixed_aspect]
    cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    if (!useFisheye && flag & cv::CALIB_FIX_ASPECT_RATIO)
        cameraMatrix.at<double>(0, 0) = aspectRatio;
    //! [fixed_aspect]
    if (useFisheye) {
        distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
    }
    else {
        distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
    }

    std::vector<std::vector<cv::Point3f> > objectPoints(1);
    cv::Size boardSize(Config::Camera::hCornersCount(), Config::Camera::vCornersCount());
    calcBoardCornerPositions(boardSize, Config::Camera::squareSize(), objectPoints[0], Config::Camera::calibrationPattern());
    objectPoints[0][boardSize.width - 1].x = objectPoints[0][0].x + grid_width;
    newObjPoints = objectPoints[0];

    objectPoints.resize(imagePoints.size(), objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters
    double rms;

    if (useFisheye) {
        cv::Mat _rvecs, _tvecs;
        rms = cv::fisheye::calibrate(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, _rvecs,
            _tvecs, flag);

        rvecs.reserve(_rvecs.rows);
        tvecs.reserve(_tvecs.rows);
        for (int i = 0; i < int(objectPoints.size()); i++) {
            rvecs.push_back(_rvecs.row(i));
            tvecs.push_back(_tvecs.row(i));
        }
    }
    else {
        int iFixedPoint = -1;
        if (release_object)
            iFixedPoint = boardSize.width - 1;
        rms = calibrateCameraRO(objectPoints, imagePoints, imageSize, iFixedPoint,
            cameraMatrix, distCoeffs, rvecs, tvecs, newObjPoints,
            flag | cv::CALIB_USE_LU);
    }

    qLogD << "Re-projection error reported by calibrateCamera: " << rms;

    ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    objectPoints.clear();
    objectPoints.resize(imagePoints.size(), newObjPoints);
    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints, rvecs, tvecs, cameraMatrix,
        distCoeffs, reprojErrs, useFisheye);

    return ok;
}

//! [compute_errors]
double DistortionCalibrator::computeReprojectionErrors(const std::vector<std::vector<cv::Point3f> >& objectPoints,
    const std::vector<std::vector<cv::Point2f> >& imagePoints,
    const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs,
    const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs,
    std::vector<float>& perViewErrors, bool fisheye)
{
    std::vector<cv::Point2f> imagePoints2;
    size_t totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for (size_t i = 0; i < objectPoints.size(); ++i)
    {
        if (fisheye)
        {
            cv::fisheye::projectPoints(objectPoints[i], imagePoints2, rvecs[i], tvecs[i], cameraMatrix,
                distCoeffs);
        }
        else
        {
            projectPoints(objectPoints[i], rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
        }
        err = norm(imagePoints[i], imagePoints2, cv::NORM_L2);

        size_t n = objectPoints[i].size();
        perViewErrors[i] = (float)std::sqrt(err * err / n);
        totalErr += err * err;
        totalPoints += n;
    }

    return std::sqrt(totalErr / totalPoints);
}
//! [compute_errors]
//! [board_corners]
void DistortionCalibrator::calcBoardCornerPositions(cv::Size boardSize, float squareSize, std::vector<cv::Point3f>& corners,
    CalibrationPattern patternType /*= Settings::CHESSBOARD*/)
{
    corners.clear();

    switch (patternType)
    {
    case CP_CHESSBOARD:
    case CP_CIRCLES_GRID:
        for (int i = 0; i < boardSize.height; ++i)
            for (int j = 0; j < boardSize.width; ++j)
                corners.push_back(cv::Point3f(j * squareSize, i * squareSize, 0));
        break;

    case CP_ASYMMETRIC_CIRCLES_GRID:
        for (int i = 0; i < boardSize.height; i++)
            for (int j = 0; j < boardSize.width; j++)
                corners.push_back(cv::Point3f((2 * j + i % 2) * squareSize, i * squareSize, 0));
        break;
    case CP_CHARUCO_BOARD:
    default:
        break;
    }
}

