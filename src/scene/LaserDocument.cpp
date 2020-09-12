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

//int LaserDocument::m_engravingLayersCount(8);
//int LaserDocument::m_cuttingLayersCount(8);
int LaserDocument::m_layersCount(16);

LaserDocument::LaserDocument(LaserScene* scene, QObject* parent)
    : QObject(parent)
    , m_blockSignals(false)
    , m_isOpened(false)
    , m_scene(scene)
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
        m_layers[1]->addItem(item);
    }
    else if (item->isBitmap())
    {
        m_layers[0]->addItem(item);
    }

    updateLayersStructure();
}

void LaserDocument::addItem(LaserPrimitive * item, LaserLayer * layer)
{
    item->layer()->removeItem(item);
    layer->addItem(item);
    updateLayersStructure();
}

void LaserDocument::removeItem(LaserPrimitive * item)
{
    item->layer()->removeItem(item);
    m_items.remove(item->objectName());
    item->deleteLater();
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

QList<LaserLayer*> LaserDocument::layers() const
{
    return m_layers;
}

void LaserDocument::addLayer(LaserLayer* layer)
{
    m_layers.append(layer);

    updateLayersStructure();
}

void LaserDocument::removeLayer(LaserLayer * layer)
{
    LaserLayer* initLayer = nullptr;
    
    int i = m_layers.indexOf(layer);
    if (i < 2)
        return;
    m_layers.removeOne(layer);

    updateLayersStructure();
}

QString LaserDocument::newLayerName(LaserLayerType type) const
{
    QString prefix(tr("Layer"));
    
    int n = m_layers.size() + 1;
    bool used = true;
    QString name;
    while (used)
    {
        used = false;
        name = prefix + QString::number(n);
        for (QList<LaserLayer*>::const_iterator i = m_layers.begin(); i != m_layers.end(); i++)
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
    for (int i = 0; i < m_layers.count(); i++)
    {
        LaserLayer* layer = m_layers[i];
        if (layer->isEmpty())
            continue;
        QJsonObject layerObj;
        layerObj["LayerId"] = i;
        layerObj["MinSpeed"] = layer->minSpeed();
        layerObj["RunSpeed"] = layer->runSpeed();
        layerObj["LaserPower"] = layer->laserPower();
        layerObj["MinSpeedPower"] = layer->minSpeedPower();
        layerObj["RunSpeedPower"] = layer->runSpeedPower();
        if (layer->type() == LLT_ENGRAVING)
        {
            layerObj["CarveForward"] = layer->engravingForward();
            layerObj["CarveStyle"] = layer->engravingStyle();
            layerObj["HStep"] = layer->lineSpacing();
            layerObj["LStep"] = layer->columnSpacing();
            layerObj["ErrorX"] = layer->errorX();
            layerObj["MinSpeedPower"] = layer->minSpeedPower();
            layerObj["RunSpeedPower"] = layer->runSpeedPower();
        }
        
        layerInfo.append(layerObj);

        QList<LaserPrimitive*> laserItems = layer->items();
        for (int li = 0; li < laserItems.size(); li++)
        {
            LaserPrimitive* laserItem = laserItems[li];
            QJsonObject itemObj;
            if (layer->type() == LLT_ENGRAVING)
            {
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
            else if (layer->type() == LLT_CUTTING)
            {
                itemObj["Layer"] = layerId;
                itemObj["FinishRun"] = 0;
                std::vector<cv::Point2f> points = laserItem->cuttingPoints(canvas);
                if (!points.empty())
                {
                    itemObj["Type"] = laserItem->typeName();
                    itemObj["Data"] = QString(pltUtils::points2Plt(points));
                    dataInfo.append(itemObj);
                }
            }
        }
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

void LaserDocument::swapLayers(int i, int j)
{
    LaserLayer* layer = m_layers[i];
    m_layers[i] = m_layers[j];
    m_layers[j] = layer;
    updateLayersStructure();
}

void LaserDocument::bindLayerButtons(const QList<LayerButton*>& layerButtons)
{
    for (int i = 0; i < layersCount(); i++)
    {
        m_layers[i]->bindButton(layerButtons[i]);
    }
    updateLayersStructure();
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
        destroy();
        m_isOpened = false;
        emit closed();
    }
}

void LaserDocument::init()
{
    QString layerName = newLayerName(LLT_ENGRAVING);
    LaserLayer* layer = new LaserLayer(layerName, LLT_ENGRAVING, this);
    addLayer(layer);

    layerName = newLayerName(LLT_CUTTING);
    layer = new LaserLayer(layerName, LLT_CUTTING, this);
    addLayer(layer);

    for (int i = 2; i < m_layersCount; i++)
    {
        QString layerName = newLayerName(LLT_ENGRAVING);
        LaserLayer* layer = new LaserLayer(layerName, LLT_ENGRAVING, this);
        addLayer(layer);
    }

    ADD_TRANSITION(documentEmptyState, documentWorkingState, this, SIGNAL(opened()));
    ADD_TRANSITION(documentWorkingState, documentEmptyState, this, SIGNAL(closed()));
}

