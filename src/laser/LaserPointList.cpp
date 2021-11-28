#include "LaserPointList.h"

#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "algorithm/OptimizeNode.h"
#include "common/Config.h"
#include <QtMath>

LaserPointList::LaserPointList()
    : m_kdtree(nullptr)
{
    m_weights.resize(3);
    for (int i = 0; i < 3; i++)
    {
        m_weights[i] = 1;
    }
}

LaserPointList::~LaserPointList()
{
    if (m_kdtree)
    {
        delete m_kdtree;
        m_kdtree = nullptr;
    }
}

void LaserPointList::buildKdtree()
{
    if (isEmpty())
        return;

    if (m_kdtree)
    {
        delete m_kdtree;
        m_kdtree = nullptr;
    }

    // re-calculate weights, making w1/w2 equale to layout(m_width or m_height)/gridSize
    int w1 = Config::PathOptimization::searchingXYWeight() / 2;
    int w2 = 1 - w1 * 2;
    int n = Config::PathOptimization::groupingOrientation() == Qt::Horizontal ?
        LaserApplication::device->layoutWidth() / Config::PathOptimization::groupingGridInterval() :
        LaserApplication::device->layoutHeight() / Config::PathOptimization::groupingGridInterval();
    m_weights[0] = Config::PathOptimization::groupingOrientation() == Qt::Horizontal ?
        2 * n * w1 / (n + 1) : 2 * w1 / (n + 1);
    m_weights[1] = Config::PathOptimization::groupingOrientation() == Qt::Horizontal ?
        2 * w1 / (n + 1) : 2 * n * w1 / (n + 1);
    m_weights[2] = w2;

    m_matrix.clear();

    m_matrix.resize(3 * 2 * count());
    int index = 0;
    for (LaserPoint& point : *this)
    {
        // convert element value range to 0..1
        m_matrix[index] = point.x() * m_weights[0] / (LaserApplication::device->layoutWidth());
        m_matrix[index + 1] = point.y() * m_weights[1] / (LaserApplication::device->layoutHeight());
        m_matrix[index + 2] = point.angle1() * m_weights[2] / 360;
        m_matrix[index + 3] = point.x() * m_weights[0] / (LaserApplication::device->layoutWidth());
        m_matrix[index + 4] = point.y() * m_weights[1] / (LaserApplication::device->layoutHeight());
        m_matrix[index + 5] = point.angle2() * m_weights[2] / 360;
        index += 6;
    }

    flann::Matrix<int> samplePoints = flann::Matrix<int>((int*)(m_matrix.data()),
            count() * 2, 3);
    flann::KDTreeSingleIndexParams singleIndexParams = flann::KDTreeSingleIndexParams(10);
    m_kdtree = new flann::KDTreeSingleIndex<flann::L2_Simple<int>>(samplePoints, singleIndexParams);
    m_kdtree->buildIndex();
}

void LaserPointList::addNode(OptimizeNode* node)
{
    int index = 0;
    for (LaserPoint& point : node->startingPoints())
    {
        append(point);
        m_indexNodeMap.insert(size() - 1, node);
        m_nodeIndicesMap.insert(node, size() - 1);
        m_indexMap.insert(size() - 1, index);
        index++;
    }
}

void LaserPointList::removeNode(OptimizeNode* node)
{
    for (int index : m_nodeIndicesMap.values(node))
    {
        m_indexNodeMap.remove(index);
        m_indexMap.remove(index);

        m_kdtree->removePoint(index * 2);
        m_kdtree->removePoint(index * 2 + 1);
    }
    m_nodeIndicesMap.remove(node);
}

void LaserPointList::addNodes(const QSet<OptimizeNode*>& nodes)
{
    for (OptimizeNode* node : nodes)
    {
        addNode(node);
    }
}

int LaserPointList::nearestSearch(const LaserPoint& point)
{
    QVector<int> queries;
    queries.resize(3 * 4);

    // 生成4个角度的数据
    int angle1 = point.angle1();
    int angle2 = point.angle2();
    int angle3 = qMax(angle1, angle2) - 360;
    int angle4 = qMin(angle1, angle2) + 360 ;

    int x = point.x() * m_weights[0] / (LaserApplication::device->layoutWidth());
    int y = point.y() * m_weights[1] / (LaserApplication::device->layoutHeight());
    angle1 = angle1 * m_weights[2] / 360;
    angle2 = angle2 * m_weights[2] / 360;
    angle3 = angle3 * m_weights[2] / 360;
    angle4 = angle4 * m_weights[2] / 360;

    queries[0] = x;
    queries[1] = y;
    queries[2] = angle1;

    queries[3] = x;
    queries[4] = y;
    queries[5] = angle2;

    queries[6] = x;
    queries[7] = y;
    queries[8] = angle3;

    queries[9] = x;
    queries[10] = y;
    queries[11] = angle4;

    QVector<int> indices;
    indices.resize(4);
    flann::Matrix<int> indicesMatrix(indices.data(), 4, 1);

    QVector<float> dists;
    dists.resize(4);
    flann::Matrix<float> distsMatrix(dists.data(), 4, 1);

    flann::SearchParams searchParams = flann::SearchParams(128);
    //searchParams.checks = -1;
    searchParams.sorted = true;
    searchParams.use_heap = flann::FLANN_True;
    flann::Matrix<int> queryMatrix = flann::Matrix<int>(queries.data(), 4, 3);
    m_kdtree->knnSearch(queryMatrix, indicesMatrix, distsMatrix, 1, searchParams);

    int index = 0;
    int minDist = dists[0];
    for (int i = 1; i < 4; i++)
    {
        if (dists[i] < minDist)
        {
            index = i;
            minDist = dists[i];
        }
    }

#ifdef _DEBUG
    //qLogD << "nearest point: " << m_matrix[index * 3] << ", " << m_matrix[index * 3 + 1] << ", " << m_matrix[index * 3 + 2];
#endif

    return indices[index] / 2;
}

OptimizeNode* LaserPointList::nearestSearch(OptimizeNode* srcNode, bool remove)
{
    int globalIndex = nearestSearch(srcNode->currentPos());
    OptimizeNode* node = m_indexNodeMap[globalIndex];
    int index = m_indexMap[globalIndex];

    if (remove)
        removeNode(node);

    node->setCurrentIndex(index);
    //qLogD << node->currentPos();
    node->setLastPoint(srcNode->currentPos());
    return node;
}

QList<QPoint> LaserPointList::toPoints() const
{
    QList<QPoint> points;
    points.reserve(count());
    for (const LaserPoint& point : *this)
    {
        points.append(point.toPoint());
    }
    return points;
}

QPainterPath LaserPointList::toPainterPath() const
{
    QPainterPath path;
    for (int i = 0; i < length(); i++)
    {
        LaserPoint point = at(i);
        if (i == 0)
            path.moveTo(point.toPoint());
        else
            path.lineTo(point.toPoint());
    }

    return path;
}

LaserPointListList::LaserPointListList()
{
}

LaserPointListList::~LaserPointListList()
{
}

QPainterPath LaserPointListList::toPainterPath() const
{
    QPainterPath path;
    for (const LaserPointList& list : *this)
    {
        path.addPath(list.toPainterPath());
    }
    return path;
}
