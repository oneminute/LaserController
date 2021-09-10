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
#include "LaserLayer.h"
#include "state/StateController.h"
#include "svg/qsvgtinydocument.h"
#include "LaserScene.h"
#include "laser/LaserPointList.h"

class LaserDocumentPrivate : public ILaserDocumentItemPrivate
{
    Q_DECLARE_PUBLIC(LaserDocument)
public:
    LaserDocumentPrivate(LaserDocument* ptr)
        : ILaserDocumentItemPrivate(ptr, LNT_DOCUMENT)
        , blockSignals(false)
        , isOpened(false)
        //, boundingRect(0, 0, Config::SystemRegister::xMaxLength(), Config::SystemRegister::yMaxLength())
    {}
    QMap<QString, LaserPrimitive*> primitives;
    QList<LaserLayer*> layers;
    PageInformation pageInfo;
    bool blockSignals;
    bool isOpened;
    LaserScene* scene;
    FinishRun finishRun;
    SizeUnit unit;

    //QRectF boundingRect;
    QElapsedTimer boundingRectTimer;
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

    /*if (item->isShape())
    {
        d->layers[1]->addPrimitive(item);
    }
    else if (item->isBitmap())
    {
        d->layers[0]->addPrimitive(item);
    }*/

    updateLayersStructure();
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

//qreal LaserDocument::scale() const
//{
//    Q_D(const LaserDocument);
//    return d->scale;
//}
//
//void LaserDocument::setScale(qreal scale)
//{
//    Q_D(LaserDocument);
//    d->scale = scale;
//}

void LaserDocument::exportJSON(const QString& filename)
{
    exportJSON2(filename);
}

void LaserDocument::exportJSON1(const QString& filename)
{
    Q_D(LaserDocument);

    QFile saveFile(filename);
    QJsonObject jsonObj;

    QJsonObject laserDocumentInfo;
    qDebug() << &LaserDriver::instance();
    laserDocumentInfo["APIVersion"] = LaserDriver::instance().getVersion();
    laserDocumentInfo["CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    laserDocumentInfo["PrinterDrawUnit"] = 1016;
    laserDocumentInfo["FinishRun"] = d->finishRun.code;
    jsonObj["LaserDocumentInfo"] = laserDocumentInfo;

    QJsonArray layers;
    float pageWidth = Global::convertToMM(SU_PX, d->pageInfo.width());
    float pageHeight = Global::convertToMM(SU_PX, d->pageInfo.height(), Qt::Vertical);
    cv::Mat canvas(pageHeight * 40, pageWidth * 40, CV_8UC3, cv::Scalar(255, 255, 255));
    int layerId = 0;
    for (int i = 0; i < d->layers.count(); i++)
    {
        LaserLayer* layer = d->layers[i];
        if (layer->isEmpty())
            continue;
        QJsonObject layerObj;
        QJsonObject paramObj;
        QJsonObject engravingParamObj;
        QJsonObject cuttingParamObj;
        if (layer->type() == LLT_ENGRAVING)
        {
            engravingParamObj["LayerId"] = i;
            engravingParamObj["Type"] = layer->type();
            engravingParamObj["MinSpeed"] = layer->minSpeed();
            engravingParamObj["RunSpeed"] = layer->runSpeed();
            engravingParamObj["LaserPower"] = layer->laserPower();
            engravingParamObj["MinSpeedPower"] = layer->minSpeedPower();
            engravingParamObj["RunSpeedPower"] = layer->runSpeedPower();
            engravingParamObj["CarveForward"] = layer->engravingForward();
            engravingParamObj["CarveStyle"] = layer->engravingStyle();
            engravingParamObj["HStep"] = layer->lineSpacing();
            engravingParamObj["LStep"] = layer->columnSpacing();
            engravingParamObj["ErrorX"] = layer->errorX();
            engravingParamObj["MinSpeedPower"] = layer->minSpeedPower();
            engravingParamObj["RunSpeedPower"] = layer->runSpeedPower();
        }
        else if (layer->type() == LLT_CUTTING)
        {
            cuttingParamObj["LayerId"] = i;
            cuttingParamObj["Type"] = layer->type();
            cuttingParamObj["MinSpeed"] = layer->minSpeed();
            cuttingParamObj["RunSpeed"] = layer->runSpeed();
            cuttingParamObj["LaserPower"] = layer->laserPower();
            cuttingParamObj["MinSpeedPower"] = layer->minSpeedPower();
            cuttingParamObj["RunSpeedPower"] = layer->runSpeedPower();
        }
        paramObj["EngravingParams"] = engravingParamObj;
        paramObj["CuttingParams"] = cuttingParamObj;
        layerObj["Params"] = paramObj;

        QJsonArray items;
        QList<LaserPrimitive*> laserItems = layer->primitives();
        for (int li = 0; li < laserItems.size(); li++)
        {
            LaserPrimitive* laserItem = laserItems[li];
            QJsonObject itemObj;
            bool add = false;
            if (layer->type() == LLT_ENGRAVING)
            {
                itemObj["Layer"] = layerId;
                itemObj["Width"] = Global::convertToMM(SU_PX, laserItem->boundingRect().width());
                itemObj["Height"] = Global::convertToMM(SU_PX, laserItem->boundingRect().height(), Qt::Vertical);

                QByteArray data = laserItem->engravingImage(canvas);
                if (!data.isEmpty())
                {
                    itemObj["Type"] = laserItem->typeLatinName();
                    itemObj["ImageType"] = "PNG";
                    itemObj["Data"] = QString(data.toBase64());
                    add = true;
                }
            }
            else if (layer->type() == LLT_CUTTING)
            {
                itemObj["Layer"] = layerId;
                //QList<QPainterPath> modifyPathList = laserItem->subBoundings();
                LaserPointList points = laserItem->updateMachiningPoints(canvas);
                if (!points.empty())
                {
                    itemObj["Type"] = laserItem->typeLatinName();
                    itemObj["Data"] = QString(machiningUtils::points2Plt(points));
                    add = true;
                }
                /*if (modifyPathList.isEmpty())
                {

                }
                else
                {
                    QString pltString;
                    for (QPainterPath subPath : modifyPathList)
                    {
                        QVector<QPointF> points;
                        if (machiningUtils::path2Points(subPath, points, canvas))
                        {
                            pltString.append(QString(machiningUtils::points2Plt(points)));
                        }
                    }
                    if (!pltString.isEmpty())
                    {
                        itemObj["Type"] = laserItem->typeLatinName();
                        itemObj["Data"] = pltString;
                        add = true;
                    }
                }*/
            }
            if (add)
                items.append(itemObj);
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

    qint64 writtenBytes = saveFile.write(jsonDoc.toJson(QJsonDocument::Indented));
    qDebug() << "written bytes:" << writtenBytes;

    if (!canvas.empty())
        cv::imwrite("tmp/canvas_test.png", canvas);
}

void LaserDocument::exportJSON2(const QString& filename)
{
    Q_D(LaserDocument);

    float pageWidth = Global::convertToMM(SU_PX, d->pageInfo.width()) * 40;
    float pageHeight = Global::convertToMM(SU_PX, d->pageInfo.height(), Qt::Vertical) * 40;
    cv::Mat canvas(pageHeight, pageWidth, CV_8UC3, cv::Scalar(255, 255, 255));

    QElapsedTimer timer;
    timer.start();
    OptimizerController* optimizer = new OptimizerController(d->optimizeNode, primitives().count());
    PathOptimizer::Path path = optimizer->optimize(pageWidth, pageHeight, canvas);
    qLogD << "optimized duration: " << timer.elapsed() / 1000.0;
    delete optimizer;

    timer.start();
    QFile saveFile(filename);
    QJsonObject jsonObj;

    QJsonObject laserDocumentInfo;
    qDebug() << &LaserDriver::instance();
    laserDocumentInfo["APIVersion"] = LaserDriver::instance().getVersion();
    laserDocumentInfo["CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    laserDocumentInfo["PrinterDrawUnit"] = 1016;
    laserDocumentInfo["FinishRun"] = d->finishRun.code;
    jsonObj["LaserDocumentInfo"] = laserDocumentInfo;

    QJsonArray layers;
    QJsonArray items;
    LaserPoint lastPoint;

    QList<LaserLayer*> layerList;

    for (OptimizeNode* pathNode : path)
    {
        LaserPrimitive* primitive = pathNode->primitive();
        LaserLayer* layer = primitive->layer();
        bool newLayer = false;
        if (!layerList.contains(layer))
        {
            layerList.append(layer);
            newLayer = true;
        }
        int layerId = layerList.indexOf(layer);

        QJsonObject itemObj;
        if (layer->type() == LLT_ENGRAVING)
        {
            itemObj["Layer"] = layerId;
            itemObj["Width"] = Global::convertToMM(SU_PX, primitive->boundingRect().width());
            itemObj["Height"] = Global::convertToMM(SU_PX, primitive->boundingRect().height(), Qt::Vertical);

            QByteArray data = primitive->engravingImage(canvas);
            if (!data.isEmpty())
            {
                itemObj["Type"] = primitive->typeLatinName();
                itemObj["ImageType"] = "PNG";
                itemObj["Data"] = QString(data.toBase64());
                items.append(itemObj);
            }
        }
        else if (layer->type() == LLT_CUTTING)
        {
            itemObj["Layer"] = layerId;
            QList<QPainterPath> paths = primitive->subPaths();
            //std::vector<cv::Point2f> points = primitive->updateMachiningPoints(canvas);
            LaserPointList points = primitive->arrangedPoints();
            if (!points.empty())
            {
                itemObj["Type"] = primitive->typeLatinName();
                itemObj["Data"] = QString(machiningUtils::points2Plt(points));
                items.append(itemObj);
            }
        }

        if (newLayer)
        {
            QJsonObject layerObj;
            QJsonObject paramObj;
            QJsonObject engravingParamObj;
            QJsonObject cuttingParamObj;
            if (layer->type() == LLT_ENGRAVING)
            {
                engravingParamObj["LayerId"] = layerId;
                engravingParamObj["Type"] = layer->type();
                engravingParamObj["MinSpeed"] = layer->minSpeed();
                engravingParamObj["RunSpeed"] = layer->runSpeed();
                engravingParamObj["LaserPower"] = layer->laserPower();
                engravingParamObj["MinSpeedPower"] = layer->minSpeedPower();
                engravingParamObj["RunSpeedPower"] = layer->runSpeedPower();
                engravingParamObj["CarveForward"] = layer->engravingForward();
                engravingParamObj["CarveStyle"] = layer->engravingStyle();
                engravingParamObj["HStep"] = layer->lineSpacing();
                engravingParamObj["LStep"] = layer->columnSpacing();
                engravingParamObj["ErrorX"] = layer->errorX();
                engravingParamObj["MinSpeedPower"] = layer->minSpeedPower();
                engravingParamObj["RunSpeedPower"] = layer->runSpeedPower();
            }
            else if (layer->type() == LLT_CUTTING)
            {
                cuttingParamObj["LayerId"] = layerId;
                cuttingParamObj["Type"] = layer->type();
                cuttingParamObj["MinSpeed"] = layer->minSpeed();
                cuttingParamObj["RunSpeed"] = layer->runSpeed();
                cuttingParamObj["LaserPower"] = layer->laserPower();
                cuttingParamObj["MinSpeedPower"] = layer->minSpeedPower();
                cuttingParamObj["RunSpeedPower"] = layer->runSpeedPower();
            }
            paramObj["EngravingParams"] = engravingParamObj;
            paramObj["CuttingParams"] = cuttingParamObj;
            layerObj["Params"] = paramObj;
            layerObj["Index"] = layerId;
            //layerObj["Items"] = items;
            layers.append(layerObj);
        }
    }

    QJsonObject actionObj;

    jsonObj["Layers"] = layers;
    jsonObj["Items"] = items;

    QJsonDocument jsonDoc(jsonObj);

    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qWarning("Couldn't open save file.");
        return;
    }

    qint64 writtenBytes = saveFile.write(jsonDoc.toJson(QJsonDocument::Indented));
    qDebug() << "written bytes:" << writtenBytes;

    if (!canvas.empty())
        cv::imwrite("tmp/canvas_test.png", canvas);
    qLogD << "exported json duration: " << timer.elapsed() / 1000.0;
}

void LaserDocument::blockSignals(bool block)
{
    Q_D(LaserDocument);
    d->blockSignals = block;
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
    return Global::matrixToMM(SU_PX, 40, 40).map(docOrigin());
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

void LaserDocument::updateLayersStructure()
{
    Q_D(LaserDocument);
    if (!d->blockSignals)
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
    qLogD << space << node->nodeName();

    for (OptimizeNode* item : node->childNodes())
    {
        printOutline(item, level + 1);
    }
}

void LaserDocument::arrange()
{
}

void LaserDocument::optimize()
{
    Q_D(LaserDocument);
    //float pageWidth = Global::convertToMM(SU_PX, d->pageInfo.width()) * 40;
    //float pageHeight = Global::convertToMM(SU_PX, d->pageInfo.height(), Qt::Vertical) * 40;

    //qLogD << "LaserDocument::optimize";
    //OptimizerController* optimizer = new OptimizerController(this, totalNodes());
    //optimizer->optimize(pageWidth, pageHeight);
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
	/*for (int i = 0; i < layers.size(); i++) {
		LaserLayer* layer = layers[i];
		if()
	}*/
	
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
					rect = new LaserRect(bounds, this, saveTransform, layerIndex);
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
				this->addPrimitive(rect);
				this->scene()->addItem(rect);
			}
			else if (className == "LaserLine") {
				
				//line
				QJsonArray lineArray = primitiveJson["line"].toArray();
				QPointF p1 = QPointF(lineArray[0].toDouble(), lineArray[1].toDouble());
				QPointF p2 = QPointF(lineArray[2].toDouble(), lineArray[3].toDouble());

				LaserLine* line = new LaserLine(QLineF(p1, p2), this, saveTransform, layerIndex);
				laserLayers[index]->addPrimitive(line);
				this->addPrimitive(line);
				this->scene()->addItem(line);
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
				this->addPrimitive(poly);
				this->scene()->addItem(poly);
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
                //text->sceneTransformToItemTransform(saveTransform);
                this->addPrimitive(text);
                this->scene()->addItem(text);
            }
		}
	}
    outline();
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
	d->name = "document";
	QString layerName = newLayerName();
	LaserLayer* layer = new LaserLayer(layerName, LLT_ENGRAVING, this, true);
	addLayer(layer);

	layerName = newLayerName();
	layer = new LaserLayer(layerName, LLT_CUTTING, this, true);
	addLayer(layer);

	for (int i = 2; i < Config::Layers::maxLayersCount(); i++)
	{
		QString layerName = newLayerName();
		LaserLayer* layer = new LaserLayer(layerName, LLT_ENGRAVING, this);
		addLayer(layer);
	}
	ADD_TRANSITION(documentEmptyState, documentWorkingState, this, SIGNAL(opened()));
	ADD_TRANSITION(documentWorkingState, documentEmptyState, this, SIGNAL(closed()));
    
    d->boundingRectTimer.start();
}

RELATION LaserDocument::determineRelationship(const QPainterPath& a, const QPainterPath& b)
{
    RELATION rel;
    if (a.contains(b))
    {
        // candidate primitive contains tree node primitive
        rel = A_CONTAINS_B;
    }
    else if (b.contains(a))
    {
        // tree node primitive contains candidate primitive
        rel = B_CONTAINS_A;
    }
    else if (a.intersects(b))
    {
        // a intersects with b
        rel = INTERSECTION;
    }
    else
    {
        // no relationship between candidate primitive and tree node primitive
        rel = RELATION::NONE;
    }
    return rel;
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

void LaserDocument::optimizeGroups(OptimizeNode* node, int level)
{
    qLogD << "node " << node->nodeName() << " has " << node->childCount() << " child nodes.";
    if (!node->hasChildren())
    {
        return;
    }

    QMap<int, QList<OptimizeNode*>> childrenMap;
    for (OptimizeNode* node : node->childNodes())
    {
        int groupIndex = 0;
        if (Config::PathOptimization::groupingOrientation() == Qt::Horizontal)
        {
            groupIndex = node->position().y() / Config::PathOptimization::maxGroupingGridSize();
        }
        else if (Config::PathOptimization::groupingOrientation() == Qt::Vertical)
        {
            groupIndex = node->position().x() / Config::PathOptimization::maxGroupingGridSize();
        }
        childrenMap[groupIndex].append(node);
    }
    qLogD << "  child nodes were seperated into " << childrenMap.size() << " groups by grid.";

    if (childrenMap.size() > 1)
    {
        // 清空当前的子节点列表。
        node->childNodes().clear();
        for (QMap<int, QList<OptimizeNode*>>::Iterator i = childrenMap.begin(); i != childrenMap.end(); i++)
        {
            OptimizeNode* newNode = new OptimizeNode();
            QString nodeName = QString("vnode_%1_%2").arg(level).arg(node->childNodes().count() + 1);
            newNode->setNodeName(nodeName);
            newNode->addChildNodes(i.value());
            node->addChildNode(newNode);

            // 递归调用
            optimizeGroups(newNode, level + 1);
        }
    }

    // 获取当前节点的所有子节点，进行排序。
    QList<OptimizeNode*> children = node->childNodes();
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

    // 获取每个分组中子节点的最大个数。
    //int maxChildNodes = Config::PathOptimization::maxGroupSize();
    // 如果当前节点下的子节点数大于允许的最大个数，则进行分拆。即在该父节点下，每maxChildNodes个子节点将会
    // 新建一个父节点，将该父节点作为当前父节点的子节点。
    //if (children.count() > maxChildNodes)
    //{
    //    node->childNodes().clear();
    //    OptimizeNode* newNode = nullptr;
    //    for (int i = 0, count = 0; i < children.count(); i++)
    //    {
    //        if ((count++ % maxChildNodes) == 0)
    //        {
    //            newNode = new OptimizeNode();
    //            QString nodeName = QString("vnode_%1_%2").arg(level).arg(node->childNodes().count() + 1);
    //            newNode->setNodeName(nodeName);
    //            node->addChildNode(newNode);
    //        }
    //        newNode->addChildNode(children.at(i));
    //    }
    //    //optimizeGroups(node, level);
    //    qLogD << "  child nodes were seperated into " << node->childNodes().size() << " groups by maxChildNodes.";
    //}

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

        RELATION rel = determineRelationship(primitive->outline(), childPrimitive->outline());
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


