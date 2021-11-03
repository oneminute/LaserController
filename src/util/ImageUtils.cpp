#include "ImageUtils.h"

#include <iostream>

#include <QDebug>
#include <QFile>
#include <QtMath>
#include <QMap>
#include <QPainterPath>
#include <QRandomGenerator>

#include "common/common.h"
#include "common/Config.h"
#include "laser/LaserDriver.h"
#include "UnitUtils.h"
#include "LaserApplication.h"
#include "task/ProgressItem.h"
#include "task/ProgressModel.h"

cv::Mat imageUtils::halftone(cv::Mat src, float mmWidth, float mmHeight, float lpi, float dpi, float degrees)
{
    float inchWidth = mmWidth * MM_TO_INCH;
    float inchHeight = mmHeight * MM_TO_INCH;

    float outHPixels = inchWidth * dpi;
    float outVPixels = inchHeight * dpi;

    int dotsHCount = std::round(inchWidth * lpi);
    int dotsVCount = std::round(inchHeight * lpi);

    float dotsPerPixelH = std::round(dotsHCount * 1.0 / src.cols);
    float dotsPerPixelV = std::round(dotsVCount * 1.0 / src.rows);

    float dotWidth = std::round(outHPixels / dotsHCount);
    float dotHeight = std::round(outVPixels / dotsVCount);

    int gridSize = std::round(dotWidth * 1.);

    cv::Mat outMat(outVPixels, outHPixels, CV_8UC1, cv::Scalar(255));
    cv::Mat dotsMat(dotsVCount, dotsHCount, CV_32S, cv::Scalar(255));
    int grayGrades;
    cv::Mat ditchMat = generateSpiralPattern(gridSize, grayGrades, degrees);

    qDebug().noquote().nospace() << "          lpi: " << lpi;
    qDebug().noquote().nospace() << "          dpi: " << dpi;
    qDebug().noquote().nospace() << "     src cols: " << src.cols;
    qDebug().noquote().nospace() << "     src rows: " << src.rows;
    qDebug().noquote().nospace() << "      mmWidth: " << mmWidth;
    qDebug().noquote().nospace() << "     mmHeight: " << mmHeight;
    qDebug().noquote().nospace() << "    inchWidth: " << inchWidth;
    qDebug().noquote().nospace() << "   inchHeight: " << inchHeight;
    qDebug().noquote().nospace() << "   outHPixels: " << outHPixels;
    qDebug().noquote().nospace() << "   outVPixels: " << outVPixels;
    qDebug().noquote().nospace() << "   dotsHCount: " << dotsHCount;
    qDebug().noquote().nospace() << "   dotsVCount: " << dotsVCount;
    qDebug().noquote().nospace() << "dotsPerPixelH: " << dotsPerPixelH;
    qDebug().noquote().nospace() << "dotsPerPixelV: " << dotsPerPixelV;
    qDebug().noquote().nospace() << "     dotWidth: " << dotWidth;
    qDebug().noquote().nospace() << "    dotHeight: " << dotHeight;
    qDebug().noquote().nospace() << "     gridSize: " << gridSize;
    qDebug().noquote().nospace() << "   grayGrades: " << grayGrades;
    qDebug().noquote().nospace() << "ditchMat size: " << ditchMat.cols;


    /*for (int r = 0; r < dotsVCount; r++)
    {
        for (int c = 0; c < dotsHCount; c++)
        {
            int x = std::floor(c * 1.0 * (src.cols - 1) / (dotsHCount - 1));
            int y = std::floor(r * 1.0 * (src.rows - 1) / (dotsVCount - 1));

            dotsMat.ptr<float>(r)[c] = src.ptr<uchar>(y)[x];
        }
    }*/
    cv::Mat tmp;
    src.convertTo(tmp, CV_32S);
    cv::resize(tmp, dotsMat, dotsMat.size());
    cv::imwrite("dotsMat.tiff", dotsMat);

    float threshold = 64.5f;

    float coa = 7.f / 16;
    float cob = 3.f / 16;
    float coc = 5.f / 16;
    float cod = 1.f / 16;

    for (int r = 0; r < dotsVCount; r++)
    {
        for (int c = 0; c < dotsHCount; c++)
        {
            cv::Point start(c * dotWidth + dotWidth / 2, r * dotWidth + dotWidth / 2);
            for (int x = start.x - ditchMat.cols / 2; x < start.x - ditchMat.cols / 2 + ditchMat.cols; x++)
            {
                for (int y = start.y - ditchMat.rows / 2; y < start.y - ditchMat.rows / 2 + ditchMat.rows; y++)
                {
#pragma region calculate gray value using lerp of neighbour points
                    int nx = x < start.x ? c - 1 : c + 1;   // x of neighbour point
                    int ny = y < start.y ? r - 1 : r + 1;   // b of neighbour point

                    // constrain nx and ny
                    nx = std::max(nx, 0);
                    nx = std::min(nx, dotsMat.cols - 1);
                    ny = std::max(ny, 0);
                    ny = std::min(ny, dotsMat.rows - 1);
                    
                    cv::Point na(c, r);     // center point
                    cv::Point nb(nx, r);    // neighbour point at same hori line
                    cv::Point nc(c, ny);    // neighbour point at same vert line
                    cv::Point nd(nx, ny);   // neighbour point at diagonal line

                    // gray grades value of each point
                    float ga = (255 - dotsMat.ptr<float>(na.y)[na.x]) / 255 * grayGrades;
                    float gb = (255 - dotsMat.ptr<float>(nb.y)[nb.x]) / 255 * grayGrades;
                    float gc = (255 - dotsMat.ptr<float>(nc.y)[nc.x]) / 255 * grayGrades;
                    float gd = (255 - dotsMat.ptr<float>(nd.y)[nd.x]) / 255 * grayGrades;

                    float maxDist = gridSize / 2.f;  // max distance
                    float ratiox = std::abs(x - start.x) / maxDist;     // hori distance to center / max distance
                    float ratioy = std::abs(y - start.y) / maxDist;     // vert distance to center / max distance

                    float g1 = (1 - ratiox) * ga + ratiox * gb;
                    float g2 = (1 - ratiox) * gc + ratiox * gd;
                    float g3 = (1 - ratioy) * ga + ratioy * gc;
                    float g4 = (1 - ratioy) * gb + ratioy * gd;

                    float gray = std::round(
                        (1 - ratioy) / 2 * g1 +
                        ratioy / 2 * g2 +
                        (1 - ratiox) / 2 * g3 +
                        ratiox / 2 * g4);
#pragma endregion

                    if (gray > ditchMat.ptr<uchar>(y - start.y + ditchMat.rows / 2)[x - start.x + ditchMat.cols / 2])
                    {
                        if (y < outMat.rows && x < outMat.cols && x >= 0 && y >= 0)
                            outMat.ptr<uchar>(y)[x] = 0;
                    }
                }
            }
        }
    }
    cv::imwrite("dotsMat2.tiff", dotsMat);
    cv::imwrite("outMat.bmp", outMat);
    return cv::Mat();
}

cv::Mat imageUtils::halftone2(cv::Mat src, float lpi, float dpi, float degrees, float nonlinearCoefficient)
{
    // convert unit from mm to inch
    float inchWidth = src.cols / dpi;
    float inchHeight = src.rows / dpi;

    // choose the bigger one between gridwidth and grid m_height
    //int gridSize = std::round(std::ceil(dpi / lpi) * sqrt(2));
    int gridSize = std::ceil(dpi / lpi) / 2;
    if (gridSize % 2)
        gridSize += 1;
    //gridSize = ((gridSize / 2) + 1) * 2;

    // out image
    cv::Mat outMat(src.rows, src.cols, CV_8UC1, cv::Scalar(255));

    int grayGrades;
    //cv::Mat ditchMat = generateRoundSpiralPattern(gridSize, grayGrades, degrees);
    //cv::Mat ditchMat = generateCircleMat(gridSize, grayGrades, degrees);
    cv::Mat ditchMat = generateRoundSpiralMat(gridSize);
    ditchMat = generateRotatedPattern45(ditchMat);
    grayGrades = gridSize * gridSize * 3;

    qDebug().noquote().nospace() << "          lpi: " << lpi;
    qDebug().noquote().nospace() << "          dpi: " << dpi;
    qDebug().noquote().nospace() << "angle degrees: " << degrees;
    qDebug().noquote().nospace() << "     src cols: " << src.cols;
    qDebug().noquote().nospace() << "     src rows: " << src.rows;
    qDebug().noquote().nospace() << "    src dpi h: " << src.cols / inchWidth;
    qDebug().noquote().nospace() << "    src dpi v: " << src.rows / inchHeight;
    qDebug().noquote().nospace() << "    inchWidth: " << inchWidth;
    qDebug().noquote().nospace() << "   inchHeight: " << inchHeight;
    qDebug().noquote().nospace() << "     gridSize: " << gridSize;
    qDebug().noquote().nospace() << "   grayGrades: " << grayGrades;
    qDebug().noquote().nospace() << "ditchMat size: " << ditchMat.cols;

    for (int r = 0; r < src.rows; r += ditchMat.rows)
    {
        for (int c = 0; c < src.cols; c += ditchMat.cols)
        {
            cv::Point start(c, r);
            for (int x = 0; x < ditchMat.cols; x++)
            {
                for (int y = 0; y < ditchMat.rows; y++)
                {
                    if (r + y < src.rows && c + x < src.cols)
                    {
                        uchar srcPixel = src.ptr<uchar>(r + y)[c + x];
                        //int grayValue = std::ceil((255.f - srcPixel) * grayGrades / 255);
                        float grayValue = std::pow((255.f - srcPixel) / 255, nonlinearCoefficient) * grayGrades;
                        float ditchPixel = ditchMat.ptr<float>(y)[x];

                        if (grayValue > ditchPixel)
                            outMat.ptr<uchar>(r + y)[c + x] = 0;
                    }
                }
            }
        }
    }
    std::vector<int>param; 
    param.push_back(cv::IMWRITE_PNG_BILEVEL);
    param.push_back(1);
    param.push_back(cv::IMWRITE_PNG_COMPRESSION);
    param.push_back(0);
    cv::imwrite("outMat.png", outMat, param);
    return outMat;
}

cv::Mat imageUtils::halftone3(cv::Mat src, float lpi, float dpi, float degrees)
{
    qreal cos = qCos(qDegreesToRadians(degrees));
    //int lpi2 = qFloor(lpi * cos);
    cv::Point2f center((src.cols - 1) / 2.f, (src.rows - 1) / 2.f);
    cv::Mat rot = cv::getRotationMatrix2D(center, degrees, 1.);
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), degrees).boundingRect2f();

	int rotatedWidth = qMax(static_cast<int>(bbox.width), src.cols);
	int rotatedHeight = qMax(static_cast<int>(bbox.height), src.rows);
    // adjust transformation matrix
    rot.at<double>(0, 2) += rotatedWidth / 2.0 - src.cols / 2.0;
    rot.at<double>(1, 2) += rotatedHeight / 2.0 - src.rows / 2.0;

    cv::imwrite("tmp/src.bmp", src);
    cv::Mat rotated;
    cv::warpAffine(src, rotated, rot, cv::Size(rotatedWidth, rotatedHeight), cv::INTER_AREA, 0, cv::Scalar(255));
    cv::imwrite("tmp/halfton3_rot.bmp", rotated);

    // convert unit from mm to inch
    float inchWidth = src.cols / dpi;
    float inchHeight = src.rows / dpi;

    // choose the bigger one between gridwidth and grid m_height
    int gridSize = qRound(dpi / lpi * cos);

    // out image
    cv::Mat outMat(rotated.rows, rotated.cols, CV_8UC1, cv::Scalar(0));

    int grayGrades;
    cv::Mat ditchMat = generateRoundSpiralMat(gridSize);
    ditchMat.convertTo(ditchMat, CV_8UC1);
    grayGrades = gridSize * gridSize;
    for (int x = 0; x < ditchMat.cols; x++)
    {
        for (int y = 0; y < ditchMat.rows; y++)
        {
            quint8 ditchPixel = ditchMat.ptr<quint8>(y)[x];
            ditchPixel = grayGrades - ditchPixel - 1;
            ditchPixel = ditchPixel * 255 / (grayGrades - 1);
            ditchMat.ptr<quint8>(y)[x] = ditchPixel;
        }
    }
    std::cout << "ditchMat:" << std::endl << ditchMat << std::endl;

    qDebug().noquote().nospace() << "          lpi: " << lpi;
    qDebug().noquote().nospace() << "          dpi: " << dpi;
    qDebug().noquote().nospace() << "angle degrees: " << degrees;
    qDebug().noquote().nospace() << "     src cols: " << src.cols;
    qDebug().noquote().nospace() << "     src rows: " << src.rows;
    qDebug().noquote().nospace() << "    src dpi h: " << src.cols / inchWidth;
    qDebug().noquote().nospace() << "    src dpi v: " << src.rows / inchHeight;
    qDebug().noquote().nospace() << "    inchWidth: " << inchWidth;
    qDebug().noquote().nospace() << "   inchHeight: " << inchHeight;
    qDebug().noquote().nospace() << "     gridSize: " << gridSize;
    qDebug().noquote().nospace() << "   grayGrades: " << grayGrades;
    qDebug().noquote().nospace() << "ditchMat size: " << ditchMat.cols;

    for (int r = 0; r < rotated.rows; r += ditchMat.rows)
    {
        for (int c = 0; c < rotated.cols; c += ditchMat.cols)
        {
            cv::Point start(c, r);
            for (int x = 0; x < ditchMat.cols; x++)
            {
                for (int y = 0; y < ditchMat.rows; y++)
                {
                    if (r + y < rotated.rows && c + x < rotated.cols)
                    {
                        uchar srcPixel = rotated.ptr<uchar>(r + y)[c + x];
                        uchar ditchPixel = ditchMat.ptr<uchar>(y)[x];

                        if (srcPixel >= ditchPixel)
                            outMat.ptr<uchar>(r + y)[c + x] = 255;
                    }
                }
            }
        }
    }
    std::vector<int>param; 
    param.push_back(cv::IMWRITE_PNG_BILEVEL);
    param.push_back(1);
    param.push_back(cv::IMWRITE_PNG_COMPRESSION);
    param.push_back(0);
    cv::imwrite("tmp/halftone3_outMat.png", outMat, param);

    center = cv::Point2f((rotated.cols - 1) / 2, (rotated.rows - 1) / 2);
    rot = cv::getRotationMatrix2D(center, -degrees, 1.);
    bbox = cv::RotatedRect(cv::Point2f(), rotated.size(), 0).boundingRect2f();
    cv::warpAffine(outMat, rotated, rot, bbox.size(), cv::INTER_NEAREST, 0, cv::Scalar(255));
    cv::imwrite("tmp/halfton3_rot_inv.png", rotated, param);

	int roix = (rotated.cols - src.cols - 1) / 2;
	int roiy = (rotated.rows - src.rows - 1) / 2;
	int roiCols = src.cols;
	int roiRows = src.rows;
	if (roix < 0)
	{
		roix = 0;

	}
    cv::Rect roi(roix, roiy, roiCols, roiRows);
    outMat = rotated(roi);
    cv::imwrite("tmp/halfton3_processed.png", outMat, param);

#ifdef _DEBUG
    cv::imshow("halftone3_processed", outMat);
#endif

    /*cv::threshold(outMat, outMat, 225, 255, cv::THRESH_BINARY);
    cv::imwrite("temp/halfton3_threshed.bmp", outMat);

    cv::Mat kernel = (cv::Mat_ <uchar>(3, 3) << 1, 1, 1, 1, 0, 1, 1, 1, 1);
    cv::Mat filter2DMat;
    cv::filter2D(outMat, filter2DMat, outMat.depth(), kernel);
    cv::threshold(filter2DMat, filter2DMat, 0, 255, cv::THRESH_BINARY);

    outMat = outMat & filter2DMat;
    cv::imwrite("temp/halfton3_final.bmp", outMat);*/

    return outMat;
}

cv::Mat imageUtils::halftone4(ProgressItem* progress, cv::Mat src, float degrees, int gridSize)
{
    // 根据角度计算v1和v2两个相互正交的向量，向量长度是网格长度
    QTransform t1, t2;
    t1.rotate(degrees);
    t2.rotate(degrees - 90);
    QPointF p1(gridSize, 0);
    QPointF p2(gridSize, 0);
    QVector2D v1(t1.map(p1));
    QVector2D v2(t2.map(p2));
    // 让v1表示偏x轴的向量，v2表示偏y轴的向量
    if (qAbs(v1.x()) < qAbs(v1.y()))
        qSwap(v1, v2);

    src = 255 - src;

    QRectF grid(QPointF(0, 0), QPointF(gridSize, gridSize));
    grid = t1.mapRect(grid);

    int dx = qCeil(grid.width());
    int dy = qCeil(grid.height());
    //int dx = qRound(qAbs(QVector2D::dotProduct(v1, QVector2D(1, 0))));
    //int dy = qRound(qAbs(QVector2D::dotProduct(v2, QVector2D(0, 1))));
    //int halfXCount = qCeil(src.cols / (dx * 2));
    //int halfYCount = qCeil(src.rows / (dy * 2));

    QVector2D center(src.cols * 1.0 / 2, src.rows * 1.0 / 2);
    int halfXCount = qCeil(qAbs((QVector2D::dotProduct(center, v1.normalized()) / gridSize)));
    int halfYCount = qCeil(qAbs((QVector2D::dotProduct(center, v2.normalized()) / gridSize)));

    int margin = qMax(halfXCount, halfYCount);
    qreal halfGridSize = gridSize / 2;

    progress->setMaximum(margin * 2);
    // 生成网格
    cv::Mat dst(src.size(), CV_8UC1, cv::Scalar(0));
    for (int c = -margin; c <= margin; c++)
    {
        for (int r = -margin; r <= margin; r++)
        {
            QVector2D v = v1 * c + v2 * r + center;
            int x = qRound(v.x());
            int y = qRound(v.y());
            if (x < 0 || y < 0 || x >= src.cols || y >= src.rows)
                continue;
            //cv::rectangle(dst, cv::Rect(x - dx / 2, y - dy / 2, dx, dy), cv::Scalar(0));

            QRect qRectSrc(qRound(x - halfGridSize), qRound(y - halfGridSize), gridSize, gridSize);
            qRectSrc.setLeft(qMax(0, qRectSrc.left()));
            qRectSrc.setTop(qMax(0, qRectSrc.top()));
            qRectSrc.setRight(qMin(src.cols - 1, qRectSrc.right()));
            qRectSrc.setBottom(qMin(src.rows - 1, qRectSrc.bottom()));
            if (!qRectSrc.isValid() || qRectSrc.right() < 0 || qRectSrc.left() >= src.cols || qRectSrc.bottom() < 0 || qRectSrc.top() >= src.rows)
                continue;

            QRect qRectDst(qRound(x - halfGridSize), qRound(y - halfGridSize), gridSize, gridSize);
            qRectDst.setLeft(qMax(0, qRectDst.left()));
            qRectDst.setTop(qMax(0, qRectDst.top()));
            qRectDst.setRight(qMin(src.cols - 1, qRectDst.right()));
            qRectDst.setBottom(qMin(src.rows - 1, qRectDst.bottom()));
            if (!qRectDst.isValid() || qRectDst.right() < 0 || qRectDst.left() >= src.cols || qRectDst.bottom() < 0 || qRectSrc.top() >= src.rows)
                continue;

            cv::Rect rectSrc(qRectSrc.x(), qRectSrc.y(), qRectSrc.width(), qRectSrc.height());
            cv::Rect rectDst(qRectDst.x(), qRectDst.y(), qRectDst.width(), qRectDst.height());

            cv::Mat srcRoi = src(rectSrc);
            cv::Mat dstRoi = dst(rectDst);

            //qLogD << r << c << "src:" << srcRoi.rows << srcRoi.cols << ", dst:" << dstRoi.rows << dstRoi.cols;
            //generateGroupingDitcher(srcRoi, dstRoi);

            //int sum = cv::sum(srcRoi)[0];
            QPoint center;
            int sum = sumMat(srcRoi, center);
            int initAngle = QRandomGenerator::global()->bounded(4) * 90;
            int rotationAngle = 90 + 45 * QRandomGenerator::global()->bounded(2);
            int stepAngle = -45;
            if (QRandomGenerator::global()->bounded(2))
            {
                rotationAngle = -rotationAngle;
                stepAngle = -stepAngle;
            }
            generatePattern(dstRoi, sum, center);
        }
        progress->increaseProgress();
    }
    dst = 255 - dst;
    cv::imwrite("tmp/dst.bmp", dst);
    progress->finish();

    return dst;
}

cv::Mat imageUtils::halftone5(cv::Mat src, float degrees, int gridSize)
{
    Qt::Orientation orient = degrees <= 45 ? Qt::Vertical : Qt::Horizontal;
    qreal radians = qDegreesToRadians(degrees);
    QVector2D center(src.cols * 1.0 / 2, src.rows * 1.0 / 2);
    int halfCols = qCeil(src.cols * 1.0 / gridSize / 2);
    int halfRows = qCeil(src.rows * 1.0 / gridSize / 2);
    qreal halfGridSize = gridSize / 2;
    cv::Mat dst(src.size(), CV_8UC1, cv::Scalar(0));

    if (orient == Qt::Vertical)
    {
        QVector<int> offsets;

        qreal sinValue = qSin(radians);
        for (int c = -halfCols; c <= halfCols; c++)
        {
            int offset = qRound(c * gridSize * sinValue);
            qLogD << c << ", " << offset << ", " << (offset % gridSize);
            offset = offset % gridSize;
            if (offset < 0)
                offset += gridSize;
            offsets.append(offset);
            for (int r = -halfRows; r <= halfRows; r++)
            {
                qreal x = c * gridSize + center.x();
                qreal y = r * gridSize + center.y() + offset;

                QRect qRect(qRound(x - halfGridSize), qRound(y - halfGridSize), gridSize, gridSize);
                qRect.setLeft(qMax(0, qRect.left()));
                qRect.setTop(qMax(0, qRect.top()));
                qRect.setRight(qMin(src.cols - 1, qRect.right()));
                qRect.setBottom(qMin(src.rows - 1, qRect.bottom()));

                if (!qRect.isValid() || qRect.right() < 0 || qRect.left() >= src.cols || qRect.bottom() < 0 || qRect.top() >= src.rows)
                    continue;

                cv::Rect rect(qRect.x(), qRect.y(), qRect.width(), qRect.height());

                cv::Mat srcRoi = src(rect);
                cv::Mat dstRoi = dst(rect);

                //generateGroupingDitcher(srcRoi, dstRoi);
                //int sum = cv::sum(srcRoi)[0];
                QPoint center;
                int sum = sumMat(srcRoi, center);
                int initAngle = QRandomGenerator::global()->bounded(4) * 90;
                int rotationAngle = 90 + 45 * QRandomGenerator::global()->bounded(2);
                int stepAngle = -45;
                if (QRandomGenerator::global()->bounded(2))
                {
                    rotationAngle = -rotationAngle;
                    stepAngle = -stepAngle;
                }
                generatePattern(dstRoi, sum, center);
            }
        }
    }
    else if (orient == Qt::Horizontal)
    {
        QVector<int> offsets;

        qreal cosValue = qCos(radians);
        for (int r = -halfRows; r <= halfRows; r++)
        {
            int offset = qRound(r * gridSize * cosValue);
            qLogD << r << ", " << offset << ", " << (offset % gridSize);
            offset = offset % gridSize;
            if (offset < 0)
                offset += gridSize;
            offsets.append(offset);
            for (int c = -halfCols; c <= halfCols; c++)
            {
                qreal x = c * gridSize + center.x() + offset;
                qreal y = r * gridSize + center.y();

                QRect qRect(qRound(x - halfGridSize), qRound(y - halfGridSize), gridSize, gridSize);
                qRect.setLeft(qMax(0, qRect.left()));
                qRect.setTop(qMax(0, qRect.top()));
                qRect.setRight(qMin(src.cols - 1, qRect.right()));
                qRect.setBottom(qMin(src.rows - 1, qRect.bottom()));

                if (!qRect.isValid() || qRect.right() < 0 || qRect.left() >= src.cols || qRect.bottom() < 0 || qRect.top() >= src.rows)
                    continue;

                cv::Rect rect(qRect.x(), qRect.y(), qRect.width(), qRect.height());

                cv::Mat srcRoi = src(rect);
                cv::Mat dstRoi = dst(rect);

                //generateGroupingDitcher(srcRoi, dstRoi);
                //int sum = cv::sum(srcRoi)[0];
                QPoint center;
                int sum = sumMat(srcRoi, center);
                int initAngle = QRandomGenerator::global()->bounded(4) * 90;
                int rotationAngle = 90 + 45 * QRandomGenerator::global()->bounded(2);
                int stepAngle = -45;
                if (QRandomGenerator::global()->bounded(2))
                {
                    rotationAngle = -rotationAngle;
                    stepAngle = -stepAngle;
                }
                generatePattern(dstRoi, sum, center);
            }
        }
    }
    cv::imwrite("tmp/dst.bmp", dst);
    dst = 255 - dst;

    return dst;
}

cv::Mat imageUtils::halftone6(ProgressItem* progress, cv::Mat src, float degrees, int gridSize)
{
    src = 255 - src;
    cv::Point2f center((src.cols - 1) / 2.f, (src.rows - 1) / 2.f);
    cv::Mat rot = cv::getRotationMatrix2D(center, degrees, 1.);
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), degrees).boundingRect2f();

	int rotatedWidth = qMax(static_cast<int>(bbox.width), src.cols);
	int rotatedHeight = qMax(static_cast<int>(bbox.height), src.rows);
    rot.at<double>(0, 2) += rotatedWidth / 2.0 - src.cols / 2.0;
    rot.at<double>(1, 2) += rotatedHeight / 2.0 - src.rows / 2.0;

    cv::imwrite("tmp/h6_src.bmp", src);
    cv::Mat rotated;
    cv::warpAffine(src, rotated, rot, cv::Size(rotatedWidth, rotatedHeight), cv::INTER_NEAREST, 0, cv::Scalar(255));
    cv::imwrite("tmp/h6_rotated.bmp", rotated);

    //cv::Mat bilateraled;
    //cv::bilateralFilter(rotated, bilateraled, gridSize, Config::Export::a(), Config::Export::b(), cv::BORDER_ISOLATED);
    //cv::imwrite("tmp/h6_bilateral.bmp", bilateraled);
    //rotated = bilateraled;

    cv::Mat canvas;
    cv::cvtColor(rotated, canvas, cv::COLOR_GRAY2BGR);
    cv::imwrite("tmp/h6_canvas1.bmp", canvas);

    cv::Mat dst(rotated.rows, rotated.cols, CV_8UC1, cv::Scalar(0));
    int gridCols = qCeil(dst.cols * 1.0 / gridSize);
    int gridRows = qCeil(dst.rows * 1.0 / gridSize);
    progress->setMaximum(gridCols * gridRows);
    for (int c = 0; c < gridCols; c++)
    {
        qLogD << "col: " << c << "/" << gridCols;
        for (int r = 0; r <= gridRows; r++)
        {
            int x = c * gridSize;
            int y = r * gridSize;
            if (x < 0 || y < 0 || x >= rotated.cols || y >= rotated.rows)
            {
                progress->increaseProgress();
                continue;
            }
            //cv::rectangle(dst, cv::Rect(x - dx / 2, y - dy / 2, dx, dy), cv::Scalar(0));

            QRect qRectSrc(x, y, gridSize, gridSize);
            qRectSrc.setLeft(qMax(0, qRectSrc.left()));
            qRectSrc.setTop(qMax(0, qRectSrc.top()));
            qRectSrc.setRight(qMin(rotated.cols - 1, qRectSrc.right()));
            qRectSrc.setBottom(qMin(rotated.rows - 1, qRectSrc.bottom()));
            if (!qRectSrc.isValid() || qRectSrc.right() < 0 || qRectSrc.left() >= rotated.cols || qRectSrc.bottom() < 0 || qRectSrc.top() >= rotated.rows)
            {
                progress->increaseProgress();
                continue;
            }

            QRect qRectDst(x, y, gridSize, gridSize);
            qRectDst.setLeft(qMax(0, qRectDst.left()));
            qRectDst.setTop(qMax(0, qRectDst.top()));
            qRectDst.setRight(qMin(dst.cols - 1, qRectDst.right()));
            qRectDst.setBottom(qMin(dst.rows - 1, qRectDst.bottom()));
            if (!qRectDst.isValid() || qRectDst.right() < 0 || qRectDst.left() >= dst.cols || qRectDst.bottom() < 0 || qRectSrc.top() >= dst.rows)
            {
                progress->increaseProgress();
                continue;
            }

            cv::Rect rectSrc(qRectSrc.x(), qRectSrc.y(), qRectSrc.width(), qRectSrc.height());
            cv::Rect rectDst(qRectDst.x(), qRectDst.y(), qRectDst.width(), qRectDst.height());

            cv::rectangle(canvas, rectSrc, cv::Scalar(255, 0, 0));

            cv::Mat srcRoi = rotated(rectSrc);
            cv::Mat dstRoi = dst(rectDst);

            //qLogD << r << ", " << c << " src:" << srcRoi.rows << ", " << srcRoi.cols << ", dst:" << dstRoi.rows << ", " << dstRoi.cols;
            //generateGroupingDitcher(srcRoi, dstRoi);
            qreal full = srcRoi.cols * srcRoi.rows * 255.0;
            QPoint center;
            int sum = sumMat(srcRoi, center);

            QPoint drawCenter = center + QPoint(x, y);
            //cv::circle(canvas, cv::Point(drawCenter.x(), drawCenter.y()), 1, cv::Scalar(0, 0, 255));
            canvas.ptr<cv::Vec3b>(drawCenter.y())[drawCenter.x()] = cv::Vec3b(0, 0, 255);
            //qreal def = sum * 1.0 / full * 2;
            //qreal val = 1 / (1 + qPow(M_E, -3 * (def - 1)));
            //qreal def = sum * 1.0 / full;
            //qreal val = 0.5 * qCos(M_PI * def - M_PI) + 0.5;
            //qreal def = sum * 1.0 / full;
            //qreal val = 1.4 * qAsin(2 * def - 1) / M_PI + 0.5;
            //sum = qRound(val * full);

            int initAngle = QRandomGenerator::global()->bounded(4) * 90;
            int rotationAngle = 90/* + 45 * QRandomGenerator::global()->bounded(2)*/;
            int stepAngle = -45;
            if (QRandomGenerator::global()->bounded(2))
            {
                rotationAngle = -rotationAngle;
                stepAngle = -stepAngle;
            }
            sum = qBound(0, sum, static_cast<int>(full));
            generatePattern(dstRoi, sum, center, initAngle, rotationAngle, stepAngle);
            progress->increaseProgress();
        }
    }
    cv::imwrite("tmp/h6_canvas2.bmp", canvas);

    std::vector<int>param; 
    //param.push_back(cv::IMWRITE_PNG_BILEVEL);
    //param.push_back(1);
    //param.push_back(cv::IMWRITE_PNG_COMPRESSION);
    //param.push_back(0);
    cv::imwrite("tmp/h6_dst.bmp", dst);

    cv::Mat outMat(dst.rows, dst.cols, CV_8UC1, cv::Scalar(0));
    center = cv::Point2f((dst.cols - 1) / 2, (dst.rows - 1) / 2);
    rot = cv::getRotationMatrix2D(center, -degrees, 1.);
    bbox = cv::RotatedRect(cv::Point2f(), dst.size(), 0).boundingRect2f();
    cv::Mat antiRotated;
    cv::warpAffine(dst, antiRotated, rot, bbox.size(), cv::INTER_NEAREST, 0, cv::Scalar(255));
    cv::imwrite("tmp/h6_antiRotated.bmp", dst);

	int roix = (antiRotated.cols - src.cols - 1) / 2;
	int roiy = (antiRotated.rows - src.rows - 1) / 2;
	int roiCols = src.cols;
	int roiRows = src.rows;
	if (roix < 0)
	{
		roix = 0;

	}
    cv::Rect roi(roix, roiy, roiCols, roiRows);
    outMat = antiRotated(roi);
    outMat = 255 - outMat;
    cv::imwrite("tmp/h6_outMat.bmp", outMat);

#ifdef _DEBUG
    //cv::imshow("halftone6_processed", outMat);
#endif

    progress->finish();
    return outMat;
}

inline bool operator<(const QPoint& p1, const QPoint& p2)
{
    if (p1.x() != p2.x())
        return p1.x() < p2.x();
    return p1.y() < p2.y();
}

int imageUtils::sumMat(const cv::Mat& mat, QPoint& point)
{
    int sum = 0;
    int sum2 = 0;
    QPointF center = QPointF(0, 0);
    int meanX = mat.cols / 2;
    int meanY = mat.rows / 2;
    qreal sigma = Config::Export::a();
    qreal factor = Config::Export::b();
    qreal fullLength = mat.rows / 2 + mat.rows / 2;
    qreal fullLength2 = fullLength * fullLength;
    qreal gaussianReciprocal = sigma * qSqrt(2 * M_PI);
    qreal gaussianFactor = 1.0 / gaussianReciprocal;
    qreal sigma2 = sigma * sigma;
    //qreal maxLength = qMax(mat.cols, mat.rows);
    for (int i = 0; i < mat.cols; i++)
    {
        for (int j = 0; j < mat.rows; j++)
        {
            int grayScale = mat.ptr<quint8>(j)[i];
            sum += grayScale;
            qreal length = j - meanX + i - meanY;
            qreal gaussian = factor * qExp(-length * length / (2 * fullLength2));
            sum2 += qRound(grayScale * gaussian);
        }
    }
    for (int i = 0; i < mat.cols; i++)
    {
        for (int j = 0; j < mat.rows; j++)
        {
            int grayScale = mat.ptr<quint8>(j)[i];
            if (grayScale)
            {
                center += (static_cast<qreal>(grayScale) / sum) * QPointF(i - meanX, j - meanY);
            }
        }
    }
    point.setX(qRound(center.x() + meanX));
    point.setY(qRound(center.y() + meanY));

    point.setX(qBound(0, point.x(), mat.cols - 1));
    point.setY(qBound(0, point.y(), mat.rows - 1));
    //qLogD << sum << ", " << point;
    sum2 = qBound(0, sum2, mat.cols * mat.rows * 255);
    return sum2;
}

void imageUtils::generateGroupingDitcher(cv::Mat& srcRoi, cv::Mat& dstRoi)
{
    qreal base = srcRoi.cols * srcRoi.rows * 255;
    qreal sum = base - cv::sum(srcRoi)[0];
    qreal sumRatio = qMin(1., sum / base);
    qreal sumFactor = sumRatio * qMax(dstRoi.cols, dstRoi.rows) * qSqrt(2) / 2;
    qreal sigma = (1 - sumRatio) * 0.4 - 0.2 + qSqrt(M_E);
    //qreal gaussianFactor = 1.0 / (sigma * qSqrt(2 * M_PI));
    qreal gaussianFactor = /*(1 - sumRatio) * 1 - 0.5 +*/ qSqrt(M_E) * Config::Export::gaussianFactor();
    int gridSize = qMax(dstRoi.rows, dstRoi.cols);

    //if (qFuzzyCompare(sumFactor, 1))
    //{
    //dstRoi.setTo(cv::Scalar(255));
    //continue;
    //}
    qreal dx = 1;
    qreal dy = srcRoi.cols * 1.0 / srcRoi.rows;

    int sum1 = 0;
    int sum2 = 0;
    int sum3 = 0;
    int sum4 = 0;
    for (int j = 0; j < srcRoi.rows / 2; j++)
    {
        for (int i = 0; i < srcRoi.cols / 2; i++)
        {
            if (Config::Export::imageUseGaussian())
            {
                qreal length = QVector2D(j, i).length();
                qreal gaussian = gaussianFactor * qExp(-length * length / (2 * sumFactor * sumFactor));
                sum1 += qMin(255, qRound((255 - srcRoi.ptr<quint8>(j)[i]) * gaussian));
            }
            else
                sum1 += 255 - srcRoi.ptr<quint8>(j)[i];
        }
    }
    sum1 = qMin(sum1, (srcRoi.rows / 2) * (srcRoi.cols / 2) * 255);
    for (int j = srcRoi.rows / 2; j < srcRoi.rows; j++)
    {
        for (int i = 0; i < srcRoi.cols / 2; i++)
        {
            if (Config::Export::imageUseGaussian())
            {
                qreal length = QVector2D(j, i).length();
                qreal gaussian = gaussianFactor * qExp(-length * length / (2 * sumFactor * sumFactor));
                sum2 += qMin(255, qRound((255 - srcRoi.ptr<quint8>(j)[i]) * gaussian));
            }
            else
                sum2 += 255 - srcRoi.ptr<quint8>(j)[i];
        }
    }
    sum2 = qMin(sum2, (srcRoi.rows - srcRoi.rows / 2) * (srcRoi.cols / 2) * 255);
    for (int j = 0; j < srcRoi.rows / 2; j++)
    {
        for (int i = srcRoi.cols / 2; i < srcRoi.cols; i++)
        {
            if (Config::Export::imageUseGaussian())
            {
                qreal length = QVector2D(j, i).length();
                qreal gaussian = gaussianFactor * qExp(-length * length / (2 * sumFactor * sumFactor));
                sum3 += qMin(255, qRound((255 - srcRoi.ptr<quint8>(j)[i]) * gaussian));
            }
            else
                sum3 += 255 - srcRoi.ptr<quint8>(j)[i];
        }
    }
    sum3 = qMin(sum3, (srcRoi.rows / 2) * (srcRoi.cols - srcRoi.cols / 2) * 255);
    for (int j = srcRoi.rows / 2; j < srcRoi.rows; j++)
    {
        for (int i = srcRoi.cols / 2; i < srcRoi.cols; i++)
        {
            if (Config::Export::imageUseGaussian())
            {
                qreal length = QVector2D(j, i).length();
                qreal gaussian = gaussianFactor * qExp(-length * length / (2 * sumFactor * sumFactor));
                sum4 += qMin(255, qRound((255 - srcRoi.ptr<quint8>(j)[i]) * gaussian));
            }
            else
                sum4 += 255 - srcRoi.ptr<quint8>(j)[i];
        }
    }
    sum4 = qMin(sum4, (srcRoi.rows - srcRoi.rows / 2) * (srcRoi.cols - srcRoi.cols / 2) * 255);

    QPoint pt(dstRoi.cols / 2 - 1, dstRoi.rows / 2 - 1);
    //int total = 0;
    for (int j = pt.y(); j >= 0 && sum1 > 0; j--)
    {
        for (int i = dstRoi.cols / 2 - 1; i > pt.x() && sum1 > 0; i--)
        {
            int gray = dstRoi.ptr<quint8>(pt.y())[i];
            if (gray == 0 && sum1 >= 255)
            {
                dstRoi.ptr<quint8>(pt.y())[i] = 255;
                //total += dstRoi.ptr<quint8>(pt.y())[i];
                sum1 -= 255;
            }
        }
        for (int j = dstRoi.rows / 2 - 1; j >= pt.y() && sum1 > 0; j--)
        {
            int gray = dstRoi.ptr<quint8>(j)[pt.x()];
            if (gray == 0 && sum1 >= 255)
            {
                dstRoi.ptr<quint8>(j)[pt.x()] = 255;
                //total += dstRoi.ptr<quint8>(j)[pt.x()];
                sum1 -= 255;
            }
        }
        pt.setX(qMax(0, qRound(pt.x() - dx)));
        pt.setY(qMax(0, qRound(pt.y() - dy)));
    }

    pt = QPoint(dstRoi.cols / 2 - 1, dstRoi.rows / 2);
    for (int j = pt.y(); j >= 0 && sum2 > 0; j--)
    {
        for (int i = dstRoi.cols / 2 - 1; i > pt.x() && sum2 > 0; i--)
        {
            int gray = dstRoi.ptr<quint8>(pt.y())[i];
            if (gray == 0 && sum2 >= 255)
            {
                dstRoi.ptr<quint8>(pt.y())[i] = 255;
                //total += dstRoi.ptr<quint8>(pt.y())[i];
                sum2 -= 255;
            }
        }
        for (int j = dstRoi.rows / 2; j <= pt.y() && sum2 > 0; j++)
        {
            int gray = dstRoi.ptr<quint8>(j)[pt.x()];
            if (gray == 0 && sum2 >= 255)
            {
                dstRoi.ptr<quint8>(j)[pt.x()] = 255;
                //total += dstRoi.ptr<quint8>(j)[pt.x()];
                sum2 -= 255;
            }
        }
        pt.setX(qMax(0, qRound(pt.x() - dx)));
        pt.setY(qMin(dstRoi.rows - 1, qRound(pt.y() + dy)));
    }

    pt = QPoint(dstRoi.cols / 2, dstRoi.rows / 2 - 1);
    for (int j = pt.y(); j >= 0 && sum3 > 0; j--)
    {
        for (int i = dstRoi.cols / 2; i < pt.x() && sum3 > 0; i++)
        {
            int gray = dstRoi.ptr<quint8>(pt.y())[i];
            if (gray == 0 && sum3 >= 255)
            {
                dstRoi.ptr<quint8>(pt.y())[i] = 255;
                //total += dstRoi.ptr<quint8>(pt.y())[i];
                sum3 -= 255;
            }
        }
        for (int j = dstRoi.rows / 2 - 1; j >= pt.y() && sum3 > 0; j--)
        {
            int gray = dstRoi.ptr<quint8>(j)[pt.x()];
            if (gray == 0 && sum3 >= 255)
            {
                dstRoi.ptr<quint8>(j)[pt.x()] = 255;
                //total += dstRoi.ptr<quint8>(j)[pt.x()];
                sum3 -= 255;
            }
        }
        pt.setX(qMin(dstRoi.cols - 1, qRound(pt.x() + dx)));
        pt.setY(qMax(0, qRound(pt.y() - dy)));
    }

    pt = QPoint(dstRoi.cols / 2, dstRoi.rows / 2);
    for (int j = pt.y(); j >= 0 && sum4 > 0; j--)
    {
        for (int i = dstRoi.cols / 2; i < pt.x() && sum4 > 0; i++)
        {
            int gray = dstRoi.ptr<quint8>(pt.y())[i];
            if (gray == 0 && sum4 >= 255)
            {
                dstRoi.ptr<quint8>(pt.y())[i] = 255;
                //total += dstRoi.ptr<quint8>(pt.y())[i];
                sum4 -= 255;
            }
        }
        for (int j = dstRoi.rows / 2; j <= pt.y() && sum4 > 0; j++)
        {
            int gray = dstRoi.ptr<quint8>(j)[pt.x()];
            if (gray == 0 && sum4 >= 255)
            {
                dstRoi.ptr<quint8>(j)[pt.x()] = 255;
                //total += dstRoi.ptr<quint8>(j)[pt.x()];
                sum4 -= 255;
            }
        }
        pt.setX(qMin(dstRoi.cols - 1, qRound(pt.x() + dx)));
        pt.setY(qMin(dstRoi.rows - 1, qRound(pt.y() + dy)));
    }

    int remains = sum1 + sum2 + sum3 + sum4;
    int count = remains / 255;
    if (count)
    {
        QMap<QPoint, int> candidates;
        bool odd = gridSize % 2 != 0;
        int start = odd ? 0 : 1;
        int factor = 2;
        for (int n = 0; n < gridSize / 2; n++)
        {
            int upper = odd ? n + 1 : n;
            for (int i = -1 * n; i < upper; i++)
            {
                int x = gridSize / 2 + i;
                int y1 = gridSize / 2 - n * i;
                int y2 = gridSize / 2 + n * i;

                if (x >= 0 && y1 >= 0 && y1 < dstRoi.rows && x < dstRoi.cols)
                {
                    int px1 = dstRoi.ptr<quint8>(y1)[x];
                    if (!px1)
                        candidates.insert(QPoint(x, y1), (gridSize - n) * factor);
                }

                if (x >= 0 && y2 >= 0 && y2 < dstRoi.rows && x < dstRoi.cols)
                {

                    int px2 = dstRoi.ptr<quint8>(y2)[x];
                    if (!px2)
                        candidates.insert(QPoint(x, y2), (gridSize - n) * factor);
                }
            }
            for (int j = -1 * n; j < upper; j++)
            {
                int x1 = gridSize / 2 - n * j;
                int x2 = gridSize / 2 + n * j;
                int y = gridSize / 2 + j;

                if (x1 >= 0 && y >= 0 && y < dstRoi.rows && x1 < dstRoi.cols)
                {
                    int px1 = dstRoi.ptr<quint8>(y)[x1];
                    if (!px1)
                        candidates.insert(QPoint(x1, y), (gridSize - n) * factor);
                }

                if (x2 >= 0 && y >= 0 && y < dstRoi.rows && x2 < dstRoi.cols)
                {
                    int px2 = dstRoi.ptr<quint8>(y)[x2];
                    if (!px2)
                        candidates.insert(QPoint(x2, y), (gridSize - n) * factor);
                }
            }
            if (candidates.count() >= count)
                break;
        }
        int total = 0;
        for (QMap<QPoint, int>::Iterator i = candidates.begin(); i != candidates.end(); i++)
        {
            total += i.value();
        }
        int total2 = 0;
        for (QMap<QPoint, int>::Iterator i = candidates.begin(); i != candidates.end(); i++)
        {
            int number = QRandomGenerator::global()->bounded(total);
            if (number >= total2 && number < i.value() && count)
            {
                QPoint pt = i.key();
                dstRoi.ptr<quint8>(pt.y())[pt.x()] = 255;
                count--;
            }
            total2 += i.value();
        }
    }
}

void imageUtils::generatePattern(cv::Mat& dstRoi, int sum, QPoint& center, int initAngle, int rotationAngle, int stepAngle)
{
    /*int x = dstRoi.cols / 2;
    int y = dstRoi.rows / 2;
    if (dstRoi.cols > 2)
        x -= QRandomGenerator::global()->bounded(2);
    if (dstRoi.rows > 2)
        y -= QRandomGenerator::global()->bounded(2);*/

    dstRoi.setTo(cv::Scalar(0));

    QTransform initTrans;
    initTrans.rotate(initAngle);

    QPoint pt = center;
    QPoint vec(1, 0);
    vec = initTrans.map(vec);

    int count = 0;
    while (sum > 0)
    {
        // 填充颜色并递减总颜色值
        int grayScale = sum >= 255 ? 255 : 255 - sum;
        //int grayScale = sum >= 255 ? 0 : sum;
        dstRoi.ptr<quint8>(pt.y())[pt.x()] = grayScale;
        //qLogD << ++count << ": " << pt;
        sum -= 255;

        if (sum <= 0)
            break;

        bool isAvailable = false;
        while (!isAvailable)
        {
            int angle = rotationAngle;
            // 当旋转角度还没有变号时循环
            int factor = angle * rotationAngle;
            while (/*!utils::fuzzyEquals(angle, 0, 0.1) && */factor >= 0)
            {
                QTransform t;
                t.rotate(angle);
                QPoint candidateVec = t.map(vec);
                QPoint candidate = pt + candidateVec;
                if (candidate.x() < 0 || candidate.y() < 0 || candidate.x() >= dstRoi.cols || candidate.y() >= dstRoi.rows)
                {
                    // 如果超出范围
                    pt = candidate;
                    vec = candidateVec;
                    break;
                }
                else if (!dstRoi.ptr<quint8>(candidate.y())[candidate.x()])
                {
                    // 如果该位置有效
                    isAvailable = true;
                    pt = candidate;
                    vec = candidateVec;
                    break;
                }

                // 该位置已经被占据
                angle += stepAngle;
                factor = angle * rotationAngle;
            }
        }
    }
}

cv::Mat imageUtils::floydSteinberg(cv::Mat src, float mmWidth, float mmHeight, float lpi, float dpi)
{
    float inchWidth = mmWidth * MM_TO_INCH;
    float inchHeight = mmHeight * MM_TO_INCH;

    float outHPixels = inchWidth * dpi;
    float outVPixels = inchHeight * dpi;

    int dotsHCount = std::round(inchWidth * lpi);
    int dotsVCount = std::round(inchHeight * lpi);

    float dotsPerPixelH = std::round(dotsHCount * 1.0 / src.cols);
    float dotsPerPixelV = std::round(dotsVCount * 1.0 / src.rows);

    float dotWidth = std::round(outHPixels / dotsHCount);
    float dotHeight = std::round(outVPixels / dotsVCount);

    int ditchSize = std::round(dotWidth * 1.);

    qDebug().noquote().nospace() << "          lpi: " << lpi;
    qDebug().noquote().nospace() << "          dpi: " << dpi;
    qDebug().noquote().nospace() << "     src cols: " << src.cols;
    qDebug().noquote().nospace() << "     src rows: " << src.rows;
    qDebug().noquote().nospace() << "      mmWidth: " << mmWidth;
    qDebug().noquote().nospace() << "     mmHeight: " << mmHeight;
    qDebug().noquote().nospace() << "    inchWidth: " << inchWidth;
    qDebug().noquote().nospace() << "   inchHeight: " << inchHeight;
    qDebug().noquote().nospace() << "   outHPixels: " << outHPixels;
    qDebug().noquote().nospace() << "   outVPixels: " << outVPixels;
    qDebug().noquote().nospace() << "   dotsHCount: " << dotsHCount;
    qDebug().noquote().nospace() << "   dotsVCount: " << dotsVCount;
    qDebug().noquote().nospace() << "dotsPerPixelH: " << dotsPerPixelH;
    qDebug().noquote().nospace() << "dotsPerPixelV: " << dotsPerPixelV;
    qDebug().noquote().nospace() << "     dotWidth: " << dotWidth;
    qDebug().noquote().nospace() << "    dotHeight: " << dotHeight;
    qDebug().noquote().nospace() << "    ditchSize: " << ditchSize;

    cv::Mat outMat(outVPixels, outHPixels, CV_8UC1, cv::Scalar(255));
    cv::Mat dotsMat(dotsVCount, dotsHCount, CV_32S, cv::Scalar(255));
    cv::Mat dotsMatBin(dotsVCount, dotsHCount, CV_8UC1, cv::Scalar(255));
    //cv::Mat ditchMat = generateDitchMatRec(ditchSize);
    //cv::Mat ditchMat = generateBayerDitchMatRec(std::ceil(std::sqrt(ditchSize)) + 1);

    for (int r = 0; r < src.rows; r++)
    {
        for (int c = 0; c < src.cols; c++)
        {
            int x = std::floor(c * 1.0 * (dotsHCount - 1) / (src.cols - 1));
            int y = std::floor(r * 1.0 * (dotsVCount - 1) / (src.rows - 1));

            dotsMat.ptr<float>(y)[x] = src.ptr<uchar>(r)[c];
        }
    }
    for (int r = 0; r < dotsVCount; r++)
    {
        for (int c = 0; c < dotsHCount; c++)
        {
            int x = std::floor(c * 1.0 * (src.cols - 1) / (dotsHCount - 1));
            int y = std::floor(r * 1.0 * (src.rows - 1) / (dotsVCount - 1));

            dotsMat.ptr<float>(r)[c] = src.ptr<uchar>(y)[x];
        }
    }
    cv::imwrite("dotsMat.tiff", dotsMat);

    float threshold = 64.5f;

    float coa = 7.f / 16;
    float cob = 3.f / 16;
    float coc = 5.f / 16;
    float cod = 1.f / 16;

    for (int r = 0; r < dotsVCount; r++)
    {
        for (int c = 0; c < dotsHCount; c++)
        {
            float pixel = dotsMat.ptr<float>(r)[c];
            uchar target = 255;
            if (pixel > threshold)
            {
                target = 255;
            }
            else
            {
                target = 0;
            }
            
            if (pixel < threshold)
            {
                cv::Point start(c * dotWidth + dotWidth / 2, r * dotWidth + dotWidth / 2);
                cv::circle(outMat, start, dotWidth / 2, cv::Scalar(0), cv::FILLED);
                /*int gray = std::round((255 - pixel) / 255 * (ditchMat.rows * ditchMat.cols));
                cv::Point start(c * dotWidth, r * dotWidth);
                for (int x = 0; x < ditchMat.cols; x++)
                {
                    for (int y = 0; y < ditchMat.rows; y++)
                    {
                        if (gray > ditchMat.ptr<uchar>(y)[x])
                        {
                            if (start.y + y < outMat.rows && start.x + x < outMat.cols)
                                outMat.ptr<uchar>(start.y + y)[start.x + x] = 0;
                        }
                    }
                }*/
            }
            dotsMatBin.ptr<uchar>(r)[c] = target;

            float error = pixel - target;
            //std::cout << "[" << pixel << "\t" << error << "]" << std::endl;

            if (c < dotsMat.cols - 1)
                dotsMat.ptr<float>(r)[c + 1] += error * coa;
            if (r < dotsMat.rows - 1)
            {
                if (c > 0)
                    dotsMat.ptr<float>(r + 1)[c - 1] += error * cob;
                dotsMat.ptr<float>(r + 1)[c] += error * coc;
                if (c < dotsMat.cols - 1)
                    dotsMat.ptr<float>(r + 1)[c + 1] += error * cod;
            }
        }
    }
    cv::imwrite("dotsMat2.tiff", dotsMat);
    cv::imwrite("dotsMatBin.png", dotsMatBin);
    cv::imwrite("outMat.bmp", outMat);
    return cv::Mat();
}

cv::Mat imageUtils::generateSpiralPattern(int gridSize, int& grades, float degrees)
{
    int ditchSize = gridSize / std::sqrt(2);
    grades = ditchSize * ditchSize;
    cv::Mat spiralMat = generateSpiralDitchMatRec(ditchSize);
    std::cout << "spiral" << std::endl;
    std::cout << spiralMat << std::endl;
    cv::imwrite("spiral.bmp", spiralMat);
    cv::Mat dst = generateRotatedPattern(spiralMat, gridSize, degrees);
    return dst;
}

cv::Mat imageUtils::generateSpiralDitchMatRec(int circleNum)
{
    if (circleNum == 1)
    {
        cv::Mat mat(1, 1, CV_8UC1, cv::Scalar(0));
        return mat;
    }
    else
    {
        cv::Mat subMat = generateSpiralDitchMatRec(circleNum - 1);
        cv::Mat mat(circleNum, circleNum, CV_8UC1, cv::Scalar(0));
        bool odd = circleNum % 2 == 1;
        cv::Rect roiRect;
        if (odd)
        {
            roiRect = cv::Rect(1, 1, circleNum - 1, circleNum - 1);
        }
        else
        {
            roiRect = cv::Rect(0, 0, circleNum - 1, circleNum - 1);
        }
        cv::Mat roi = mat(roiRect);
        subMat.copyTo(roi);

        int min = (circleNum - 1) * (circleNum - 1);
        int max = circleNum * circleNum - 1;
        int mid = (min + max) / 2;
        for (int i = min; i <= max; i++)
        {
            if (i < mid)
            {
                if (odd)
                {
                    mat.ptr<uchar>(circleNum - (i - min) - 1)[0] = i;
                }
                else
                {
                    mat.ptr<uchar>(i - min)[circleNum - 1] = i;
                }
            }
            else
            {
                if (odd)
                {
                    mat.ptr<uchar>(0)[i - mid] = i;
                }
                else
                {
                    mat.ptr<uchar>(circleNum - 1)[circleNum - 1 - (i - mid)] = i;
                }
            }
        }
        return mat;
    }
}

cv::Mat imageUtils::generateBayerPattern(int gridSize, int& grades, float degrees)
{
    int ditchSize = gridSize / std::sqrt(2);
    grades = ditchSize * ditchSize;
    cv::Mat mat = generateBayerDitchMatRec(gridSize);
    cv::Mat dst = generateRotatedPattern(mat, degrees);
    return dst;
}

cv::Mat imageUtils::generateBayerDitchMatRec(int circleNum)
{
    cv::Mat mat;
    int size = std::pow(2, (circleNum - 1));
    mat = cv::Mat(size, size, CV_8UC1, cv::Scalar(0));
    
    if (circleNum > 1)
    {
        cv::Mat M = generateBayerDitchMatRec(circleNum - 1);
        cv::Mat U(M.rows, M.cols, CV_8UC1, cv::Scalar(1));

        cv::Mat roi = mat(cv::Rect(0, 0, M.rows, M.cols));
        cv::Mat subMat = M * 4;
        subMat.copyTo(roi);

        roi = mat(cv::Rect(M.cols, 0, M.rows, M.cols));
        subMat = M * 4 + U * 2;
        subMat.copyTo(roi);

        roi = mat(cv::Rect(0, M.rows, M.rows, M.cols));
        subMat = M * 4 + U * 3;
        subMat.copyTo(roi);

        roi = mat(cv::Rect(M.cols, M.rows, M.rows, M.cols));
        subMat = M * 4 + U;
        subMat.copyTo(roi);
    }
    return mat;
}

cv::Mat imageUtils::generateCircleMat(int gridSize, int& grades, float degrees)
{
    int ditchSize = gridSize / std::sqrt(2);
    grades = ditchSize / 2;
    cv::Mat mat(ditchSize, ditchSize, CV_8UC1, cv::Scalar(255));
    cv::Point2f center((ditchSize - 1) / 2., (ditchSize - 1) / 2.);
    for (int r = 0; r < ditchSize; r++)
    {
        for (int c = 0; c < ditchSize; c++)
        {
            cv::Point2f pt(c, r);
            int length = std::floor(cv::norm(pt - center));
            if (length < ditchSize / 2)
                mat.ptr<uchar>(r)[c] = std::round(ditchSize / 2 * std::sin(length * 2. / ditchSize));
            else
                mat.ptr<uchar>(r)[c] = ditchSize / 2 + 1;

        }
    }
    cv::imwrite("circle.png", mat);
    cv::Mat dst = generateRotatedPattern(mat, gridSize, degrees);
    return dst;
}

cv::Mat imageUtils::generateRotatedPattern(cv::Mat src, int gridSize, float degrees)
{
    cv::Mat src32F;
    src.convertTo(src32F, CV_32F);
    cv::Mat arranged(src32F.cols * 3, src32F.rows * 3, CV_32F, cv::Scalar(255));
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            cv::Rect rect = cv::Rect(i * src32F.cols, j * src32F.rows, src32F.cols, src32F.rows);
            cv::Mat roi = arranged(rect);
            src32F.copyTo(roi);
        }
    }
    //std::cout << arranged << std::endl;
    cv::imwrite("arranged.bmp", arranged);

    cv::Point2f center((arranged.cols - 1) / 2.f, (arranged.rows - 1) / 2.f);
    cv::Mat rot = cv::getRotationMatrix2D(center, degrees, 1.);
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), arranged.size(), degrees).boundingRect2f();
    //cv::Rect2f bbox(0, 0, mat.cols * 2, mat.rows * 2);

    // adjust transformation matrix
    rot.at<double>(0, 2) += bbox.width / 2.0 - arranged.cols / 2.0;
    rot.at<double>(1, 2) += bbox.height / 2.0 - arranged.rows / 2.0;

    cv::Mat rotated;
    cv::warpAffine(arranged, rotated, rot, bbox.size(), cv::INTER_AREA, 0, cv::Scalar(255));
    std::cout << "rotated" << std::endl;
    std::cout << rotated << std::endl;
    cv::imwrite("rot.bmp", rotated);

    double d = src.cols * sqrt(2) * 2;
    cv::Mat roi = rotated(cv::Rect2d((rotated.cols - d) / 2, (rotated.cols - d) / 2, d, d));
    cv::Mat pattern;
    roi.copyTo(pattern);
    for (int r = 0; r < pattern.rows; r++)
    {
        for (int c = 0; c < pattern.cols; c++)
        {
            pattern.ptr<float>(r)[c] = std::abs(pattern.ptr<float>(r)[c]);
        }
    }
    cv::imwrite("pattern.bmp", pattern);

    std::cout << "pattern: " << std::endl;
    std::cout << pattern << std::endl;
    return pattern;
}

cv::Mat imageUtils::generateRotatedPattern2(cv::Mat src, int gridSize, float degrees)
{
    cv::Mat rotated = src;
    std::cout << "rotated:" << std::endl;
    cv::imwrite("rotated.bmp", rotated);
    std::cout << rotated << std::endl;
    cv::Mat mask;
    rotated.convertTo(mask, CV_8UC1);
    rotated.convertTo(rotated, CV_8UC1);
    cv::threshold(mask, mask, 254, 255, cv::THRESH_BINARY_INV);
    std::cout << "mask:" << std::endl;
    std::cout << mask << std::endl;

    float x = gridSize * std::cos(degrees * M_PI / 180);
    float y = gridSize * std::sin(degrees * M_PI / 180);

    int ditchSize = std::round(rotated.cols * sqrt(2));
    cv::Mat pattern(ditchSize, ditchSize, CV_8UC1, cv::Scalar(0));
    cv::Mat arranged(rotated.rows * 3, rotated.cols * 3, CV_8UC1, cv::Scalar(255));

    cv::Point2f center = cv::Point2f((arranged.cols - 1) / 2.f, (arranged.rows - 1) / 2.f);
    {
        cv::Point2f p1(x, y);
        p1 += center;
        cv::Rect roiRect(p1.x - rotated.cols / 2, p1.y - rotated.rows / 2, rotated.cols, rotated.rows);
        cv::Mat roi = arranged(roiRect);
        rotated.copyTo(roi, mask);
        std::cout << p1 << std::endl;
        std::cout << "arranged1:" << std::endl;
        std::cout << arranged << std::endl;
    }
    {
        cv::Point2f p1(-x, y);
        p1 += center;
        cv::Rect roiRect(p1.x - rotated.cols / 2, p1.y - rotated.rows / 2, rotated.cols, rotated.rows);
        cv::Mat roi = arranged(roiRect);
        rotated.copyTo(roi, mask);
        std::cout << p1 << std::endl;
        std::cout << "arranged2:" << std::endl;
        std::cout << arranged << std::endl;
    }
    {
        cv::Point2f p1(x, -y);
        p1 += center;
        cv::Rect roiRect(p1.x - rotated.cols / 2, p1.y - rotated.rows / 2, rotated.cols, rotated.rows);
        cv::Mat roi = arranged(roiRect);
        rotated.copyTo(roi, mask);
        std::cout << p1 << std::endl;
        std::cout << "arranged3:" << std::endl;
        std::cout << arranged << std::endl;
    }
    {
        cv::Point2f p1(-x, -y);
        p1 += center;
        cv::Rect roiRect(p1.x - rotated.cols / 2, p1.y - rotated.rows / 2, rotated.cols, rotated.rows);
        cv::Mat roi = arranged(roiRect);
        rotated.copyTo(roi, mask);
        std::cout << p1 << std::endl;
        std::cout << "arranged4:" << std::endl;
        std::cout << arranged << std::endl;
    }
    {
        cv::Point2f p1(0, 0);
        p1 += center;
        cv::Rect roiRect(p1.x - rotated.cols / 2, p1.y - rotated.rows / 2, rotated.cols, rotated.rows);
        cv::Mat roi = arranged(roiRect);
        rotated.copyTo(roi, mask);
        std::cout << p1 << std::endl;
        std::cout << "arranged5:" << std::endl;
        std::cout << arranged << std::endl;
    }
    {
        cv::Point2f p1(0, 0);
        p1 += center;
        cv::Rect roiRect(p1.x - ditchSize / 2, p1.y - ditchSize / 2, ditchSize, ditchSize);
        cv::Mat roi = arranged(roiRect);

        roi.copyTo(pattern);
        pattern.convertTo(pattern, CV_32F);
        cv::imwrite("pattern.bmp", pattern);
        std::cout << "pattern: " << std::endl;
        std::cout << pattern << std::endl;
    }
    std::cout << "arranged:" << std::endl;
    std::cout << arranged << std::endl;
    cv::imwrite("arranged.bmp", arranged);

    return pattern;
}

cv::Mat imageUtils::generateRotatedPattern45(cv::Mat src)
{
    cv::Mat pattern(src.rows * 2, src.cols * 2, CV_8U, cv::Scalar(255));

    cv::Mat ucharMat;
    src.convertTo(ucharMat, CV_8UC1);
    std::cout << ucharMat << std::endl;
    
    int middle = (src.cols - 1) / 2;

    cv::Mat roi = pattern(cv::Rect(src.cols / 2, src.rows / 2, src.cols, src.rows));
    ucharMat.copyTo(roi);
    std::cout << std::endl << pattern << std::endl;

    roi = pattern(cv::Rect(0, 0, src.cols / 2, src.rows / 2));
    cv::Mat srcRoi = ucharMat(cv::Rect(src.cols / 2, src.rows / 2, src.cols / 2, src.rows / 2));
    srcRoi.copyTo(roi);
    std::cout << std::endl << pattern << std::endl;

    roi = pattern(cv::Rect(src.cols + src.cols / 2, src.rows + src.rows / 2, src.cols / 2, src.rows / 2));
    srcRoi = ucharMat(cv::Rect(0, 0, src.cols / 2, src.rows / 2));
    srcRoi.copyTo(roi);
    std::cout << std::endl << pattern << std::endl;

    roi = pattern(cv::Rect(0, src.rows + src.rows / 2, src.cols / 2, src.rows / 2));
    srcRoi = ucharMat(cv::Rect(src.cols / 2, 0, src.cols / 2, src.rows / 2));
    srcRoi.copyTo(roi);
    std::cout << std::endl << pattern << std::endl;

    roi = pattern(cv::Rect(src.cols + src.cols / 2, 0, src.cols / 2, src.rows / 2));
    srcRoi = ucharMat(cv::Rect(0, src.rows / 2, src.cols / 2, src.rows / 2));
    srcRoi.copyTo(roi);
    std::cout << std::endl << pattern << std::endl;

    double length = 0.0;
    int max = src.cols * src.rows * 3 - 1;
    int it = src.cols * src.rows;
    double degreesStep = M_PI / 90;
    cv::Point2f center((pattern.cols - 1) / 2.0, (pattern.rows - 1) / 2.0);
    while (it <= max)
    {
        for (double degrees = 0; degrees < M_PI * 2; degrees += degreesStep)
        {
            double fx = length * std::cos(degrees) + center.x;
            double fy = length * std::sin(degrees) + center.y;
            int x = std::round(fx);
            int y = std::round(fy);

            if (x >= 0 && x < pattern.cols && y >= 0 && y < pattern.rows)
            {
                int v = pattern.ptr<uchar>(y)[x];
               
                if (v == 255)
                {
                    pattern.ptr<uchar>(y)[x] = it;
                    it++;
                }
            }
        }
        
        length += 0.2;
        if (length > sqrt(2) * pattern.cols)
            break;
    }
    std::cout << std::endl << pattern << std::endl;

    pattern.convertTo(pattern, CV_32F);
    return pattern;
}

cv::Mat imageUtils::generateRoundSpiralPattern(int gridSize, int& grades, float degrees)
{
    //int ditchSize = gridSize / std::sqrt(2);
    grades = gridSize * gridSize;
    cv::Mat spiralMat = generateRoundSpiralMat(gridSize);
    std::cout << "round spiral mat:" << std::endl;
    std::cout << spiralMat << std::endl;
    cv::imwrite("round_spiral.bmp", spiralMat);
    cv::Mat dst = generateRotatedPattern2(spiralMat, gridSize, degrees);
    return dst;
}

cv::Mat imageUtils::generateRoundSpiralMat(int gridSize)
{
    cv::Mat dst(gridSize, gridSize, CV_32S, cv::Scalar(-1));

    //double degreesStep = std::atan(2.0 / gridSize);
    double degreesStep = M_PI / 90;
    int count = gridSize * gridSize;
    int max = count;
    cv::Point2f center((gridSize - 1) / 2.0, (gridSize - 1) / 2.0);

    qDebug() << "degreesStep:" << degreesStep;
    qDebug() << "        max:" << max;
    qDebug() << "     center:" << center.x << center.y;

    double length = 0.0;
    while (count)
    {
        for (double degrees = 0; degrees < M_PI * 2; degrees += degreesStep)
        {
            double fx = length * std::cos(degrees) + center.x;
            double fy = length * std::sin(degrees) + center.y;
            //qDebug() << fx << fy;
            int x = std::round(fx);
            int y = std::round(fy);

            if (x >= 0 && x < gridSize && y >= 0 && y < gridSize)
            {
                int v = dst.ptr<int>(y)[x];
               
                if (v < 0)
                {
                    dst.ptr<int>(y)[x] = max - count;
                    count--;
                }
            }
        }
        
        length += 0.2;
    }
    return dst;
}

cv::Mat imageUtils::rotateMat(cv::Mat src, float degrees)
{
    //cv::Point2f center((src.cols - 1) / 2, (src.rows - 1) / 2);
    double cosValue = std::cos(degrees * M_PI / 180);
    double sinValue = std::sin(degrees * M_PI / 180);

    //int c = src.cols;
    //int a = std::round(c * cosValue);
    //int b = std::round(c * sinValue);
    double c = src.cols;
    double a = c * cosValue;
    double b = c * sinValue;

    qDebug() << a << b << c;
    
    //double i0 = 0.5;
    //double j0 = 0.5;
    //double x0 = 0.5;
    //double y0 = 0.5;
    //int i0 = 0;
    //int j0 = 0;
    //int x0 = 0;
    //int y0 = 0;

    QMap<QPair<int, int>, bool> coords;
    for (int ri = 0; ri < src.rows; ri++)
    {
        for(int ci = 0; ci < src.cols; ci++)
        {
            int i0 = ci;
            int j0 = ri;
            int x0 = std::round(ci * cosValue - ri * sinValue);
            int y0 = std::round(ci * sinValue + ri * cosValue);
            double x = (ci + 0.5 - i0) * a / c - (ri + 0.5 - j0) * b / c;
            double y = (ci + 0.5 - i0) * b / c + (ri + 0.5 - j0) * a / c;
            int ix = std::round(x + x0);
            int iy = std::round(y + y0);
            QPair<int, int> pair(ix, iy);
            bool exists = coords.contains(pair);
            std::cout << "[" << ci << ", " << ri << "] --> [" << ix << ", " << iy << "] " << exists << " [" << x << ", " << y << "]" << std::endl;
            coords.insert(pair, true);
        }
    }
    return cv::Mat();
}

QByteArray imageUtils::image2EngravingData(ProgressItem* progress, cv::Mat mat, qreal x, qreal y, qreal rowInterval, qreal width)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::ReadWrite);
    stream.setByteOrder(QDataStream::LittleEndian);
    int xStart = unitUtils::mm2MicroMM(x);
    int xEnd = unitUtils::mm2MicroMM(x + width);
    FillStyleAndPixelsCount fspc;
    fspc.setCount(mat.cols);
    fspc.setSame(false);
    bool forward = true;
    progress->setMaximum(mat.rows);
    for (int r = 0; r < mat.rows; r++)
    {
        int yStart = unitUtils::mm2MicroMM(y + r * rowInterval);
        int bitCount = 0;
        quint8 byte = 0;

        quint8 binCheck = 0;
        QList<quint8> rowBytes;
        //QString rowString;
        for (int c = 0; c < mat.cols; c++)
        {
            quint8 pixel = forward ? mat.ptr<quint8>(r)[c] : mat.ptr<quint8>(r)[mat.cols - c - 1];
            quint8 bin = pixel >= 255 ? 0 : 1;
            //rowString.append(QString::number(bin));
            binCheck |= bin;
            byte = byte << 1;
            byte |= bin;
            bitCount++;
            if (bitCount == 8)
            {
                rowBytes.append(byte);
                //stream << byte;
                bitCount = 0;
                byte = 0;
            }
        }
        //rowString.append("\n\r");
        if (mat.cols % 8 != 0)
            rowBytes.append(byte);
            //stream << byte;

        if (binCheck)
        {
            if (forward)
                stream << yStart << xStart << xEnd << fspc.code;
            else
                stream << yStart << xEnd << xStart << fspc.code;

            for (int i = 0; i < rowBytes.length(); i++)
            {
                stream << rowBytes.at(i);
            }
            forward = !forward;
        }

        progress->increaseProgress();
    }
    progress->finish();
    return bytes;
}

/*!
    Returns the closest element (position) in \a sourcePath to \a target,
    using \l{QPoint::manhattanLength()} to determine the distances.
*/
QPointF imageUtils::closestPointTo(const QPointF& target, const QPainterPath& sourcePath)
{
    Q_ASSERT(!sourcePath.isEmpty());
    QPointF shortestDistance = sourcePath.elementAt(0) - target;
    qreal shortestLength = shortestDistance.manhattanLength();
    for (int i = 1; i < sourcePath.elementCount(); ++i) {
        const QPointF distance(sourcePath.elementAt(i) - target);
        const qreal length = distance.manhattanLength();
        if (length < shortestLength) {
            shortestDistance = sourcePath.elementAt(i);
            shortestLength = length;
        }
    }
    return shortestDistance;
}

/*!
    Returns \c true if \a projectilePath intersects with any items in \a scene,
    setting \a hitPos to the position of the intersection.
*/
bool imageUtils::hit(const QLineF& ray, const QPainterPath& target, QPointF& hitPos)
{
    // QLineF has normalVector(), which is useful for extending our path to a rectangle.
    // The path needs to be a rectangle, as QPainterPath::intersected() only accounts
    // for intersections between fill areas, which ray doesn't have.
    // Extend the first point in the path out by 1 pixel.
    QLineF startEdge = ray.normalVector();
    startEdge.setLength(1);

    // Swap the points in the line so the normal vector is at the other end of the line.
    QLineF revertLine(ray.p2(), ray.p1());
    QLineF endEdge = revertLine.normalVector();

    // The end point is currently pointing the wrong way; move it to face the same
    // direction as startEdge.
    endEdge.setLength(-1);

    // Now we can create a rectangle from our edges.
    QPainterPath rectPath(startEdge.p1());
    rectPath.lineTo(startEdge.p2());
    rectPath.lineTo(endEdge.p2());
    rectPath.lineTo(endEdge.p1());
    rectPath.lineTo(startEdge.p1());

    // The hit position will be the element (point) of the rectangle that is the
    // closest to where the projectile was fired from.
    QPainterPath intersection = target.intersected(rectPath);
    if (intersection.isEmpty())
        return false;

    hitPos = closestPointTo(ray.p1(), intersection);
    return true;
}

