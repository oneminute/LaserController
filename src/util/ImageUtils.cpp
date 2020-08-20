#include "ImageUtils.h"

#include <QDebug>
#include <QtMath>

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
    //int ditchSize = std::round(gridSize / std::cos((45 - degrees) * M_PI / 180) / std::sqrt(2));
    int ditchSize = gridSize;

    cv::Mat outMat(outVPixels, outHPixels, CV_8UC1, cv::Scalar(255));
    cv::Mat dotsMat(dotsVCount, dotsHCount, CV_32S, cv::Scalar(255));
    cv::Mat dotsMatBin(dotsVCount, dotsHCount, CV_8UC1, cv::Scalar(255));
    //cv::Mat ditchMat = generateSpiralDitchMat(ditchSize);
    //cv::Mat ditchMat = generateBayerDitchMatRec(std::ceil(std::sqrt(ditchSize)) + 1);
    cv::Mat ditchMat = generateCircleMat(ditchSize);

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
    qDebug().noquote().nospace() << "    ditchSize: " << ditchSize;
    qDebug().noquote().nospace() << "ditchMat size: " << ditchMat.cols;


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
            cv::Point start(c * dotWidth + dotWidth / 2, r * dotWidth + dotWidth / 2);
            //cv::circle(outMat, start, dotWidth / 2, cv::Scalar(0), cv::FILLED);
            float pixel = dotsMat.ptr<float>(r)[c];
            //int gray = std::round((255 - pixel) / 255 * (ditchMat.rows * ditchMat.cols));
            int gray = std::round((255 - pixel) / 255 * (ditchSize / 2 + 1));
            //cv::Point start(c * gridSize, r * gridSize);
            for (int x = start.x - ditchMat.cols / 2; x < start.x - ditchMat.cols / 2 + ditchMat.cols; x++)
            {
                for (int y = start.y - ditchMat.rows / 2; y < start.y - ditchMat.rows / 2 + ditchMat.rows; y++)
                {
                    if (gray >= ditchMat.ptr<uchar>(y - start.y + ditchMat.rows / 2)[x - start.x + ditchMat.cols / 2])
                    {
                        if (y < outMat.rows && x < outMat.cols && x >= 0 && y >= 0)
                            outMat.ptr<uchar>(y)[x] = 0;
                    }
                }
            }
        }
    }
    cv::imwrite("dotsMat2.tiff", dotsMat);
    cv::imwrite("dotsMatBin.png", dotsMatBin);
    cv::imwrite("outMat.bmp", outMat);
    return cv::Mat();
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
    //cv::Mat

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

cv::Mat imageUtils::generateSpiralDitchMat(int circleNum, float degrees)
{
    cv::Mat mat = generateSpiralDitchMatRec(circleNum);
    cv::Mat dst = generateRotatedPattern(mat, degrees);
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

cv::Mat imageUtils::generateBayerDitchMat(int circleNum, float degrees)
{
    cv::Mat mat = generateBayerDitchMatRec(circleNum);
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

cv::Mat imageUtils::generateCircleMat(int size, float degrees)
{
    cv::Mat mat(size, size, CV_8UC1, cv::Scalar(255));
    cv::Point2f center(size / 2., size / 2.);
    for (int r = 0; r < size; r++)
    {
        for (int c = 0; c < size; c++)
        {
            cv::Point2f pt(c, r);
            int length = std::floor(cv::norm(pt - center));
            if (length <= size / 2)
                mat.ptr<uchar>(r)[c] = std::round(size / 2 * std::sin(length * 1. / size * 2));
            //else
                //mat.ptr<uchar>(r)[c] = size / 2;

        }
    }
    cv::Mat dst = generateRotatedPattern(mat, degrees);
    return dst;
}

cv::Mat imageUtils::generateRotatedPattern(cv::Mat src, float degrees)
{
    cv::Mat arranged(src.rows * 3, src.cols * 3, CV_8UC1, cv::Scalar(255));
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            cv::Mat roi = arranged(cv::Rect(i * src.cols, j * src.rows, src.cols, src.rows));
            src.copyTo(roi);
        }
    }
    //std::cout << arranged << std::endl;
    cv::imwrite("arranged.png", arranged);

    cv::Point2f center((arranged.cols - 1) / 2.f, (arranged.rows - 1) / 2.f);
    cv::Mat rot = cv::getRotationMatrix2D(center, degrees, 1.);
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), arranged.size(), degrees).boundingRect2f();
    //cv::Rect2f bbox(0, 0, mat.cols * 2, mat.rows * 2);

    // adjust transformation matrix
    rot.at<double>(0, 2) += bbox.width / 2.0 - arranged.cols / 2.0;
    rot.at<double>(1, 2) += bbox.height / 2.0 - arranged.rows / 2.0;

    cv::Mat rotated;
    cv::warpAffine(arranged, rotated, rot, bbox.size(), 1, 0, cv::Scalar(255));
    //std::cout << rotated << std::endl;
    cv::imwrite("rot.png", rotated);

    double d = src.cols * sqrt(2);
    cv::Mat roi = rotated(cv::Rect2d(d, d, d, d));
    cv::Mat pattern;
    roi.copyTo(pattern);
    cv::imwrite("pattern1.png", pattern);

    cv::resize(pattern, pattern, src.size());
    cv::imwrite("pattern2.png", pattern);

    //std::cout << pattern << std::endl;
    return pattern;
}


