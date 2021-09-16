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
    int childCount() const;

    //bool isVirtual() const;
    bool isDocument() const;
    bool isLayer() const;
    bool isPrimitive() const;

    OptimizeNode* parentNode() const;
    void setParentNode(OptimizeNode* parent);

    QList<OptimizeNode*> findAllLeaves(OptimizeNode* exclude = nullptr);
    QList<OptimizeNode*> findLeaves();
    QList<OptimizeNode*> findSiblings(bool onlyLeaves = false);

    QPointF position() const;
    QPointF machiningPosition() const;

    void update(quint32 progressCode = 0, qreal progressQuota = 0);

    ILaserDocumentItem* documentItem() const;

    void addEdge(OptimizeEdge* edge);
    void setOutEdge(OptimizeEdge* edge);

    QList<OptimizeEdge*> edges() const;
    OptimizeEdge* outEdge() const;

    LaserPoint startPos() const;
    LaserPoint nearestPoint(const LaserPoint& point);
    LaserPoint nearestPoint(OptimizeNode* node);
    LaserPoint currentPos(const LaserPoint& hint = LaserPoint()) const;
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
    LaserPointListList arrangeMachiningPoints();
    LaserPointListList arrangedPoints() const;
    LaserPoint arrangedStartingPoint() const;
    LaserPoint arrangedEndingPoint() const;

protected:
    QScopedPointer<OptimizeNodePrivate> d_ptr;

    Q_DECLARE_PRIVATE(OptimizeNode)
    Q_DISABLE_COPY(OptimizeNode)
};

#endif // OPTIMIZENODE_H