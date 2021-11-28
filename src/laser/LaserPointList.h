#ifndef LASERPOINTLIST_H
#define LASERPOINTLIST_H

#include <QList>
#include <QPainterPath>
#include <QVector>
#include <QMap>
#include "LaserPoint.h"
#include <flann/flann.hpp>

class OptimizeNode;
class LaserPointList : public QList<LaserPoint>
{
public:
    explicit LaserPointList();
    ~LaserPointList();

    void buildKdtree();

    void addNode(OptimizeNode* node);

    void removeNode(OptimizeNode* node);

    void addNodes(const QSet<OptimizeNode*>& nodes);

    int nearestSearch(const LaserPoint& point);

    OptimizeNode* nearestSearch(OptimizeNode* srcNode, bool remove = true);

    QList<QPoint> toPoints() const;

    QPainterPath toPainterPath() const;

private:
    QVector<int> m_matrix;
    QVector<int> m_weights;
    flann::NNIndex<flann::L2_Simple<int>>* m_kdtree;
    QMap<int, OptimizeNode*> m_indexNodeMap;
    QMultiMap<OptimizeNode*, int> m_nodeIndicesMap;
    QMap<int, int> m_indexMap;
};

class LaserPointListList : public QList<LaserPointList>
{
public:
    explicit LaserPointListList();
    ~LaserPointListList();

    QPainterPath toPainterPath() const;
};

#endif // LASERPOINTLIST_H