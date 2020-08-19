#include "LaserDocument.h"

#include <QSharedData>
#include <QList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QtMath>
#include <QSaveFile>

#include <opencv2/opencv.hpp>

#include "LaserItem.h"
#include "PageInformation.h"
#include "laser/LaserDriver.h"
#include "util/PltUtils.h"
#include "LaserLayer.h"
#include "state/StateController.h"

LaserDocument::LaserDocument(QObject* parent)
    : QObject(parent)
    , m_blockSignals(false)
    , m_isOpened(false)
{
    init();
}

LaserDocument::~LaserDocument()
{
    close();
}

void LaserDocument::addItem(LaserPrimitive * item)
{
    m_items.insert(item->objectName(), item);

    if (item->isShape())
    {
        m_cuttingLayers[0]->addItem(item);
    }
    else if (item->isBitmap())
    {
        m_engravingLayers[0]->addItem(item);
    }

    updateLayersStructure();
}

void LaserDocument::addItem(LaserPrimitive * item, LaserLayer * layer)
{
}

void LaserDocument::removeItem(LaserPrimitive * item)
{
}

PageInformation LaserDocument::pageInformation() const
{
    return m_pageInfo;
}

void LaserDocument::setPageInformation(const PageInformation & page)
{
    m_pageInfo = page;
}

QRectF LaserDocument::pageBounds() const
{
    return QRectF(0, 0, m_pageInfo.width(), m_pageInfo.height());
}

QMap<QString, LaserPrimitive*> LaserDocument::items() const
{
    return m_items;
}

LaserPrimitive * LaserDocument::laserPrimitive(const QString & id) const
{
    return m_items[id];
}

QMap<QString, LaserLayer*> LaserDocument::layers() const
{
    return m_layers;
}

LaserLayer * LaserDocument::laserLayer(const QString & id) const
{
    return m_layers[id];
}

QList<LaserLayer*> LaserDocument::engravingLayers() const
{
    return m_engravingLayers;
}

QList<LaserLayer*> LaserDocument::cuttingLayers() const
{
    return m_cuttingLayers;
}

void LaserDocument::addLayer(LaserLayer* layer)
{
    switch (layer->type())
    {
    case LLT_ENGRAVING:
        m_engravingLayers.append(layer);
        break;
    case LLT_CUTTING:
        m_cuttingLayers.append(layer);
        break;
    }
    m_layers.insert(layer->objectName(), layer);

    updateLayersStructure();
}

void LaserDocument::removeLayer(LaserLayer * layer)
{
    LaserLayer* initLayer = nullptr;
    QList<LaserLayer*>* layers = nullptr;
    if (layer->type() == LLT_CUTTING)
    {
        layers = &m_cuttingLayers;
    }
    else
    {
        layers = &m_engravingLayers;
    }
    initLayer = (*layers)[0];
    for (QList<LaserPrimitive*>::iterator i = layer->items().begin(); i != layer->items().end(); i++)
    {
        LaserPrimitive* item = *i;
        initLayer->addItem(item);
    }
    layers->removeOne(layer);
    m_layers.remove(layer->objectName());

    updateLayersStructure();
}

QString LaserDocument::newLayerName(LaserLayerType type) const
{
    QList<LaserLayer*> layers;
    QString prefix;
    switch (type)
    {
    case LLT_ENGRAVING:
        layers = m_engravingLayers;
        prefix = tr("Engraving");
        break;
    case LLT_CUTTING:
        layers = m_cuttingLayers;
        prefix = tr("Cutting");
        break;
    }
    int n = layers.size() + 1;
    bool used = true;
    QString name;
    while (used)
    {
        used = false;
        name = prefix + QString::number(n);
        for (QList<LaserLayer*>::iterator i = layers.begin(); i != layers.end(); i++)
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
    return m_scale;
}

void LaserDocument::setScale(qreal scale)
{
    scale = scale;
}

void LaserDocument::exportJSON(const QString& filename)
{
    QFile saveFile(filename);

    QJsonObject jsonObj;

    QJsonObject laserDocumentInfo;
    laserDocumentInfo["APIVersion"] = LaserDriver::instance().getVersion();
    laserDocumentInfo["CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    laserDocumentInfo["PrinterDrawUnit"] = 1016;
    jsonObj["LaserDocumentInfo"] = laserDocumentInfo;

    QJsonArray layerInfo;
    cv::Mat canvas(m_pageInfo.height() * 40, m_pageInfo.width() * 40, CV_8UC3, cv::Scalar(255, 255, 255));
    int layerId = 0;
    QJsonArray dataInfo;
    {
        for (int i = 0; i < m_cuttingLayers.size(); i++)
        {
            LaserLayer* layer = m_cuttingLayers[i];
            //QString layerId = "Layer" + QString::number(i + 1);
            QJsonObject layerObj;
            layerObj["LayerId"] = layerId;
            layerObj["MinSpeed"] = layer->minSpeed();
            layerObj["RunSpeed"] = layer->runSpeed();
            layerObj["MoveSpeed"] = layer->moveSpeed();
            layerObj["LaserPower"] = layer->laserPower();
            layerObj["MinSpeedPower"] = layer->minSpeedPower();
            layerObj["RunSpeedPower"] = layer->runSpeedPower();
            //layerInfo[layerId] = layerObj;
            layerInfo.append(layerObj);

            QList<LaserPrimitive*> laserItems = layer->items();
            for (int li = 0; li < laserItems.size(); li++)
            {
                LaserPrimitive* laserItem = laserItems[li];
                QJsonObject itemObj;
                itemObj["Layer"] = layerId;
                //itemObj["PrinterDrawUnit"] = 1016;
                itemObj["FinishRun"] = 0;
                std::vector<cv::Point2f> points = laserItem->cuttingPoints(canvas);
                if (!points.empty())
                {
                    itemObj["Type"] = laserItem->typeName();
                    itemObj["Data"] = QString(pltUtils::points2Plt(points));
                    dataInfo.append(itemObj);
                }
            }
            layerId++;
        }
    }
    {
        QJsonObject imageObj;
        for (int i = 0; i < m_engravingLayers.size(); i++)
        {
            LaserLayer* layer = m_engravingLayers[i];
            //QString layerId = "Layer" + QString::number(i + 1);
            QJsonObject layerObj;
            layerObj["LayerId"] = layerId;
            layerObj["CarveForward"] = layer->engravingForward();
            layerObj["CarveStyle"] = layer->engravingStyle();
            layerObj["MinSpeed"] = layer->minSpeed();
            layerObj["RunSpeed"] = layer->runSpeed();
            layerObj["LaserPower"] = layer->laserPower();
            layerObj["HStep"] = layer->lineSpacing();
            layerObj["LStep"] = layer->columnSpacing();
            layerObj["ErrorX"] = layer->errorX();
            layerObj["ErrorY"] = layer->errorY();
            layerObj["MinSpeedPower"] = layer->minSpeedPower();
            layerObj["RunSpeedPower"] = layer->runSpeedPower();

            QList<LaserPrimitive*> laserItems = layer->items();
            for (int li = 0; li < laserItems.size(); li++)
            {
                LaserPrimitive* laserItem = laserItems[li];
                QJsonObject itemObj;

                itemObj["Layer"] = layerId;
                itemObj["FinishRun"] = 0;
                QPointF pos = laserItem->laserStartPos();
                itemObj["StartX"] = laserItem->boundingRect().left();
                itemObj["StartY"] = laserItem->boundingRect().top();
                layerObj["StartX"] = laserItem->boundingRect().left();
                layerObj["StartY"] = laserItem->boundingRect().top();
                layerObj["Width"] = laserItem->boundingRect().width();
                layerObj["Height"] = laserItem->boundingRect().height();
                
                QByteArray data = laserItem->engravingImage(canvas);
                if (!data.isEmpty())
                {
                    itemObj["Type"] = "PNG";
                    itemObj["Data"] = QString(data.toBase64());
                    dataInfo.append(itemObj);
                }
            }
            layerInfo.append(layerObj);
        }
        layerId++;
    }
    QJsonObject actionObj;

    jsonObj["LayerInfo"] = layerInfo;
    jsonObj["DataInfo"] = dataInfo;

    QJsonDocument jsonDoc(jsonObj);

    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qWarning("Couldn't open save file.");
        return;
    }

    saveFile.write(jsonDoc.toJson(QJsonDocument::Indented));

    cv::imwrite("canvas_test.png", canvas);
}

void LaserDocument::blockSignals(bool block)
{
    m_blockSignals = block;
}

void LaserDocument::updateLayersStructure()
{
    if (!m_blockSignals)
        emit layersStructureChanged();
}

void LaserDocument::destroy()
{
    emit readyToDestroyed();
    deleteLater();
}

void LaserDocument::open()
{
    m_isOpened = true;
    emit opened();
}

void LaserDocument::close()
{
    if (m_isOpened)
    {
        m_isOpened = false;
        emit closed();
    }
}

void LaserDocument::init()
{
    QString layerName = newLayerName(LLT_ENGRAVING);
    addLayer(new LaserLayer(layerName, LLT_ENGRAVING, this));

    layerName = newLayerName(LLT_CUTTING);
    addLayer(new LaserLayer(layerName, LLT_CUTTING, this));

    ADD_TRANSITION(documentEmptyState, documentWorkingState, this, SIGNAL(opened()));
    ADD_TRANSITION(documentWorkingState, documentEmptyState, this, SIGNAL(closed()));
}

