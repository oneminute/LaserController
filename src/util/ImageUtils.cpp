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
#include "laser/LaserDevice.h"
#include "task/ProgressItem.h"
#include "task/ProgressModel.h"

cv::Mat imageUtils::halftone6(ProgressItem* progress, cv::Mat src, float degrees, int gridSize)
{
    //src = 255 - src;
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

    cv::Mat dst(rotated.rows, rotated.cols, CV_8UC1, cv::Scalar(255));
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

            cv::Mat srcRoi = rotated(rectSrc);
            cv::Mat dstRoi = dst(rectDst);

            //qLogD << r << ", " << c << " src:" << srcRoi.rows << ", " << srcRoi.cols << ", dst:" << dstRoi.rows << ", " << dstRoi.cols;
            //generateGroupingDitcher(srcRoi, dstRoi);
            int full = srcRoi.cols * srcRoi.rows * 255;
            QPoint center;
            int sum = sumMat(srcRoi, center);

            QPoint drawCenter = center + QPoint(x, y);
            //cv::circle(canvas, cv::Point(drawCenter.x(), drawCenter.y()), 1, cv::Scalar(0, 0, 255));

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

    std::vector<int>param; 
    cv::imwrite("tmp/h6_dst.bmp", dst);

    //cv::Mat outMat(dst.rows, dst.cols, CV_8UC1, cv::Scalar(255));
    center = cv::Point2f((dst.cols - 1) / 2, (dst.rows - 1) / 2);
    rot = cv::getRotationMatrix2D(center, -degrees, 1.);
    bbox = cv::RotatedRect(cv::Point2f(), dst.size(), 0).boundingRect2f();
    cv::Mat antiRotated;
    cv::warpAffine(dst, antiRotated, rot, bbox.size(), cv::INTER_NEAREST, 0, cv::Scalar(0));
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
    cv::Mat outMat = antiRotated(roi);
    //outMat = 255 - outMat;
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
    qreal factor = Config::Export::gaussianFactorA();
    qreal fullLength = mat.rows / 2 + mat.rows / 2;
    qreal fullLength2 = fullLength * fullLength;
    //qreal gaussianReciprocal = sigma * qSqrt(2 * M_PI);
    //qreal gaussianFactor = 1.0 / gaussianReciprocal;
    //qreal sigma2 = sigma * sigma;
    //qreal maxLength = qMax(mat.cols, mat.rows);
    for (int i = 0; i < mat.cols; i++)
    {
        for (int j = 0; j < mat.rows; j++)
        {
            int grayScale = 255 - mat.ptr<quint8>(j)[i];
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
            int grayScale = 255 - mat.ptr<quint8>(j)[i];
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

void imageUtils::generatePattern(cv::Mat& dstRoi, int sum, QPoint& center, int initAngle, int rotationAngle, int stepAngle)
{
    dstRoi.setTo(cv::Scalar(255));

    QTransform initTrans;
    initTrans.rotate(initAngle);

    QPoint pt = center;
    QPoint vec(1, 0);
    vec = initTrans.map(vec);

    int count = 0;
    //while (false)
    while (sum > 0)
    {
        // 填充颜色并递减总颜色值
        int grayScale = sum >= 255 ? 0 : 255;
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
                else if (dstRoi.ptr<quint8>(candidate.y())[candidate.x()])
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

QByteArray imageUtils::image2EngravingData(ProgressItem* progress, cv::Mat mat, 
    const QRect& boundingRect, int rowInterval, QPoint& lastPoint, int accLength)
{
    cv::imwrite("tmp/engraving.bmp", mat);
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::ReadWrite);
    stream.setByteOrder(QDataStream::LittleEndian);
    int xStart = boundingRect.left();
    int xEnd = boundingRect.right();
    int yStart = boundingRect.top();
    FillStyleAndPixelsCount fspc;
    fspc.setCount(mat.cols);
    fspc.setSame(false);
    bool forward = true;
    progress->setMaximum(mat.rows);
    for (int r = 0; r < mat.rows; r++)
    {
        yStart = boundingRect.top() + r * rowInterval;
        quint8 byte = 0;
        int bitCount = 0;

        quint8 binCheck = 0;
        quint8 lastBin;
        QList<quint8> rowBytes;
        //QString rowString;
        bool same = true;
        for (int c = 0; c < mat.cols; c++)
        {
            quint8 pixel = forward ? mat.ptr<quint8>(r)[c] : mat.ptr<quint8>(r)[mat.cols - c - 1];
            quint8 bin = pixel >= 128 ? 0 : 0x80;
            if (c == 0)
                lastBin = bin;
            else
            {
                same = same && (lastBin == bin);
                lastBin = bin;
            }
            //rowString.append(QString::number(bin));
            binCheck |= bin;
            byte = byte | (bin >> (c % 8));
            bitCount++;
            if (bitCount == 8)
            {
                bitCount = 0;
                rowBytes.append(byte);
                //stream << byte;
                byte = 0;
            }
        }
        //rowString.append("\n\r");
        //rowBytes.append(byte);
        if (bitCount != 0)
            rowBytes.append(byte);
            //stream << byte;

        if (binCheck)
        {
            //fspc.setSame(same);
            if (forward)
            {
                lastPoint = QPoint(xEnd + accLength, yStart);
                stream << yStart << xStart << xEnd << fspc.code;
            }
            else
            {
                lastPoint = QPoint(xStart - accLength, yStart);
                stream << yStart << xEnd << xStart << fspc.code;
            }

            for (int i = 0; i < rowBytes.length(); i++)
            {
                stream << rowBytes.at(i);
            }
            forward = !forward;
        }

        progress->increaseProgress();
    }

    progress->finish();

    parseImageData(bytes, rowInterval);

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

QImage imageUtils::parseImageData(const QByteArray& data, int rowInterval)
{
    QDataStream stream(const_cast<QByteArray*>(&data), QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    FillStyleAndPixelsCount fspc;
    int bytesRead = 0;
    int line = 0;
    int minY = 0x7fffffff;
    int maxY = 0;
    int width = 0;
    int height = 0;
    QMap<int, QByteArray> lineBytes;
    while (true)
    {
        int xStart;
        int xEnd;
        int yStart;
        stream >> yStart
            >> xStart
            >> xEnd
            >> fspc.code;
        bytesRead += 4;
        bool forward = xStart < xEnd;

        if (yStart < minY)
            minY = yStart;
        if (yStart > maxY)
            maxY = yStart;

        if (width == 0)
            width = fspc.count();
        QByteArray grays;
        grays.resize(fspc.count());
        int bitCount = fspc.count() % 8 == 0 ? fspc.count() / 8 : fspc.count() / 8 + 1;
        int counter = 0;
        quint8 byte;
        for (int i = 0; i < width; i++)
        {
            if (i % 8 == 0)
            {
                stream >> byte;
                bytesRead++;
            }
            int bit = byte & (0x80 >> (i % 8));
            quint8 pixel = bit ? 0 : 255;
            if (forward)
                grays[i] = pixel;
            else
                grays[width - i - 1] = pixel;
        }
        lineBytes.insert(line, grays);
        line++;

        if (bytesRead >= data.length())
            break;
    }

    height = (maxY - minY) / rowInterval;
    QImage image(width, height, QImage::Format_Grayscale8);
    image.fill(Qt::white);

    for (QMap<int, QByteArray>::ConstIterator i = lineBytes.constBegin(); i != lineBytes.constEnd(); i++)
    {
        int row = i.key() - minY;
        const QByteArray& data = i.value();
        for (int col = 0; col < data.length(); col++)
        {
            image.setPixel(col, row, data[col]);
        }
    }
    image.save("tmp/bitmap.bmp");

    return image;
}

