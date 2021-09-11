#include "LaserPointList.h"

#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "algorithm/OptimizeNode.h"
#include "common/Config.h"

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
    }

    // re-calculate weights, making w1/w2 equale to layout(width or height)/gridSize
    qreal w1 = Config::PathOptimization::searchingXYWeight() / 2;
    qreal w2 = 1 - w1 * 2;
    qreal n = Config::PathOptimization::groupingOrientation() == Qt::Horizontal ?
        LaserApplication::device->layoutWidth() / Config::PathOptimization::maxGroupingGridSize() :
        LaserApplication::device->layoutHeight() / Config::PathOptimization::maxGroupingGridSize();
    m_weights[0] = Config::PathOptimization::groupingOrientation() == Qt::Horizontal ?
        2 * n * w1 / (n + 1) : 2 * w1 / (n + 1);
    m_weights[1] = Config::PathOptimization::groupingOrientation() == Qt::Horizontal ?
        2 * w1 / (n + 1) : 2 * n * w1 / (n + 1);
    m_weights[2] = w2;

    /*qreal sum = 0;
    for (qreal w : m_weights)
    {
        sum += w;
    }
    QVector<qreal> weights;
    for (qreal w : m_weights)
    {
        weights.append(w / sum);
    }*/

    m_matrix.clear();

    m_matrix.resize(3 * 2 * count());
    int index = 0;
    for (LaserPoint& point : *this)
    {
        // convert element value range to 0..1
        m_matrix[index] = point.x() * m_weights[0] / (LaserApplication::device->layoutWidth() * 40);
        m_matrix[index + 1] = point.y() * m_weights[1] / (LaserApplication::device->layoutHeight() * 40);
        m_matrix[index + 2] = point.angle1() * m_weights[2] / 360;
        m_matrix[index + 3] = point.x() * m_weights[0] / (LaserApplication::device->layoutWidth() * 40);
        m_matrix[index + 4] = point.y() * m_weights[1] / (LaserApplication::device->layoutHeight() * 40);
        m_matrix[index + 5] = point.angle2() * m_weights[2] / 360;
        index += 6;
    }

    flann::Matrix<qreal> samplePoints = flann::Matrix<qreal>((qreal*)(m_matrix.data()),
            count() * 2, 3);
    flann::KDTreeSingleIndexParams singleIndexParams = flann::KDTreeSingleIndexParams(4);
    m_kdtree = new flann::KDTreeSingleIndex<flann::L2_Simple<qreal>>(samplePoints, singleIndexParams);
    m_kdtree->buildIndex();
}

void LaserPointList::addOptimizeNode(OptimizeNode* node)
{
    int index = 0;
    for (LaserPoint& point : node->startingPoints())
    {
        append(point);
        m_nodeMap.insert(size() - 1, node);
        m_indexMap.insert(size() - 1, index);
        index++;
    }
}

int LaserPointList::nearestSearch(const LaserPoint& point)
{
    QVector<qreal> queries;
    queries.resize(3 * 4);

    qreal angle1 = point.angle1();
    qreal angle2 = point.angle2();
    qreal angle3 = qMax(angle1, angle2) - 360;
    qreal angle4 = qMin(angle1, angle2) + 360 ;

    qreal x = point.x() * m_weights[0] / (LaserApplication::device->layoutWidth() * 40);
    qreal y = point.y() * m_weights[1] / (LaserApplication::device->layoutHeight() * 40);
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

    QVector<qreal> dists;
    dists.resize(4);
    flann::Matrix<qreal> distsMatrix(dists.data(), 4, 1);

    flann::SearchParams searchParams = flann::SearchParams(128);
    //searchParams.checks = -1;
    searchParams.sorted = true;
    searchParams.use_heap = flann::FLANN_True;
    flann::Matrix<qreal> queryMatrix = flann::Matrix<qreal>(queries.data(), 4, 3);
    m_kdtree->knnSearch(queryMatrix, indicesMatrix, distsMatrix, 1, searchParams);

    int index = 0;
    qreal minDist = dists[0];
    for (int i = 1; i < 4; i++)
    {
        if (dists[i] < minDist)
        {
            index = i;
            minDist = dists[i];
        }
    }

    return indices[index] / 2;
}

OptimizeNode* LaserPointList::nearestSearch(OptimizeNode* srcNode)
{
    int globalIndex = nearestSearch(srcNode->currentPos());
    OptimizeNode* node = m_nodeMap[globalIndex];
    int index = m_indexMap[globalIndex];
    node->setCurrentIndex(index);
    node->setLastPoint(srcNode->currentPos());
    return node;
}

QVector<QPointF> LaserPointList::toPoints() const
{
    QVector<QPointF> points;
    points.resize(count());
    int i = 0;
    for (const LaserPoint& point : *this)
    {
        points[i++] = point.toPointF();
    }
    return points;
}

QPainterPath LaserPointList::toPainterPath() const
{
    QPainterPath path;
    int i = 0;
    for (const LaserPoint& point : *this)
    {
        if (i == 0)
            path.moveTo(point.toPointF());
        else
            path.lineTo(point.toPointF());
        i++;
    }
    return path;
}

