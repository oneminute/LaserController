#include "LaserBitmap.h"
#include "LaserShapePrivate.h"

#include <QBuffer>
#include <QFileDialog>
#include <QImageReader>
#include <QJsonArray>
#include <QPainter>
#include <QtMath>

#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "undo/PrimitiveAddingCommand.h"
#include "util/ImageUtils.h"
#include "util/Utils.h"
#include "widget/LaserViewer.h"

class LaserBitmapPrivate : public LaserPrimitivePrivate
{
    Q_DECLARE_PUBLIC(LaserBitmap)
public:
    LaserBitmapPrivate(LaserBitmap* ptr)
        : LaserPrimitivePrivate(ptr)
    {}

    QImage image;
};

LaserBitmap::LaserBitmap(LaserDocument* doc, QTransform transform, int layerIndex)
    : LaserPrimitive(new LaserBitmapPrivate(this), doc,  LPT_BITMAP, 
        transform, layerIndex)
{
    Q_D(LaserBitmap);
    d->primitiveType = LPT_BITMAP;
	sceneTransformToItemTransform(transform);
	setFlags(ItemIsSelectable | ItemIsMovable);
	installEventFilter(doc->scene());
}

LaserBitmap::LaserBitmap(const QImage & image, const QRect& bounds, 
    LaserDocument * doc, QTransform saveTransform, int layerIndex)
    : LaserPrimitive(new LaserBitmapPrivate(this), doc,  LPT_BITMAP, saveTransform, layerIndex)
{
    Q_D(LaserBitmap);
    d->image = image.convertToFormat(QImage::Format_Grayscale8);
    d->boundingRect = bounds;
    d->path.addRect(bounds);
    d->outline.addRect(bounds);
	sceneTransformToItemTransform(saveTransform);
	setFlags(ItemIsSelectable | ItemIsMovable);
	installEventFilter(doc->scene());
}

QImage LaserBitmap::image() const 
{
    Q_D(const LaserBitmap);
    return d->image; 
}

void LaserBitmap::setImage(const QImage& image)
{
    Q_D(LaserBitmap);
    d->image = image;
}

QRectF LaserBitmap::bounds() const 
{
    Q_D(const LaserBitmap);
    return d->boundingRect; 
}

void LaserBitmap::setRect(QRect rect)
{
    Q_D(LaserBitmap);
    d->boundingRect = rect;
    //d->originalBoundingRect = rect;
    d->path = QPainterPath();
    d->path.addRect(d->boundingRect);
    d->outline = QPainterPath();
    d->outline.addRect(rect);
}

QByteArray LaserBitmap::engravingImage(ProgressItem* parentProgress, QPoint& lastPoint)
{ 
    Q_D(LaserBitmap);
    QByteArray ba;

    parentProgress->setMaximum(2);
    QImage srcImage = d->image.copy();
    QImage rotated = srcImage.transformed(sceneTransform(), Qt::SmoothTransformation);
    QImage outImage(rotated.size(), QImage::Format_Grayscale8);;
    outImage.fill(Qt::white);
    QPainter p(&outImage);
    p.begin(&outImage);
    p.drawImage(0, 0, rotated);
    p.end();
    outImage = outImage.convertToFormat(QImage::Format_Grayscale8);
    QRect boundingRect = sceneBoundingRect();
    cv::Mat src(outImage.height(), outImage.width(), CV_8UC1, (void*)outImage.constBits(), outImage.bytesPerLine());

    LaserLayer* layer = this->layer();
    int dpi = layer->dpi();
    int pixelWidth = boundingRect.width() * dpi / 25400.0;
    int pixelHeight = boundingRect.height() * dpi / 25400.0;

    int gridSize = qRound(dpi * 1.0 / layer->lpi());

    cv::Mat pixelScaled;
    cv::resize(src, pixelScaled, cv::Size(pixelWidth, pixelHeight), 0.0, 0.0, cv::INTER_NEAREST);

    cv::Mat halfToneMat = src;
    if (layer->useHalftone())
        halfToneMat = imageUtils::halftone6(parentProgress, pixelScaled, layer->halftoneAngles(), gridSize);

    qreal pixelInterval = layer->engravingRowInterval();

    int outWidth = pixelWidth;
    int outHeight = qRound(boundingRect.height() / pixelInterval);
    qLogD << "bounding rect: " << boundingRect;
    qDebug() << "out width:" << outWidth;
    qDebug() << "out height:" << outHeight;

    cv::Mat resized;
    cv::resize(halfToneMat, resized, cv::Size(outWidth, outHeight), cv::INTER_NEAREST);
    
    ba = imageUtils::image2EngravingData(parentProgress, resized, boundingRect, pixelInterval, lastPoint);

    parentProgress->finish();
    return ba; 
}

void LaserBitmap::draw(QPainter * painter)
{
    Q_D(LaserBitmap);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
	//QImage image = d->image.transformed(d->allTransform, Qt::TransformationMode::SmoothTransformation);
    //painter->setBackgroundMode(Qt::TransparentMode);
    //d->image.fill(Qt::transparent);
    
    
    //d->image = d->image.createMaskFromColor(Qt::black);
    //d->image.toImageFormat(QImage::Format_ARGB32);
    //pixmap.fill(QColor(0, 0, 0, 125));
    
	painter->drawImage(d->boundingRect, d->image);
    //painter->drawRect(d->boundingRect);
    
    
    //painter->drawImage()
}

QJsonObject LaserBitmap::toJson()
{
	Q_D(const LaserBitmap);
	QJsonObject object;
	//QJsonArray position = { pos().x(), pos().y() };
	QTransform transform = QTransform();
	QJsonArray matrix = {
		transform.m11(), transform.m12(), transform.m13(),
		transform.m21(), transform.m22(), transform.m23(),
		transform.m31(), transform.m32(), transform.m33()
	};
	
	QTransform parentTransform = this->sceneTransform();
	QJsonArray parentMatrix = { parentTransform.m11(), parentTransform.m12(), parentTransform.m13(), parentTransform.m21(), parentTransform.m22(), parentTransform.m23(), parentTransform.m31(), parentTransform.m32(), parentTransform.m33() };
	object.insert("parentMatrix", parentMatrix);
	//rect
	QJsonArray bounds = { d->boundingRect.x(), d->boundingRect.y(),d->boundingRect.width(), d->boundingRect.height() };
	QJsonArray();
	//image
	//QByteArray imageBits(d->image.byteCount(), (char)0);
	QByteArray imageBits;
	QBuffer buffer(&imageBits);
	buffer.open(QIODevice::ReadWrite);
	d->image.save(&buffer, "tiff");
	buffer.close();

	object.insert("name", name());
	object.insert("className", this->metaObject()->className());
	//object.insert("position", position);
	object.insert("matrix", matrix);
	object.insert("bounds", bounds);
	object.insert("image", QLatin1String(imageBits.toBase64()));
	object.insert("layerIndex", layerIndex());
	return object;
}

LaserPointListList LaserBitmap::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserBitmap);
    ProgressItem* progress = new ProgressItem(tr("%1 update machining points").arg(name()), ProgressItem::PT_Simple, parentProgress);
    d->machiningPointsList.clear();
    d->startingIndices.clear();

	//QTransform t = sceneTransform() * Global::matrixToMachining();
    QPolygon poly = sceneTransform().map(d->path).toFillPolygon().toPolygon();
    QPoint pt1 = poly.at(0);
    QPoint pt2 = poly.at(1);
    QPoint pt3 = poly.at(2);
    QPoint pt4 = poly.at(3);

    QLineF line11(pt1, pt2);
    QLineF line12(pt1, pt4);
    qreal angle11 = line11.angle();
    qreal angle12 = line12.angle();

    QLineF line21(pt2, pt3);
    QLineF line22(pt2, pt1);
    qreal angle21 = line21.angle();
    qreal angle22 = line22.angle();

    QLineF line31(pt3, pt4);
    QLineF line32(pt3, pt2);
    qreal angle31 = line31.angle();
    qreal angle32 = line32.angle();

    QLineF line41(pt4, pt1);
    QLineF line42(pt4, pt3);
    qreal angle41 = line41.angle();
    qreal angle42 = line42.angle();

    LaserPointList points;
    points.push_back(LaserPoint(pt1.x(), pt1.y()/*, qRound(angle11), qRound(angle12)*/));
    points.push_back(LaserPoint(pt2.x(), pt2.y()/*, qRound(angle21), qRound(angle22)*/));
    points.push_back(LaserPoint(pt3.x(), pt3.y()/*, qRound(angle31), qRound(angle32)*/));
    points.push_back(LaserPoint(pt4.x(), pt4.y()/*, qRound(angle41), qRound(angle42)*/));
    d->machiningCenter = utils::center(points).toPoint();
    points.push_back(points.first());
    d->startingIndices.append(0);
    d->startingIndices.append(1);
    d->startingIndices.append(2);
    d->startingIndices.append(3);
    d->machiningPointsList.append(points);

    progress->finish();
    return d->machiningPointsList;
}

//QRect LaserBitmap::sceneBoundingRect() const
//{
//	Q_D(const LaserBitmap);
//	QPainterPath path;
//	path.addRect(d->boundingRect);
//	return sceneTransform().map(path).boundingRect().toRect();
//}

void LaserBitmap::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mousePressEvent(event);
	//QList<QGraphicsView*> views = scene()->views();
	//views[0]->viewport()->repaint();
	//event->accept();
	//event->ignore();
}

void LaserBitmap::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mouseMoveEvent(event);
	//QList<QGraphicsView*> views = scene()->views();
	//views[0]->viewport()->repaint();
	//event->accept();
}

void LaserBitmap::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mouseReleaseEvent(event);
	QList<QGraphicsView*> views = scene()->views();
	LaserViewer* viewer = qobject_cast<LaserViewer*> (views[0]);
	viewer->onEndSelecting();
}

void LaserBitmap::setBoundingRectWidth(qreal width)
{
    Q_D(LaserBitmap);
    //d->boundingRect = QRect(d->boundingRect.left(), d->boundingRect.top(), width, d->boundingRect.height());
    //d->outline = QPainterPath();
    //d->outline.addRect(d->boundingRect);
    setRect(QRect(d->boundingRect.left(), d->boundingRect.top(), width, d->boundingRect.height()));
}

void LaserBitmap::setBoundingRectHeight(qreal height)
{
    Q_D(LaserBitmap);
    //d->boundingRect = QRect(d->boundingRect.left(), d->boundingRect.top(), d->boundingRect.width(), height);
    //d->outline = QPainterPath();
    //d->outline.addRect(d->boundingRect);
    setRect(QRect(d->boundingRect.left(), d->boundingRect.top(), d->boundingRect.width(), height));

}

QVector<QLine> LaserBitmap::edges()
{
	Q_D(const LaserBitmap);
	QPainterPath path;
	path.addRect(d->boundingRect);
    path = sceneTransform().map(path);
	return LaserPrimitive::edges(path);
}

void LaserBitmap::sceneMousePressEvent(LaserViewer* viewer, LaserScene* scene,
    const QPoint& point, QMouseEvent* event)
{
}

void LaserBitmap::sceneMouseMoveEvent(LaserViewer* viewer, LaserScene* scene,
    const QPoint& point, QMouseEvent* event, bool isPressed)
{
}

void LaserBitmap::sceneMouseReleaseEvent(LaserViewer* viewer, LaserScene* scene,
    const QPoint& point, QMouseEvent* event, bool isPressed)
{
    Q_D(LaserBitmap);
    if (isEditing())
    {
        QString name = QFileDialog::getOpenFileName(nullptr, "open image", ".", "Images (*.jpg *.jpeg *.tif *.bmp *.png *.svg *.ico)");
        if (name.isEmpty())
            return;
        QFile file(name);
        file.open(QFile::ReadOnly);
        QByteArray data = file.readAll();
        QImage image;
        bool bl = image.loadFromData(data);
        QImageReader r;
        r.setFileName(name);
        QSize size = r.size();
        qreal w = size.width();
        qreal h = size.height();
        qreal ratioWH = w / h;
        qreal base = 8192;
        qreal bigger = w;
        if (bigger < h) {
            bigger = h;
            if (bigger > base) {
                h = base;
                w = ratioWH * h;
            }
        }
        else {
            if (bigger > base) {
                w = base;
                h = (1 / ratioWH) * w;
            }
        }

        r.setScaledSize(QSize(qFloor(w), qFloor(h)));
        image = r.read();
        // 这里像素要转微米
        int width = Global::sceneToMechH(image.size().width());
        int height = Global::sceneToMechV(image.size().height());
        QRect layout = LaserApplication::device->layoutRect();
        QRect bitmapRect(point.x(), point.y(), width, height);
        this->setImage(image);
        this->setRect(bitmapRect);
        LaserLayer* layer = this->layer();
        if (layer)
        {
            /*PrimitiveAddingCommand* cmdAdding = new PrimitiveAddingCommand(
                tr("Add Rect"), viewer, scene, this->document(), this->id(),
                layer->id(), this);
            document()->removePrimitive(this, false, true, true);

            viewer->addUndoCommand(cmdAdding);*/
        }
        emit viewer->endEditing();
    }
}

void LaserBitmap::sceneKeyPressEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

void LaserBitmap::sceneKeyReleaseEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

void LaserBitmap::beginCreatingInternal(QUndoCommand* parentCmd, 
    PrimitiveAddingCommand* addingCmd)
{
}

void LaserBitmap::endCreatingInterval(QUndoCommand* parentCmd,
    PrimitiveRemovingCommand* removingCmd)
{
}

LaserPrimitive * LaserBitmap::cloneImplement()
{
	Q_D(LaserBitmap);
	LaserBitmap* bitmap = new LaserBitmap(d->image, d->boundingRect, 
        document(), sceneTransform(), d->layerIndex);
	return bitmap;
}

bool LaserBitmap::isClosed() const
{
    return true;
}

QPointF LaserBitmap::position() const
{
    return sceneBoundingRect().topLeft();
}

