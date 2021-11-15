#ifndef OPTIMIZENODE_H
#define OPTIMIZENODE_H

#include <QObject>
#include <QScopedPointer>
#include <QVector2D>
#include <common/common.h>
#include <opencv2/opencv.hpp>

#include "laser/LaserPointList.h"
#include "laser/LaserPoint.h"

class ILaserDocumentItem;
class OptimizeNode;
class OptimizeEdge;
class LaserPrimitive;
class ProgressItem;
class LaserLayer;

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
    void clearEdges();
    bool hasChildren() const;
    bool isLeaf() const;
    int childCount() const;

    //bool isVirtual() const;
    bool isDocument() const;
    bool isLayer() const;
    bool isPrimitive() const;

    bool exportable() const;

    OptimizeNode* parentNode() const;
    void setParentNode(OptimizeNode* parent);

    QSet<OptimizeNode*> findAllLeaves(const QSet<OptimizeNode*>& excludes = QSet<OptimizeNode*>());
    QSet<OptimizeNode*> findLeaves(const QSet<OptimizeNode*>& excludes = QSet<OptimizeNode*>());
    QSet<OptimizeNode*> findSiblings(bool onlyLeaves = false);
    void findSiblings(QSet<OptimizeNode*>& leaves, QSet<OptimizeNode*>& branches,
        const QSet<OptimizeNode*>& excludes = QSet<OptimizeNode*>());

    QPointF positionInScene() const;
    QPointF positionInDevice() const;

    void update(ProgressItem* parentProgress);

    ILaserDocumentItem* documentItem() const;

    void addEdge(OptimizeEdge* edge);
    void setOutEdge(OptimizeEdge* edge);

    QList<OptimizeEdge*> edges() const;
    OptimizeEdge* outEdge() const;

    LaserPoint startPos() const;
    LaserPoint nearestPoint(const LaserPoint& point);
    LaserPoint nearestPoint(OptimizeNode* node);
    LaserPoint currentPos(const LaserPoint& hint = LaserPoint()) const;
    void setCurrentPos(const LaserPoint& point);
    void setCurrentIndex(int index);
    LaserPoint lastPoint() const;
    void setLastPoint(const LaserPoint& point);

    LaserPointList& startingPoints();

    bool isClosed() const;
    LaserPoint headPoint() const;
    LaserPoint tailPoint() const;
    LaserPoint point(int index) const;

    bool isVirtual() const;

    LaserPrimitive* primitive() const;
    LaserLayer* layer() const;
    LaserPointListList arrangeMachiningPoints();
    LaserPointListList arrangedPoints() const;
    LaserPoint arrangedStartingPoint() const;
    LaserPoint arrangedEndingPoint() const;

    QSet<int> laneIndices();

protected:
    QScopedPointer<OptimizeNodePrivate> d_ptr;

    Q_DECLARE_PRIVATE(OptimizeNode)
    Q_DISABLE_COPY(OptimizeNode)
};

#endif // OPTIMIZENODE_H