#ifndef LASERLINELIST_H
#define LASERLINELIST_H

#include <QList>
#include <QLine>
#include <flann/flann.hpp>

class LaserLineList : public QList<QLine>
{
public:
    explicit LaserLineList();
    ~LaserLineList();

    void buildKdtree();

    QLine nearestSearch(const QPoint& point, bool remove = true);

private:
    flann::NNIndex<flann::L2_Simple<int>>* m_kdtree;
    QVector<qreal> m_matrix;
};

class LaserLineListList : public QList<LaserLineList>
{
public:
    explicit LaserLineListList();
    ~LaserLineListList();
};

#endif // LASERLINELIST_H