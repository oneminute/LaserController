#include "ImageUtils.h"

#include <iostream>

#include <QDebug>
#include <QFile>
#include <QtMath>
#include <QMap>
#include <QPainterPath>

#include "common/common.h"
#include "common/Config.h"
#include "laser/LaserDriver.h"
#include "UnitUtils.h"

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

cv::Mat imageUtils::halftone4(cv::Mat src, float degrees, int gridSize)
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

    // 生成网格
    cv::Mat dst(src.size(), CV_8UC1, cv::Scalar(255));
    for (int c = -margin; c <= margin; c++)
    {
        for (int r = -margin; r <= margin; r++)
        {
            QVector2D v = v1 * c + v2 * r + center;
            int x = qRound(v.x());
            int y = qRound(v.y());
            if (x < 0 || y < 0 || x >= src.cols || y >= src.rows)
                continue;
            cv::rectangle(dst, cv::Rect(x - dx / 2, y - dy / 2, dx, dy), cv::Scalar(0));
        }
    }
    cv::imwrite("tmp/dst.bmp", dst);

    // [TODO:] 根据生成策略决定是否给原图片描边

    // 迭代网格

    // 在每个网格内通过径向向量，进行像素吸附
    return cv::Mat();
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

                generateGroupingDitcher(srcRoi, dstRoi);
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

                generateGroupingDitcher(srcRoi, dstRoi);
            }
        }
    }
    dst = 255 - dst;
    cv::imwrite("tmp/dst.bmp", dst);

    return dst;
}

void imageUtils::generateGroupingDitcher(cv::Mat& srcRoi, cv::Mat& dstRoi)
{
    qreal base = srcRoi.cols * srcRoi.rows * 255;
    qreal sum = base - cv::sum(srcRoi)[0];
    qreal sumRatio = qMin(1., sum / base);
    qreal sumFactor = sumRatio * qMax(srcRoi.cols, srcRoi.rows) * qSqrt(2) / 2;
    qreal sigma = 1 - sumRatio;

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
                qreal gaussian = qExp(-length * length / (2 * sumFactor * sumFactor));
                sum1 += qRound((255 - srcRoi.ptr<quint8>(j)[i]) * gaussian);
            }
            else
                sum1 += 255 - srcRoi.ptr<quint8>(j)[i];
        }
    }
    for (int j = srcRoi.rows / 2; j < srcRoi.rows; j++)
    {
        for (int i = 0; i < srcRoi.cols / 2; i++)
        {
            if (Config::Export::imageUseGaussian())
            {
                qreal length = QVector2D(j, i).length();
                qreal gaussian = qExp(-length * length / (2 * sumFactor * sumFactor));
                sum2 += qRound((255 - srcRoi.ptr<quint8>(j)[i]) * gaussian);
            }
            else
                sum2 += 255 - srcRoi.ptr<quint8>(j)[i];
        }
    }
    for (int j = 0; j < srcRoi.rows / 2; j++)
    {
        for (int i = srcRoi.cols / 2; i < srcRoi.cols; i++)
        {
            if (Config::Export::imageUseGaussian())
            {
                qreal length = QVector2D(j, i).length();
                qreal gaussian = qExp(-length * length / (2 * sumFactor * sumFactor));
                sum3 += qRound((255 - srcRoi.ptr<quint8>(j)[i]) * gaussian);
            }
            else
                sum3 += 255 - srcRoi.ptr<quint8>(j)[i];
        }
    }
    for (int j = srcRoi.rows / 2; j < srcRoi.rows; j++)
    {
        for (int i = srcRoi.cols / 2; i < srcRoi.cols; i++)
        {
            if (Config::Export::imageUseGaussian())
            {
                qreal length = QVector2D(j, i).length();
                qreal gaussian = qExp(-length * length / (2 * sumFactor * sumFactor));
                sum4 += qRound((255 - srcRoi.ptr<quint8>(j)[i]) * gaussian);
            }
            else
                sum4 += 255 - srcRoi.ptr<quint8>(j)[i];
        }
    }

    QPoint pt(srcRoi.cols / 2 - 1, srcRoi.rows / 2 - 1);
    for (int j = pt.y(); j >= 0 && sum1 > 0; j--)
    {
        for (int i = srcRoi.cols / 2 - 1; i > pt.x() && sum1 > 0; i--)
        {
            int gray = dstRoi.ptr<quint8>(pt.y())[i];
            if (gray == 0)
            {
                dstRoi.ptr<quint8>(pt.y())[i] = sum1 > 255 ? 255 : sum1;
                sum1 -= 255;
            }
        }
        for (int j = srcRoi.rows / 2 - 1; j >= pt.y() && sum1 > 0; j--)
        {
            int gray = dstRoi.ptr<quint8>(j)[pt.x()];
            if (gray == 0)
            {
                dstRoi.ptr<quint8>(j)[pt.x()] = sum1 > 255 ? 255 : sum1;
                sum1 -= 255;
            }
        }
        pt.setX(qMax(0, qRound(pt.x() - dx)));
        pt.setY(qMax(0, qRound(pt.y() - dy)));
    }

    //// 修形
    //for (int j = srcRoi.cols / 2 - 1; j >= 0; j--)
    //{
    //    for (int i = srcRoi.rows / 2 - 1; i >= 0; i--)
    //    {
    //        int gray = dstRoi.ptr<quint8>(pt.y())[i];
    //        if (gray == 0)
    //        {
    //            int leftX = qMin(j, j - 1);
    //            int topY = qMin(i, i - 1);
    //            int topPx = dstRoi.ptr<quint8>(topY)[leftX];
    //            int leftPx = dstRoi.ptr<quint8>(topY)[leftX];
    //        }
    //    }
    //}

    pt = QPoint(srcRoi.cols / 2 - 1, srcRoi.rows / 2);
    for (int j = pt.y(); j >= 0 && sum2 > 0; j--)
    {
        for (int i = srcRoi.cols / 2 - 1; i > pt.x() && sum2 > 0; i--)
        {
            int gray = dstRoi.ptr<quint8>(pt.y())[i];
            if (gray == 0)
            {
                dstRoi.ptr<quint8>(pt.y())[i] = sum2 > 255 ? 255 : sum2;
                sum2 -= 255;
            }
        }
        for (int j = srcRoi.rows / 2; j <= pt.y() && sum2 > 0; j++)
        {
            int gray = dstRoi.ptr<quint8>(j)[pt.x()];
            if (gray == 0)
            {
                dstRoi.ptr<quint8>(j)[pt.x()] = sum2 > 255 ? 255 : sum2;
                sum2 -= 255;
            }
        }
        pt.setX(qMax(0, qRound(pt.x() - dx)));
        pt.setY(qMin(dstRoi.rows - 1, qRound(pt.y() + dy)));
    }

    pt = QPoint(srcRoi.cols / 2, srcRoi.rows / 2 - 1);
    for (int j = pt.y(); j >= 0 && sum3 > 0; j--)
    {
        for (int i = srcRoi.cols / 2; i < pt.x() && sum3 > 0; i++)
        {
            int gray = dstRoi.ptr<quint8>(pt.y())[i];
            if (gray == 0)
            {
                dstRoi.ptr<quint8>(pt.y())[i] = sum3 > 255 ? 255 : sum3;
                sum3 -= 255;
            }
        }
        for (int j = srcRoi.rows / 2 - 1; j >= pt.y() && sum3 > 0; j--)
        {
            int gray = dstRoi.ptr<quint8>(j)[pt.x()];
            if (gray == 0)
            {
                dstRoi.ptr<quint8>(j)[pt.x()] = sum3 > 255 ? 255 : sum3;
                sum3 -= 255;
            }
        }
        pt.setX(qMin(dstRoi.cols - 1, qRound(pt.x() + dx)));
        pt.setY(qMax(0, qRound(pt.y() - dy)));
    }

    pt = QPoint(srcRoi.cols / 2, srcRoi.rows / 2);
    for (int j = pt.y(); j >= 0 && sum4 > 0; j--)
    {
        for (int i = srcRoi.cols / 2; i < pt.x() && sum4 > 0; i++)
        {
            int gray = dstRoi.ptr<quint8>(pt.y())[i];
            if (gray == 0)
            {
                dstRoi.ptr<quint8>(pt.y())[i] = sum4 > 255 ? 255 : sum4;
                sum4 -= 255;
            }
        }
        for (int j = srcRoi.rows / 2; j <= pt.y() && sum4 > 0; j++)
        {
            int gray = dstRoi.ptr<quint8>(j)[pt.x()];
            if (gray == 0)
            {
                dstRoi.ptr<quint8>(j)[pt.x()] = sum4 > 255 ? 255 : sum4;
                sum4 -= 255;
            }
        }
        pt.setX(qMin(dstRoi.cols - 1, qRound(pt.x() + dx)));
        pt.setY(qMin(dstRoi.rows - 1, qRound(pt.y() + dy)));
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

QByteArray imageUtils::image2EngravingData(cv::Mat mat, qreal x, qreal y, qreal rowInterval, qreal width)
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
    for (int r = 0; r < mat.rows; r++)
    {
        int yStart = unitUtils::mm2MicroMM(y + r * rowInterval);
        int bitCount = 0;
        quint8 byte = 0;

        quint8 binCheck = 0;
        QList<quint8> rowBytes;
        for (int c = 0; c < mat.cols; c++)
        {
            quint8 pixel = forward ? mat.ptr<quint8>(r)[c] : mat.ptr<quint8>(r)[mat.cols - c - 1];
            quint8 bin = pixel ? 0 : 1;
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
    }

    /*forward = true;
    QDataStream restoreStream(&bytes, QIODevice::ReadWrite);
    restoreStream.setByteOrder(QDataStream::LittleEndian);
    cv::Mat res1(mat.size(), CV_8UC1, cv::Scalar(255));
    cv::Mat res2(mat.size(), CV_8UC1, cv::Scalar(255));
    int row = 0;
    while (!restoreStream.atEnd())
    {
        int yStart;
        restoreStream >> yStart >> xEnd >> xStart >> fspc.code;
        int length = fspc.count();
        int byteCount = length % 8 == 0 ? length / 8 : length / 8 + 1;
        for (int c = 0; c < byteCount; c++)
        {
            uchar byte;
            restoreStream >> byte;
            for (int i = 0; i < qMin(length, 8); i++)
            {
                uchar bit = (1 << i) & byte;
                uchar pixel = bit ? 255 : 0;
                if (forward)
                {
                    res1.ptr<uchar>(row)[c * 8 + i] = pixel;
                }
                else
                {
                    res2.ptr<uchar>(row)[fspc.count() - c * 8 - i - 1] = pixel;
                }
            }
            length -= 8;
        }
        row++;
        forward = !forward;
    }
    cv::imwrite("tmp/res1.bmp", res1);
    cv::imwrite("tmp/res2.bmp", res2);*/
    
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

