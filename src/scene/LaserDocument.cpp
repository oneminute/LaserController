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
    QTransform printAndCutTransform;
    PointPairList pointPairs;

    QMap<LaserPrimitiveType, int> typeMax;

    QRectF bounding;
    QRectF boundingWithAccSpan;
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
	d->layers[item->layerIndex()]->addPrimitive(item);
}

void LaserDocument::addPrimitive(LaserPrimitive* item, LaserLayer* layer)
{
    item->layer()->removePrimitive(item);
    layer->addPrimitive(item);
    updateLayersStructure();
}

void LaserDocument::removePrimitive(LaserPrimitive* item)
{
    Q_D(LaserDocument);
    item->layer()->removePrimitive(item);
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

LaserLayer* LaserDocument::defaultCuttingLayer() const
{
    Q_D(const LaserDocument);
    return d->layers[1];
}

LaserLayer* LaserDocument::defaultEngravingLayer() const
{
    Q_D(const LaserDocument);
    return d->layers[0];
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

void LaserDocument::exportJSON(const QString& filename, ProgressItem* parentProgress)
{
    Q_D(LaserDocument);

    PathOptimizer optimizer(d->optimizeNode, primitives().count());
    optimizer.optimize(parentProgress);
    PathOptimizer::Path path = optimizer.optimizedPath();

    QFile saveFile(filename);
    QJsonObject jsonObj;

    QTransform transformPrintAndCut = enablePrintAndCut() ? printAndCutTransform() : QTransform();
    QPointF docOrigin(0, 0);
    switch (Config::Device::startFrom())
    {
    case SFT_AbsoluteCoords:
    case SFT_CurrentPosition:
        docOrigin = QPointF(0, 0);
        break;
    case SFT_UserOrigin:
        docOrigin = LaserApplication::device->userOriginInDevice();
        break;
    }

    ProgressItem* exportProgress = LaserApplication::progressModel->createComplexItem(tr("Export Json"), parentProgress);
    QJsonObject laserDocumentInfo;
    laserDocumentInfo["APIVersion"] = LaserApplication::driver->getVersion();
    laserDocumentInfo["CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    laserDocumentInfo["PrinterDrawUnit"] = 1016;
    laserDocumentInfo["FinishRun"] = d->finishRun;
    laserDocumentInfo["StartFrom"] = Config::Device::startFrom();
    laserDocumentInfo["JobOrigin"] = Config::Device::jobOrigin();
    laserDocumentInfo["DeviceOrigin"] = Config::SystemRegister::deviceOrigin();
    laserDocumentInfo["Origin"] = typeUtils::point2Json(docOrigin);

    QRectF outBounding = machiningDocBoundingRectInDevice(true);
    if (Config::Device::startFrom() != SFT_AbsoluteCoords)
    {
        outBounding.moveTopLeft(-outBounding.topLeft());
        QPointF outJobOrigin = jobOriginReletiveInMech(true);
        outBounding.moveTopLeft(-outJobOrigin);
    }
    laserDocumentInfo["BoundingRect"] = typeUtils::rect2Json(outBounding);
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
    QPointF lastPoint;
    QPointF residual(0, 0);
    if (Config::Device::startFrom() != SFT_AbsoluteCoords)
        lastPoint = docOriginInDevice();
    
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
        /*layerObj["Type"] = layer->type();
        if (layer->type() == LLT_FILLING && layer->fillingType() == FT_Pixel)
            layerObj["Type"] = LLT_ENGRAVING;*/

        engravingParamObj["RunSpeed"] = layer->engravingRunSpeed() * 1000;
        engravingParamObj["LaserPower"] = qRound(layer->engravingLaserPower() * 10);
        engravingParamObj["MinSpeedPower"] = qRound(layer->engravingMinSpeedPower() * 10);
        engravingParamObj["RunSpeedPower"] = qRound(layer->engravingRunSpeedPower() * 10);
        engravingParamObj["CarveForward"] = layer->engravingForward();
        engravingParamObj["CarveStyle"] = layer->engravingStyle();

        cuttingParamObj["RunSpeed"] = layer->cuttingRunSpeed() * 1000;
        cuttingParamObj["MinSpeedPower"] = qRound(layer->cuttingMinSpeedPower() * 10);
        cuttingParamObj["RunSpeedPower"] = qRound(layer->cuttingRunSpeedPower() * 10);

        fillingParamObj["RunSpeed"] = layer->fillingRunSpeed() * 1000;
        fillingParamObj["MinSpeedPower"] = qRound(layer->fillingMinSpeedPower() * 10);
        fillingParamObj["RunSpeedPower"] = qRound(layer->fillingRunSpeedPower() * 10);
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
                    QRectF boundingRect = LaserApplication::device->transformSceneToDevice()
                        .mapRect(primitive->sceneBoundingRect());
                    itemObj["Width"] = boundingRect.width();
                    itemObj["Height"] = boundingRect.height();
                    itemObj["Style"] = LaserLayerType::LLT_ENGRAVING;
                    itemObj["Absolute"] = Config::Device::startFrom() == SFT_AbsoluteCoords;

                    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(QObject::tr("%1 Engraving").arg(primitive->name()), exportProgress);
                    QByteArray data = primitive->engravingImage(progress, lastPoint, residual);
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
                    itemObjCutting["Data"] = QString(machiningUtils::pointListList2Plt(nullptr, points, lastPoint, residual, transformPrintAndCut));
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
                    itemObj["Data"] = QString(machiningUtils::pointListList2Plt(progress, points, lastPoint, residual, transformPrintAndCut));
                    items.append(itemObj);
                }
            }
            else if (layer->type() == LLT_FILLING)
            {
                if (!enablePrintAndCut())
                {
                    itemObj["Type"] = primitive->typeLatinName();
                    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem(QObject::tr("%1 Lines to Plt").arg(primitive->name()), exportProgress);
                    if (layer->fillingType() == FT_Line)
                    {
                        LaserLineListList lineList = primitive->generateFillData(lastPoint);
                        QByteArray data = machiningUtils::lineList2Plt(progress, lineList, lastPoint, residual);
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
                            QRectF boundingRect = LaserApplication::device->transformSceneToDevice()
                                .mapRect(primitive->sceneBoundingRect());
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
                    itemObjCutting["Data"] = QString(machiningUtils::pointListList2Plt(nullptr, points, lastPoint, residual, transformPrintAndCut));
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

    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qWarning("Couldn't open save file.");
        return;
    }

    QByteArray rawJson = jsonDoc.toJson(QJsonDocument::Indented);
    qint64 writtenBytes = saveFile.write(rawJson);
    qDebug() << "written bytes:" << writtenBytes;

    LaserApplication::previewWindow->addMessage(tr("Done"));
    saveFile.close();
    exportProgress->finish();
    emit exportFinished(filename);
}

void LaserDocument::exportBoundingJSON()
{
    Q_D(LaserDocument);

    PathOptimizer optimizer(d->optimizeNode, primitives().count());

    QFile saveFile("tmp/bounding.json");
    QJsonObject jsonObj;

    QTransform t = enablePrintAndCut() ? printAndCutTransform() : QTransform();
    QPointF docOrigin(0, 0);
    switch (Config::Device::startFrom())
    {
    case SFT_AbsoluteCoords:
    case SFT_CurrentPosition:
        docOrigin = QPointF(0, 0);
        break;
    case SFT_UserOrigin:
        docOrigin = LaserApplication::device->userOriginInDevice();
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
    laserDocumentInfo["BoundingRect"] = typeUtils::rect2Json(machiningDocBoundingRectInDevice());
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
    paramObj["EngravingParams"] = engravingParamObj;
    paramObj["CuttingParams"] = cuttingParamObj;
    paramObj["FillingParams"] = fillingParamObj;
    layerObj["Params"] = paramObj;

    QJsonObject itemObj;
    itemObj["Name"] = "BoundingRect";
    int index = 0;
    bool isMiddle = false;
    QPointF lastPoint = docOriginInDevice();
    QPointF residual(0, 0);
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
    
    QRectF docBoundingRect = machiningDocBoundingRectInDevice();
    LaserPointList points;
    points.append(LaserPoint(docBoundingRect.topLeft()));
    points.append(LaserPoint(docBoundingRect.topRight()));
    points.append(LaserPoint(docBoundingRect.bottomRight()));
    points.append(LaserPoint(docBoundingRect.bottomLeft()));
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
        points2.insert(0, LaserPoint(lastPoint));
        if (Config::Device::jobOrigin() == 4)
        {
            points2.append(points2.first());
        }
        if (isMiddle)
        {
            points2.append(LaserPoint(lastPoint));
        }
    }
    
    itemObj["Type"] = "Rect";
    itemObj["Style"] = LaserLayerType::LLT_CUTTING;
    itemObj["Data"] = QString(machiningUtils::pointList2Plt(nullptr, points2, lastPoint, residual, t));
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

    QByteArray rawJson = jsonDoc.toJson(QJsonDocument::Indented);
    qint64 writtenBytes = saveFile.write(rawJson);
    qDebug() << "written bytes:" << writtenBytes;

    saveFile.close();
    //emit exportFinished(filename);
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

QPointF LaserDocument::jobOriginReletiveInScene(const QRectF& docBounding) const
{
    qreal dx, dy;
    switch (Config::Device::jobOrigin())
    {
    case 0:
        dx = 0;
        dy = 0;
        break;
    case 1:
        dx = docBounding.width() / 2;
        dy = 0;
        break;
    case 2:
        dx = docBounding.width();
        dy = 0;
        break;
    case 3:
        dx = 0;
        dy = docBounding.height() / 2;
        break;
    case 4:
        dx = docBounding.width() / 2;
        dy = docBounding.height() / 2;
        break;
    case 5:
        dx = docBounding.width();
        dy = docBounding.height() / 2;
        break;
    case 6:
        dx = 0;
        dy = docBounding.height();
        break;
    case 7:
        dx = docBounding.width() / 2;
        dy = docBounding.height();
        break;
    case 8:
        dx = docBounding.width();
        dy = docBounding.height();
        break;
    }
    return QPointF(dx, dy);
}

QPointF LaserDocument::jobOriginReletiveInScene(bool includingAccSpan) const
{
    return jobOriginReletiveInScene(docBoundingRectInScene(includingAccSpan));
}

QPointF LaserDocument::jobOriginReletiveInMech(bool includingAccSpan) const
{
    QPointF origin = jobOriginReletiveInScene(includingAccSpan);
    origin = Global::matrixToUm().map(origin);
    return origin;
}

QPointF LaserDocument::jobOriginInScene() const
{
    QRectF bounding = docBoundingRectInScene();
    QPointF jobOrigin = jobOriginReletiveInScene();
    return bounding.topLeft() + jobOrigin;
}

QPointF LaserDocument::jobOriginInDevice() const
{
    return LaserApplication::device->transformSceneToDevice().map(jobOriginInScene());
}

QPointF LaserDocument::jobOriginInMech() const
{
    return Global::matrixToUm().map(jobOriginInScene());
}

QRectF LaserDocument::docBoundingRectInScene(bool includingAccSpan) const
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

QRectF LaserDocument::docBoundingRectInMech() const
{
    return Global::matrixToUm().mapRect(docBoundingRectInScene());
}

QRectF LaserDocument::docBoundingRectInDevice(bool includingAccSpan) const
{
    return LaserApplication::device->transformSceneToDevice()
        .mapRect(docBoundingRectInScene(includingAccSpan));
}

QRectF LaserDocument::machiningDocBoundingRectInScene() const
{
    QRectF bounding = docBoundingRectInScene();
    if (Config::Device::startFrom() == SFT_CurrentPosition)
    {
        bounding.moveTopLeft(LaserApplication::device->userOriginInScene());
        QPointF offset = jobOriginReletiveInScene();
        bounding.moveTopLeft(-offset);
    }
    else if (Config::Device::startFrom() == SFT_CurrentPosition)
    {
        bounding.moveTopLeft(-bounding.topLeft());
        QPointF offset = jobOriginReletiveInScene();
        bounding.moveTopLeft(-offset);
    }
    return bounding;
}

QRectF LaserDocument::machiningDocBoundingRectInMech() const
{
    QRectF bounding = docBoundingRectInMech();
    if (Config::Device::startFrom() == SFT_UserOrigin)
    {
        QPointF userOrigin = LaserApplication::device->userOriginInMech();
        QPointF jobOrigin = jobOriginInMech();
        QPointF offset = userOrigin - jobOrigin;
        QPointF target = bounding.topLeft() + offset;
        bounding.moveTopLeft(target);
    }
    else if (Config::Device::startFrom() == SFT_CurrentPosition)
    {
        QPointF userOrigin = LaserApplication::device->currentOriginInMech();
        QPointF jobOrigin = jobOriginInMech();
        QPointF offset = userOrigin - jobOrigin;
        QPointF target = bounding.topLeft() + offset;
        bounding.moveTopLeft(target);
    }
    return bounding;
}

QRectF LaserDocument::machiningDocBoundingRectInDevice(bool includingAccSpan) const
{
    QRectF bounding = docBoundingRectInDevice(includingAccSpan);
    if (Config::Device::startFrom() == SFT_UserOrigin)
    {
        QPointF userOrigin = LaserApplication::device->userOriginInDevice();
        QPointF jobOrigin = jobOriginInDevice();
        QPointF offset = userOrigin - jobOrigin;
        QPointF target = bounding.topLeft() + offset;
        bounding.moveTopLeft(target);
    }
    else if (Config::Device::startFrom() == SFT_CurrentPosition)
    {
        QPointF userOrigin = LaserApplication::device->currentOriginInDevice();
        QPointF jobOrigin = jobOriginInDevice();
        QPointF offset = userOrigin - jobOrigin;
        QPointF target = bounding.topLeft() + offset;
        bounding.moveTopLeft(target);
    }
    return bounding;
}

QPointF LaserDocument::docOriginInScene() const
{
    QRectF bounding = docBoundingRectInScene();
    QPointF origin(0, 0);
    if (Config::Device::startFrom() == SFT_AbsoluteCoords)
        origin = LaserApplication::device->originInScene();
    else
    {
        origin = jobOriginInScene();
    }
    return origin;
}

QPointF LaserDocument::docOriginInMech() const
{
    QPointF origin(0, 0);
    if (Config::Device::startFrom() == SFT_AbsoluteCoords)
        origin = LaserApplication::device->absoluteOriginInMech();
    else
    {
        origin = jobOriginInMech();
    }
    return origin;
}

QPointF LaserDocument::docOriginInDevice() const
{
    QPointF origin(0, 0);
    if (Config::Device::startFrom() == SFT_AbsoluteCoords)
        origin = LaserApplication::device->originInDevice();
    else
    {
        origin = transformToReletiveOriginInDevice().map(jobOriginInDevice());
    }
    return origin;
}

QTransform LaserDocument::transformToReletiveOriginInDevice() const
{
    QTransform transform;
    QPointF origin;
    if (Config::Device::startFrom() == SFT_UserOrigin)
        origin = LaserApplication::device->userOriginInDevice();
    else if (Config::Device::startFrom() == SFT_CurrentPosition)
        origin = LaserApplication::device->laserPositionInDevice();
    QPointF jobOrigin = jobOriginInDevice();
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

QTransform LaserDocument::printAndCutTransform() const
{
    Q_D(const LaserDocument);
    return d->printAndCutTransform;
}

void LaserDocument::setPrintAndCutTransform(const QTransform& t)
{
    Q_D(LaserDocument);
    d->printAndCutTransform = t;
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
	QJsonArray layers = doc.object()["layers"].toArray();
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
        if (layer.contains("type")) {
            laserLayers[index]->setType(static_cast<LaserLayerType>(layer.value("type").toInt()));
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
				QRectF bounds = QRectF(boundsArray[0].toDouble(), boundsArray[1].toDouble(), boundsArray[2].toDouble(), boundsArray[3].toDouble());
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
					QRectF bounds = QRectF(boundsArray[0].toDouble(), boundsArray[1].toDouble(), boundsArray[2].toDouble(), boundsArray[3].toDouble());
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
				QPointF p1 = QPointF(lineArray[0].toDouble(), lineArray[1].toDouble());
				QPointF p2 = QPointF(lineArray[2].toDouble(), lineArray[3].toDouble());

				primitive = new LaserLine(QLineF(p1, p2), this, saveTransform, layerIndex);
			}
			else if (className == "LaserPolyline" || className == "LaserPolygon") {
				QJsonArray polyArray = primitiveJson["poly"].toArray();
				QVector<QPointF> vector;
				for each(QJsonValue point in polyArray) {
					QJsonArray pointArray = point.toArray();
					vector.append(QPointF(pointArray[0].toDouble(), pointArray[1].toDouble()));
				}
				//LaserPrimitive * poly;
				if (className == "LaserPolyline") {
					primitive = new LaserPolyline(QPolygonF(vector), this, saveTransform, layerIndex);
				}
				else if (className == "LaserPolygon") {
					primitive = new LaserPolygon(QPolygonF(vector), this, saveTransform, layerIndex);
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
                font.setWordSpacing(fontObj["spaceY"].toDouble());
                //create
                LaserText* text = new LaserText(this, QPointF(startPos[0].toDouble(), startPos[1].toDouble()), 
                    font, fontObj["alignH"].toInt(), fontObj["alignV"].toInt(), saveTransform, layerIndex);
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
	}
    if (!unavailables.isEmpty())
    {
        QMessageBox::warning(LaserApplication::mainWindow, tr("Warning"),
            tr("Found unavailable primitives, count is %1. These primitives will not be loaded.").arg(unavailables.count()));
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
	QString layerName = newLayerName();
	LaserLayer* layer = new LaserLayer(layerName, LLT_ENGRAVING, this, true);
    layer->setIndex(0);
	addLayer(layer);
    
	layerName = newLayerName();
	layer = new LaserLayer(layerName, LLT_CUTTING, this, true);
    layer->setIndex(1);
	addLayer(layer);

	for (int i = 2; i < Config::Layers::maxLayersCount(); i++)
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

void LaserDocument::optimizeGroups(OptimizeNode* node, int level, bool sorted)
{
    qLogD << "node " << node->nodeName() << " has " << node->childCount() << " child nodes.";
    if (!node->hasChildren())
    {
        return;
    }

    QList<OptimizeNode*> children = node->childNodes();
    if (!sorted)
    {
        // 将当前节点下的子节点按行或列分到不同的组中
        QMap<int, QList<OptimizeNode*>> childrenMap;
        for (OptimizeNode* node : node->childNodes())
        {
            int groupIndex = 0;
            if (Config::PathOptimization::groupingOrientation() == Qt::Horizontal)
            {
                groupIndex = node->positionInDevice().y() / Config::PathOptimization::groupingGridInterval();
            }
            else if (Config::PathOptimization::groupingOrientation() == Qt::Vertical)
            {
                groupIndex = node->positionInDevice().x() / Config::PathOptimization::groupingGridInterval();
            }
            childrenMap[groupIndex].append(node);
        }
        qLogD << "  child nodes were seperated into " << childrenMap.size() << " groups by grid.";

        if (childrenMap.size() > 1)
        {
            // 清空当前的子节点列表。
            node->childNodes().clear();

            // 加入新排列的子节点列表
            for (QMap<int, QList<OptimizeNode*>>::Iterator i = childrenMap.begin(); i != childrenMap.end(); i++)
            {
                OptimizeNode* newNode = new OptimizeNode();
                QString nodeName = QString("vnode_%1_%2").arg(level).arg(node->childNodes().count() + 1);
                newNode->setNodeName(nodeName);
                newNode->addChildNodes(i.value());
                node->addChildNode(newNode);

                // 递归调用
                //optimizeGroups(newNode, level + 1);
            }
        }

        // 获取当前节点的所有子节点，进行排序。
        children = node->childNodes();
        qSort(children.begin(), children.end(),
            [=](OptimizeNode* a, OptimizeNode* b) -> bool {
                if (Config::PathOptimization::groupingOrientation() == Qt::Horizontal)
                {
                    return a->positionInDevice().x() < b->positionInDevice().x();
                }
                else if (Config::PathOptimization::groupingOrientation() == Qt::Vertical)
                {
                    return a->positionInDevice().y() < b->positionInDevice().y();
                }
                return false;
            }
        );
    }

    // 获取每个分组中子节点的最大个数。
    int maxChildNodes = Config::PathOptimization::maxGroupSize();
    // 如果当前节点下的子节点数大于允许的最大个数，则进行分拆。即在该父节点下，每maxChildNodes个子节点将会
    // 新建一个父节点，将该父节点作为当前父节点的子节点。
    if (children.count() > maxChildNodes)
    {
        node->childNodes().clear();
        OptimizeNode* newNode = nullptr;
        for (int i = 0, count = 0; i < children.count(); i++)
        {
            if ((count++ % maxChildNodes) == 0)
            {
                newNode = new OptimizeNode();
                QString nodeName = QString("vnode_%1_%2").arg(level).arg(node->childNodes().count() + 1);
                newNode->setNodeName(nodeName);
                node->addChildNode(newNode);
            }
            newNode->addChildNode(children.at(i));
        }
        qLogD << "  child nodes were seperated into " << node->childNodes().size() << " groups by maxChildNodes.";

        // 如果子对象数量还是多于最大数，则再递归处理
        if (node->childCount() > maxChildNodes)
            optimizeGroups(node, level, true);
    }

    // 对每一个子节点再次递归进行整理。
    for (OptimizeNode* item : node->childNodes())
    {
        optimizeGroups(item, level + 1);
    }
}

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


