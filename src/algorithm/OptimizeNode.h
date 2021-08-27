#ifndef OPTIMIZENODE_H
#define OPTIMIZENODE_H

#include <QObject>
#include <QScopedPointer>
#include <QVector2D>
#include <common/common.h>
#include <opencv2/opencv.hpp>

class ILaserDocumentItem;
class OptimizeNode;
class OptimizeEdge;

class OptimizeNodePrivate;
class OptimizeNode
{
public:
    OptimizeNode(LaserNodeType nodeType = LNT_VIRTUAL, ILaserDocumentItem* item = nullptr);
    ~OptimizeNode();

    QList<OptimizeNode*>& childNodes();
    LaserNodeType nodeType() const;

    QString nodeName() const;
    void setNodeName(const QString& name);

    void addChildNode(OptimizeNode* node);
    void addChildNodes(const QList<OptimizeNode*>& nodes);
    void removeChildNode(OptimizeNode* node);
    void clearChildren();
    bool hasChildren() const;
    int childCount() const;

    bool isVirtual() const;
    bool isDocument() const;
    bool isLayer() const;
    bool isPrimitive() const;

    OptimizeNode* parentNode() const;
    void setParentNode(OptimizeNode* parent);

    QList<OptimizeNode*> findAllLeaves(OptimizeNode* exclude = nullptr);

    QPointF position() const;

    void update();

    ILaserDocumentItem* documentItem() const;

    void addEdge(OptimizeEdge* edge);
    void setOutEdge(OptimizeEdge* edge);

    QList<OptimizeEdge*> edges() const;
    OptimizeEdge* outEdge() const;

    QPointF startPos() const;
    QPointF nearestPoint(const QPointF& point, int& index, float& dist);
    QPointF currentPos() const;
    QVector<QPointF> startingPoints() const;
    bool isClosed() const;
    QPointF headPoint() const;
    QPointF tailPoint() const;
    QPointF point(int index) const;
    void debugDraw(cv::Mat& canvas);

protected:
    QScopedPointer<OptimizeNodePrivate> d_ptr;

    Q_DECLARE_PRIVATE(OptimizeNode)
    Q_DISABLE_COPY(OptimizeNode)
};

#endif // OPTIMIZENODE_H