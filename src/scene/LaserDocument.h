#ifndef LASERDOCUMENT_H
#define LASERDOCUMENT_H

#include "common/common.h"

#include <QObject>
#include <QMap>
#include <QPair>
#include <QSharedPointer>
#include <QSharedDataPointer>
#include <QGraphicsItem>

#include "PageInformation.h"
#include "scene/LaserDocumentItem.h"
#include "algorithm/PathOptimizer.h"
#include "algorithm/OptimizeNode.h"

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
    struct StampItem
    {
        LaserLayer* layer;
        QString imagePath;
        QRect bounding;
        QPainterPath path;
    };
    explicit LaserDocument(LaserScene* scene, int layersCount, bool backend, QObject* parent = nullptr);
    ~LaserDocument();

    QMap<QString, LaserPrimitive*> primitives() const;
    LaserPrimitive* laserPrimitive(const QString& id) const;
	QList<LaserPrimitive*> selectedPrimitives() const;

    bool useSpecifiedOrigin() const;
    void setUseSpecifiedOrigin(bool value);

    int specifiedOriginIndex() const;
    void setSpecifiedOriginIndex(int value);

    QList<LaserLayer*> layers() const;
    void addLayer(LaserLayer* layer);
    void removeLayer(LaserLayer* layer);

    QString newLayerName() const;

	bool isOpened() const;

	LaserScene* scene() const;

    void swapLayers(int i, int j);

    void bindLayerButtons(const QList<LayerButton*>& layerButtons);

	FinishRunType& finishRun();
	void setFinishRun(const FinishRunType& value);

	SizeUnit unit() const;
	void setUnit(SizeUnit unit);

    int maxEngravingSpeed() const;

    bool isEmpty() const;

    /// <summary>
    /// 返回作业原点。该点值在画布坐标系下，由当前有效图元列表形成
    /// 的外包矩形9宫格点决定，即，该原点值是相对于外包矩形左上角
    /// 的相对坐标。
    /// </summary>
    /// <param name="docBounding">外包矩形</param>
    /// <returns></returns>
    QPoint reletiveJobOrigin() const;

    /// <summary>
    /// 返回作业原点。该点值在画布坐标系下，由当前有效图元列表形成
    /// 的外包矩形9宫格点决定，即，该原点值是在当前画布上的绝对坐标。
    /// </summary>
    /// <param name="docBounding">外包矩形</param>
    /// <returns></returns>
    QPoint jobOriginOnDocBoundingRect() const;

    QRect absoluteBoundingRect() const;
    QRect absoluteEngravingBoundingRect(bool withAcc) const;

    QRect currentDocBoundingRect() const;
    QRect currentEngravingBoundingRect(bool withAcc) const;

    /// <summary>
    /// 在当前原点下的文档外包框。如果开始方式为相对坐标，则该外包框
    /// 将按照9宫格对应到当前的相对原点下。
    /// </summary>
    /// <param name="includingAccSpan"></param>
    /// <returns></returns>
    QRect currentBoundingRect(const QRect& rect) const;

    /// <summary>
    /// 若是绝对坐标系，则返回(0, 0)；若是相对坐标系，则返回在当前画
    /// 布中文档外包框上9宫格对应的原点。
    /// </summary>
    /// <returns></returns>
    QPoint docOrigin() const;

    /// <summary>
    /// 在画布坐标系下，从图元列表外包框移动到指定原点位置的
    /// 平移变换矩阵。指定原点是指用户原点或当前激光位置。
    /// </summary>
    /// <returns></returns>
    QTransform transformToReletiveOrigin() const;

    bool enablePrintAndCut() const;
    void setEnablePrintAndCut(bool value);
    QTransform transform() const;
    void setTransform(const QTransform& t);
    //QList<QPointF> printAndCutCanvasPoints() const;
    PointPairList printAndCutPointPairs() const;
    void setPrintAndCutPointPairs(const PointPairList& pairs);

    /// <summary>
    /// 返回相对于当前确定的原点的坐标。
    /// </summary>
    /// <returns></returns>
    virtual QPointF position() const { return QPoint(0, 0); }

    void transform(const QTransform& trans);

    LaserLayer* idleLayer() const;

    void addPrimitive(LaserPrimitive* item, bool addToQuadTree = true, bool updateDocBounding = true);
    void addPrimitive(LaserPrimitive* item, LaserLayer* layer, bool addToQuadTree = true, bool updateDocBounding = true);
    void removePrimitive(LaserPrimitive* item, bool keepLayer = true, bool updateDocBounding = true);

    QImage thumbnail() const;
    void setThumbnail(const QImage& image);

    QJsonObject jsonHeader(QRect bounding, QRect boundingAcc, int deviceOriginIndex, int startFrom, QPoint startPos, QPoint lastPoint, bool absolute, const QTransform& t, bool includeThumbnail);
    void jsonBounding(QRect& bounding, QRect& boundingAcc, int& deviceOriginIndex, int& startFrom, QPoint& startPos, QPoint& lastPoint, bool absolute, const QTransform& t = QTransform());

protected:

public slots:
    void exportJSON(const QString& filename, const PathOptimizer::Path& path, ProgressItem* parentProgress, bool exportJson);
    QByteArray exportBoundingJson(bool exportJson);
    void generateBoundingPrimitive();
    void updateLayersStructure();
    void destroy();
    void open();
    void close();
    void outline(ProgressItem* item);
    //void clearOutline(bool clearLayers = false);
    void printOutline(OptimizeNode* node, int level);
    void save(const QString& filename, QWidget* window);
    void load(const QString& filename, QWidget* window);
    void stampBaseLoad(LaserPrimitive* p, QJsonObject& object, bool isLoadAntiFakePath = true);
    int totalNodes();
    void updateDocumentBounding();
    QList<StampItem> generateStampImages();
    void computeStampBasePath(LaserPrimitive* primitive, QPainter& painter, qreal offset, QTransform t1, QTransform t2);
    void computeBoundsPath(LaserPrimitive* primitive, StampItem& item, qreal distance = 500);
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
    //void optimizeGroups(OptimizeNode* node, int level = 1, bool sorted = false);
    //void clearOutline(OptimizeNode* node, bool clearLayers = false);
    void addPrimitiveToNodesTree(LaserPrimitive* primitive, OptimizeNode* node);

    QString newPrimitiveName(LaserPrimitive* primitive);

signals:
    void layersStructureChanged();
    void opened();
    void closed();
    void outlineUpdated();
    //void exportFinished(const QString& filename);
    void exportFinished(const QByteArray& data);

protected:
    Q_DISABLE_COPY(LaserDocument)
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserDocument)

    friend class OptimizeNode;
    friend class OptimizeNodePrivate;
    friend class LaserPrimitive;
    friend class LaserScene;
};

#endif // LASERDOCUMENT_H