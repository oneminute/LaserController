#include "LaserLineList.h"

#include <util/Utils.h>

LaserLineList::LaserLineList()
    : QList<QLine>()
    , m_kdtree(nullptr)
{
}

LaserLineList::~LaserLineList()
{
    if (m_kdtree)
    {
        delete m_kdtree;
        m_kdtree = nullptr;
    }


}

void LaserLineList::buildKdtree()
{
    if (isEmpty())
        return;

    if (m_kdtree)
    {
        delete m_kdtree;
    }

    m_matrix.resize(4 * count());
    int i = 0;
    for (QLine& line : *this)
    {
        m_matrix[i] = line.p1().x();
        m_matrix[i + 1] = line.p1().y();
        m_matrix[i + 2] = line.p2().x();
        m_matrix[i + 3] = line.p2().y();
        i += 4;
    }
    flann::Matrix<int> samplePoints = flann::Matrix<int>((int*)(m_matrix.data()), count() * 2, 2);
    flann::KDTreeSingleIndexParams singleIndexParams = flann::KDTreeSingleIndexParams(10);
    m_kdtree = new flann::KDTreeSingleIndex<flann::L2_Simple<int>>(samplePoints, singleIndexParams);
    m_kdtree->buildIndex();
}

QLine LaserLineList::nearestSearch(const QPoint& point, bool remove)
{
    QVector<int> queries;
    queries.resize(2);
    queries[0] = point.x();
    queries[1] = point.y();

    QVector<int> indices;
    indices.resize(1);
    flann::Matrix<int> indicesMatrix(indices.data(), 1, 1);

    QVector<float> dists;
    dists.resize(1);
    flann::Matrix<float> distsMatrix(dists.data(), 1, 1);

    flann::SearchParams searchParams = flann::SearchParams(128);
    searchParams.use_heap = flann::FLANN_True;
    flann::Matrix<int> queryMatrix = flann::Matrix<int>(queries.data(), 1, 2);
    m_kdtree->knnSearch(queryMatrix, indicesMatrix, distsMatrix, 1, searchParams);

    int index = indices[0];
    QPoint dstPoint(m_matrix[index * 2], m_matrix[index * 2 + 1]);
    QLine line = at(index >> 1);
    if (dstPoint == line.p2())
    {
        line = QLine(line.p2(), line.p1());
    }
    if (remove)
    {
        m_kdtree->removePoint(indices[0] / 2 * 2);
        m_kdtree->removePoint(indices[0] / 2 * 2 + 1);
    }
    return line;
}

LaserLineListList::LaserLineListList()
    : QList<LaserLineList>()
{
}

LaserLineListList::~LaserLineListList()
{
}
