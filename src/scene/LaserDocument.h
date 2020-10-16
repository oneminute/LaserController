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
class LaserScene;
class LayerButton;

class LaserDocument : public QObject
{
    Q_OBJECT
public:
    explicit LaserDocument(LaserScene* scene, QObject* parent = nullptr);
    ~LaserDocument();

    void addItem(LaserPrimitive* item);
    void addItem(LaserPrimitive* item, LaserLayer* layer);
    //void addItem(LaserPrimitive* item, const QString& id);
    void removeItem(LaserPrimitive* item);

    PageInformation pageInformation() const;
    void setPageInformation(const PageInformation& page);
    QRectF pageBounds() const;

    QMap<QString, LaserPrimitive*> items() const;
    LaserPrimitive* laserPrimitive(const QString& id) const;

    QList<LaserLayer*> layers() const;
    void addLayer(LaserLayer* layer);
    void removeLayer(LaserLayer* layer);

    QString newLayerName() const;

    qreal scale() const;
    void setScale(qreal scale);

    void blockSignals(bool block = true);

    bool isOpened() const { return m_isOpened; }

    LaserScene* scene() const { return m_scene; }

    void swapLayers(int i, int j);

    void bindLayerButtons(const QList<LayerButton*>& layerButtons);

    FinishRun& finishRun() { return m_finishRun; }
    void setFinishRun(const FinishRun& value) { m_finishRun = value; }

    static int layersCount() { return m_layersCount; }
    static void setLayersCount(int count) { m_layersCount = count; }

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
    //QList<LaserLayer*> m_engravingLayers;
    //QList<LaserLayer*> m_cuttingLayers;
    //QMap<QString, LaserLayer*> m_layers;
    QList<LaserLayer*> m_layers;

    PageInformation m_pageInfo;
    qreal m_scale;
    bool m_blockSignals;
    bool m_isOpened;
    LaserScene* m_scene;
    FinishRun m_finishRun;

    //static int m_engravingLayersCount;
    //static int m_cuttingLayersCount;
    static int m_layersCount;

    Q_DISABLE_COPY(LaserDocument);
};

#endif // LASERDOCUMENT_H