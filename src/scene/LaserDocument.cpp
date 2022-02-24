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
#include "algorithm/PathOptimizer.h"
#include "algorithm/OptimizeNode.h"
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
        , enablePrintAndCut(false)
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
    QRect boundingWithAccSpan;
};

LaserDocument::LaserDocument(LaserScene* scene, QObject* parent)
    : QObject(parent)
    , ILaserDocumentItem(LNT_DOCUMENT, new LaserDocumentPrivate(this))
{
    Q_D(LaserDocument);
    d->scene = scene;
    d->scene->setDocument(this);
	init();
}

LaserDocument::~LaserDocument()
{
    close();
}

void LaserDocument::addPrimitive(LaserPrimitive* item)
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
	layer->addPrimitive(item);
}

void LaserDocument::addPrimitive(LaserPrimitive* item, LaserLayer* layer)
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
    updateLayersStructure();
}

void LaserDocument::removePrimitive(LaserPrimitive* item, bool keepLayer)
{
    Q_D(LaserDocument);
    if (keepLayer) {
        item->layer()->removePrimitive(item, true);
    }
    else {
        item->layer()->removePrimitive(item, false);
    }
    
    d->primitives.remove(item->id());
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

//LaserLayer* LaserDocument::defaultCuttingLayer() const
//{
//    Q_D(const LaserDocument);
//    return d->layers[1];
//}
//
//LaserLayer* LaserDocument::defaultEngravingLayer() const
//{
//    Q_D(const LaserDocument);
//    return d->layers[0];
//}

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

void LaserDocument::exportJSON(const QString& filename, ProgressItem* parentProgress, bool exportJson)
{
    Q_D(LaserDocument);

    PathOptimizer optimizer(d->optimizeNode, primitives().count());
    optimizer.optimize(parentProgress);
    PathOptimizer::Path path = optimizer.optimizedPath();

    QJsonObject jsonObj;

    bool absolute = Config::Device::startFrom() == SFT_AbsoluteCoords;
    QTransform transformPrintAndCut = enablePrintAndCut() ? transform() : QTransform();
    QPoint docOrigin(0, 0);
    switch (Config::Device::startFrom())
    {
    case SFT_AbsoluteCoords:
    case SFT_CurrentPosition:
        docOrigin = QPoint(0, 0);
        break;
    case SFT_UserOrigin:
        docOrigin = LaserApplication::device->userOrigin().toPoint();
        break;
    }

    ProgressItem* exportProgress = LaserApplication::progressModel->createComplexItem(tr("Export Json"), parentProgress);
    QJsonObject laserDocumentInfo;
    laserDocumentInfo["APIVersion"] = LaserApplication::driver->getVersion();
    laserDocumentInfo["CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    laserDocumentInfo["PrinterDrawUnit"] = 1016;
    laserDocumentInfo["FinishRun"] = d->finishRun;
    laserDocumentInfo["StartFrom"] = Config::Device::startFrom();
    laserDocumentInfo["Absolute"] = absolute;
    laserDocumentInfo["JobOrigin"] = Config::Device::jobOrigin();
    laserDocumentInfo["DeviceOrigin"] = Config::SystemRegister::deviceOrigin();
    laserDocumentInfo["Origin"] = typeUtils::point2Json(docOrigin);

    QRect docBounding = currentDocBoundingRect();
    QRect docBoundingAcc = currentDocBoundingRect(true);
    int xOffset = docBoundingAcc.x() - docBounding.x();
    laserDocumentInfo["BoundingRect"] = typeUtils::rect2Json(docBounding, !absolute);
    laserDocumentInfo["BoundingRectAcc"] = typeUtils::rect2Json(docBoundingAcc, !absolute, xOffset);
    laserDocumentInfo["SoftwareVersion"] = LaserApplication::softwareVersion();

    jsonObj["LaserDocumentInfo"] = laserDocumentInfo;

    QList<LaserLayer*> layerList;
    QMap<LaserLayer*, QList<OptimizeNode*>> layersMap;
    exportProgress->setMaximum(path.count());
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

    QJsonArray layers;
    QPoint lastPoint;
    if (Config::Device::startFrom() != SFT_AbsoluteCoords)
        lastPoint = this->jobOriginOnDocBoundingRect();
    
    for (LaserLayer* layer : layerList)
    {
        if (!layer->exportable())
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
                itemObj["Absolute"] = Config::Device::startFrom() == SFT_AbsoluteCoords;
                if (!enablePrintAndCut())
                {
                    QRectF boundingRect = primitive->sceneBoundingRect();
                    QPoint boundingPoint = boundingRect.topLeft().toPoint();
                    if (Config::Device::startFrom() != SFT_AbsoluteCoords)
                        QPoint boundingPoint = boundingPoint - lastPoint;
                    itemObj["X"] = boundingPoint.x();
                    itemObj["Y"] = boundingPoint.y();
                    itemObj["Width"] = boundingRect.width();
                    itemObj["Height"] = boundingRect.height();
                    itemObj["Style"] = LaserLayerType::LLT_ENGRAVING;

                    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(QObject::tr("%1 Engraving").arg(primitive->name()), exportProgress);
                    QByteArray data = primitive->engravingImage(progress, lastPoint);
                    if (!data.isEmpty())
                    {
                        itemObj["Type"] = primitive->typeLatinName();
                        itemObj["ImageType"] = "PNG";
                        itemObj["Data"] = QString(data.toBase64());
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
                    itemObjCutting["Data"] = QString(machiningUtils::pointListList2Plt(nullptr, points, lastPoint, transformPrintAndCut));
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
                    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(QObject::tr("%1 Points to Plt").arg(primitive->name()), exportProgress);
                    itemObj["Data"] = QString(machiningUtils::pointListList2Plt(progress, points, lastPoint, transformPrintAndCut));
                    items.append(itemObj);
                }
            }
            else if (layer->type() == LLT_FILLING)
            {
                itemObj["Absolute"] = Config::Device::startFrom() == SFT_AbsoluteCoords;
                if (!enablePrintAndCut())
                {
                    itemObj["Type"] = primitive->typeLatinName();
                    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(QObject::tr("%1 Lines to Plt").arg(primitive->name()), exportProgress);
                    if (layer->fillingType() == FT_Line)
                    {
                        LaserLineListList lineList = primitive->generateFillData();
                        QByteArray data = machiningUtils::lineList2Plt(progress, lineList, lastPoint);
                        if (!data.isEmpty())
                        {
                            itemObj["Type"] = primitive->typeLatinName();
                            itemObj["Style"] = LaserLayerType::LLT_FILLING;
                            itemObj["Data"] = QString(data);
                            items.append(itemObj);
                        }
                    }
                    else if (layer->fillingType() == FT_Pixel)
                    {
                        QByteArray data = primitive->filling(progress, lastPoint);
                        if (!data.isEmpty())
                        {
                            QRectF boundingRect = primitive->sceneBoundingRect();
                            QPoint boundingPoint = boundingRect.topLeft().toPoint();
                            if (Config::Device::startFrom() != SFT_AbsoluteCoords)
                                QPoint boundingPoint = boundingPoint - lastPoint;
                            itemObj["X"] = boundingPoint.x();
                            itemObj["Y"] = boundingPoint.y();
                            itemObj["Width"] = boundingRect.width();
                            itemObj["Height"] = boundingRect.height();
                            itemObj["Type"] = "Bitmap";
                            itemObj["ImageType"] = "PNG";
                            itemObj["Data"] = QString(data.toBase64());
                            itemObj["Style"] = LaserLayerType::LLT_ENGRAVING;
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
                    itemObjCutting["Data"] = QString(machiningUtils::pointListList2Plt(nullptr, points, lastPoint, transformPrintAndCut));
                    items.append(itemObjCutting);
                }
            }

        }
        layerObj["Items"] = items;
        layers.append(layerObj);
    }

    QJsonObject actionObj;

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
        //qDebug() << "written bytes:" << writtenBytes;
        saveFile.close();
    }
    else
    {
        rawJson = jsonDoc.toJson(QJsonDocument::Compact);
    }

    LaserApplication::previewWindow->addMessage(tr("Done"));
    exportProgress->finish();
    //emit exportFinished(filename);
    emit exportFinished(rawJson);
}

void LaserDocument::exportBoundingJSON()
{
    Q_D(LaserDocument);

    PathOptimizer optimizer(d->optimizeNode, primitives().count());

    QFile saveFile("tmp/bounding.json");
    QJsonObject jsonObj;

    QTransform t = enablePrintAndCut() ? transform() : QTransform();
    QPoint docOrigin(0, 0);
    switch (Config::Device::startFrom())
    {
    case SFT_AbsoluteCoords:
    case SFT_CurrentPosition:
        docOrigin = QPoint(0, 0);
        break;
    case SFT_UserOrigin:
        docOrigin = LaserApplication::device->userOrigin().toPoint();
        break;
    }
    QJsonObject laserDocumentInfo;
    laserDocumentInfo["APIVersion"] = LaserApplication::driver->getVersion();
    laserDocumentInfo["CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    laserDocumentInfo["PrinterDrawUnit"] = 1016;
    laserDocumentInfo["FinishRun"] = d->finishRun;
    laserDocumentInfo["StartFrom"] = Config::Device::startFrom();
    laserDocumentInfo["JobOrigin"] = Config::Device::jobOrigin();
    laserDocumentInfo["DeviceOrigin"] = Config::SystemRegister::deviceOrigin();
    laserDocumentInfo["Origin"] = typeUtils::point2Json(docOrigin);
    laserDocumentInfo["BoundingRect"] = typeUtils::rect2Json(currentDocBoundingRect());
    laserDocumentInfo["BoundingRectAcc"] = typeUtils::rect2Json(currentDocBoundingRect(true));
    laserDocumentInfo["SoftwareVersion"] = LaserApplication::softwareVersion();

    jsonObj["LaserDocumentInfo"] = laserDocumentInfo;

    QJsonArray layers;

    QJsonObject layerObj;
    QJsonObject paramObj;
    QJsonArray items;
    QJsonObject engravingParamObj;
    QJsonObject cuttingParamObj;
    QJsonObject fillingParamObj;
    layerObj["Type"] = LLT_CUTTING;
    cuttingParamObj["RunSpeed"] = Config::UserRegister::cuttingMoveSpeed();
    cuttingParamObj["MinSpeedPower"] = 0;
    cuttingParamObj["RunSpeedPower"] = 0;

    engravingParamObj["RunSpeed"] = 0;
    engravingParamObj["LaserPower"] = 0;
    engravingParamObj["MinSpeedPower"] = 0;
    engravingParamObj["RunSpeedPower"] = 0;
    engravingParamObj["CarveForward"] = false;
    engravingParamObj["CarveStyle"] = 0;

    fillingParamObj["RunSpeed"] = 0;
    fillingParamObj["MinSpeedPower"] = 0;
    fillingParamObj["RunSpeedPower"] = 0;
    fillingParamObj["RowInterval"] = 0;

    paramObj["EngravingParams"] = engravingParamObj;
    paramObj["CuttingParams"] = cuttingParamObj;
    paramObj["FillingParams"] = fillingParamObj;
    layerObj["Params"] = paramObj;

    QJsonObject itemObj;
    itemObj["Name"] = "BoundingRect";
    int index = 0;
    bool isMiddle = false;
    QPoint lastPoint = this->docOrigin();
    if (Config::Device::startFrom() == SFT_AbsoluteCoords)
    {
        switch (Config::SystemRegister::deviceOrigin())
        {
        case 0:
            index = 0;
            break;
        case 3:
            index = 1;
            break;
        case 2:
            index = 2;
            break;
        case 1:
            index = 3;
            break;
        }
    }
    else
    {
        switch (Config::Device::jobOrigin())
        {
        case 0:
            index = 1;
            break;
        case 1:
            index = 1;
            isMiddle = true;
            break;
        case 2:
            index = 2;
            break;
        case 3:
            index = 0;
            isMiddle = true;
            break;
        case 4:
            index = 0;
            isMiddle = true;
            break;
        case 5:
            index = 2;
            isMiddle = true;
            break;
        case 6:
            index = 0;
            break;
        case 7:
            index = 3;
            isMiddle = true;
            break;
        case 8:
            index = 3;
            break;
        }
    }
    
    QRect docBoundingRect = absoluteDocBoundingRect();
    int docLeft = docBoundingRect.left();
    int docTop = docBoundingRect.top();
    int docRight = docLeft + docBoundingRect.width();
    int docBottom = docTop + docBoundingRect.height();
    LaserPointList points;
    points.append(LaserPoint(docLeft, docTop));
    points.append(LaserPoint(docRight, docTop));
    points.append(LaserPoint(docRight, docBottom));
    points.append(LaserPoint(docLeft, docBottom));
    qLogD << "bounding rect point index: " << index;
    LaserPointList points2;
    for (int i = 0; i < 4; i++)
    {
        index = (index + 4) % 4;
        points2.append(points.at(index));
        index++;
    }

    if (Config::Device::startFrom() == SFT_AbsoluteCoords)
    {
        points2.append(points2.first());
    }
    else
    {
        if (Config::Device::jobOrigin() == 4)
        {
            points2.append(points2.first());
        }
        points2.insert(0, LaserPoint(lastPoint));
        if (isMiddle)
        {
            points2.append(LaserPoint(lastPoint));
        }
    }
    
    itemObj["Type"] = "Rect";
    itemObj["Style"] = LaserLayerType::LLT_CUTTING;
    itemObj["Data"] = QString(machiningUtils::pointList2Plt(nullptr, points2, lastPoint, t));
    items.append(itemObj);

    layerObj["Items"] = items;
    layers.append(layerObj);

    QJsonObject actionObj;

    jsonObj["Layers"] = layers;

    QJsonDocument jsonDoc(jsonObj);

    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qWarning("Couldn't open save file.");
        return;
    }

    QByteArray rawJson = jsonDoc.toJson(QJsonDocument::Compact);
    //qint64 writtenBytes = saveFile.write(rawJson);
    //qDebug() << "written bytes:" << writtenBytes;

    saveFile.close();
    //emit exportFinished(filename);
    //qLogD << "rawJson: " << rawJson;
    emit exportFinished(rawJson);
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

QRect LaserDocument::absoluteDocBoundingRect(bool includingAccSpan) const
{
    Q_D(const LaserDocument);
    if (includingAccSpan)
    {
        return d->boundingWithAccSpan;
    }
    else
    {
        return d->bounding;
    }
}

QRect LaserDocument::currentDocBoundingRect(bool includingAccSpan) const
{
    Q_D(const LaserDocument);
    QRect bounding = absoluteDocBoundingRect(includingAccSpan);
    if (Config::Device::startFrom() == SFT_UserOrigin)
    {
        QPoint userOrigin = LaserApplication::device->userOrigin().toPoint();
        QPoint jobOrigin = this->jobOriginOnDocBoundingRect();
        QPoint offset = userOrigin - jobOrigin;
        QPoint target = bounding.topLeft() + offset;
        bounding.moveTopLeft(target);
    }
    else if (Config::Device::startFrom() == SFT_CurrentPosition)
    {
        QPoint userOrigin = LaserApplication::device->laserPosition();
        QPoint jobOrigin = this->jobOriginOnDocBoundingRect();
        QPoint offset = userOrigin - jobOrigin;
        QPoint target = bounding.topLeft() + offset;
        bounding.moveTopLeft(target);
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
    ProgressItem* clearProgress = LaserApplication::progressModel->createSimpleItem(tr("Clear tree"), parentProgress);
    ProgressItem* outlineProgress = LaserApplication::progressModel->createSimpleItem(tr("Outline by layers"), parentProgress);
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
                primitive = new LaserStar(this, center, radius, saveTransform, layerIndex);
            }
            else if (className == "LaserPartyEmblem") {
                QJsonArray cArray = primitiveJson["center"].toArray();
                qreal radius = primitiveJson["radius"].toDouble();
                QPoint center(cArray[0].toInt(), cArray[1].toInt());
                primitive = new LaserPartyEmblem(this, center, radius, saveTransform, layerIndex);
            }
            else if (className == "LaserRing") {
                QJsonArray boundsArray = primitiveJson["bounds"].toArray();
                qreal width = primitiveJson["width"].toDouble();
                QRectF bounds(boundsArray[0].toDouble(), boundsArray[1].toDouble(), boundsArray[2].toDouble(), boundsArray[3].toDouble());
                primitive = new LaserRing(this, bounds, width, saveTransform, layerIndex);
            }
            else if (className == "LaserFrame") {
                QJsonArray boundsArray = primitiveJson["bounds"].toArray();
                qreal width = primitiveJson["width"].toDouble();
                QRect bounds(boundsArray[0].toInt(), boundsArray[1].toInt(), boundsArray[2].toInt(), boundsArray[3].toInt());
                int cornerType = primitiveJson["cornerType"].toInt();
                qreal cornerRadius = primitiveJson["cornerRadius"].toDouble();
                primitive = new LaserFrame(this, bounds, width, cornerRadius, saveTransform, layerIndex, cornerType);
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
                bool fill = primitiveJson["fill"].toBool();
                QString family = primitiveJson["family"].toString();
                primitive = new LaserHorizontalText(this, content, size, bL, bold, italic, uppercase, fill, family,space, saveTransform, layerIndex);
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
                bool fill = primitiveJson["fill"].toBool();
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
                bool fill = primitiveJson["fill"].toBool();
                primitive = new LaserCircleText(this, content, bounds, angle, bold,italic, uppercase,fill,family, false, maxRadian, minRadian,size, saveTransform, layerIndex);
            }
            
            if (primitive)
            {
                if (primitive->isAvailable())
                    this->scene()->addLaserPrimitive(primitive, true);
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
    utils::boundingRect(d->primitives.values(), d->bounding, d->boundingWithAccSpan);
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


