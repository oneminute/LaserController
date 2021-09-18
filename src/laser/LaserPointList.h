#ifndef LASERPOINTLIST_H
#define LASERPOINTLIST_H

#include <QList>
#include <QPainterPath>
#include <QVector>
#include "LaserPoint.h"
#include <flann/flann.hpp>

class OptimizeNode;
class LaserPointList : public QList<LaserPoint>
{
public:
    explicit LaserPointList();
    ~LaserPointList();

    void buildKdtree();

    void addOptimizeNode(OptimizeNode* node);

    int nearestSearch(const LaserPoint& point);

    OptimizeNode* nearestSearch(OptimizeNode* srcNode);

    QList<QPointF> toPoints() const;

    QPainterPath toPainterPath() const;

private:
    QVector<qreal> m_matrix;
    QVector<qreal> m_weights;
    flann::NNIndex<flann::L2_Simple<qreal>>* m_kdtree;
    QMap<int, OptimizeNode*> m_nodeMap;
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