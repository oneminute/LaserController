#include "LaserDocument.h"

#include <QSharedData>
#include <QList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QtMath>

#include <opencv2/opencv.hpp>

#include "LaserItem.h"
#include "PageInformation.h"
#include "laser/LaserDriver.h"
#include "util/PltUtils.h"

class LaserDocumentPrivate: public QSharedData
{
public:
    LaserDocumentPrivate()
        : scale(1.0)
    {}

    ~LaserDocumentPrivate()
    {
        qDeleteAll(items);
    }

private:
    QList<LaserItem*> items;
    QList<LaserLayer> engravingLayers;
    QList<LaserLayer> cuttingLayers;
    PageInformation pageInfo;
    qreal scale;

    friend class LaserDocument;
};

LaserDocument::LaserDocument(QObject* parent)
    : QObject(parent)
    , d_ptr(new LaserDocumentPrivate)
{
    init();
}

LaserDocument::LaserDocument(const LaserDocument& other, QObject* parent)
    : QObject(parent)
    , d_ptr(other.d_ptr)
{
    init();
}

LaserDocument::~LaserDocument()
{
}

void LaserDocument::addItem(LaserItem * item)
{
    d_ptr->items.append(item);

    if (item->isShape())
    {
        d_ptr->cuttingLayers[0].addItem(item);
    }
    else if (item->isBitmap())
    {
        d_ptr->engravingLayers[0].addItem(item);
    }
}

PageInformation LaserDocument::pageInformation() const
{
    return d_ptr->pageInfo;
}

QList<LaserItem*> LaserDocument::items() const
{
    return d_ptr->items;
}

QList<LaserLayer> LaserDocument::layers() const
{
    return d_ptr->engravingLayers + d_ptr->cuttingLayers;
}

QList<LaserLayer> LaserDocument::engravingLayers() const
{
    return d_ptr->engravingLayers;
}

QList<LaserLayer> LaserDocument::cuttingLayers() const
{
    return d_ptr->cuttingLayers;
}

void LaserDocument::addLayer(const LaserLayer & layer)
{
    switch (layer.type())
    {
    case LaserLayer::LLT_ENGRAVING:
        d_ptr->engravingLayers.append(layer);
        break;
    case LaserLayer::LLT_CUTTING:
        d_ptr->cuttingLayers.append(layer);
        break;
    }
}

QString LaserDocument::newLayerName(LaserLayer::LayerType type) const
{
    QList<LaserLayer> layers;
    QString prefix;
    switch (type)
    {
    case LaserLayer::LLT_ENGRAVING:
        layers = d_ptr->engravingLayers;
        prefix = tr("Engraving");
        break;
    case LaserLayer::LLT_CUTTING:
        layers = d_ptr->cuttingLayers;
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
        for (QList<LaserLayer>::iterator i = layers.begin(); i != layers.end(); i++)
        {
            if (i->id() == name)
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
    return d_ptr->scale;
}

void LaserDocument::setScale(qreal scale)
{
    d_ptr->scale = scale;
}

void LaserDocument::exportJSON()
{
    QJsonObject jsonObj;

    QJsonObject laserDocumentInfo;
    laserDocumentInfo["APIVersion"] = LaserDriver::instance().getVersion();
    laserDocumentInfo["CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    jsonObj["LaserDocumentInfo"] = laserDocumentInfo;

    QJsonObject dataInfo;
    {
        QJsonObject incisionInfo;
        QJsonArray layerInfo;
        QJsonObject polygonObj;
        QJsonArray elementsArray;
        for (int i = 0; i < d_ptr->cuttingLayers.size(); i++)
        {
            LaserLayer layer = d_ptr->cuttingLayers[i];
            //QString layerId = "Layer" + QString::number(i + 1);
            QJsonObject layerObj;
            layerObj["LayerId"] = i;
            layerObj["MinSpeed"] = layer.minSpeed();
            layerObj["RunSpeed"] = layer.runSpeed();
            layerObj["MoveSpeed"] = layer.moveSpeed();
            layerObj["LaserPower"] = layer.laserPower();
            layerObj["MinSpeedPower"] = layer.minSpeedPower();
            layerObj["RunSpeedPower"] = layer.runSpeedPower();
            //layerInfo[layerId] = layerObj;
            layerInfo.append(layerObj);

            QList<LaserItem*> laserItems = layer.items();
            for (int li = 0; li < laserItems.size(); li++)
            {
                LaserItem* laserItem = laserItems[li];
                QJsonObject itemObj;
                itemObj["Layer"] = i;
                itemObj["PrinterDrawUnit"] = 1016;
                std::vector<cv::Point2f> points = laserItem->cuttingPoints();
                if (!points.empty())
                {
                    itemObj["Points"] = QString(pltUtils::points2Plt(points));
                    elementsArray.append(itemObj);
                }
            }
        }
        polygonObj["Elements"] = elementsArray;
        incisionInfo["LayerInfo"] = layerInfo;
        incisionInfo["Polygon"] = polygonObj;
        dataInfo["IncisionInfo"] = incisionInfo;
    }
    {
        QJsonObject carveInfo;
        QJsonArray layerInfo;
        QJsonObject imageObj;
        QJsonArray elementsArray;
        for (int i = 0; i < d_ptr->engravingLayers.size(); i++)
        {
            LaserLayer layer = d_ptr->engravingLayers[i];
            //QString layerId = "Layer" + QString::number(i + 1);
            QJsonObject layerObj;
            layerObj["CarveForward"] = layer.engravingForward();
            layerObj["CarveStyle"] = layer.engravingStyle();
            layerObj["MinSpeed"] = layer.minSpeed();
            layerObj["RunSpeed"] = layer.runSpeed();
            layerObj["LaserPower"] = layer.laserPower();

            layerObj["HStep"] = layer.lineSpacing();
            layerObj["LStep"] = layer.columnSpacing();
            //layerObj["StartX"] = layer.startX();
            //layerObj["StartY"] = layer.startY();
            layerObj["ErrorX"] = layer.errorX();
            layerObj["ErrorY"] = layer.errorY();
            //layerInfo[layerId] = layerObj;
            layerInfo.append(layerObj);

            QList<LaserItem*> laserItems = layer.items();
            for (int li = 0; li < laserItems.size(); li++)
            {
                LaserItem* laserItem = laserItems[li];
                QJsonObject itemObj;

                itemObj["Layer"] = i;
                itemObj["ImageFormat"] = "png";
                QPointF pos = laserItem->laserStartPos();
                itemObj["StartX"] = qFloor(pos.x());
                itemObj["StartY"] = qFloor(pos.y());
                
                QByteArray data = laserItem->engravingImage();
                if (!data.isEmpty())
                {
                    itemObj["ImageData"] = QString(data.toBase64());
                    elementsArray.append(itemObj);
                }
            }
        }
        imageObj["Elements"] = elementsArray;
        carveInfo["LayerInfo"] = layerInfo;
        carveInfo["Image"] = imageObj;
        dataInfo["CarveInfo"] = carveInfo;
    }

    jsonObj["DataInfo"] = dataInfo;

    QJsonDocument jsonDoc(jsonObj);
    qDebug().noquote() << jsonDoc.toJson(QJsonDocument::Indented);
}

void LaserDocument::init()
{
    QString layerName = newLayerName(LaserLayer::LLT_ENGRAVING);
    addLayer(LaserLayer(layerName, LaserLayer::LLT_ENGRAVING));

    layerName = newLayerName(LaserLayer::LLT_CUTTING);
    addLayer(LaserLayer(layerName, LaserLayer::LLT_CUTTING));
}

