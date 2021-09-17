#ifndef LASERDOCUMENT_H
#define LASERDOCUMENT_H

#include "common/common.h"

#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <QSharedDataPointer>
#include <QGraphicsItem>

#include "PageInformation.h"
#include "scene/LaserDocumentItem.h"

class LaserDocumentPrivate;
class LaserPrimitive;
class LaserLayer;
class LaserScene;
class LayerButton;

class LaserDocumentPrivate;
class LaserDocument : public QObject, public ILaserDocumentItem
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
	QList<LaserPrimitive*> selectedPrimitives() const;

    QList<LaserLayer*> layers() const;
    void addLayer(LaserLayer* layer);
    void removeLayer(LaserLayer* layer);
	LaserLayer* defaultCuttingLayer() const;
	LaserLayer* defaultEngravingLayer() const;

    QString newLayerName() const;

    void blockSignals(bool block = true);

	bool isOpened() const;

	LaserScene* scene() const;

    void swapLayers(int i, int j);

    void bindLayerButtons(const QList<LayerButton*>& layerButtons);

	FinishRun& finishRun();
	void setFinishRun(const FinishRun& value);

	SizeUnit unit() const;
	void setUnit(SizeUnit unit);

    QPointF docOrigin() const;
    QPointF docOriginMM() const;
    QPointF docOriginMachining() const;
    QTransform docTransform() const;
    QTransform docTransformMM() const;
    QRectF docBoundingRect() const;
    QRectF docBoundingRectMM() const;
    QRectF docBoundingRectMachining() const;

    /// <summary>
    /// 返回相对于当前确定的原点的坐标。
    /// </summary>
    /// <returns></returns>
    virtual QPointF position() const { return QPoint(0, 0); }

public slots:
    void exportJSON(const QString& filename);
    void updateLayersStructure();
    void destroy();
    void open();
    void close();
	void analysis();
    void outline();
    //void clearOutline(bool clearLayers = false);
    void printOutline(OptimizeNode* node, int level);
    void save(const QString& filename, QWidget* window);
    void load(const QString& filename, QWidget* window);
    int totalNodes();

protected:
	void init();
    RELATION determineRelationship(const QPainterPath& a, const QPainterPath& b);
    void outlineByLayers(OptimizeNode* node);
    void outlineByGroups(OptimizeNode* node);
    void clearTree(OptimizeNode* node);
    /// <summary>
    /// 对当前的节点树进行整理，使每个分组树中的叶结点不要超过最大数量。
    /// </summary>
    /// <param name="node">待分组节点</param>
    /// <param name="level">当前的组级别</param>
    void optimizeGroups(OptimizeNode* node, int level = 1);
    //void clearOutline(OptimizeNode* node, bool clearLayers = false);
    void addPrimitiveToNodesTree(LaserPrimitive* primitive, OptimizeNode* node);

signals:
    void layersStructureChanged();
    void opened();
    void closed();
    void outlineUpdated();
    void exportFinished(const QString& filename);

protected:
    Q_DISABLE_COPY(LaserDocument)
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserDocument)

    friend class OptimizeNode;
    friend class OptimizeNodePrivate;
};

#endif // LASERDOCUMENT_H