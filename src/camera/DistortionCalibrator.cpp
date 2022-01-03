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
    loadCoeffs();
}

DistortionCalibrator::~DistortionCalibrator()
{
}

bool DistortionCalibrator::validate()
{
    bool goodInput = true;
    QSize resolution = Config::Camera::resolution();

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

bool DistortionCalibrator::process(cv::Mat& processing, cv::Mat origin)
{
    validate();
    bool result = false;
    switch (m_role)
    {
    case Role_Idle:
        result = true;
        break;
    case Role_Capture:
        result = captureSample(processing, origin);
        break;
    case Role_Undistortion:
        result = undistortImage(processing);
        break;
    }

    if (m_requestCalibration)
    {
        calibrate();
    }

    return result;
}

bool DistortionCalibrator::captureSample(cv::Mat& processing, cv::Mat origin)
{
    if (processing.empty())
        return false;

    cv::Mat undistortedMat = origin.clone();

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
        found = cv::findChessboardCorners(processing, boardSize, pointBuf, chessBoardFlags);
        break;
    case CP_CIRCLES_GRID:
        found = cv::findCirclesGrid(processing, boardSize, pointBuf);
        break;
    case CP_ASYMMETRIC_CIRCLES_GRID:
        found = cv::findCirclesGrid(processing, boardSize, pointBuf, cv::CALIB_CB_ASYMMETRIC_GRID);
        break;
    case CP_CHARUCO_BOARD:
    default:
        found = false;
        break;
    }

    if (found)                // If done with success,
    {
        // improve the found corners' coordinate accuracy for chessboard
        if (Config::Camera::calibrationPattern() == CP_CHESSBOARD)
        {
            cv::Mat viewGray;
            cvtColor(processing, viewGray, cv::COLOR_BGR2GRAY);
            cornerSubPix(viewGray, pointBuf, cv::Size(winSize, winSize),
                cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.0001));
        }

        // Draw the corners.
        drawChessboardCorners(processing, boardSize, cv::Mat(pointBuf), found);

        if (m_requestCapture)
        {
            //m_imagePoints.push_back(pointBuf);
            CalibratorItem item;
            item.points = pointBuf;
            m_samples.append(item);
            qreal error = this->calibrate();
            m_samples.last().confidence = error;
            m_requestCapture = false;
            undistortImage(undistortedMat);
            emit sampleCaptured(undistortedMat, error);
        }
    }

    return true;
}

bool DistortionCalibrator::undistortImage(cv::Mat& processing)
{
    cv::Mat temp = processing.clone();
    if (cameraMatrix.empty() || distCoeffs.empty())
        return false;
    if (useFisheye)
    {
        cv::Mat newCamMat;
        cv::fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, processing.size(),
            cv::Matx33d::eye(), newCamMat, 1);
        cv::fisheye::undistortImage(temp, processing, cameraMatrix, distCoeffs, newCamMat);
        return true;
    }
    else
        undistort(temp, processing, cameraMatrix, distCoeffs);
    return true;
}

qreal DistortionCalibrator::calibrate()
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
    m_requestCalibration = false;

    if (!ok)
        return -1;
    return totalAvgErr;
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
            //flag | cv::CALIB_USE_LU);
            flag);
    }

    qLogD << "Re-projection error reported by calibrateCamera: " << rms;

    ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    objectPoints.clear();
    objectPoints.resize(imagePoints.size(), newObjPoints);
    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints, rvecs, tvecs, cameraMatrix,
        distCoeffs, reprojErrs, useFisheye);

    qLogD << "totalAvgErr: " << totalAvgErr;

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

const CalibratorItem& DistortionCalibrator::currentItem() const
{
    return m_samples.last();
}

void DistortionCalibrator::removeCurrentItem()
{
    m_samples.removeLast();
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
    qreal error = 0;
    for (cv::FileNodeIterator it = samplesNode.begin(); it != samplesNode.end(); it++)
    {
        CalibratorItem item;
        //(*it)["sample"] >> item.sample;
        (*it)["points"] >> item.points;
        (*it)["confidence"] >> item.confidence;
        (*it)["transform"] >> item.transform;
        m_samples.append(item);
        error = item.confidence;
    }
    emit sampleCaptured(cv::Mat(), error);
    s.release();
}

bool DistortionCalibrator::saveCoeffs()
{
    QList<QVariant> coeffs;
    coeffs
        << cameraMatrix.at<double>(0, 0)
        << cameraMatrix.at<double>(1, 1)
        << cameraMatrix.at<double>(0, 2)
        << cameraMatrix.at<double>(1, 2)
        << distCoeffs.at<double>(0)
        << distCoeffs.at<double>(1)
        << distCoeffs.at<double>(2)
        << distCoeffs.at<double>(3)
        << distCoeffs.at<double>(4)
        ;
    qLogD << coeffs;
    Config::Camera::undistortionCoeffsItem()->setValue(coeffs, SS_DIRECTLY, this);
    return true;
}

bool DistortionCalibrator::loadCoeffs()
{
    QList<QVariant> coeffs = Config::Camera::undistortionCoeffs();
    qLogD << coeffs;
    cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
    cameraMatrix.at<double>(0, 0) = coeffs.at(0).toReal();
    cameraMatrix.at<double>(1, 1) = coeffs.at(1).toReal();
    cameraMatrix.at<double>(0, 2) = coeffs.at(2).toReal();
    cameraMatrix.at<double>(1, 2) = coeffs.at(3).toReal();
    distCoeffs.at<double>(0) = coeffs.at(4).toReal();
    distCoeffs.at<double>(1) = coeffs.at(5).toReal();
    distCoeffs.at<double>(2) = coeffs.at(6).toReal();
    distCoeffs.at<double>(3) = coeffs.at(7).toReal();
    distCoeffs.at<double>(4) = coeffs.at(8).toReal();
    return true;
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
    int squareSize = Config::Camera::squareSize() * 10;
    qreal halfSqureSize = squareSize * 1.0 / 2;
    qreal radius = squareSize * Config::Camera::radiusRate() / 2;
    int pageWidth = cols * squareSize + squareSize * 2;
    int pageHeight = rows * squareSize + squareSize * 2;

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
            QPointF center(c * squareSize + halfSqureSize + squareSize, r * squareSize + halfSqureSize + squareSize);
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
    int squareSize = Config::Camera::squareSize() * 10;

    QImage image(cols * squareSize, rows * squareSize, QImage::Format_Grayscale8);
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
            QRect rect(c * squareSize, r * squareSize, squareSize, squareSize);
            painter.drawRect(rect);
        }
    }
    painter.end();
    QString path = QFileDialog::getSaveFileName(LaserApplication::mainWindow, tr("Save"), tr("chessboard.tiff"), tr("Images (*.png *.jpg *.tiff *.bmp)"));
    if (!path.isEmpty())
        image.save(path);
}

