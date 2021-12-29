#include "DistortionCalibrator.h"

#include "common/common.h"
#include "common/Config.h"
#include "LaserApplication.h"
#include "ui/LaserControllerWindow.h"
#include <QImage>
#include <QPainter>
#include <QRect>
#include <QFileDialog>

DistortionCalibrator::DistortionCalibrator(QObject* parent)
    : QObject(parent)
    , winSize(11)
    , m_role(Role_Idle)
    , m_requestCalibration(false)
    , m_requestCapture(false)
    , aspectRatio(0)
    , useFisheye(false)
    , calibFixPrincipalPoint(false)
    , calibZeroTangentDist(false)
    , fixK1(false)
    , fixK2(false)
    , fixK3(false)
    , fixK4(false)
    , fixK5(false)
{

}

DistortionCalibrator::~DistortionCalibrator()
{
}

bool DistortionCalibrator::validate()
{
    bool goodInput = true;
    QSize resolution = Config::Camera::resolution();
    aspectRatio = resolution.width() * 1.0 / resolution.height();

    flag = 0;
    if (calibFixPrincipalPoint) flag |= cv::CALIB_FIX_PRINCIPAL_POINT;
    if (calibZeroTangentDist)   flag |= cv::CALIB_ZERO_TANGENT_DIST;
    if (aspectRatio)            flag |= cv::CALIB_FIX_ASPECT_RATIO;
    if (fixK1)                  flag |= cv::CALIB_FIX_K1;
    if (fixK2)                  flag |= cv::CALIB_FIX_K2;
    if (fixK3)                  flag |= cv::CALIB_FIX_K3;
    if (fixK4)                  flag |= cv::CALIB_FIX_K4;
    if (fixK5)                  flag |= cv::CALIB_FIX_K5;

    //flag |= cv::CALIB_USE_INTRINSIC_GUESS;

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
    validate();
    bool result = false;
    switch (m_role)
    {
    case Role_Idle:
        result = true;
        break;
    case Role_Capture:
        result = captureSample(mat);
        break;
    case Role_Undistortion:
        result = undistortImage(mat);
        break;
    }

    if (m_requestCalibration)
    {
        calibration();
    }

    return result;
}

bool DistortionCalibrator::captureSample(cv::Mat& mat)
{
    cv::Mat view;
    bool blinkOutput = false;

    if (mat.empty())
        return false;

    view = mat;

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
    //qLogD << "point buf size: " << pointBuf.size();

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

        if (m_requestCapture)
        {
            //m_imagePoints.push_back(pointBuf);
            CalibratorItem item;
            item.sample = view;
            item.confidence = 1.0;
            item.points = pointBuf;
            m_samples.append(item);
            m_requestCapture = false;
            emit sampleCaptured();
        }
    }

    return true;
}

bool DistortionCalibrator::undistortImage(cv::Mat& inMat)
{
    cv::Mat temp = inMat.clone();
    if (cameraMatrix.empty() || distCoeffs.empty())
        return false;
    if (useFisheye)
    {
        cv::Mat newCamMat;
        cv::fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, inMat.size(),
            cv::Matx33d::eye(), newCamMat, 1);
        cv::fisheye::undistortImage(temp, inMat, cameraMatrix, distCoeffs, newCamMat);
        return true;
    }
    else
        undistort(temp, inMat, cameraMatrix, distCoeffs);
    return true;
}

bool DistortionCalibrator::calibration()
{
    QSize resolution = Config::Camera::resolution();
    cv::Size imageSize(resolution.width(), resolution.height());
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
    std::vector<cv::Mat> rvecs, tvecs;
    std::vector<float> reprojErrs;
    double totalAvgErr = 0;
    std::vector<cv::Point3f> newObjPoints;

    m_imagePoints.clear();
    for (CalibratorItem& item : m_samples)
    {
        m_imagePoints.push_back(item.points);
    }

    bool ok = false;
    ok = runCalibration(imageSize, cameraMatrix, distCoeffs, m_imagePoints, rvecs, tvecs, 
        reprojErrs, totalAvgErr, newObjPoints, 
        (Config::Camera::hCornersCount() - 1) * Config::Camera::squareSize(), false);
    qLogD << (ok ? "Calibration succeeded" : "Calibration failed") << ". avg re projection error = " << totalAvgErr << endl;

    this->cameraMatrix = cameraMatrix;
    this->distCoeffs = distCoeffs;
    std::cout << "camera matrix: " << std::endl;
    std::cout << cameraMatrix << std::endl;
    std::cout << "dist coeffs: " << std::endl;
    std::cout << distCoeffs << std::endl;
    /*if (ok)
        saveCameraParams(s, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, reprojErrs, imagePoints,
            totalAvgErr, newObjPoints);*/
    if (ok)
        emit calibrated();

    m_requestCalibration = false;

    return ok;
}

bool DistortionCalibrator::runCalibration(cv::Size& imageSize, cv::Mat& cameraMatrix, cv::Mat& distCoeffs,
    std::vector<std::vector<cv::Point2f>> imagePoints, std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs,
    std::vector<float>& reprojErrs, double& totalAvgErr, std::vector<cv::Point3f>& newObjPoints, 
    float grid_width, bool release_object)
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
        std::cout << cameraMatrix << std::endl;
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

void DistortionCalibrator::requestCalibration()
{
    m_requestCalibration = true;
}

void DistortionCalibrator::requestCapture()
{
    m_requestCapture = true;
}

QList<CalibratorItem> DistortionCalibrator::calibrationSamples() const
{
    return m_samples;
}

int DistortionCalibrator::calibrationSamplesCount() const
{
    return m_samples.size();
}

void DistortionCalibrator::setRole(Role role)
{
    m_role = role;
}

bool DistortionCalibrator::isCaptureRole() const
{
    return m_role == Role_Capture;
}

bool DistortionCalibrator::isUndistortionRole() const
{
    return m_role == Role_Undistortion;
}

void DistortionCalibrator::saveSamples()
{
    cv::FileStorage s("tmp/calib_samples.yml", cv::FileStorage::WRITE);
    s << "samples_size" << m_samples.size();
    s << "samples" << "[";
    for (CalibratorItem& item : m_samples)
    {
        s << "{";
        //s << "sample" << item.sample;
        s << "points" << item.points;
        s << "confidence" << item.confidence;
        s << "transform" << item.transform;
        s << "}";
    }
    s << "]";
    s.release();
}

void DistortionCalibrator::loadSamples()
{
    cv::FileStorage s("tmp/calib_samples.yml", cv::FileStorage::READ);
    if (!s.isOpened())
        return;
    int count;
    s["samples_size"] >> count;
    cv::FileNode samplesNode = s["samples"];
    for (cv::FileNodeIterator it = samplesNode.begin(); it != samplesNode.end(); it++)
    {
        CalibratorItem item;
        //(*it)["sample"] >> item.sample;
        (*it)["points"] >> item.points;
        (*it)["confidence"] >> item.confidence;
        (*it)["transform"] >> item.transform;
        m_samples.append(item);
    }
    emit sampleCaptured();
    s.release();
}

void DistortionCalibrator::generateCalibrationBoard()
{
    switch (Config::Camera::calibrationPattern())
    {
    case CP_CHESSBOARD:
        generateChessBoard();
        break;
    case CP_CIRCLES_GRID:
        break;
    case CP_ASYMMETRIC_CIRCLES_GRID:
        generateACirclesBoard();
        break;
    }
}

void DistortionCalibrator::generateCirclesBoard()
{
}

void DistortionCalibrator::generateACirclesBoard()
{
    int rows = Config::Camera::vCornersCount();
    int cols = Config::Camera::hCornersCount() * 2;
    int squreSize = Config::Camera::squareSize() * 10;
    qreal halfSqureSize = squreSize * 1.0 / 2;
    qreal radius = squreSize * Config::Camera::radiusRate() / 2;
    int pageWidth = cols * squreSize + squreSize * 2;
    int pageHeight = rows * squreSize + squreSize * 2;

    QImage image(pageWidth, pageHeight, QImage::Format_Grayscale8);
    image.setDotsPerMeterX(10000);
    image.setDotsPerMeterY(10000);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.begin(&image);
    painter.setBrush(QBrush(Qt::black));
    painter.setRenderHint(QPainter::Antialiasing);
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            bool draw = (r + c) % 2 == 0;
            if (!draw)
                continue;
            QPointF center(c * squreSize + halfSqureSize + squreSize, r * squreSize + halfSqureSize + squreSize);
            painter.drawEllipse(center, radius, radius);
        }
    }
    painter.end();
    QString path = QFileDialog::getSaveFileName(LaserApplication::mainWindow, tr("Save"), tr("acircles.tiff"), tr("Images (*.png *.jpg *.tiff *.bmp)"));
    if (!path.isEmpty())
        image.save(path);

}

void DistortionCalibrator::generateChessBoard()
{
    int rows = Config::Camera::vCornersCount() + 1;
    int cols = Config::Camera::hCornersCount() + 1;
    int squreSize = Config::Camera::squareSize() * 10;

    QImage image(cols * squreSize, rows * squreSize, QImage::Format_Grayscale8);
    image.setDotsPerMeterX(10000);
    image.setDotsPerMeterY(10000);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.begin(&image);
    painter.setPen(Qt::PenStyle::NoPen);
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            bool even = (r + c) % 2 == 0;
            if (even)
                painter.setBrush(QBrush(Qt::black));
            else
                painter.setBrush(QBrush(Qt::white));
            QRect rect(c * squreSize, r * squreSize, squreSize, squreSize);
            painter.drawRect(rect);
        }
    }
    painter.end();
    QString path = QFileDialog::getSaveFileName(LaserApplication::mainWindow, tr("Save"), tr("chessboard.tiff"), tr("Images (*.png *.jpg *.tiff *.bmp)"));
    if (!path.isEmpty())
        image.save(path);
}

