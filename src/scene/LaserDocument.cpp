#include "LaserDocument.h"

#include <QBuffer>
#include <QByteArray>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QImageReader>
#include <QList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSaveFile>
#include <QSharedData>
#include <QStack>
#include <QtMath>
#include <QPainterPath>
#include <opencv2/opencv.hpp>

#include <LaserApplication.h>
#include <laser/LaserDevice.h>
#include "LaserPrimitive.h"
#include "PageInformation.h"
#include "common/Config.h"
#include "laser/LaserDriver.h"
#include "util/MachiningUtils.h"
#include "util/TypeUtils.h"
#include "util/Utils.h"
#include "LaserLayer.h"
#include "state/StateController.h"
#include "svg/qsvgtinydocument.h"
#include "LaserScene.h"
#include "laser/LaserPointList.h"
#include "ui/LaserControllerWindow.h"
#include "task/ProgressModel.h"

class LaserDocumentPrivate : public ILaserDocumentItemPrivate
{
    Q_DECLARE_PUBLIC(LaserDocument)
public:
    LaserDocumentPrivate(LaserDocument* ptr)
        : ILaserDocumentItemPrivate(ptr, LNT_DOCUMENT)
        , isOpened(false)
        , scene(nullptr)
        , enablePrintAndCut(false)
        , useSpecifiedOrigin(false)
        , specifiedOriginIndex(0)
        //, boundingRect(0, 0, Config::SystemRegister::xMaxLength(), Config::SystemRegister::yMaxLength())
    {}
    QMap<QString, LaserPrimitive*> primitives;
    QList<LaserLayer*> layers;
    //PageInformation pageInfo;
    bool isOpened;
    LaserScene* scene;
    FinishRunType finishRun;
    SizeUnit unit;

    bool enablePrintAndCut;
    QTransform transform;
    PointPairList pointPairs;

    QMap<LaserPrimitiveType, int> typeMax;

    QRect bounding;
    QRect engravingBounding;

    bool useSpecifiedOrigin;
    int specifiedOriginIndex;
    QImage thumbnail;
};

LaserDocument::LaserDocument(LaserScene* scene, QObject* parent)
    : QObject(parent)
    , ILaserDocumentItem(LNT_DOCUMENT, new LaserDocumentPrivate(this))
{
    Q_D(LaserDocument);
    d->scene = scene;
    if (d->scene)
        d->scene->setDocument(this);
	init();
}

LaserDocument::~LaserDocument()
{
    close();
}

void LaserDocument::addPrimitive(LaserPrimitive* item, bool addToQuadTree, bool updateDocBounding)
{
    Q_D(LaserDocument);
    d->primitives.insert(item->id(), item);
    LaserLayer* layer = nullptr;
    if (item->layerIndex() >= 0 && item->layerIndex() < d->layers.size()) {
        layer = d->layers[item->layerIndex()];
    }
    else {
        return;
    }
    if (layer->isEmpty())
    {
        if (item->isShape())
        {
            layer->setType(LLT_CUTTING);
        }
        else if (item->isBitmap())
        {
            layer->setType(LLT_ENGRAVING);
        }
        layer->init();
    }
	//layer->addPrimitive(item);
    addPrimitive(item, layer, addToQuadTree, updateDocBounding);
}

void LaserDocument::addPrimitive(LaserPrimitive* item, LaserLayer* layer, bool addToQuadTree, bool updateDocBounding)
{
    Q_D(LaserDocument);
    if (item->layer()) {
        item->layer()->removePrimitive(item);
    }
    //not in d->primitives
    else {
        d->primitives.insert(item->id(), item);
    }
    layer->addPrimitive(item);
    if (d->scene)
        d->scene->addLaserPrimitive(item, addToQuadTree);
    if (updateDocBounding)
        updateDocumentBounding();
    updateLayersStructure();
}

void LaserDocument::removePrimitive(LaserPrimitive* item, bool keepLayer, bool updateDocBounding)
{
    Q_D(LaserDocument);
    if (keepLayer) {
        item->layer()->removePrimitive(item, true);
    }
    else {
        item->layer()->removePrimitive(item, false);
    }
    
    d->primitives.remove(item->id());
    if (d->scene)
        d->scene->removeLaserPrimitive(item);
    if (updateDocBounding)
        updateDocumentBounding();
    updateLayersStructure();
}

QImage LaserDocument::thumbnail() const
{
    Q_D(const LaserDocument);
    return d->thumbnail;
}

void LaserDocument::setThumbnail(const QImage& image)
{
    Q_D(LaserDocument);
    d->thumbnail = image;
}

QMap<QString, LaserPrimitive*> LaserDocument::primitives() const
{
    Q_D(const LaserDocument);
    return d->primitives;
}

LaserPrimitive* LaserDocument::laserPrimitive(const QString& id) const
{
    Q_D(const LaserDocument);
    return d->primitives[id];
}

QList<LaserPrimitive*> LaserDocument::selectedPrimitives() const
{
	Q_D(const LaserDocument);
	QList<LaserPrimitive*>list;
	for each(LaserPrimitive* item in d->primitives) {
		if (item->isSelected()) {
			list.append(item);
		}

	}
	return list;
}

bool LaserDocument::useSpecifiedOrigin() const
{
    Q_D(const LaserDocument);
    return d->useSpecifiedOrigin;
}

void LaserDocument::setUseSpecifiedOrigin(bool value)
{
    Q_D(LaserDocument);
    d->useSpecifiedOrigin = value;
}

int LaserDocument::specifiedOriginIndex() const
{
    Q_D(const LaserDocument);
    return d->specifiedOriginIndex;
}

void LaserDocument::setSpecifiedOriginIndex(int value)
{
    Q_D(LaserDocument);
    d->specifiedOriginIndex = value;
}

QList<LaserLayer*> LaserDocument::layers() const
{
    Q_D(const LaserDocument);
    return d->layers;
}

void LaserDocument::addLayer(LaserLayer* layer)
{
    Q_D(LaserDocument);
    d->layers.append(layer);

    updateLayersStructure();
}

void LaserDocument::removeLayer(LaserLayer* layer)
{
    Q_D(LaserDocument);
    LaserLayer* initLayer = nullptr;

    int i = d->layers.indexOf(layer);
    if (i < 2)
        return;
    d->layers.removeOne(layer);

    updateLayersStructure();
}

QString LaserDocument::newLayerName() const
{
    Q_D(const LaserDocument);
    QString prefix(tr("Layer"));

    int n = d->layers.size() + 1;
    bool used = true;
    QString name;
    while (used)
    {
        used = false;
        name = prefix + QString::number(n);
        for (QList<LaserLayer*>::const_iterator i = d->layers.begin(); i != d->layers.end(); i++)
        {
            if ((*i)->name() == name)
            {
                used = true;
                break;
            }
        }
        n++;
    }
    return name;
}

void LaserDocument::exportJSON(const QString& filename, const PathOptimizer::Path& path, ProgressItem* parentProgress, bool exportJson)
{
    Q_D(LaserDocument);

    updateDocumentBounding();

    QList<LaserLayer*> layerList;
    QMap<LaserLayer*, QList<OptimizeNode*>> layersMap;
    for (OptimizeNode* pathNode : path)
    {
        LaserPrimitive* primitive = pathNode->primitive();
        LaserLayer* layer = primitive->layer();
        layersMap[layer].append(pathNode);
        if (!layerList.contains(layer))
        {
            layerList.append(layer);
        }
    }

    ProgressItem* exportProgress = new ProgressItem(tr("Export Json"), ProgressItem::PT_Complex, parentProgress);
    exportProgress->setMaximum(path.count());

    bool absolute = Config::Device::startFrom() == SFT_AbsoluteCoords;
    QTransform t = enablePrintAndCut() ? transform() : QTransform();
    t = t * LaserApplication::device->to1stQuad();
    
    QRect docBounding = currentDocBoundingRect();
    QRect docBoundingAcc = currentEngravingBoundingRect(false);
    //if (!docBoundingAcc.isValid())
        //docBoundingAcc = docBounding;
    QPoint startPos;
    int deviceOriginIndex = Config::SystemRegister::deviceOrigin();
    if (useSpecifiedOrigin())
    {
        deviceOriginIndex = specifiedOriginIndex();
    }

    QPoint lastPoint;
    if (absolute)
    {
        docBounding = t.mapRect(docBounding);
        docBoundingAcc = t.mapRect(docBoundingAcc);
        switch (deviceOriginIndex)
        {
        case 0:
            startPos = docBounding.topLeft();
            break;
        case 1:
            startPos = QPoint(docBounding.left(), docBounding.top() + docBounding.height());
            break;
        case 2:
            startPos = QPoint(docBounding.left() + docBounding.width(), docBounding.top() + docBounding.height());
            break;
        case 3:
            startPos = QPoint(docBounding.left() + docBounding.width(), docBounding.top());
            break;
        }
        lastPoint = startPos;
    }
    else
    {
        lastPoint = this->jobOriginOnDocBoundingRect();
        lastPoint = t.map(lastPoint);
        startPos = QPoint(0, 0);
    }

    int startFrom = 0;
    switch (Config::Device::startFrom())
    {
    case SFT_CurrentPosition:
        startFrom = 0;
        break;
    case SFT_UserOrigin:
        startFrom = Config::Device::userOriginSelected() + 1;
        break;
    case SFT_AbsoluteCoords:
        startFrom = 4;
        QPoint offset = docBounding.topLeft() - startPos;
        docBounding.moveTo(offset);
        offset = docBoundingAcc.topLeft() - startPos;
        docBoundingAcc.moveTo(offset);
        break;
    }

    QList<QPoint> boundingPoints;
    if (absolute)
    {
        boundingPoints = machiningUtils::boundingPoints(deviceOriginIndex, docBounding);
    }
    else
    {
        boundingPoints = machiningUtils::boundingPoints(
            Config::Device::jobOrigin(), docBounding, startPos
        );
    }
    utils::makePointsRelative(boundingPoints, QPoint(0, 0));

    QByteArray thumbnailData;
    QBuffer thumbnailBuffer(&thumbnailData);
    thumbnailBuffer.open(QIODevice::WriteOnly);
    d->thumbnail.save(&thumbnailBuffer, "PNG");
    QString thumbnail64 = QString::fromLatin1(thumbnailData.toBase64());

    QByteArray ba = QByteArray::fromBase64(thumbnail64.toLatin1());
    QImage thmb = QImage::fromData(ba);
    thmb.save("tmp/thumbnail_restore.png", "PNG");

    QJsonObject jsonObj;
    QJsonObject laserDocumentInfo;
    laserDocumentInfo["APIVersion"] = LaserApplication::driver->getVersion();
    laserDocumentInfo["CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    laserDocumentInfo["PrinterDrawUnit"] = 1016;
    laserDocumentInfo["FinishRun"] = d->finishRun;
    laserDocumentInfo["StartFrom"] = startFrom;
    laserDocumentInfo["StartFromPos"] = typeUtils::point2Json(startPos);
    laserDocumentInfo["JobOrigin"] = Config::Device::jobOrigin();
    laserDocumentInfo["DeviceOrigin"] = Config::SystemRegister::deviceOrigin();
    laserDocumentInfo["Origin"] = typeUtils::point2Json(startPos);
    laserDocumentInfo["BoundingRect"] = typeUtils::rect2Json(docBounding, Config::Device::switchToU(), true);
    laserDocumentInfo["BoundingRectAcc"] = typeUtils::rect2Json(docBoundingAcc, Config::Device::switchToU(), true);
    laserDocumentInfo["MaxEngravingSpeed"] = maxEngravingSpeed();
    laserDocumentInfo["SoftwareVersion"] = LaserApplication::softwareVersion();
    laserDocumentInfo["BoundingPoints"] = typeUtils::pointsToJson(boundingPoints);
    laserDocumentInfo["Thumbnail"] = thumbnail64;

    jsonObj["LaserDocumentInfo"] = laserDocumentInfo;

    QJsonArray layers;
    
    
    for (LaserLayer* layer : layerList)
    {
        if (!layer->exportable())
            continue;

        if (layer->isEmpty())
            continue;

        QJsonObject layerObj;
        QJsonObject paramObj;
        QJsonArray items;
        QJsonObject engravingParamObj;
        QJsonObject cuttingParamObj;
        QJsonObject fillingParamObj;

        engravingParamObj["RunSpeed"] = layer->engravingRunSpeed();
        engravingParamObj["LaserPower"] = layer->engravingLaserPower();
        engravingParamObj["MinSpeedPower"] = layer->engravingMinSpeedPower();
        engravingParamObj["RunSpeedPower"] = layer->engravingRunSpeedPower();
        engravingParamObj["CarveForward"] = layer->engravingForward();
        engravingParamObj["CarveStyle"] = layer->engravingStyle();

        cuttingParamObj["RunSpeed"] = layer->cuttingRunSpeed();
        cuttingParamObj["MinSpeedPower"] = layer->cuttingMinSpeedPower();
        cuttingParamObj["RunSpeedPower"] = layer->cuttingRunSpeedPower();

        fillingParamObj["RunSpeed"] = layer->fillingRunSpeed();
        fillingParamObj["MinSpeedPower"] = layer->fillingMinSpeedPower();
        fillingParamObj["RunSpeedPower"] = layer->fillingRunSpeedPower();
        fillingParamObj["RowInterval"] = layer->fillingRowInterval();

        paramObj["EngravingParams"] = engravingParamObj;
        paramObj["CuttingParams"] = cuttingParamObj;
        paramObj["FillingParams"] = fillingParamObj;
        layerObj["Params"] = paramObj;

        for (OptimizeNode* pathNode : layersMap[layer])
        {
            LaserPrimitive* primitive = pathNode->primitive();

            QJsonObject itemObj;
            itemObj["Name"] = pathNode->nodeName();
            if (layer->type() == LLT_ENGRAVING)
            {
                if (!enablePrintAndCut())
                {
                    itemObj["Style"] = LaserLayerType::LLT_ENGRAVING;

                    ProgressItem* progress = new ProgressItem(QObject::tr("%1 Engraving").arg(primitive->name()), ProgressItem::PT_Complex, exportProgress);
                    QByteArray data = primitive->engravingImage(progress, QPoint());
                    if (!data.isEmpty())
                    {
                        QRect boundingRect = t.mapRect(primitive->sceneBoundingRect());
                        itemObj["X"] = boundingRect.topLeft().x() - lastPoint.x();
                        itemObj["Y"] = boundingRect.topLeft().y() - lastPoint.y();
                        itemObj["Type"] = primitive->typeLatinName();
                        itemObj["ImageType"] = "PNG";
                        itemObj["Data"] = QString(data.toBase64());
                        lastPoint = boundingRect.topLeft();
                        items.append(itemObj);
                    }
                }

                if (layer->engravingEnableCutting())
                {
                    QJsonObject itemObjCutting;
                    itemObjCutting["Name"] = pathNode->nodeName() + "_cutting";
                    itemObjCutting["Type"] = primitive->typeLatinName();
                    itemObjCutting["Style"] = LaserLayerType::LLT_CUTTING;
                    pathNode->nearestPoint(LaserPoint(lastPoint));
                    LaserPointListList points = primitive->arrangedPoints();
                    itemObjCutting["Data"] = QString(machiningUtils::pointListList2Plt(nullptr, points, lastPoint, t));
                    items.append(itemObjCutting);
                }
            }
            else if (layer->type() == LLT_CUTTING)
            {
                LaserPointListList points = primitive->arrangedPoints();
                if (!points.empty())
                {
                    itemObj["Type"] = primitive->typeLatinName();
                    itemObj["Style"] = LaserLayerType::LLT_CUTTING;
                    ProgressItem* progress = new ProgressItem(QObject::tr("%1 Points to Plt").arg(primitive->name()), ProgressItem::PT_Simple, exportProgress);
                    itemObj["Data"] = QString(machiningUtils::pointListList2Plt(progress, points, lastPoint, t));
                    items.append(itemObj);
                }
            }
            else if (layer->type() == LLT_FILLING)
            {
                if (!enablePrintAndCut())
                {
                    itemObj["Type"] = primitive->typeLatinName();
                    ProgressItem* progress = new ProgressItem(QObject::tr("%1 Lines to Plt").arg(primitive->name()), ProgressItem::PT_Complex, exportProgress);
                    if (layer->fillingType() == FT_Line)
                    {
                        LaserLineListList lineList = primitive->generateFillData();
                        QByteArray data = machiningUtils::lineList2Plt(progress, lineList, QPoint());
                        if (!data.isEmpty())
                        {
                            QRect boundingRect = t.mapRect(primitive->sceneBoundingRect());
                            itemObj["X"] = boundingRect.topLeft().x() - lastPoint.x();
                            itemObj["Y"] = boundingRect.topLeft().y() - lastPoint.y();
                            itemObj["Type"] = primitive->typeLatinName();
                            itemObj["Style"] = LaserLayerType::LLT_FILLING;
                            itemObj["Data"] = QString(data);
                            lastPoint = boundingRect.topLeft();
                            items.append(itemObj);
                        }
                    }
                    else if (layer->fillingType() == FT_Pixel)
                    {
                        QByteArray data = primitive->filling(progress, QPoint());
                        if (!data.isEmpty())
                        {
                            QRect boundingRect = t.mapRect(primitive->sceneBoundingRect());
                            itemObj["X"] = boundingRect.topLeft().x() - lastPoint.x();
                            itemObj["Y"] = boundingRect.topLeft().y() - lastPoint.y();
                            itemObj["Type"] = "Bitmap";
                            itemObj["ImageType"] = "PNG";
                            itemObj["Data"] = QString(data.toBase64());
                            itemObj["Style"] = LaserLayerType::LLT_ENGRAVING;
                            lastPoint = boundingRect.topLeft();
                            items.append(itemObj);
                        }
                    }
                }

                if (layer->fillingEnableCutting())
                {
                    QJsonObject itemObjCutting;
                    itemObjCutting["Name"] = pathNode->nodeName() + "_cutting";
                    itemObjCutting["Type"] = primitive->typeLatinName();
                    itemObjCutting["Style"] = LaserLayerType::LLT_CUTTING;
                    pathNode->nearestPoint(LaserPoint(lastPoint));
                    LaserPointListList points = primitive->arrangedPoints();
                    itemObjCutting["Data"] = QString(machiningUtils::pointListList2Plt(nullptr, points, lastPoint, t));
                    items.append(itemObjCutting);
                }
            }

        }
        layerObj["Items"] = items;
        layers.append(layerObj);
    }

    jsonObj["Layers"] = layers;
    QJsonDocument jsonDoc(jsonObj);

    QByteArray rawJson;
    if (exportJson)
    {
        rawJson = jsonDoc.toJson(QJsonDocument::Indented);
        QFile saveFile(filename);
        if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            qWarning("Couldn't open save file.");
            return;
        }
        qint64 writtenBytes = saveFile.write(rawJson);
        saveFile.close();
    }
    else
    {
        rawJson = jsonDoc.toJson(QJsonDocument::Compact);
    }

    exportProgress->finish();
    emit exportFinished(rawJson);
}

void LaserDocument::generateBoundingPrimitive()
{
}

bool LaserDocument::isOpened() const
{
    Q_D(const LaserDocument);
    return d->isOpened;
}

LaserScene* LaserDocument::scene() const
{
    Q_D(const LaserDocument);
    return d->scene;
}

void LaserDocument::swapLayers(int i, int j)
{
    Q_D(LaserDocument);
    LaserLayer* layer = d->layers[i];
    d->layers[i] = d->layers[j];
    d->layers[j] = layer;
    updateLayersStructure();
}

void LaserDocument::bindLayerButtons(const QList<LayerButton*>& layerButtons)
{
    Q_D(LaserDocument);
    for (int i = 0; i < Config::Layers::maxLayersCount(); i++)
    {
        d->layers[i]->bindButton(layerButtons[i], i);

    }
    updateLayersStructure();
}

FinishRunType& LaserDocument::finishRun()
{
    Q_D(LaserDocument);
    return d->finishRun;
}

void LaserDocument::setFinishRun(const FinishRunType& value)
{
    Q_D(LaserDocument);
    d->finishRun = value;
}

SizeUnit LaserDocument::unit() const
{
    Q_D(const LaserDocument);
    return d->unit;
}

void LaserDocument::setUnit(SizeUnit unit)
{
    Q_D(LaserDocument);
    d->unit = unit;
}

int LaserDocument::maxEngravingSpeed() const
{
    Q_D(const LaserDocument);
    int maxSpeed = 0;
    for (LaserLayer* layer : d->layers)
    {
        if (layer->engravingRunSpeed() > maxSpeed)
            maxSpeed = layer->engravingRunSpeed();
    }
    return maxSpeed;
}

bool LaserDocument::isEmpty() const
{
    Q_D(const LaserDocument);
    return d->primitives.isEmpty();
}

QPoint LaserDocument::reletiveJobOrigin() const
{
    Q_D(const LaserDocument);
    int dx, dy;
    switch (Config::Device::jobOrigin())
    {
    case 0:
        dx = 0;
        dy = 0;
        break;
    case 1:
        dx = d->bounding.width() / 2;
        dy = 0;
        break;
    case 2:
        dx = d->bounding.width();
        dy = 0;
        break;
    case 3:
        dx = 0;
        dy = d->bounding.height() / 2;
        break;
    case 4:
        dx = d->bounding.width() / 2;
        dy = d->bounding.height() / 2;
        break;
    case 5:
        dx = d->bounding.width();
        dy = d->bounding.height() / 2;
        break;
    case 6:
        dx = 0;
        dy = d->bounding.height();
        break;
    case 7:
        dx = d->bounding.width() / 2;
        dy = d->bounding.height();
        break;
    case 8:
        dx = d->bounding.width();
        dy = d->bounding.height();
        break;
    }
    return QPoint(dx, dy);
}

QPoint LaserDocument::jobOriginOnDocBoundingRect() const
{
    Q_D(const LaserDocument);
    QPoint jobOrigin = reletiveJobOrigin();
    return d->bounding.topLeft() + jobOrigin;
}

QRect LaserDocument::absoluteBoundingRect() const
{
    Q_D(const LaserDocument);
    return d->bounding;
}

QRect LaserDocument::absoluteEngravingBoundingRect(bool withAcc) const
{
    Q_D(const LaserDocument);
    QRect bounding = d->engravingBounding;
    if (bounding.isValid() && withAcc)
    {
        int accLength = LaserApplication::device->engravingAccLength(maxEngravingSpeed());
        bounding.setRight(bounding.right() + accLength);
        bounding.setLeft(bounding.left() - accLength);
    }
    return bounding;
}

QRect LaserDocument::currentDocBoundingRect() const
{
    return currentBoundingRect(absoluteBoundingRect());
}

QRect LaserDocument::currentEngravingBoundingRect(bool withAcc) const
{
    return currentBoundingRect(absoluteEngravingBoundingRect(withAcc));
}

QRect LaserDocument::currentBoundingRect(const QRect& rect) const
{
    Q_D(const LaserDocument);
    QRect bounding = rect;
    if (!bounding.isValid())
        return rect;
    if (Config::Device::startFrom() != SFT_AbsoluteCoords)
    {
        QPoint jobOrigin = this->jobOriginOnDocBoundingRect();
        QPoint offset =  bounding.topLeft() - jobOrigin;
        bounding.moveTopLeft(offset);
    }
    return bounding;
}

QPoint LaserDocument::docOrigin() const
{
    QPoint origin(0, 0);
    if (Config::Device::startFrom() == SFT_AbsoluteCoords)
        origin = LaserApplication::device->origin();
    else
    {
        origin = jobOriginOnDocBoundingRect();
    }
    return origin;
}

QTransform LaserDocument::transformToReletiveOrigin() const
{
    QTransform transform;
    QPointF origin;
    if (Config::Device::startFrom() == SFT_UserOrigin)
        origin = LaserApplication::device->userOrigin().toPoint();
    else if (Config::Device::startFrom() == SFT_CurrentPosition)
        origin = LaserApplication::device->laserPosition();
    QPointF jobOrigin = jobOriginOnDocBoundingRect();
    QPointF offset = origin - jobOrigin;
    transform = QTransform::fromTranslate(offset.x(), offset.y());
    return transform;
}

bool LaserDocument::enablePrintAndCut() const
{
    Q_D(const LaserDocument);
    return d->enablePrintAndCut;
}

void LaserDocument::setEnablePrintAndCut(bool value)
{
    Q_D(LaserDocument);
    d->enablePrintAndCut = value;
}

QTransform LaserDocument::transform() const
{
    Q_D(const LaserDocument);
    return d->transform;
}

void LaserDocument::setTransform(const QTransform& t)
{
    Q_D(LaserDocument);
    d->transform = t;
}

PointPairList LaserDocument::printAndCutPointPairs() const
{
    Q_D(const LaserDocument);
    return d->pointPairs;
}

void LaserDocument::setPrintAndCutPointPairs(const PointPairList& pairs)
{
    Q_D(LaserDocument);
    d->pointPairs = pairs;
}

void LaserDocument::transform(const QTransform& trans)
{
    Q_D(LaserDocument);
    for (LaserPrimitive* primitive : d->primitives)
    {
        QTransform t = primitive->transform() * trans;
        primitive->setTransform(t);
    }
}

LaserLayer* LaserDocument::idleLayer() const
{
    Q_D(const LaserDocument);
    LaserLayer* idle = nullptr;
    for (LaserLayer* layer : d->layers)
    {
        qLogD << layer->isEmpty();
        if (layer->isEmpty())
        {
            idle = layer;
            break;
        }
    }
    if (!idle)
    {
        idle = d->layers.last();
    }
    return idle;
}

void LaserDocument::updateLayersStructure()
{
    Q_D(LaserDocument);
    emit layersStructureChanged();
}

void LaserDocument::destroy()
{
    deleteLater();
}

void LaserDocument::open()
{
    Q_D(LaserDocument);
    updateDocumentBounding();
    if (d->scene)
        d->scene->updateTree();
    d->isOpened = true;
    emit opened();
}

void LaserDocument::close()
{
    Q_D(LaserDocument);
    if (d->isOpened)
    {
        d->primitives.clear();
        destroy();
        d->isOpened = false;
        emit closed();
    }
}

void LaserDocument::outline(ProgressItem* parentProgress)
{
    Q_D(LaserDocument);
    qLogD << "Before outline:";
    ProgressItem* clearProgress = new ProgressItem(tr("Clear tree"), ProgressItem::PT_Simple, parentProgress);
    ProgressItem* outlineProgress = new ProgressItem(tr("Outline by layers"), ProgressItem::PT_Simple, parentProgress);
    clearTree(d->optimizeNode, clearProgress);
#ifdef _DEBUG
    //printOutline(d->optimizeNode, 0);
#endif
    outlineByLayers(d->optimizeNode, outlineProgress);
    qLogD << "After outline:";
#ifdef _DEBUG
    //printOutline(d->optimizeNode, 0);
#endif

    emit outlineUpdated();
}

void LaserDocument::printOutline(OptimizeNode* node, int level)
{
    Q_D(LaserDocument);
    QString space = "";
    for (int i = 0; i < level; i++)
    {
        space.append("  ");
    }
#ifdef _DEBUG
    qLogD << space << node->nodeName();
#endif

    for (OptimizeNode* item : node->childNodes())
    {
        printOutline(item, level + 1);
    }
}

void LaserDocument::save(const QString& filename, QWidget* window)
{
	QJsonDocument doc;
	QJsonObject obj;
	QJsonArray array;
	QList<LaserLayer*> layers = this->layers();

	for(int i = 0; i < layers.size(); i ++)
	{
		LaserLayer* layer = layers[i];
		QJsonObject layerObj = layer->toJson(window);
		if (layerObj.isEmpty()) {
			continue;
		}
		layerObj.insert("index", i);
		array.append(layerObj);
	}
	obj.insert("layers", array);
    obj.insert("deviceOrigin", typeUtils::point2Json(LaserApplication::device->originOffset()));

	doc.setObject(obj);
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QFile::Truncate)) {
		return;
	}

	file.write(doc.toJson());
	file.close();
}

void LaserDocument::load(const QString& filename, QWidget* window)
{
    Q_D(LaserDocument);
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)){
		return;
	}
	QJsonParseError *error = new QJsonParseError;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), error);
	if (error->error != QJsonParseError::NoError)
	{
		qDebug() << "parseJson:" << error->errorString();
		return;
	}

    QJsonObject docObject = doc.object();
    QPoint originOffset = LaserApplication::device->originOffset();
    if (docObject.contains("deviceOrigin"))
    {
        originOffset = typeUtils::json2Point(docObject["deviceOrigin"]);
    }

    QJsonArray layers;
    if (docObject.contains("layers"))
    {
        layers = doc.object()["layers"].toArray();
    }

	QList<LaserLayer*> laserLayers = this->layers();
    QList<LaserPrimitive*> unavailables;
    this->blockSignals(true);
	for (int i = 0; i < layers.size(); i++) {
		QJsonObject layer = layers[i].toObject();
		QJsonArray array = layer["primitives"].toArray();
		
		//layer
		/*LaserLayerType type = (LaserLayerType)layer["type"].toInt();
		qDebug() << layer["name"].toString();
		LaserLayer* laserLayer = new LaserLayer(layer["name"].toString(), type, this, true);
		//laserLayer.
		this->addLayer(laserLayer);*/
		int index = layer["index"].toInt();
		if (index < 0 || index > laserLayers.size() - 1) {
			QMessageBox::critical(window, "critical", "your layer index have changed");
			qLogD << "your layer index have changed";
			return;
		}
        if (layer.contains("name")) {
            laserLayers[index]->setName(layer.value("name").toString());
        }
        
        if (layer.contains("cuttingRunSpeed"))
        {
            laserLayers[index]->setCuttingRunSpeed(layer.value("cuttingRunSpeed").toInt());
        }
        if (layer.contains("cuttingMinSpeedPower"))
        {
            laserLayers[index]->setCuttingMinSpeedPower(layer.value("cuttingMinSpeedPower").toDouble());
        }
        if (layer.contains("cuttingRunSpeedPower"))
        {
            laserLayers[index]->setCuttingRunSpeedPower(layer.value("cuttingRunSpeedPower").toDouble());
        }
        if (layer.contains("engravingRunSpeed"))
        {
            laserLayers[index]->setEngravingRunSpeed(layer.value("engravingRunSpeed").toInt());
        }
        if (layer.contains("engravingLaserPower"))
        {
            laserLayers[index]->setEngravingLaserPower(layer.value("engravingLaserPower").toDouble());
        }
        if (layer.contains("engravingMinSpeedPower"))
        {
            laserLayers[index]->setEngravingMinSpeedPower(layer.value("engravingMinSpeedPower").toDouble());
        }
        if (layer.contains("engravingRunSpeedPower"))
        {
            laserLayers[index]->setEngravingRunSpeedPower(layer.value("engravingRunSpeedPower").toDouble());
        }
        if (layer.contains("engravingRowInterval")) 
        {
            laserLayers[index]->setEngravingRowInterval(layer.value("engravingRowInterval").toInt());
        }
        if (layer.contains("engravingEnableCutting")) 
        {
            laserLayers[index]->setEngravingEnableCutting(layer.value("engravingEnableCutting").toBool());
        }
        if (layer.contains("fillingRunSpeed"))
        {
            laserLayers[index]->setFillingRunSpeed(layer.value("fillingRunSpeed").toInt());
        }
        if (layer.contains("fillingMinSpeedPower"))
        {
            laserLayers[index]->setFillingMinSpeedPower(layer.value("fillingMinSpeedPower").toDouble());
        }
        if (layer.contains("fillingRunSpeedPower"))
        {
            laserLayers[index]->setFillingRunSpeedPower(layer.value("fillingRunSpeedPower").toDouble());
        }
        if (layer.contains("fillingRowInterval")) 
        {
            laserLayers[index]->setFillingRowInterval(layer.value("fillingRowInterval").toInt());
        }
        if (layer.contains("fillingEnableCutting")) 
        {
            laserLayers[index]->setFillingEnableCutting(layer.value("fillingEnableCutting").toBool());
        }
        if (layer.contains("fillingType"))
        {
            laserLayers[index]->setFillingType(layer.value("fillingType").toInt());
        }
        if (layer.contains("errorX"))
        {
            laserLayers[index]->setErrorX(layer.value("errorX").toInt());
        }
        if (layer.contains("useHalftone"))
        {
            laserLayers[index]->setUseHalftone(layer.value("useHalftone").toBool());
        }
        if (layer.contains("lpi"))
        {
            laserLayers[index]->setLpi(layer.value("lpi").toInt());
        }
        if (layer.contains("dpi"))
        {
            laserLayers[index]->setDpi(layer.value("dpi").toInt());
        }
        if (layer.contains("halftoneAngles"))
        {
            laserLayers[index]->setHalftoneAngles(layer.value("halftoneAngles").toDouble());
        }

		//primitive
		for (int j = 0; j < array.size(); j++) {
			QJsonObject primitiveJson = array[j].toObject();
			QString className = primitiveJson["className"].toString();
            QString name = primitiveJson["name"].toString();
			int layerIndex = primitiveJson["layerIndex"].toInt();
			//postion
			//transform
			QJsonArray matrixArray = primitiveJson["matrix"].toArray();
			QTransform transform;
			transform.setMatrix(matrixArray[0].toDouble(), matrixArray[1].toDouble(), matrixArray[2].toDouble(),
				matrixArray[3].toDouble(), matrixArray[4].toDouble(), matrixArray[5].toDouble(),
				matrixArray[6].toDouble(), matrixArray[7].toDouble(), matrixArray[8].toDouble());
			//parent transform
			QTransform transformP;
			QJsonValue matrixPJson = primitiveJson["parentMatrix"];
			if (!matrixPJson.isNull()) {
				QJsonArray matrixPArray = matrixPJson.toArray();
				transformP.setMatrix(matrixPArray[0].toDouble(), matrixPArray[1].toDouble(), matrixPArray[2].toDouble(),
					matrixPArray[3].toDouble(), matrixPArray[4].toDouble(), matrixPArray[5].toDouble(),
					matrixPArray[6].toDouble(), matrixPArray[7].toDouble(), matrixPArray[8].toDouble());
			}
			QTransform saveTransform = transform * transformP;
			//
            LaserPrimitive* primitive = nullptr;
			if (className == "LaserEllipse" || className == "LaserRect" || className == "LaserBitmap") {
				//bounds
				QJsonArray boundsArray = primitiveJson["bounds"].toArray();
				QRect bounds = QRect(boundsArray[0].toDouble(), boundsArray[1].toDouble(), boundsArray[2].toDouble(), boundsArray[3].toDouble());
				if (className == "LaserEllipse") {
					primitive = new LaserEllipse(bounds, this, saveTransform, layerIndex);
				}
				else if (className == "LaserRect") {
                    qreal cornerRadius = primitiveJson["cornerRadius"].toDouble();
					primitive = new LaserRect(bounds, cornerRadius, this, saveTransform, layerIndex);
				}
				else if (className == "LaserBitmap") {
					//bounds
					QJsonArray boundsArray = primitiveJson["bounds"].toArray();
					QRect bounds = QRect(boundsArray[0].toDouble(), boundsArray[1].toDouble(), boundsArray[2].toDouble(), boundsArray[3].toDouble());
					//image
					QByteArray array = QByteArray::fromBase64(primitiveJson["image"].toString().toLatin1());
					
					QImage img = QImage::fromData(array, "tiff");

					qDebug() << img.size();
					primitive = new LaserBitmap(img, bounds, this, saveTransform, layerIndex);
				}
			}
			else if (className == "LaserLine") {
				
				//line
				QJsonArray lineArray = primitiveJson["line"].toArray();
				QPoint p1 = QPoint(lineArray[0].toInt(), lineArray[1].toInt());
				QPoint p2 = QPoint(lineArray[2].toInt(), lineArray[3].toInt());

				primitive = new LaserLine(QLine(p1, p2), this, saveTransform, layerIndex);
			}
			else if (className == "LaserPolyline" || className == "LaserPolygon") {
				QJsonArray polyArray = primitiveJson["poly"].toArray();
				QVector<QPoint> vector;
				for each(QJsonValue point in polyArray) {
					QJsonArray pointArray = point.toArray();
					vector.append(QPoint(pointArray[0].toInt(), pointArray[1].toInt()));
				}
				//LaserPrimitive * poly;
				if (className == "LaserPolyline") {
					primitive = new LaserPolyline(QPolygon(vector), this, saveTransform, layerIndex);
				}
				else if (className == "LaserPolygon") {
					primitive = new LaserPolygon(QPolygon(vector), this, saveTransform, layerIndex);
				}
				
            }
            else if (className == "LaserText") {
                QString content = primitiveJson["content"].toString();
                QJsonArray startPos = primitiveJson["startPos"].toArray();
                QJsonObject fontObj = primitiveJson["font"].toObject();
                QFont font(fontObj["family"].toString());
                font.setPixelSize(fontObj["size"].toDouble());
                font.setBold(fontObj["bold"].toBool());
                font.setItalic(fontObj["italic"].toBool());
                font.setLetterSpacing(QFont::SpacingType(fontObj["letterSpaceTpye"].toInt()), fontObj["spaceX"].toDouble());
                //font.setWordSpacing(fontObj["spaceY"].toDouble());
                //create
                LaserText* text = new LaserText(this, QPointF(startPos[0].toDouble(), startPos[1].toDouble()), 
                    font, fontObj["spaceY"].toDouble(),
                    fontObj["alignH"].toInt(), fontObj["alignV"].toInt(), saveTransform, layerIndex);
                text->setContent(content);
                text->modifyPathList();   
                primitive = text;
            }
            else if (className == "LaserPath") {
                QByteArray buffer = QByteArray::fromBase64(primitiveJson["path"].toString().toLatin1());
                QPainterPath path;
                QDataStream stream(buffer);
                stream >> path;
                primitive = new LaserPath(path, this, saveTransform, layerIndex);
            }
            else if (className == "LaserStar") {
                QJsonArray cArray = primitiveJson["center"].toArray();
                qreal radius = primitiveJson["radius"].toDouble();
                QPoint center(cArray[0].toInt(), cArray[1].toInt());
                bool stampIntaglio = primitiveJson["stampIntaglio"].toBool();
                primitive = new LaserStar(this, center, radius, stampIntaglio, saveTransform, layerIndex);
            }
            else if (className == "LaserPartyEmblem") {
                QJsonArray cArray = primitiveJson["center"].toArray();
                qreal radius = primitiveJson["radius"].toDouble();
                QPoint center(cArray[0].toInt(), cArray[1].toInt());
                bool stampIntaglio = primitiveJson["stampIntaglio"].toBool();
                primitive = new LaserPartyEmblem(this, center, radius, stampIntaglio, saveTransform, layerIndex);
            }
            else if (className == "LaserRing") {
                QJsonArray boundsArray = primitiveJson["bounds"].toArray();
                qreal width = primitiveJson["width"].toDouble();
                QRectF bounds(boundsArray[0].toDouble(), boundsArray[1].toDouble(), boundsArray[2].toDouble(), boundsArray[3].toDouble());
                bool stampIntaglio = primitiveJson["stampIntaglio"].toBool();
                primitive = new LaserRing(this, bounds, width, stampIntaglio, saveTransform, layerIndex);
            }
            else if (className == "LaserFrame") {
                QJsonArray boundsArray = primitiveJson["bounds"].toArray();
                qreal width = primitiveJson["width"].toDouble();
                QRect bounds(boundsArray[0].toInt(), boundsArray[1].toInt(), boundsArray[2].toInt(), boundsArray[3].toInt());
                int cornerType = primitiveJson["cornerType"].toInt();
                qreal cornerRadius = primitiveJson["cornerRadius"].toDouble();
                bool stampIntaglio = primitiveJson["stampIntaglio"].toBool();
                primitive = new LaserFrame(this, bounds, width, cornerRadius, stampIntaglio, saveTransform, layerIndex, cornerType);
            }
            else if (className == "LaserHorizontalText") {
                QJsonArray sizeArray = primitiveJson["size"].toArray();
                QSize size(sizeArray[0].toInt(), sizeArray[1].toInt());
                QString content = primitiveJson["content"].toString();
                QJsonArray bLArray = primitiveJson["bottomLeft"].toArray();
                QPointF bL(bLArray[0].toDouble(), bLArray[1].toDouble());
                qreal space = primitiveJson["space"].toDouble();
                bool bold = primitiveJson["bold"].toBool();
                bool italic = primitiveJson["italic"].toBool();
                bool uppercase = primitiveJson["uppercase"].toBool();
                bool stampIntaglio = primitiveJson["stampIntaglio"].toBool();
                QString family = primitiveJson["family"].toString();
                primitive = new LaserHorizontalText(this, content, size, bL, bold, italic, uppercase, stampIntaglio, family,space, saveTransform, layerIndex);
            }
            else if (className == "LaserVerticalText") {
                QJsonArray sizeArray = primitiveJson["size"].toArray();
                QSize size(sizeArray[0].toInt(), sizeArray[1].toInt());
                QString content = primitiveJson["content"].toString();
                QJsonArray bLArray = primitiveJson["topLeft"].toArray();
                QPointF bL(bLArray[0].toDouble(), bLArray[1].toDouble());
                qreal space = primitiveJson["space"].toDouble();
                bool bold = primitiveJson["bold"].toBool();
                bool italic = primitiveJson["italic"].toBool();
                bool uppercase = primitiveJson["uppercase"].toBool();
                QString family = primitiveJson["family"].toString();
                bool fill = primitiveJson["stampIntaglio"].toBool();
                primitive = new LaserVerticalText(this, content, size, bL, bold, italic, uppercase,fill,family, space, saveTransform, layerIndex);
            }
            else if (className == "LaserCircleText") {
                QJsonArray sizeArray = primitiveJson["size"].toArray();
                QSize size(sizeArray[0].toInt(), sizeArray[1].toInt());
                QString content = primitiveJson["content"].toString();
                qreal angle = primitiveJson["angle"].toDouble();
                qreal maxRadian = primitiveJson["maxRadian"].toDouble();
                qreal minRadian = primitiveJson["minRadian"].toDouble();
                QJsonArray boundsArray = primitiveJson["bounds"].toArray();
                QRectF bounds(boundsArray[0].toDouble(), boundsArray[1].toDouble(), boundsArray[2].toDouble(), boundsArray[3].toDouble());
                bool bold = primitiveJson["bold"].toBool();
                bool italic = primitiveJson["italic"].toBool();
                bool uppercase = primitiveJson["uppercase"].toBool();
                QString family = primitiveJson["family"].toString();
                bool stampIntaglio = primitiveJson["stampIntaglio"].toBool();
                qreal space = primitiveJson["space"].toDouble();
                primitive = new LaserCircleText(this, content, bounds, angle, bold,italic, uppercase, stampIntaglio,family, space, false, maxRadian, minRadian,size, saveTransform, layerIndex);
            }
            
            if (primitive)
            {
                if (primitive->isAvailable())
                    addPrimitive(primitive, false, false);
                else
                    unavailables.append(primitive);

                QStringList segs = name.split("_");
                if (segs.length() == 2)
                {
                    LaserPrimitiveType type = primitive->primitiveType();
                    bool ok = false;
                    int maxValue = segs[1].toInt(&ok);
                    if (ok)
                    {
                        int typeMax = 0;
                        if (d->typeMax.contains(type))
                        {
                            typeMax = d->typeMax[type];
                        }
                        if (maxValue > typeMax)
                        {
                            d->typeMax[type] = maxValue;
                        }
                    }
                }
            }
		}
        if (layer.contains("visible")) {
            bool bl = layer.value("visible").toBool();
            laserLayers[index]->setVisible(bl);
        }
        if (layer.contains("exportable")) {
            bool exportable = layer.value("exportable").toBool();
            laserLayers[index]->setExportable(exportable);
        }
        if (layer.contains("type")) {
            laserLayers[index]->setType(static_cast<LaserLayerType>(layer.value("type").toInt()));
        }
	}
    if (!unavailables.isEmpty())
    {
        QMessageBox::warning(LaserApplication::mainWindow, tr("Warning"),
            tr("Found unavailable primitives, count is %1. These primitives will not be loaded.").arg(unavailables.count()));
    }

    if (originOffset != LaserApplication::device->originOffset())
    {
        QPoint offset = originOffset - LaserApplication::device->originOffset();
        QTransform t = QTransform::fromTranslate(offset.x(), offset.y());
        transform(t);
    }
    this->blockSignals(false);
    emit updateLayersStructure();
    open();
}

int LaserDocument::totalNodes()
{
    Q_D(LaserDocument);
    QStack<OptimizeNode*> stack;
    stack.push(d->optimizeNode);
    int count = 0;
    while (!stack.isEmpty())
    {
        OptimizeNode* node = stack.pop();
        count++;
        for (OptimizeNode* child : node->childNodes())
        {
            stack.push(child);
        }
    }
    return count;
}

void LaserDocument::updateDocumentBounding()
{
    Q_D(LaserDocument);
    utils::boundingRect(d->primitives.values(), d->bounding, d->engravingBounding);
}

QMap<LaserLayer*, QImage> LaserDocument::generateStampImages()
{
    QMap<LaserLayer*, QImage>images;
    
    
    int i = 0;
    for (LaserLayer* layer : layers()) {
        if (layer->primitives().size() == 0) {
            continue;
        }
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
        
        QRect boundingRectInDevice;
        QList<LaserPrimitive*> pList = layer->primitives();
        
        utils::boundingRect(pList, boundingRectInDevice);
        qreal offset = 2000;
        boundingRectInDevice = QRect(boundingRectInDevice .left()-offset,  boundingRectInDevice.top() - offset, 
            boundingRectInDevice.width() + 2*offset, boundingRectInDevice.height() + 2 * offset);
        qreal ratio = boundingRectInDevice.width() * 1.0 / boundingRectInDevice.height();
        
        
        int canvasWidth = qMin(boundingRectInDevice.width(), maxImageSize);
        int canvasHeight = qRound(canvasWidth / ratio);
        if (ratio < 1)
        {
            canvasHeight = qMin(boundingRectInDevice.height(), maxImageSize);
            canvasWidth = qRound(canvasHeight * ratio);
        }

        QTransform t1 = QTransform::fromScale(
            canvasWidth * 1.0 / boundingRectInDevice.width(),
            canvasHeight * 1.0 / boundingRectInDevice.height()
        );
        boundingRectInDevice = t1.mapRect(boundingRectInDevice);
        QTransform t2 = QTransform::fromTranslate(-boundingRectInDevice.x(), -boundingRectInDevice.y());
        boundingRectInDevice = t2.mapRect(boundingRectInDevice);
        
        QImage image(boundingRectInDevice.width(), boundingRectInDevice.height(), QImage::Format_ARGB32);
        image.fill(Qt::white);
        for (LaserPrimitive* p : layer->primitives()) {
            if(!p->isStamepPrimitive()){
                continue;
            }
            QPainter painter(&image);
            //绘制印章外面的基准线
            computeStampBasePath(p, painter, offset, t1, t2);
            painter.setPen(Qt::NoPen);
            if (p->stampIntaglio()) {
                int type = p->primitiveType();
                QPainterPath pPath = p->getPath();
                //QPainterPath innerPath;
                painter.setBrush(Qt::white);
                pPath = p->sceneTransform().map(pPath);
                pPath = t1.map(pPath);
                pPath = t2.map(pPath);
                painter.drawPath(pPath);
                if (type == LPT_FRAME) {
                    LaserFrame* frame = qgraphicsitem_cast<LaserFrame*>(p);
                    if (frame->isInner()) {
                        pPath = frame->innerPath();
                    }
                    else {
                        pPath = frame->outerPath();
                    }
                    painter.setBrush(Qt::black);
                    pPath = p->sceneTransform().map(pPath);
                    pPath = t1.map(pPath);
                    pPath = t2.map(pPath);
                    painter.drawPath(pPath);
                }
                else if (type == LPT_RING) {
                    
                    LaserRing* ring = qgraphicsitem_cast<LaserRing*>(p);
                    if (ring->isInner()) {
                        pPath = ring->innerPath();
                    }
                    else {
                        pPath = ring->outerPath();
                    }
                    painter.setBrush(Qt::black);
                    pPath = p->sceneTransform().map(pPath);
                    pPath = t1.map(pPath);
                    pPath = t2.map(pPath);
                    painter.drawPath(pPath);
                }
                               
            }
            else {
                painter.setBrush(Qt::black);
                QPainterPath pPath = p->getPath();
                pPath = p->sceneTransform().map(pPath);
                pPath = t1.map(pPath);
                pPath = t2.map(pPath);
                painter.drawPath(pPath);
                painter.setBrush(Qt::white);
            }
            painter.setBrush(Qt::NoBrush);
        }
        QString fileName = "tmp/images/"+QString::number(i)+"_img.png";
        image.save(fileName);
        /*cv::Mat src(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());

        int dpi = layer->dpi();
        int pixelWidth = boundingRectInDevice.width() * dpi / 25400.0;
        int pixelHeight = boundingRectInDevice.height() * dpi / 25400.0;

        int pixelInterval = layer->engravingRowInterval();
        int outWidth = pixelWidth;
        int outHeight = qCeil(boundingRectInDevice.height() * 1.0 / pixelInterval);
        cv::Mat resized;
        cv::resize(src, resized, cv::Size(outWidth, outHeight), 0.0, 0.0, cv::INTER_CUBIC);

        cv::imwrite("tmp/images/" + QString(i).toStdString() + "_resized.png", resized);*/

        images.insert(layer, image);
        i++;
    }
    return images;
}

void LaserDocument::computeStampBasePath(LaserPrimitive* primitive, QPainter& painter, qreal offset, QTransform t1, QTransform t2)
{
    int type = primitive->primitiveType();
    QPen pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);
    QPainterPath path;
    if (type == LPT_FRAME) {
        LaserFrame* frame = qgraphicsitem_cast<LaserFrame*>(primitive);
        if (frame->isInner()) {
            return;
        }
        path = frame->outerPath();
    }
    else if (type == LPT_RING) {
        LaserRing* ring = qgraphicsitem_cast<LaserRing*>(primitive);
        if (ring->isInner()) {
            return;
        }
        path = ring->outerPath();
    }
    else {
        return;
    }
    QRectF bounds = path.boundingRect();
    qreal ratioX = (bounds.width() + 2 * offset) / bounds.width();
    qreal ratioY = (bounds.height() + 2 * offset) / bounds.height();
    path = primitive->sceneTransform().map(path);
    path = t1.map(path);
    path = t2.map(path);
    QPointF originalCenter = path.boundingRect().center();
    path = QTransform::fromScale(ratioX, ratioY).map(path);
    QPointF center = path.boundingRect().center();
    //qreal x = -path.boundingRect().left();
    //qreal y = -path.boundingRect().top();
    qreal x = originalCenter.x() - center.x();
    qreal y = originalCenter.y() - center.y();
    path = QTransform::fromTranslate(x, y).map(path);
    painter.drawPath(path);
}

void LaserDocument::init()
{
	Q_D(LaserDocument);
	d->name = tr("Untitled");

	/*QString layerName = newLayerName();
	LaserLayer* layer = new LaserLayer(layerName, LLT_ENGRAVING, this, true);
    layer->setIndex(0);
	addLayer(layer);
    
	layerName = newLayerName();
	layer = new LaserLayer(layerName, LLT_CUTTING, this, true);
    layer->setIndex(1);
	addLayer(layer);*/

	for (int i = 0; i < Config::Layers::maxLayersCount(); i++)
	{
		QString layerName = newLayerName();
		LaserLayer* layer = new LaserLayer(layerName, LLT_ENGRAVING, this);
        layer->setIndex(i);
		addLayer(layer);
	}

	ADD_TRANSITION(documentEmptyState, documentWorkingState, this, SIGNAL(opened()));
	ADD_TRANSITION(documentWorkingState, documentEmptyState, this, SIGNAL(closed()));

    connect(LaserApplication::mainWindow->viewer(), &LaserViewer::selectedChangedFromToolBar,
        this, &LaserDocument::updateDocumentBounding);
    connect(LaserApplication::mainWindow->viewer(), &LaserViewer::selectedChangedFromMouse,
        this, &LaserDocument::updateDocumentBounding);
}

void LaserDocument::outlineByLayers(OptimizeNode* node, ProgressItem* progress)
{
    Q_D(LaserDocument);
    progress->setMaximum(d->primitives.count());
    for (LaserLayer* layer : d->layers)
    {
        if (layer->isEmpty())
            continue;
        d->optimizeNode->addChildNode(layer->optimizeNode());
        for (LaserPrimitive* primitive : layer->primitives())
        {
            addPrimitiveToNodesTree(primitive, layer->optimizeNode());
            progress->increaseProgress();
        }
        //optimizeGroups(layer->optimizeNode());
    }
    progress->finish();
}

void LaserDocument::outlineByGroups(OptimizeNode* node)
{
    if (node->nodeType() == LNT_DOCUMENT)
    {
        for (LaserPrimitive* primitive : primitives())
        {
            addPrimitiveToNodesTree(primitive, this->optimizeNode());
        }
    }
}

void LaserDocument::clearTree(OptimizeNode* node, ProgressItem* progress)
{
    Q_D(LaserDocument);
    QStack<OptimizeNode*> stack;
    stack.push(node);
    QList<OptimizeNode*> deletingNodes;
    progress->setMaximum(d->primitives.count());
    while (!stack.isEmpty())
    {
        OptimizeNode* topNode = stack.pop();
        progress->increaseProgress();
        for (OptimizeNode* childNode : topNode->childNodes())
        {
            stack.push(childNode);
        }
        topNode->clearChildren();
        if (topNode->nodeType() == LNT_VIRTUAL)
            deletingNodes.append(topNode);
    }
    if (!deletingNodes.isEmpty())
        qDeleteAll(deletingNodes);
    progress->finish();
}

//void LaserDocument::optimizeGroups(OptimizeNode* node, int level, bool sorted)
//{
//    qLogD << "node " << node->nodeName() << " has " << node->childCount() << " child nodes.";
//    if (!node->hasChildren())
//    {
//        return;
//    }
//
//    QList<OptimizeNode*> children = node->childNodes();
//    if (!sorted)
//    {
//        // 将当前节点下的子节点按行或列分到不同的组中
//        QMap<int, QList<OptimizeNode*>> childrenMap;
//        for (OptimizeNode* node : node->childNodes())
//        {
//            int groupIndex = 0;
//            if (Config::PathOptimization::groupingOrientation() == Qt::Horizontal)
//            {
//                groupIndex = node->currentPos().y() / Config::PathOptimization::groupingGridInterval();
//            }
//            else if (Config::PathOptimization::groupingOrientation() == Qt::Vertical)
//            {
//                groupIndex = node->positionInDevice().x() / Config::PathOptimization::groupingGridInterval();
//            }
//            childrenMap[groupIndex].append(node);
//        }
//        qLogD << "  child nodes were seperated into " << childrenMap.size() << " groups by grid.";
//
//        if (childrenMap.size() > 1)
//        {
//            // 清空当前的子节点列表。
//            node->childNodes().clear();
//
//            // 加入新排列的子节点列表
//            for (QMap<int, QList<OptimizeNode*>>::Iterator i = childrenMap.begin(); i != childrenMap.end(); i++)
//            {
//                OptimizeNode* newNode = new OptimizeNode();
//                QString nodeName = QString("vnode_%1_%2").arg(level).arg(node->childNodes().count() + 1);
//                newNode->setNodeName(nodeName);
//                newNode->addChildNodes(i.value());
//                node->addChildNode(newNode);
//
//                // 递归调用
//                //optimizeGroups(newNode, level + 1);
//            }
//        }
//
//        // 获取当前节点的所有子节点，进行排序。
//        children = node->childNodes();
//        qSort(children.begin(), children.end(),
//            [=](OptimizeNode* a, OptimizeNode* b) -> bool {
//                if (Config::PathOptimization::groupingOrientation() == Qt::Horizontal)
//                {
//                    return a->positionInDevice().x() < b->positionInDevice().x();
//                }
//                else if (Config::PathOptimization::groupingOrientation() == Qt::Vertical)
//                {
//                    return a->positionInDevice().y() < b->positionInDevice().y();
//                }
//                return false;
//            }
//        );
//    }
//
//    // 获取每个分组中子节点的最大个数。
//    int maxChildNodes = Config::PathOptimization::maxGroupSize();
//    // 如果当前节点下的子节点数大于允许的最大个数，则进行分拆。即在该父节点下，每maxChildNodes个子节点将会
//    // 新建一个父节点，将该父节点作为当前父节点的子节点。
//    if (children.count() > maxChildNodes)
//    {
//        node->childNodes().clear();
//        OptimizeNode* newNode = nullptr;
//        for (int i = 0, count = 0; i < children.count(); i++)
//        {
//            if ((count++ % maxChildNodes) == 0)
//            {
//                newNode = new OptimizeNode();
//                QString nodeName = QString("vnode_%1_%2").arg(level).arg(node->childNodes().count() + 1);
//                newNode->setNodeName(nodeName);
//                node->addChildNode(newNode);
//            }
//            newNode->addChildNode(children.at(i));
//        }
//        qLogD << "  child nodes were seperated into " << node->childNodes().size() << " groups by maxChildNodes.";
//
//        // 如果子对象数量还是多于最大数，则再递归处理
//        if (node->childCount() > maxChildNodes)
//            optimizeGroups(node, level, true);
//    }
//
//    // 对每一个子节点再次递归进行整理。
//    for (OptimizeNode* item : node->childNodes())
//    {
//        optimizeGroups(item, level + 1);
//    }
//}

//void LaserDocument::clearOutline(OptimizeNode* node, bool clearLayers)
//{
//    if (node->hasChildren())
//    {
//        for (OptimizeNode* node : node->childNodes())
//        {
//            clearOutline(node);
//        }
//    }
//
//    node->clearChildren();
//    if (node->nodeType() == LNT_DOCUMENT && !clearLayers)
//    {
//        LaserDocument* doc = static_cast<LaserDocument*>(node->documentItem());
//        for (LaserLayer* layer : doc->layers())
//        {
//            if (layer->isAvailable())
//                layer->optimizeNode()->addChildNode(layer->optimizeNode());
//        }
//    }
//
//}

void LaserDocument::addPrimitiveToNodesTree(LaserPrimitive* primitive, OptimizeNode* node)
{
    if (!node->hasChildren())
    {
        node->addChildNode(primitive->optimizeNode());
        return;
    }

    for (int i = node->childNodes().length() - 1; i >= 0; i--)
    {
        OptimizeNode* childNode = node->childNodes()[i];
        if (childNode->nodeType() != LNT_PRIMITIVE)
            continue;
        LaserPrimitive* childPrimitive = static_cast<LaserPrimitive*>(childNode->documentItem());

        RELATION rel = utils::determineRelationship(primitive->outline(), childPrimitive->outline());
        //qDebug().noquote() << primitive->nodeName() << childPrimitive->nodeName() << rel;
        if (rel == A_CONTAINS_B)
        {
            primitive->optimizeNode()->addChildNode(childPrimitive->optimizeNode());
            node->removeChildNode(childPrimitive->optimizeNode());
        }
        else if (rel == B_CONTAINS_A)
        {
            addPrimitiveToNodesTree(primitive, childNode);
            return;
        }
    }
    node->addChildNode(primitive->optimizeNode());
}

QString LaserDocument::newPrimitiveName(LaserPrimitive* primitive)
{
    Q_D(LaserDocument);

    LaserPrimitiveType type = primitive->primitiveType();
    QString typeName = primitive->typeName();

    int maxCount = 0;
    if (d->typeMax.contains(type))
    {
        maxCount = d->typeMax[type];
        
    }
    maxCount++;
    QString name = QString("%1_%2").arg(typeName).arg(maxCount);
    d->typeMax[type] = maxCount;
    return name;
}


