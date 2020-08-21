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

cv::Mat imageUtils::halftone2(cv::Mat src, float mmWidth, float mmHeight, float lpi, float dpi, float degrees)
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

    cv::Mat resized;
    cv::resize(src, resized, outMat.size());
    cv::imwrite("resized.bmp", resized);
    int grayGrades;
    cv::Mat ditchMat = generateRoundSpiralPattern(gridSize, grayGrades, degrees);
    //cv::Mat ditchMat = generateCircleMat(gridSize, grayGrades, degrees);

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

    float threshold = 64.5f;

    float coa = 7.f / 16;
    float cob = 3.f / 16;
    float coc = 5.f / 16;
    float cod = 1.f / 16;

    for (int r = 0; r < resized.rows; r += ditchMat.rows)
    {
        for (int c = 0; c < resized.cols; c += ditchMat.cols)
        {
            cv::Point start(c, r);
            for (int x = 0; x < ditchMat.cols; x++)
            {
                for (int y = 0; y < ditchMat.rows; y++)
                {
                    if (r + y < resized.rows && c + x < resized.cols)
                    {
                        uchar srcPixel = resized.ptr<uchar>(r + y)[c + x];
                        //int grayValue = std::ceil((255.f - srcPixel) * grayGrades / 255);
                        float grayValue = (255.f - srcPixel) * grayGrades / 255;
                        float ditchPixel = ditchMat.ptr<float>(y)[x];

                        if (grayValue > ditchPixel)
                            outMat.ptr<uchar>(r + y)[c + x] = 0;
                    }
                }
            }
        }
    }
    //cv::imwrite("dotsMat2.tiff", dotsMat);
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
            //if (i == 1 && j == 1)
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
    cv::warpAffine(arranged, rotated, rot, bbox.size(), cv::INTER_CUBIC, 0, cv::Scalar(255));
    std::cout << "rotated" << std::endl;
    std::cout << rotated << std::endl;
    cv::imwrite("rot.bmp", rotated);

    double d = gridSize;
    cv::Mat roi = rotated(cv::Rect2d(d, d, d, d));
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

cv::Mat imageUtils::generateRoundSpiralPattern(int gridSize, int& grades, float degrees)
{
    int ditchSize = gridSize / std::sqrt(2);
    grades = ditchSize * ditchSize;
    cv::Mat spiralMat = generateRoundSpiralMat(ditchSize);
    std::cout << "round spiral mat:" << std::endl;
    std::cout << spiralMat << std::endl;
    cv::imwrite("round_spiral.bmp", spiralMat);
    cv::Mat dst = generateRotatedPattern(spiralMat, gridSize, degrees);
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
                /*if (max == count)
                {
                    qDebug() << length << x << y << 0;
                    dst.ptr<uchar>(y)[x] = 0;
                    count--;
                }
                else
                {*/
                    if (v < 0)
                    {
                        dst.ptr<int>(y)[x] = max - count;
                        //qDebug() << length << x << y << max - count;
                        count--;
                    }
                //}
            }
        }
        
        length += 0.2;
    }
    return dst;
}


