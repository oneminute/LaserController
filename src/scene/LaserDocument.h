#ifndef LASERDOCUMENT_H
#define LASERDOCUMENT_H

#include "common/common.h"

#include <QObject>
#include <QSharedPointer>
#include <QSharedDataPointer>

#include "PageInformation.h"

class LaserDocumentPrivate;
class LaserItem;
class LaserLayer;

class LaserDocument : public QObject
{
    Q_OBJECT
public:
    explicit LaserDocument(QObject* parent = nullptr);
    ~LaserDocument();

    void addItem(LaserItem* item);
    void addItem(LaserItem* item, LaserLayer* layer);
    void removeItem(LaserItem* item);

    PageInformation pageInformation() const;
    void setPageInformation(const PageInformation& page);
    QRectF pageBounds() const;

    QList<LaserItem*> items() const;

    QList<LaserLayer*> layers() const;
    QList<LaserLayer*> engravingLayers() const;
    QList<LaserLayer*> cuttingLayers() const;
    void addLayer(LaserLayer* layer);

    QString newLayerName(LayerType type) const;

    qreal scale() const;
    void setScale(qreal scale);

    void blockSignals(bool block = true);

public slots:
    void exportJSON(const QString& filename);
    void updateLayersStructure();
    void destroy();

protected:
    void init();

signals:
    void layersStructureChanged();
    void readyToDestroyed();

private:
    QList<LaserItem*> m_items;
    QList<LaserLayer*> m_engravingLayers;
    QList<LaserLayer*> m_cuttingLayers;
    PageInformation m_pageInfo;
    qreal m_scale;
    bool m_blockSignals;

    Q_DISABLE_COPY(LaserDocument);
};

#endif // LASERDOCUMENT_H