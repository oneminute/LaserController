#include "LaserDocument.h"

#include <QSharedData>
#include <QList>

#include "LaserItem.h"
#include "PageInformation.h"

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

}

LaserDocument::LaserDocument(const LaserDocument& other, QObject* parent)
    : QObject(parent)
    , d_ptr(other.d_ptr)
{

}

LaserDocument::~LaserDocument()
{
}

void LaserDocument::addItem(LaserItem * item)
{
    d_ptr->items.append(item);
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

