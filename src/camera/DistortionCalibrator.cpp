#include "DistortionCalibrator.h"

#include "common/common.h"

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
    if (boardSize.width <= 0 || boardSize.height <= 0)
    {
        qLogW << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
        goodInput = false;
    }
    if (squareSize <= 10e-6)
    {
        qLogW << "Invalid square size " << squareSize << endl;
        goodInput = false;
    }
    if (nrFrames <= 0)
    {
        qLogW << "Invalid number of frames " << nrFrames << endl;
        goodInput = false;
    }

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

    calibrationPattern = NOT_EXISTING;
    calibrationPattern = CHESSBOARD;
    
    return goodInput;
}

bool DistortionCalibrator::process(cv::Mat& mat)
{
    cv::Mat view;
    bool blinkOutput = false;

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

    switch (calibrationPattern) // Find feature points on the input format
    {
    case CHESSBOARD:
        found = cv::findChessboardCorners(view, boardSize, pointBuf, chessBoardFlags);
        break;
    case CIRCLES_GRID:
        found = cv::findCirclesGrid(view, boardSize, pointBuf);
        break;
    case ASYMMETRIC_CIRCLES_GRID:
        found = cv::findCirclesGrid(view, boardSize, pointBuf, cv::CALIB_CB_ASYMMETRIC_GRID);
        break;
    default:
        found = false;
        break;
    }
    //! [find_pattern]
    //! [pattern_found]
    if (found)                // If done with success,
    {
        // improve the found corners' coordinate accuracy for chessboard
        if (calibrationPattern == CHESSBOARD)
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
    /*ok = cv::runCalibration(s, imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs, reprojErrs,
        totalAvgErr, newObjPoints, grid_width, release_object);
    cout << (ok ? "Calibration succeeded" : "Calibration failed")
        << ". avg re projection error = " << totalAvgErr << endl;

    if (ok)
        saveCameraParams(s, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, reprojErrs, imagePoints,
            totalAvgErr, newObjPoints);*/
    return ok;
}

bool DistortionCalibrator::runCalibration(cv::Size& imageSize, cv::Mat& cameraMatrix, cv::Mat& distCoeffs, std::vector<std::vector<cv::Point2f>> imagePoints, std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs, std::vector<float>& reprojErrs, double& totalAvgErr, std::vector<cv::Point3f>& newObjPoints, float grid_width, bool release_object)
{
    bool ok = false;
    ////! [fixed_aspect]
    //cameraMatrix = Mat::eye(3, 3, CV_64F);
    //if (!s.useFisheye && s.flag & CALIB_FIX_ASPECT_RATIO)
    //    cameraMatrix.at<double>(0, 0) = s.aspectRatio;
    ////! [fixed_aspect]
    //if (s.useFisheye) {
    //    distCoeffs = Mat::zeros(4, 1, CV_64F);
    //}
    //else {
    //    distCoeffs = Mat::zeros(8, 1, CV_64F);
    //}

    //vector<vector<Point3f> > objectPoints(1);
    //calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0], s.calibrationPattern);
    //objectPoints[0][s.boardSize.width - 1].x = objectPoints[0][0].x + grid_width;
    //newObjPoints = objectPoints[0];

    //objectPoints.resize(imagePoints.size(), objectPoints[0]);

    ////Find intrinsic and extrinsic camera parameters
    //double rms;

    //if (s.useFisheye) {
    //    Mat _rvecs, _tvecs;
    //    rms = fisheye::calibrate(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, _rvecs,
    //        _tvecs, s.flag);

    //    rvecs.reserve(_rvecs.rows);
    //    tvecs.reserve(_tvecs.rows);
    //    for (int i = 0; i < int(objectPoints.size()); i++) {
    //        rvecs.push_back(_rvecs.row(i));
    //        tvecs.push_back(_tvecs.row(i));
    //    }
    //}
    //else {
    //    int iFixedPoint = -1;
    //    if (release_object)
    //        iFixedPoint = s.boardSize.width - 1;
    //    rms = calibrateCameraRO(objectPoints, imagePoints, imageSize, iFixedPoint,
    //        cameraMatrix, distCoeffs, rvecs, tvecs, newObjPoints,
    //        s.flag | CALIB_USE_LU);
    //}

    //if (release_object) {
    //    cout << "New board corners: " << endl;
    //    cout << newObjPoints[0] << endl;
    //    cout << newObjPoints[s.boardSize.width - 1] << endl;
    //    cout << newObjPoints[s.boardSize.width * (s.boardSize.height - 1)] << endl;
    //    cout << newObjPoints.back() << endl;
    //}

    //cout << "Re-projection error reported by calibrateCamera: " << rms << endl;

    //ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    //objectPoints.clear();
    //objectPoints.resize(imagePoints.size(), newObjPoints);
    //totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints, rvecs, tvecs, cameraMatrix,
    //    distCoeffs, reprojErrs, s.useFisheye);

    return ok;
}
