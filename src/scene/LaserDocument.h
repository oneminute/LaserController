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
class ProgressItem;

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

    QMap<QString, LaserPrimitive*> primitives() const;
    LaserPrimitive* laserPrimitive(const QString& id) const;
	QList<LaserPrimitive*> selectedPrimitives() const;

    QList<LaserLayer*> layers() const;
    void addLayer(LaserLayer* layer);
    void removeLayer(LaserLayer* layer);
	LaserLayer* defaultCuttingLayer() const;
	LaserLayer* defaultEngravingLayer() const;

    QString newLayerName() const;

	bool isOpened() const;

	LaserScene* scene() const;

    void swapLayers(int i, int j);

    void bindLayerButtons(const QList<LayerButton*>& layerButtons);

	FinishRunType& finishRun();
	void setFinishRun(const FinishRunType& value);

	SizeUnit unit() const;
	void setUnit(SizeUnit unit);

    /// <summary>
    /// 返回作业原点。该点值在画布坐标系下，由当前有效图元列表形成
    /// 的外包矩形9宫格点决定，即，该原点值是相对于外包矩形左上角
    /// 的相对坐标。
    /// </summary>
    /// <param name="docBounding">外包矩形</param>
    /// <returns></returns>
    QPointF jobOriginReletiveInScene(const QRectF& docBounding) const;

    QPointF jobOriginReletiveInScene(bool includingAccSpan = false) const;

    QPointF jobOriginReletiveInMech(bool includingAccSpan = false) const;

    /// <summary>
    /// 返回作业原点。该点值在画布坐标系下，由当前有效图元列表形成
    /// 的外包矩形9宫格点决定，即，该原点值是在当前画布上的绝对坐标。
    /// </summary>
    /// <param name="docBounding">外包矩形</param>
    /// <returns></returns>
    QPointF jobOriginInScene() const;

    QPointF jobOriginInDevice() const;

    QPointF jobOriginInMech() const;


    QRectF docBoundingRectInScene(bool includingAccSpan = false) const;
    QRectF docBoundingRectInMech() const;
    QRectF docBoundingRectInDevice(bool includingAccSpan = false) const;

    QRectF machiningDocBoundingRectInScene() const;
    QRectF machiningDocBoundingRectInMech() const;
    QRectF machiningDocBoundingRectInDevice(bool includingAccSpan = false) const;

    QRectF imagesBoundingRectInScene() const;
    QRectF imagesBoundingRectInMech() const;
    QRectF imagesBoundingRectInDevice() const;

    QRectF machiningImagesBoundingRectInScene() const;
    QRectF machiningImagesBoundingRectInMech() const;
    QRectF machiningImagesBoundingRectInDevice() const;

    QPointF docOriginInScene() const;
    QPointF docOriginInMech() const;
    QPointF docOriginInDevice() const;

    /// <summary>
    /// 在画布坐标系下，从图元列表外包框移动到指定原点位置的
    /// 平移变换矩阵。指定原点是指用户原点或当前激光位置。
    /// </summary>
    /// <returns></returns>
    QTransform transformToReletiveOriginInDevice() const;

    bool enablePrintAndCut() const;
    void setEnablePrintAndCut(bool value);
    QTransform printAndCutTransform() const;
    void setPrintAndCutTransform(const QTransform& t);
    //QList<QPointF> printAndCutCanvasPoints() const;
    PointPairList printAndCutPointPairs() const;
    void setPrintAndCutPointPairs(const PointPairList& pairs);

    /// <summary>
    /// 返回相对于当前确定的原点的坐标。
    /// </summary>
    /// <returns></returns>
    virtual QPointF position() const { return QPoint(0, 0); }

    void transform(const QTransform& trans);

public slots:
    void exportJSON(const QString& filename, ProgressItem* parentProgress);
    void exportBoundingJSON();
    void updateLayersStructure();
    void destroy();
    void open();
    void close();
    void outline(ProgressItem* item);
    //void clearOutline(bool clearLayers = false);
    void printOutline(OptimizeNode* node, int level);
    void save(const QString& filename, QWidget* window);
    void load(const QString& filename, QWidget* window);
    int totalNodes();

protected:
	void init();
    void outlineByLayers(OptimizeNode* node, ProgressItem* parentProgress);
    void outlineByGroups(OptimizeNode* node);
    void clearTree(OptimizeNode* node, ProgressItem* parentProgress);
    /// <summary>
    /// 对当前的节点树进行整理，使每个分组树中的叶结点不要超过最大数量。
    /// </summary>
    /// <param name="node">待分组节点</param>
    /// <param name="level">当前的组级别</param>
    void optimizeGroups(OptimizeNode* node, int level = 1, bool sorted = false);
    //void clearOutline(OptimizeNode* node, bool clearLayers = false);
    void addPrimitiveToNodesTree(LaserPrimitive* primitive, OptimizeNode* node);

    QString newPrimitiveName(LaserPrimitive* primitive);

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
    friend class LaserPrimitive;
};

#endif // LASERDOCUMENT_H