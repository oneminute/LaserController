#include "LaserShape.h"
#include "LaserShapePrivate.h"

#include <QPainter>
#include <QtMath>

#include "common/Config.h"
#include "scene/LaserLayer.h"
#include "task/ProgressItem.h"
#include "util/ImageUtils.h"

LaserShape::LaserShape(LaserShapePrivate* data, LaserDocument* doc, LaserPrimitiveType type, int layerIndex, QTransform saveTransform)
    : LaserPrimitive(data, doc, type,  saveTransform, layerIndex)
{
	Q_D(LaserShape);
    //m_type = LPT_SHAPE;
	d->layerIndex = layerIndex;
}

QByteArray LaserShape::filling(ProgressItem* progress, QPoint& lastPoint)
{
    Q_D(LaserShape);
    QByteArray bytes;
    QPainterPath path = sceneTransform().map(d->path);
    QRect boundingRectInDevice = path.boundingRect().toRect();
    qreal ratio = boundingRectInDevice.width() * 1.0 / boundingRectInDevice.height();
    int maxImageSize;
    switch (Config::Export::imageQuality())
    {
    case IQ_Normal:
        maxImageSize = 1024;
        break;
    case IQ_High:
        maxImageSize = 4096;
        break;
    case IQ_Perfect:
        maxImageSize = 8192;
        break;
    }
    int canvasWidth = qMin(boundingRectInDevice.width(), maxImageSize);
    int canvasHeight = qRound(canvasWidth / ratio);
    if (ratio < 1)
    {
        canvasHeight = qMin(boundingRectInDevice.height(), maxImageSize);
        canvasWidth = qRound(canvasHeight * ratio);
    }

    QTransform t = QTransform::fromScale(
        canvasWidth * 1.0 / boundingRectInDevice.width(),
        canvasHeight * 1.0 / boundingRectInDevice.height()
    );

    path = t.map(path);
    QRect boundingRect = path.boundingRect().toRect();
    t = QTransform::fromTranslate(-boundingRect.x(), -boundingRect.y());
    path = t.map(path);
    QImage canvas(boundingRect.width(), boundingRect.height(), QImage::Format_Grayscale8);
    if (canvas.isNull())
    {
        progress->finish();
        return QByteArray();
    }
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    painter.setBrush(Qt::black);
    painter.drawPath(path);
    canvas.save("tmp/" + name() + "_canvas.png");

    cv::Mat src(canvas.height(), canvas.width(), CV_8UC1, (void*)canvas.constBits(), canvas.bytesPerLine());

    int dpi = d->layer->dpi();
    int pixelWidth = boundingRectInDevice.width() * dpi / 25400.0;
    int pixelHeight = boundingRectInDevice.height() * dpi / 25400.0;

    int pixelInterval = layer()->engravingRowInterval();
    int outWidth = pixelWidth;
    int outHeight = qCeil(boundingRectInDevice.height() * 1.0 / pixelInterval);
    cv::Mat resized;
    cv::resize(src, resized, cv::Size(outWidth, outHeight), 0.0, 0.0, cv::INTER_CUBIC);

    cv::imwrite("tmp/" + name().toStdString() + "_resized.png", resized);

    qLogD << "bounding rect: " << boundingRectInDevice;
    qDebug() << "out width:" << outWidth;
    qDebug() << "out height:" << outHeight;
    
    //int accLength = LaserApplication::device->engravingAccLength(layer()->engravingRunSpeed());
    bytes = imageUtils::image2EngravingData(progress, resized, boundingRectInDevice, pixelInterval, lastPoint);

    return bytes;
}

int LaserShape::layerIndex()
{
	Q_D(LaserShape);
	return d->layerIndex;
}


