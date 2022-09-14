#include "LaserStampBitmap.h"
#include "LaserStampBasePrivate.h"

#include <QBitmap>
#include <QBuffer>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>

#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "widget/LaserViewer.h"

class LaserStampBitmapPrivate : public LaserStampBasePrivate
{
    Q_DECLARE_PUBLIC(LaserStampBitmap)
public:
    LaserStampBitmapPrivate(LaserStampBitmap* ptr)
        : LaserStampBasePrivate(ptr)
    {
    }
    QImage image;
    QImage originalImage;
    QImage antiFakeImage;
    QImage fingerprintImage;
};
LaserStampBitmap::LaserStampBitmap(const QImage& image, const QRect& bounds, bool stampIntaglio, LaserDocument* doc, QTransform transform, int layerIndex)
    :LaserStampBase(new LaserStampBitmapPrivate(this), doc, LPT_STAMPBITMAP, stampIntaglio, transform, layerIndex)
{
    Q_D(LaserStampBitmap);
    setTransform(transform);
    d->boundingRect = bounds;
    
    d->image = image.convertToFormat(QImage::Format_ARGB32);
    d->originalImage = d->image;
    setBounds(bounds);
    setZValue(3);
    //QSize s = d->image.size();
    //QTransform t;
    //d->image = d->image.transformed(t, Qt::TransformationMode::SmoothTransformation);
    //computeImage();
}

LaserStampBitmap::~LaserStampBitmap()
{
}

void LaserStampBitmap::computeImage(bool generateStamp)
{
    Q_D(LaserStampBitmap);
    QImage ia = d->image;
    QSize size = d->image.size();
    for (int i = 0; i < size.width(); i++) {
        for (int j = 0; j < size.height(); j++) {
            
            QRgb rgb = QRgb(d->image.pixel(i, j));
            QColor col(rgb);
            int r = qRed(rgb);
            int g = qGreen(rgb);
            int b = qBlue(rgb);
            int a = qAlpha(rgb);
            
            if (a == 0) {
                col = QColor(0, 0, 0, 0);
            }
            else {
                
                if (r == 255 && g == 255 && b == 255) {
                    if (d->stampIntaglio) {
                        if (generateStamp) {
                            col = Qt::black;
                        }
                        else {
                            col = d->doc->layers()[d->layerIndex]->color();
                        }
                    }
                    else {
                        if (generateStamp) {
                            col = Qt::black;
                        }
                        else {
                            col = Qt::white;
                        }
                    }
                }
                else {
                    if (d->stampIntaglio) {
                        if (generateStamp) {
                            col = Qt::white;
                        }
                        else {
                            col = Qt::white;
                        }
                    }
                    else {
                        if (generateStamp) {
                            col = Qt::white;
                        }
                        else {
                            col = d->doc->layers()[d->layerIndex]->color();
                            
                        }
                    }
                }
            }
            d->image.setPixel(i, j, col.rgba());
        }       
    }
    d->originalImage = d->image;
    setAntiFakePath(d->antiFakePath);
}

void LaserStampBitmap::setStampIntaglio(bool bl)
{
    Q_D(LaserStampBitmap);
    LaserStampBase::setStampIntaglio(bl);
    computeImage();

}

LaserPrimitive* LaserStampBitmap::cloneImplement()
{
    Q_D(LaserStampBitmap);
    LaserStampBitmap* p = new LaserStampBitmap(d->originalImage, d->boundingRect, 
        d->stampIntaglio, d->doc, sceneTransform(), d->layerIndex);
    stampBaseClone(p);
    p->setAntiFakeImage(d->antiFakeImage);
    p->setFingerprint();
    p->computeMask();
    return p;
}

QJsonObject LaserStampBitmap::toJson()
{
    Q_D(LaserStampBitmap);
    QJsonObject object;
    QTransform pt = QTransform();
    QJsonArray pm = {
        pt.m11(), pt.m12(), pt.m13(),
        pt.m21(), pt.m22(), pt.m23(),
        pt.m31(), pt.m32(), pt.m33()
    };
    object.insert("parentMatrix", pm);
    QTransform t = this->sceneTransform();
    QJsonArray matrix = { t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31(), t.m32(), t.m33() };
    object.insert("matrix", matrix);
    object.insert("name", name());
    object.insert("className", this->metaObject()->className());    
    object.insert("layerIndex", layerIndex());
    //bounds
    QJsonArray bounds = { d->boundingRect.x(), d->boundingRect.y(),d->boundingRect.width(), d->boundingRect.height() };
    object.insert("bounds", bounds);
    //originalImage
    QByteArray imageBits;
    QBuffer buffer(&imageBits);
    buffer.open(QIODevice::ReadWrite);
    d->originalImage.save(&buffer, "tiff");
    buffer.close();
    object.insert("originalImage", QLatin1String(imageBits.toBase64()));
    //antiFakeImage
    QByteArray antiFakeImageBits;
    QBuffer antiFakeBuffer(&antiFakeImageBits);
    antiFakeBuffer.open(QIODevice::ReadWrite);
    d->antiFakeImage.save(&antiFakeBuffer, "tiff");
    antiFakeBuffer.close();
    object.insert("antiFakeImage", QLatin1String(antiFakeImageBits.toBase64()));
    
    stampBaseToJson(object);
    return object;
}

void LaserStampBitmap::draw(QPainter* painter)
{
    Q_D(LaserStampBitmap);
    painter->drawImage(d->boundingRect, d->image);

}

void LaserStampBitmap::setOriginalImage(QImage image)
{
    Q_D(LaserStampBitmap);
    d->originalImage = image;
}

void LaserStampBitmap::setFingerprint()
{
    Q_D(LaserStampBitmap);   
    if (d->fingerNoDensityMap == QPixmap()) {

        d->fingerprintImage = QImage();
    }
    else {
        QImage maskImage(d->image.width(), d->image.height(), QImage::Format_ARGB32);
        maskImage.fill(Qt::transparent);
        QPainter maskPainter(&maskImage);
        QRect bounds(0, 0, d->boundingRect.width(), d->boundingRect.height());
        setStampBrush(&maskPainter, d->doc->layers()[d->layerIndex]->color(), QSize(d->image.width(), d->image.height()));
        maskPainter.drawRect(bounds);
        d->fingerprintImage = maskImage;
    }
}

void LaserStampBitmap::computeMask()
{
    Q_D(LaserStampBitmap);
    QPixmap antiFakeMap, fingerprintMap;
    if (d->antiFakeImage != QImage()) {
        antiFakeMap = QPixmap::fromImage(d->originalImage);
        antiFakeMap.setMask(QBitmap(QPixmap::fromImage(d->antiFakeImage)));
    }
    
    if (d->fingerprintImage != QImage()) {
        fingerprintMap = QPixmap::fromImage(d->originalImage);
        fingerprintMap.setMask(QPixmap::fromImage(d->fingerprintImage).mask());
    }
    if (antiFakeMap != QPixmap() && fingerprintMap != QPixmap()) {
        antiFakeMap.setMask(fingerprintMap.mask());
        d->image = antiFakeMap.toImage();
    }
    else if (antiFakeMap != QPixmap()) {
        d->image = antiFakeMap.toImage();
    }
    else if (fingerprintMap != QPixmap()) {
        d->image = fingerprintMap.toImage();
    }
    
}

void LaserStampBitmap::setBounds(QRect bounds)
{
    Q_D(LaserStampBitmap);
    d->boundingRect = bounds;
    //d->originalBoundingRect = rect;
    d->path = QPainterPath();
    d->path.addRect(d->boundingRect);
    d->outline = QPainterPath();
    d->outline.addRect(bounds);
}
QImage LaserStampBitmap::generateStampImage()
{
    Q_D(LaserStampBitmap);
    QImage image(d->image.size(), d->image.format());
    for (int x = 0; x < d->image.width(); x++) {
        for (int y = 0; y < d->image.height(); y++) {
            QRgb rgb = QRgb(d->image.pixel(x, y));
            QColor col(rgb);
            if (d->stampIntaglio) {
                if (col == Qt::white) {
                    col = Qt::black;
                }
                else {
                    col = Qt::white;
                }
            }
            else {
                if (col == d->doc->layers()[d->layerIndex]->color()) {
                    col = Qt::white;
                }
                else {
                    col = Qt::black;
                }
            }
            
            /*if (d->stampIntaglio) {
                if (col == Qt::white) {
                    col = Qt::black;
                }
                else {

                }
            }
            else {
                if (col == Qt::white) {

                }
                else {

                }
            }*/
            image.setPixel(x, y, col.rgba());
        }
    }
    return image;
}
void LaserStampBitmap::setBoundingRectWidth(qreal width)
{
    Q_D(LaserStampBitmap);
    qreal top, left, w, h;
    w = width;
    h = d->boundingRect.height();
    qreal diffW = w - d->boundingRect.width();
    left = d->boundingRect.left() - diffW * 0.5;
    top = d->boundingRect.top();
    setBounds(QRect(left, top, w, h));
}

void LaserStampBitmap::setBoundingRectHeight(qreal height)
{
    Q_D(LaserStampBitmap);
    qreal top, left, w, h;
    w = d->boundingRect.width();
    h = height;
    qreal diffH = h - d->boundingRect.height();
    top = d->boundingRect.top() - diffH * 0.5;
    left = d->boundingRect.left();
    setBounds(QRect(left, top, w, h));
}

void LaserStampBitmap::setAntiFakePath(QPainterPath path)
{
    Q_D(LaserStampBitmap);
    QSize size = d->originalImage.size();
    QImage antiFakeImage(d->originalImage.size(), d->originalImage.format());
    antiFakeImage.fill(Qt::transparent);
    QPainter painter(&antiFakeImage);
    painter.setBrush(Qt::white);
    QTransform t;
    QRectF bounds = path.boundingRect();
    //qreal w = Global::mmToPixel(bounds.width() * 0.001);
    //qreal h = Global::mmToPixel(bounds.height() * 0.001);
    qreal w = bounds.width();
    qreal h = bounds.height();
    t.translate(-bounds.left(), -bounds.top());
    path = t.map(path);
    QTransform t1;
    qreal rX = size.width() / w;
    qreal rY = size.height() / h;
    t1.scale(rX, rY);
    path = t1.map(path);
    painter.drawPath(path);
    setAntiFakeImage(antiFakeImage);
    computeMask();
}

void LaserStampBitmap::setAntiFakeImage(QImage image)
{
    Q_D(LaserStampBitmap);
    d->antiFakeImage = image;
    
}

void LaserStampBitmap::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
}

void LaserStampBitmap::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

void LaserStampBitmap::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    QList<QGraphicsView*> views = scene()->views();
    LaserViewer* viewer = qobject_cast<LaserViewer*> (views[0]);
    viewer->onEndSelecting();
}

void LaserStampBitmap::sceneMousePressEvent(LaserViewer* viewer, LaserScene* scene, const QPoint& point, QMouseEvent* event)
{
}

void LaserStampBitmap::sceneMouseMoveEvent(LaserViewer* viewer, LaserScene* scene, const QPoint& point, QMouseEvent* event, bool isPressed)
{
}

void LaserStampBitmap::sceneMouseReleaseEvent(LaserViewer* viewer, LaserScene* scene, const QPoint& point, QMouseEvent* event, bool isPressed)
{
}

void LaserStampBitmap::sceneKeyPressEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

void LaserStampBitmap::sceneKeyReleaseEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

void LaserStampBitmap::beginCreatingInternal(QUndoCommand* parentCmd, PrimitiveAddingCommand* addingCmd)
{
}

void LaserStampBitmap::endCreatingInterval(QUndoCommand* parentCmd, PrimitiveRemovingCommand* removingCmd)
{
}
