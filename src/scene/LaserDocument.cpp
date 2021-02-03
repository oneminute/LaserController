#include "LaserDocument.h"

#include <QDateTime>
#include <QFile>
#include <QList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtMath>
#include <QSaveFile>
#include <QStack>
#include <QSharedData>

#include <opencv2/opencv.hpp>

#include "LaserPrimitive.h"
#include "PageInformation.h"
#include "common/Config.h"
#include "laser/LaserDriver.h"
#include "util/PltUtils.h"
#include "LaserLayer.h"
#include "state/StateController.h"

class LaserDocumentPrivate
{
	Q_DECLARE_PUBLIC(LaserDocument)
public:
	LaserDocumentPrivate(LaserDocument* ptr)
		: q_ptr(ptr)
		, blockSignals(false)
		, isOpened(false)
	{}
    QMap<QString, LaserPrimitive*> primitives;
    QList<LaserLayer*> layers;
    PageInformation pageInfo;
    qreal scale;
    bool blockSignals;
    bool isOpened;
    LaserScene* scene;
    FinishRun finishRun;
	SizeUnit unit;

	LaserDocument* q_ptr;
};

LaserDocument::LaserDocument(LaserScene* scene, QObject* parent)
    : QObject(parent)
	, d_ptr(new LaserDocumentPrivate(this))
{
	d_ptr->scene = scene;
    init();
}

LaserDocument::~LaserDocument()
{
    close();
}

void LaserDocument::addPrimitive(LaserPrimitive * item)
{
	Q_D(LaserDocument);
    d->primitives.insert(item->objectName(), item);

    if (item->isShape())
    {
        d->layers[1]->addPrimitive(item);
    }
    else if (item->isBitmap())
    {
        d->layers[0]->addPrimitive(item);
    }

    updateLayersStructure();
}

void LaserDocument::addPrimitive(LaserPrimitive * item, LaserLayer * layer)
{
    item->layer()->removePrimitive(item);
    layer->addPrimitive(item);
    updateLayersStructure();
}

void LaserDocument::removePrimitive(LaserPrimitive * item)
{
	Q_D(LaserDocument);
    item->layer()->removePrimitive(item);
    d->primitives.remove(item->objectName());
    item->deleteLater();
}

PageInformation LaserDocument::pageInformation() const
{
	Q_D(const LaserDocument);
    return d->pageInfo;
}

void LaserDocument::setPageInformation(const PageInformation & page)
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

LaserPrimitive * LaserDocument::laserPrimitive(const QString & id) const
{
	Q_D(const LaserDocument);
    return d->primitives[id];
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

void LaserDocument::removeLayer(LaserLayer * layer)
{
	Q_D(LaserDocument);
    LaserLayer* initLayer = nullptr;
    
    int i = d->layers.indexOf(layer);
    if (i < 2)
        return;
    d->layers.removeOne(layer);

    updateLayersStructure();
}

LaserLayer * LaserDocument::defaultCuttingLayer() const
{
	Q_D(const LaserDocument);
	return d->layers[1];
}

LaserLayer * LaserDocument::defaultEngravingLayer() const
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

qreal LaserDocument::scale() const
{
	Q_D(const LaserDocument);
    return d->scale;
}

void LaserDocument::setScale(qreal scale)
{
	Q_D(LaserDocument);
    d->scale = scale;
}

void LaserDocument::exportJSON(const QString& filename)
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
                QList<QPainterPath> paths = laserItem->subPaths();
                if (paths.isEmpty())
                {
                    std::vector<cv::Point2f> points = laserItem->cuttingPoints(canvas);
                    if (!points.empty())
                    {
                        itemObj["Type"] = laserItem->typeLatinName();
                        itemObj["Data"] = QString(pltUtils::points2Plt(points));
                        add = true;
                    }
                }
                else
                {
                    QString pltString;
                    for (QPainterPath subPath : paths)
                    {
                        std::vector<cv::Point2f> points;
                        if (pltUtils::pathPoints(subPath, points, canvas))
                        {
                            pltString.append(QString(pltUtils::points2Plt(points)));
                        }
                    }
                    if (!pltString.isEmpty())
                    {
                        itemObj["Type"] = laserItem->typeLatinName();
                        itemObj["Data"] = pltString;
                        add = true;
                    }
                }
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

LaserScene * LaserDocument::scene() const 
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
    for (int i = 0; i < Config::LayersMaxLayersCount(); i++)
    {
        d->layers[i]->bindButton(layerButtons[i]);
    }
    updateLayersStructure();
}

FinishRun & LaserDocument::finishRun() 
{ 
	Q_D(LaserDocument);
	return d->finishRun; 
}

void LaserDocument::setFinishRun(const FinishRun & value)
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
        destroy();
        d->isOpened = false;
        emit closed();
    }
}

void LaserDocument::analysis()
{
	Q_D(LaserDocument);
	qDebug() << "begin analysising";

	for (LaserPrimitive* primitive : d->primitives)
	{
		qDebug() << primitive->objectName() << primitive->primitiveType();
		if (primitive->primitiveType() == LPT_PATH)
		{
			LaserPath* laserPath = qobject_cast<LaserPath*>(primitive);
			QList<QPainterPath> subPaths = laserPath->subPaths();
			for (int i = 0; i < subPaths.size(); i++)
			{
				qDebug() << "sub path" << i << ":" << subPaths[i];
			}
		}
	}
}

void LaserDocument::outline()
{
    QStack<LaserPrimitive*> stack;
    QList<LaserPrimitive*> outlineTree;
    for (LaserPrimitive* primitive : primitives().values())
    {
        stack.push(primitive);
    }

    while (!stack.isEmpty())
    {
        LaserPrimitive* candidate = stack.pop();

        // first primitive
        if (outlineTree.isEmpty())
        {
            outlineTree.append(candidate);
            continue;
        }

        // determine relation between candidate and each primitive within outlineTree
        for (LaserPrimitive* treeNode : outlineTree)
        {

        }
    }
    emit outlineUpdated();
}

void LaserDocument::init()
{
    QString layerName = newLayerName();
    LaserLayer* layer = new LaserLayer(layerName, LLT_ENGRAVING, this, true);
    addLayer(layer);

    layerName = newLayerName();
    layer = new LaserLayer(layerName, LLT_CUTTING, this, true);
    addLayer(layer);

    for (int i = 2; i < Config::LayersMaxLayersCount(); i++)
    {
        QString layerName = newLayerName();
        LaserLayer* layer = new LaserLayer(layerName, LLT_ENGRAVING, this);
        addLayer(layer);
    }

    ADD_TRANSITION(documentEmptyState, documentWorkingState, this, SIGNAL(opened()));
    ADD_TRANSITION(documentWorkingState, documentEmptyState, this, SIGNAL(closed()));
}

