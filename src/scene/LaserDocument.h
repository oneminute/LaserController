#ifndef LASERDOCUMENT_H
#define LASERDOCUMENT_H

#include "common/common.h"

#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <QSharedDataPointer>
#include <QGraphicsItem>

#include "PageInformation.h"

class LaserDocumentPrivate;
class LaserPrimitive;
class LaserLayer;
class LaserScene;
class LayerButton;

class LaserDocumentPrivate;
class LaserDocument : public QObject
{
    Q_OBJECT
public:
    explicit LaserDocument(LaserScene* scene, QObject* parent = nullptr);
    ~LaserDocument();

    void addPrimitive(LaserPrimitive* item);
    void addPrimitive(LaserPrimitive* item, LaserLayer* layer);
    void removePrimitive(LaserPrimitive* item);

    PageInformation pageInformation() const;
    void setPageInformation(const PageInformation& page);
    QRectF pageBounds() const;

    QMap<QString, LaserPrimitive*> primitives() const;
    LaserPrimitive* laserPrimitive(const QString& id) const;

    QList<LaserLayer*> layers() const;
    void addLayer(LaserLayer* layer);
    void removeLayer(LaserLayer* layer);
	LaserLayer* defaultCuttingLayer() const;
	LaserLayer* defaultEngravingLayer() const;

    QString newLayerName() const;

    qreal scale() const;
    void setScale(qreal scale);

    void blockSignals(bool block = true);

	bool isOpened() const;

	LaserScene* scene() const;

    void swapLayers(int i, int j);

    void bindLayerButtons(const QList<LayerButton*>& layerButtons);

	FinishRun& finishRun();
	void setFinishRun(const FinishRun& value);

	SizeUnit unit() const;
	void setUnit(SizeUnit unit);

    //static int layersCount() { return m_layersCount; }
    //static void setLayersCount(int count) { m_layersCount = count; }

public slots:
    void exportJSON(const QString& filename);
    void updateLayersStructure();
    void destroy();
    void open();
    void close();
	void analysis();
    void outline();
    void printOutline(LaserPrimitive* primitive, int level);

protected:
    void init();
    RELATION determineRelationship(const QPainterPath& a, const QPainterPath& b);
    bool iterateOutlineNodes(LaserPrimitive* candidate, QList<QGraphicsItem*>& nodes);

signals:
    void layersStructureChanged();
    void opened();
    void closed();
    void outlineUpdated();

protected:
	QScopedPointer<LaserDocumentPrivate> d_ptr;

    Q_DISABLE_COPY(LaserDocument);
	Q_DECLARE_PRIVATE(LaserDocument);
};

#endif // LASERDOCUMENT_H