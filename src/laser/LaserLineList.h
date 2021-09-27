#ifndef LASERLINELIST_H
#define LASERLINELIST_H

#include <QList>
#include <QLineF>
#include <flann/flann.hpp>

class LaserLineList : public QList<QLineF>
{
public:
    explicit LaserLineList();
    ~LaserLineList();

    void buildKdtree();

    QLineF nearestSearch(const QPointF& point, bool remove = true);

private:
    flann::NNIndex<flann::L2_Simple<qreal>>* m_kdtree;
    QVector<qreal> m_matrix;
};

class LaserLineListList : public QList<LaserLineList>
{
public:
    explicit LaserLineListList();
    ~LaserLineListList();
};

#endif // LASERLINELIST_H