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
    PageInformation pageInfo;
    bool isOpened;
    LaserScene* scene;
    FinishRun finishRun;
    SizeUnit unit;

    bool enablePrintAndCut;
    QTransform printAndCutTransform;
    PointPairList pointPairs;

    //QRectF boundingRect;
    QElapsedTimer boundingRectTimer;

    QMap<LaserPrimitiveType, int> typeMax;
};

LaserDocument::LaserDocument(LaserScene* scene, QObject* parent)
    : QObject(parent)
    , ILaserDocumentItem(LNT_DOCUMENT, new LaserDocumentPrivate(this))
{
    Q_D(LaserDocument);
    d->scene = scene;
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
    //item->QObject::deleteLater();

}

PageInformation LaserDocument::pageInformation() const
{
    Q_D(const LaserDocument);
    return d->pageInfo;
}

void LaserDocument::setPageInformation(const PageInformation& page)
{
    Q_D(LaserDocument);
    d->pageInfo = page;
}

QRectF LaserDocument::pageBounds() const
{
    Q_D(const LaserDocument);
    return QRectF(0, 0, d->pageInfo.width(), d->pageInfo.height());
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

void LaserDocument::exportJSON(const QString& filename)
{
    Q_D(LaserDocument);

    LaserApplication::previewWindow->registerProgressCode(this, 0.1);

    QElapsedTimer timer;
    timer.start();
    OptimizerController* optimizer = new OptimizerController(d->optimizeNode, primitives().count());
    optimizer->optimize();
    optimizer->setFinishedCallback(
        [=](OptimizerController* controller)
        {
            PathOptimizer::Path path = controller->path();
            qLogD << "optimized duration: " << timer.elapsed() / 1000.0;

            QFile saveFile(filename);
            QJsonObject jsonObj;

            QTransform t = enablePrintAndCut() ? printAndCutTransform() : QTransform();

            QJsonObject laserDocumentInfo;
            laserDocumentInfo["APIVersion"] = LaserApplication::driver->getVersion();
            laserDocumentInfo["CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            laserDocumentInfo["PrinterDrawUnit"] = 1016;
            laserDocumentInfo["FinishRun"] = d->finishRun.code;
            laserDocumentInfo["StartFrom"] = Config::Device::startFrom();
            laserDocumentInfo["JobOrigin"] = Config::Device::jobOrigin();
            laserDocumentInfo["DeviceOrigin"] = Config::SystemRegister::deviceOrigin();
            //QPointF docOrigin = docOriginMachining();
            //docOrigin = LaserApplication::device->deviceTransformMachining().map(docOrigin);
            QPointF docOrigin(0, 0);
            laserDocumentInfo["Origin"] = typeUtils::point2Json(docOrigin);
            laserDocumentInfo["UserOrigin"] = typeUtils::point2Json(docOrigin);
            QRectF docBoundingRect = docBoundingRectMachining();
            laserDocumentInfo["BoundingRect"] = typeUtils::rect2Json(docBoundingRect);
            laserDocumentInfo["ImageBoundingRect"] = typeUtils::rect2Json(imagesBoundingRectMachining());
            laserDocumentInfo["SoftwareVersion"] = LaserApplication::softwareVersion();

            jsonObj["LaserDocumentInfo"] = laserDocumentInfo;

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

            QJsonArray layers;
            //QPointF lastPoint = docOriginMachining();
            QPointF lastPoint = docOrigin;
            for (LaserLayer* layer : layerList)
            {
                QJsonObject layerObj;
                QJsonObject paramObj;
                QJsonArray items;
                QJsonObject engravingParamObj;
                QJsonObject cuttingParamObj;
                QJsonObject fillingParamObj;
                layerObj["Type"] = layer->type();
                if (layer->type() == LLT_ENGRAVING)
                {
                    engravingParamObj["RunSpeed"] = layer->engravingRunSpeed() * 1000;
                    engravingParamObj["LaserPower"] = layer->engravingLaserPower() * 10;
                    engravingParamObj["MinSpeedPower"] = layer->engravingMinSpeedPower() * 10;
                    engravingParamObj["RunSpeedPower"] = layer->engravingRunSpeedPower() * 10;
                    engravingParamObj["RowInterval"] = layer->engravingRowInterval();
                    engravingParamObj["CarveForward"] = layer->engravingForward();
                    engravingParamObj["CarveStyle"] = layer->engravingStyle();
                    engravingParamObj["ErrorX"] = layer->errorX();
                }
                else if (layer->type() == LLT_CUTTING)
                {
                    cuttingParamObj["RunSpeed"] = layer->cuttingRunSpeed() * 1000;
                    cuttingParamObj["MinSpeedPower"] = layer->cuttingMinSpeedPower() * 10;
                    cuttingParamObj["RunSpeedPower"] = layer->cuttingRunSpeedPower() * 10;
                }
                else if (layer->type() == LLT_FILLING)
                {
                    layerObj["Type"] = 2;
                    cuttingParamObj["RunSpeed"] = layer->cuttingRunSpeed() * 1000;
                    cuttingParamObj["MinSpeedPower"] = layer->cuttingMinSpeedPower() * 10;
                    cuttingParamObj["RunSpeedPower"] = layer->cuttingRunSpeedPower() * 10;
                    fillingParamObj["RunSpeed"] = layer->fillingRunSpeed() * 1000;
                    fillingParamObj["MinSpeedPower"] = layer->fillingMinSpeedPower() * 10;
                    fillingParamObj["RunSpeedPower"] = layer->fillingRunSpeedPower() * 10;
                    fillingParamObj["RowInterval"] = layer->fillingRowInterval();
                }
                paramObj["EngravingParams"] = engravingParamObj;
                paramObj["CuttingParams"] = cuttingParamObj;
                paramObj["FillingParams"] = fillingParamObj;
                layerObj["Params"] = paramObj;
                for (OptimizeNode* pathNode : layersMap[layer])
                {
                    LaserPrimitive* primitive = pathNode->primitive();

                    QJsonObject itemObj;
                    itemObj["Name"] = pathNode->nodeName();
                    //itemObj["Absolute"] = Config::Device::startFrom() == SFT_AbsoluteCoords;
                    if (layer->type() == LLT_ENGRAVING)
                    {
                        if (!enablePrintAndCut())
                        {
                            itemObj["Width"] = Global::convertToMM(SU_PX, primitive->boundingRect().width());
                            itemObj["Height"] = Global::convertToMM(SU_PX, primitive->boundingRect().height(), Qt::Vertical);
                            itemObj["Style"] = LaserLayerType::LLT_ENGRAVING;

                            QByteArray data = primitive->engravingImage();
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
                            itemObjCutting["Absolute"] = Config::Device::startFrom() == SFT_AbsoluteCoords;
                            itemObjCutting["Type"] = primitive->typeLatinName();
                            itemObjCutting["Style"] = LaserLayerType::LLT_CUTTING;
                            pathNode->nearestPoint(LaserPoint(lastPoint));
                            LaserPointListList points = primitive->arrangedPoints();
                            itemObjCutting["Data"] = QString(machiningUtils::pointListList2Plt(points, lastPoint, t));
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
                            itemObj["Data"] = QString(machiningUtils::pointListList2Plt(points, lastPoint, t));
                            items.append(itemObj);
                        }
                    }
                    else if (layer->type() == LLT_FILLING)
                    {
                        if (!enablePrintAndCut())
                        {
                            itemObj["Type"] = primitive->typeLatinName();
                            LaserLineListList lineList = primitive->generateFillData(lastPoint);
                            itemObj["Data"] = QString(machiningUtils::lineList2Plt(lineList, lastPoint));
                            itemObj["Style"] = LaserLayerType::LLT_FILLING;
                            items.append(itemObj);
                        }

                        if (layer->fillingEnableCutting())
                        {
                            QJsonObject itemObjCutting;
                            itemObjCutting["Name"] = pathNode->nodeName() + "_cutting";
                            itemObjCutting["Absolute"] = Config::Device::startFrom() == SFT_AbsoluteCoords;
                            itemObjCutting["Type"] = primitive->typeLatinName();
                            itemObjCutting["Style"] = LaserLayerType::LLT_CUTTING;
                            pathNode->nearestPoint(LaserPoint(lastPoint));
                            LaserPointListList points = primitive->arrangedPoints();
                            itemObjCutting["Data"] = QString(machiningUtils::pointListList2Plt(points, lastPoint, t));
                            items.append(itemObjCutting);
                        }
                    }

                    LaserApplication::previewWindow->addProgress(this, 1.0 * 0.9 / path.length(), tr("Primitve %1 finished.").arg(pathNode->nodeName()));
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

            //QByteArray rawJson = jsonDoc.toJson(QJsonDocument::Compact);
            QByteArray rawJson = jsonDoc.toJson(QJsonDocument::Indented);
            //qLogD << rawJson;
            qint64 writtenBytes = saveFile.write(rawJson);
            LaserApplication::previewWindow->addProgress(this, 0.1 / path.length(), tr("File saved."));
            qDebug() << "written bytes:" << writtenBytes;

            qLogD << "exported json duration: " << timer.elapsed() / 1000.0;
            LaserApplication::previewWindow->addMessage(tr("Done"));
            LaserApplication::previewWindow->setProgress(1);
            saveFile.close();
            emit exportFinished(filename);
        }
    );
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

FinishRun& LaserDocument::finishRun()
{
    Q_D(LaserDocument);
    return d->finishRun;
}

void LaserDocument::setFinishRun(const FinishRun& value)
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

QPointF LaserDocument::docOrigin() const
{
    Q_D(const LaserDocument);
    QRectF bounding = docBoundingRect();
    int posIndex = 0;
    qreal dx = 0, dy = 0;
    switch (Config::Device::startFrom())
    {
    case SFT_AbsoluteCoords:
        dx = bounding.x();
        dy = bounding.y();
        break;
    case SFT_UserOrigin:
    {
        switch (Config::Device::jobOrigin())
        {
        case 0:
            dx = bounding.x();
            dy = bounding.y();
            break;
        case 1:
            dx = bounding.x() + bounding.width() / 2;
            dy = bounding.y();
            break;
        case 2:
            dx = bounding.x() + bounding.width();
            dy = bounding.y();
            break;
        case 3:
            dx = bounding.x();
            dy = bounding.y() + bounding.height() / 2;
            break;
        case 4:
            dx = bounding.x() + bounding.width() / 2;
            dy = bounding.y() + bounding.height() / 2;
            break;
        case 5:
            dx = bounding.x() + bounding.width();
            dy = bounding.y() + bounding.height() / 2;
            break;
        case 6:
            dx = bounding.x();
            dy = bounding.y() + bounding.height();
            break;
        case 7:
            dx = bounding.x() + bounding.width() / 2;
            dy = bounding.y() + bounding.height();
            break;
        case 8:
            dx = bounding.x() + bounding.width();
            dy = bounding.y() + bounding.height();
            break;
        }
    }
        break;
    case SFT_CurrentPosition:
    {
        QVector3D laserPos = LaserApplication::device->getCurrentLaserPos();
        dx = laserPos.x();
        dy = laserPos.y();
    }
        break;
    }
    return QPointF(dx, dy);
}

QPointF LaserDocument::docOriginMM() const
{
    return Global::matrixToMM(SU_PX).map(docOrigin());
}

QPointF LaserDocument::docOriginMachining() const
{
    return Global::matrixToMachining().map(docOrigin());
}

QTransform LaserDocument::docTransform() const
{
    Q_D(const LaserDocument);
    QRectF bounding = docBoundingRect();
    int posIndex = 0;
    QPointF origin(0, 0);
    qreal dx = 0, dy = 0;
    switch (Config::Device::startFrom())
    {
    case SFT_AbsoluteCoords:
        dx = bounding.x();
        dy = bounding.y();
        break;
    case SFT_UserOrigin:
    {
        switch (Config::Device::jobOrigin())
        {
        case 0:
            dx = bounding.x();
            dy = bounding.y();
            break;
        case 1:
            dx = bounding.x() + bounding.width() / 2;
            dy = bounding.y();
            break;
        case 2:
            dx = bounding.x() + bounding.width();
            dy = bounding.y();
            break;
        case 3:
            dx = bounding.x();
            dy = bounding.y() + bounding.height() / 2;
            break;
        case 4:
            dx = bounding.x() + bounding.width() / 2;
            dy = bounding.y() + bounding.height() / 2;
            break;
        case 5:
            dx = bounding.x() + bounding.width();
            dy = bounding.y() + bounding.height() / 2;
            break;
        case 6:
            dx = bounding.x();
            dy = bounding.y() + bounding.height();
            break;
        case 7:
            dx = bounding.x() + bounding.width() / 2;
            dy = bounding.y() + bounding.height();
            break;
        case 8:
            dx = bounding.x() + bounding.width();
            dy = bounding.y() + bounding.height();
            break;
        }
    }
        break;
    case SFT_CurrentPosition:
    {
        QVector3D laserPos = LaserApplication::device->getCurrentLaserPos();
        switch (Config::Device::jobOrigin())
        {
        case 0:
            dx = laserPos.x();
            dy = laserPos.y();
            break;
        case 1:
            dx = bounding.x() - bounding.width() / 2;
            dy = bounding.y();
            break;
        case 2:
            dx = bounding.x() - bounding.width();
            dy = bounding.y();
            break;
        case 3:
            dx = bounding.x();
            dy = bounding.y() - bounding.height() / 2;
            break;
        case 4:
            dx = bounding.x() - bounding.width() / 2;
            dy = bounding.y() - bounding.height() / 2;
            break;
        case 5:
            dx = bounding.x() - bounding.width();
            dy = bounding.y() - bounding.height() / 2;
            break;
        case 6:
            dx = bounding.x();
            dy = bounding.y() - bounding.height();
            break;
        case 7:
            dx = bounding.x() - bounding.width() / 2;
            dy = bounding.y() - bounding.height();
            break;
        case 8:
            dx = bounding.x() - bounding.width();
            dy = bounding.y() - bounding.height();
            break;
        }
    }
        break;
    }
    return QTransform::fromTranslate(-dx, -dy);
}

QTransform LaserDocument::docTransformMM() const
{
    QTransform t = docTransform();
    qreal dx = Global::convertToMM(SU_PX, t.dx());
    qreal dy = Global::convertToMM(SU_PX, t.dy());
    return QTransform::fromTranslate(dx, dy);
}

QRectF LaserDocument::docBoundingRect() const
{
    Q_D(const LaserDocument);
    return utils::boundingRect(primitives().values());
}

QRectF LaserDocument::docBoundingRectMM() const
{
    return Global::matrixToMM(SU_PX).map(docBoundingRect()).boundingRect();
}

QRectF LaserDocument::docBoundingRectMachining() const
{
    return Global::matrixToMachining().map(docBoundingRect()).boundingRect();
}

QRectF LaserDocument::imagesBoundingRect() const
{
    Q_D(const LaserDocument);
    QRectF bounding(0, 0, 0, 0);
    int count = 0;
    for (QMap<QString, LaserPrimitive*>::ConstIterator i = d->primitives.constBegin();
        i != d->primitives.constEnd(); i++)
    {
        if (i.value()->primitiveType() == LPT_BITMAP)
        {
            QRectF rect = i.value()->sceneBoundingRect();
            qreal minSpeed = Config::UserRegister::scanXStartSpeed() / 1000;
            qreal acc = Config::UserRegister::scanXAcc() / 1000;
            qreal maxSpeed = i.value()->layer()->engravingRunSpeed();
            qreal span = (maxSpeed * maxSpeed - minSpeed * minSpeed) / (acc * 2);
            rect.setLeft(rect.left() - span);
            rect.setRight(rect.right() + span);
            if (count++ == 0)
            {
                bounding = rect;
                continue;
            }
            if (rect.left() < bounding.left())
                bounding.setLeft(rect.left());
            if (rect.top() < bounding.top())
                bounding.setTop(rect.top());
            if (rect.right() > bounding.right())
                bounding.setRight(rect.right());
            if (rect.bottom() > bounding.bottom())
                bounding.setBottom(rect.bottom());
        }
    }
    return bounding;
}

QRectF LaserDocument::imagesBoundingRectMachining() const
{
    Q_D(const LaserDocument);
    QRectF bounding(0, 0, 0, 0);
    int count = 0;
    for (QMap<QString, LaserPrimitive*>::ConstIterator i = d->primitives.constBegin();
        i != d->primitives.constEnd(); i++)
    {
        if (i.value()->primitiveType() == LPT_BITMAP)
        {
            QRectF rect = Global::matrixToMachining().map(i.value()->sceneBoundingRect()).boundingRect();
            qreal minSpeed = Config::UserRegister::scanXStartSpeed();
            qreal acc = Config::UserRegister::scanXAcc();
            qreal maxSpeed = i.value()->layer()->engravingRunSpeed() * 1000;
            qreal span = (maxSpeed * maxSpeed - minSpeed * minSpeed) / (acc * 2);
            rect.setLeft(rect.left() - span);
            rect.setRight(rect.right() + span);
            if (count++ == 0)
            {
                bounding = rect;
                continue;
            }
            if (rect.left() < bounding.left())
                bounding.setLeft(rect.left());
            if (rect.top() < bounding.top())
                bounding.setTop(rect.top());
            if (rect.right() > bounding.right())
                bounding.setRight(rect.right());
            if (rect.bottom() > bounding.bottom())
                bounding.setBottom(rect.bottom());
        }
    }
    return bounding;
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

void LaserDocument::analysis()
{
    Q_D(LaserDocument);
    qLogD << "begin analysising";

    /*for (LaserPrimitive* primitive : d->primitives)
    {
        if (primitive->primitiveType() == LPT_PATH)
        {
            LaserPath* laserPath = qobject_cast<LaserPath*>(primitive);
            QList<QPainterPath> subBoundings = laserPath->subBoundings();
            for (int i = 0; i < subBoundings.size(); i++)
            {
                qLogD << "sub path " << i << ":" << subBoundings[i];
            }
        }
    }*/

    outline();
}

void LaserDocument::outline()
{
    Q_D(LaserDocument);
    qLogD << "Before outline:";
    //clearOutline(true);
    clearTree(d->optimizeNode);
    printOutline(d->optimizeNode, 0);
    outlineByLayers(d->optimizeNode);
    //outlineByGroups(d->optimizeNode);
    //optimizeGroups(d->optimizeNode);
    qLogD << "After outline:";
    printOutline(d->optimizeNode, 0);

    emit outlineUpdated();
}

//void LaserDocument::clearOutline(bool clearLayers)
//{
//    Q_D(LaserDocument);
//    clearOutline(d->optimizeNode, clearLayers);
//}

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
        /*if (layer.contains("halftoneGridSize"))
        {
            laserLayers[index]->setHalftoneGridSize(layer.value("halftoneGridSize").toInt());
        }*/

		//primitive
		for (int j = 0; j < array.size(); j++) {
			QJsonObject primitiveJson = array[j].toObject();
			QString className = primitiveJson["className"].toString();
			int layerIndex = primitiveJson["layerIndex"].toInt();
			//postion
			//QJsonArray posArray = primitiveJson["position"].toArray();
			//QPointF position(posArray[0].toDouble(), posArray[1].toDouble());
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
			if (className == "LaserEllipse" || className == "LaserRect" || className == "LaserBitmap") {
				//QTransform saveTransform = transform * transformP;
				
				//bounds
				QJsonArray boundsArray = primitiveJson["bounds"].toArray();
				QRectF bounds = QRectF(boundsArray[0].toDouble(), boundsArray[1].toDouble(), boundsArray[2].toDouble(), boundsArray[3].toDouble());
				LaserPrimitive* rect;
				if (className == "LaserEllipse") {
					rect = new LaserEllipse(bounds, this, saveTransform, layerIndex);
				}
				else if (className == "LaserRect") {
                    qreal cornerRadius = primitiveJson["cornerRadius"].toDouble();
					rect = new LaserRect(bounds, cornerRadius, this, saveTransform, layerIndex);
				}
				else if (className == "LaserBitmap") {
					//bounds
					QJsonArray boundsArray = primitiveJson["bounds"].toArray();
					QRectF bounds = QRectF(boundsArray[0].toDouble(), boundsArray[1].toDouble(), boundsArray[2].toDouble(), boundsArray[3].toDouble());
					//image
					QByteArray array = QByteArray::fromBase64(primitiveJson["image"].toString().toLatin1());
					//QBuffer buffer(&array);
					//buffer.open(QIODevice::ReadOnly);
					
					QImage img = QImage::fromData(array, "tiff");

					qDebug() << img.size();
					rect = new LaserBitmap(img, bounds, this, saveTransform, layerIndex);
				}
				laserLayers[index]->addPrimitive(rect);
				//this->addPrimitive(rect);
				this->scene()->addLaserPrimitive(rect);
			}
			else if (className == "LaserLine") {
				
				//line
				QJsonArray lineArray = primitiveJson["line"].toArray();
				QPointF p1 = QPointF(lineArray[0].toDouble(), lineArray[1].toDouble());
				QPointF p2 = QPointF(lineArray[2].toDouble(), lineArray[3].toDouble());

				LaserLine* line = new LaserLine(QLineF(p1, p2), this, saveTransform, layerIndex);
				laserLayers[index]->addPrimitive(line);
				//this->addPrimitive(line);
				//this->scene()->addItem(line);
                this->scene()->addLaserPrimitive(line);
			}
			else if (className == "LaserPolyline" || className == "LaserPolygon") {
				QJsonArray polyArray = primitiveJson["poly"].toArray();
				QVector<QPointF> vector;
				for each(QJsonValue point in polyArray) {
					QJsonArray pointArray = point.toArray();
					vector.append(QPointF(pointArray[0].toDouble(), pointArray[1].toDouble()));
				}
				LaserPrimitive * poly;
				if (className == "LaserPolyline") {
					poly = new LaserPolyline(QPolygonF(vector), this, saveTransform, layerIndex);
				}
				else if (className == "LaserPolygon") {
					poly = new LaserPolygon(QPolygonF(vector), this, saveTransform, layerIndex);
				}
				
				laserLayers[index]->addPrimitive(poly);
				//this->addPrimitive(poly);
				//this->scene()->addItem(poly);
                this->scene()->addLaserPrimitive(poly);
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
                //this->addPrimitive(text);
                //this->scene()->addItem(text);
                this->scene()->addLaserPrimitive(text);
            }
            else if (className == "LaserPath") {
                QByteArray buffer = QByteArray::fromBase64(primitiveJson["path"].toString().toLatin1());
                QPainterPath path;
                QDataStream stream(buffer);
                stream >> path;
                LaserPath * laserPath = new LaserPath(path, this, saveTransform, layerIndex);
                //this->addPrimitive(laserPath);
                //this->scene()->addItem(laserPath);
                this->scene()->addLaserPrimitive(laserPath);
                
            }
            
		}
        if (layer.contains("visible")) {
            bool bl = layer.value("visible").toBool();
            laserLayers[index]->setVisible(bl);
            //LaserControllerWindow* window = LaserApplication::mainWindow;
        }
	}
    this->blockSignals(false);
    emit updateLayersStructure();
    //outline();
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
    
    d->boundingRectTimer.start();
}

void LaserDocument::outlineByLayers(OptimizeNode* node)
{
    Q_D(LaserDocument);
    for (LaserLayer* layer : d->layers)
    {
        if (layer->isEmpty())
            continue;
        d->optimizeNode->addChildNode(layer->optimizeNode());
        for (LaserPrimitive* primitive : layer->primitives())
        {
            addPrimitiveToNodesTree(primitive, layer->optimizeNode());
        }
        //optimizeGroups(layer->optimizeNode());
    }
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

void LaserDocument::clearTree(OptimizeNode* node)
{
    QStack<OptimizeNode*> stack;
    stack.push(node);
    QList<OptimizeNode*> deletingNodes;
    while (!stack.isEmpty())
    {
        OptimizeNode* topNode = stack.pop();
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
                groupIndex = Global::convertToMM(SU_PX, node->position().y()) / Config::PathOptimization::groupingGridInterval();
            }
            else if (Config::PathOptimization::groupingOrientation() == Qt::Vertical)
            {
                groupIndex = Global::convertFromMM(SU_PX, node->position().x()) / Config::PathOptimization::groupingGridInterval();
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
                    return a->position().x() < b->position().x();
                }
                else if (Config::PathOptimization::groupingOrientation() == Qt::Vertical)
                {
                    return a->position().y() < b->position().y();
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


