#ifndef LASERDOCUMENT_H
#define LASERDOCUMENT_H

#include <QObject>
#include <QSharedPointer>
#include <QSharedDataPointer>

#include "PageInformation.h"
#include "LaserLayer.h"

class LaserDocumentPrivate;
class LaserItem;

class LaserDocument : public QObject
{
    Q_OBJECT
public:
    explicit LaserDocument(QObject* parent = nullptr);
    LaserDocument(const LaserDocument& other, QObject* parent = nullptr);
    ~LaserDocument();

    void addItem(LaserItem* item);

    PageInformation pageInformation() const;

    QList<LaserItem*> items() const;

    QList<LaserLayer> layers() const;
    QList<LaserLayer> engravingLayers() const;
    QList<LaserLayer> cuttingLayers() const;
    void addLayer(const LaserLayer& layer);

    QString newLayerName(LaserLayer::LayerType type) const;

    qreal scale() const;
    void setScale(qreal scale);

protected:
    void init();

private:
    QSharedDataPointer<LaserDocumentPrivate> d_ptr;
    friend class LaserDocumentPrivate;
};

#endif // LASERDOCUMENT_H