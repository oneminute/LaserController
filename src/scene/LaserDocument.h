#ifndef LASERDOCUMENT_H
#define LASERDOCUMENT_H

#include "common/common.h"

#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <QSharedDataPointer>

#include "PageInformation.h"

class LaserDocumentPrivate;
class LaserPrimitive;
class LaserLayer;

class LaserDocument : public QObject
{
    Q_OBJECT
public:
    explicit LaserDocument(QObject* parent = nullptr);
    ~LaserDocument();

    void addItem(LaserPrimitive* item);
    void addItem(LaserPrimitive* item, LaserLayer* layer);
    void addItem(LaserPrimitive* item, const QString& id);
    void removeItem(LaserPrimitive* item);

    PageInformation pageInformation() const;
    void setPageInformation(const PageInformation& page);
    QRectF pageBounds() const;

    QMap<QString, LaserPrimitive*> items() const;
    LaserPrimitive* laserPrimitive(const QString& id) const;

    QMap<QString, LaserLayer*> layers() const;
    LaserLayer* laserLayer(const QString& id) const;
    QList<LaserLayer*> engravingLayers() const;
    QList<LaserLayer*> cuttingLayers() const;
    void addLayer(LaserLayer* layer);
    void removeLayer(LaserLayer* layer);

    QString newLayerName(LaserLayerType type) const;

    qreal scale() const;
    void setScale(qreal scale);

    void blockSignals(bool block = true);

    bool isOpened() const { return m_isOpened; }

    static int engravingLayersCount();
    static void setEngravingLayersCount(int count);

    static int cuttingLayersCount();
    static void setCuttingLayersCount(int count);

public slots:
    void exportJSON(const QString& filename);
    void updateLayersStructure();
    void destroy();
    void open();
    void close();

protected:
    void init();

signals:
    void layersStructureChanged();
    void readyToDestroyed();
    void opened();
    void closed();

private:
    QMap<QString, LaserPrimitive*> m_items;
    QList<LaserLayer*> m_engravingLayers;
    QList<LaserLayer*> m_cuttingLayers;
    QMap<QString, LaserLayer*> m_layers;

    PageInformation m_pageInfo;
    qreal m_scale;
    bool m_blockSignals;
    bool m_isOpened;

    static int m_engravingLayersCount;
    static int m_cuttingLayersCount;

    Q_DISABLE_COPY(LaserDocument);
};

#endif // LASERDOCUMENT_H