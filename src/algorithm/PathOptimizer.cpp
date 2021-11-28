#include "PathOptimizer.h"
#include <opencv2/opencv.hpp>

#include "LaserApplication.h"
#include "common/Config.h"
#include "algorithm/OptimizeNode.h"
#include "algorithm/OptimizeEdge.h"
#include "scene/LaserLayer.h"
#include "scene/LaserPrimitive.h"
#include "laser/LaserPoint.h"
#include "laser/LaserPointList.h"
#include "laser/LaserDevice.h"
#include "task/ProgressItem.h"
#include "task/ProgressModel.h"
#include "util/TypeUtils.h"
#include "util/Utils.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QQueue>
#include <QRandomGenerator>
#include <QStack>
#include <QtMath>
#include <QVector>

class PathOptimizerPrivate
{
    Q_DECLARE_PUBLIC(PathOptimizer)
public:
    PathOptimizerPrivate(PathOptimizer* ptr)
        : q_ptr(ptr)
        , totalNodes(0)
        , arrivedNodes(0)
    {}

    PathOptimizer* q_ptr;
    OptimizeNode* root;
    OptimizeNode* currentNode;

    int totalNodes;
    int arrivedNodes;
    QList<OptimizeNode*> nodes;

    PathOptimizer::Path optimizedPath;
};

PathOptimizer::PathOptimizer(OptimizeNode* root, int totalNodes, QObject* parent)
    : QObject(parent)
    , m_ptr(new PathOptimizerPrivate(this))
{
    Q_D(PathOptimizer);
    qLogD << "PathOptimizer";
    d->root = root;
    d->totalNodes = totalNodes;
}

PathOptimizer::~PathOptimizer()
{
    Q_D(PathOptimizer);
}

void PathOptimizer::arriveNode(OptimizeNode* node, QSet<OptimizeNode*>& travelled)
{
    Q_D(PathOptimizer);
    if (node->nodeType() == LNT_PRIMITIVE)
    {
        d->optimizedPath.append(node);
    }
    // 将该点从父节点的子节点中移除
    if (node->parentNode())
        node->parentNode()->removeChildNode(node);
    d->currentNode = node;
    travelled.insert(node);
#ifdef _DEBUG
    //qLogD << "arrived " << node->nodeName();
#endif
}

void PathOptimizer::optimize(ProgressItem* parentProgress)
{
    Q_D(PathOptimizer);

    LaserApplication::previewWindow->setTitle(tr("Initializing optimizer..."));

    d->arrivedNodes = 0;
    d->currentNode = d->root;
    d->currentNode->clearEdges();
    d->currentNode->update(parentProgress);
    d->optimizedPath.clear();

    ProgressItem* optimizeProgress = LaserApplication::progressModel->createComplexItem(tr("Optimize by layers"), parentProgress);
    ProgressItem* arrangeProgress = LaserApplication::progressModel->createSimpleItem(tr("Arrange points"), parentProgress);
    optimizeProgress->setMaximum(d->root->childNodes().count() * 2);
    for (OptimizeNode* layerNode : d->root->childNodes())
    {
        if (layerNode->exportable())
            optimizeFrom(layerNode, optimizeProgress);
    }
    optimizeProgress->finish();

    LaserApplication::previewWindow->addMessage(tr("Optimizing ended."));
    arrangeProgress->setMaximum(d->optimizedPath.length());
    OptimizeNode* last = d->root;
    for (OptimizeNode* node : d->optimizedPath)
    {
        LaserPointListList pointsList = node->arrangeMachiningPoints();
        LaserApplication::previewWindow->addPath(pointsList.toPainterPath(), QPen(Qt::red, 2));
        //LaserApplication::previewWindow->addProgress(this, 1.0 * 0.1 / d->totalNodes, tr("Arranged machining points of node %1.").arg(node->nodeName()));
        if (last)
        {
            QPoint from = last->arrangedEndingPoint().toPoint();
            QPoint to = node->arrangedStartingPoint().toPoint();
            /*qLogD << last->nodeName() << " --> " << node->nodeName() << ": "
                << from << ", " << to;*/
            LaserApplication::previewWindow->addLine(
                QLineF(from, to),
                QPen(Qt::blue, 2));
        }
        last = node;
        arrangeProgress->increaseProgress();
    }
    arrangeProgress->finish();

    emit finished();
}

PathOptimizer::Path PathOptimizer::optimizedPath() const
{
    Q_D(const PathOptimizer);
    return d->optimizedPath;
}

void PathOptimizer::optimizeFrom(OptimizeNode* root, ProgressItem* parentProgress)
{
    Q_D(PathOptimizer);
    ProgressItem* buildProgress = LaserApplication::progressModel->createSimpleItem(tr("Update nodes"), parentProgress);
    ProgressItem* optimizeProgress = LaserApplication::progressModel->createSimpleItem(tr("Optimize nodes"), parentProgress);

    // 计算泳道宽
    int laneInterval = Config::PathOptimization::groupingGridInterval();
    Qt::Orientation orient = static_cast<Qt::Orientation>(Config::PathOptimization::groupingOrientation());

    // 各泳道节点集合
    LaneMap laneMap;

    QStack<OptimizeNode*> stack;
    stack.push(root);
    QSet<OptimizeNode*> leaves;
    QList<OptimizeNode*> allNodes;
    while (!stack.empty())
    {
        OptimizeNode* node = stack.pop();
        allNodes.append(node);

        for (OptimizeNode* childNode : node->childNodes())
        {
            if (childNode->exportable())
                stack.push(childNode);
        }
    }
    buildProgress->setMaximum(allNodes.length());
    optimizeProgress->setMaximum(allNodes.length());
    for (OptimizeNode* node : allNodes)
    {
        node->clearEdges();
        node->update(parentProgress);

        if (node->isLeaf())
        {
            laneMap.addNode(node);
            leaves.insert(node);
        }
        buildProgress->increaseProgress();
    }
    laneMap.buildKdtree();
    buildProgress->finish();

    QSet<OptimizeNode*> travelled;
    // 找到第一个最优节点
    OptimizeNode* firstNode = laneMap.nearestSearch(d->currentNode, true, leaves);
    arriveNode(firstNode, travelled);
    while (d->currentNode != root)
    {
        // 搜索所有的兄弟节点
        QSet<OptimizeNode*> siblingLeaves;
        QSet<OptimizeNode*> siblingBranches;
        d->currentNode->findSiblings(siblingLeaves, siblingBranches, travelled);

        // 对该组兄弟叶节点建立最优路径
        if (!siblingLeaves.empty())
            optimizeNodes(siblingLeaves, travelled);

        // 搜索所有的兄弟分支节点下的叶节点
        QSet<OptimizeNode*> branchLeaves;
        for (OptimizeNode* branch : siblingBranches)
        {
            branchLeaves.unite(branch->findAllLeaves(travelled));
        }

        if (branchLeaves.isEmpty())
        {
            // 若叶节点为空，则设置父节点为当前节点
            OptimizeNode* parentNode = d->currentNode->parentNode();
            if (!parentNode || parentNode == root)
                break;

            if (parentNode->isVirtual())
            {
                parentNode->setCurrentPos(d->currentNode->currentPos());
            }
            else
            {
                LaserPoint point = parentNode->nearestPoint(d->currentNode);
                //LaserApplication::previewWindow->addLine(QLineF(d->currentNode->currentPos().toPointF(),
                    //point.toPointF()), QPen(Qt::black, 2));
            }
            arriveNode(parentNode, travelled);
        }
        else
        {
            // 找到最佳叶结点，设置为当前节点
            laneMap = LaneMap();
            laneMap.addNodes(branchLeaves);
            laneMap.buildKdtree();
            OptimizeNode* firstNode = laneMap.nearestSearch(d->currentNode, true, leaves);
            arriveNode(firstNode, travelled);
        }
        optimizeProgress->increaseProgress();
    }
    optimizeProgress->finish();
}

void PathOptimizer::optimizeNodes(QSet<OptimizeNode*>& siblingLeaves, QSet<OptimizeNode*>& travelled)
{
    Q_D(PathOptimizer);
    LaneMap laneMap;
    laneMap.addNodes(siblingLeaves);
    laneMap.buildKdtree();

    while (!siblingLeaves.empty())
    {
        // 在兄弟节点中找最优点
        OptimizeNode* leaveNode = laneMap.nearestSearch(d->currentNode, true);
        if (!leaveNode)
            break;

        // 将该点从兄弟叶节点集合中移除
        siblingLeaves.remove(leaveNode);

        arriveNode(leaveNode, travelled);
    }
}

void PathOptimizer::printNodeAndEdges()
{
    Q_D(PathOptimizer);
    for (OptimizeNode* node : d->nodes)
    {
        qLogD << "node " << node->nodeName() << "'s edges:";
        for (OptimizeEdge* edge : node->edges())
        {
            qLogD << "    " << edge->a()->nodeName() << " --> " << edge->b()->nodeName() << ", length = " << edge->length();
        }
        if (node->outEdge())
        {
            OptimizeEdge* edge = node->outEdge();
            qLogD << "    " << edge->a()->nodeName() << " --> " << edge->b()->nodeName() << " out, length = " << edge->length();
        }
    }
}

Lane::Lane()
{
}

Lane::~Lane()
{
}

void Lane::removeNode(OptimizeNode* node)
{
    m_pointList.removeNode(node);
    this->remove(node);
}

void Lane::buildKdtree()
{
    m_pointList.clear();
    m_pointList.addNodes(*this);
    m_pointList.buildKdtree();
}

OptimizeNode* Lane::nearestSearch(OptimizeNode* node, bool remove)
{
    OptimizeNode* dstNode = m_pointList.nearestSearch(node, remove);
    if (remove)
        this->removeNode(dstNode);
    return dstNode;
}

LaneMap::LaneMap()
    : QMap<int, Lane>()
{
}

LaneMap::~LaneMap()
{
}

void LaneMap::addNode(OptimizeNode* node)
{
    for (int laneIndex : node->laneIndices())
    {
        (*this)[laneIndex].insert(node);
    }
}

void LaneMap::addNodes(const QSet<OptimizeNode*>& nodes)
{
    for (OptimizeNode* node : nodes)
    {
        addNode(node);
    }
}

void LaneMap::removeNode(OptimizeNode* node)
{
    for (int laneIndex : node->laneIndices())
    {
        (*this)[laneIndex].removeNode(node);
        if ((*this)[laneIndex].empty())
            remove(laneIndex);
    }
}

void LaneMap::buildKdtree()
{
    for (LaneMap::Iterator i = begin(); i != end(); i++)
    {
        i.value().buildKdtree();
    }
}

Lane& LaneMap::nearestLane(OptimizeNode* node)
{
    return nearestLane(node->currentPos());
}

Lane& LaneMap::nearestLane(const LaserPoint& point)
{
    int index = point.laneIndex();
    int lastKey = firstKey();
    for (LaneMap::Iterator i = this->begin(); i != this->end(); i++)
    {
        if (index <= i.key())
        {
            int diff1 = qAbs(index - lastKey);
            int diff2 = qAbs(index - i.key());
            return diff1 <= diff2 ? (*this)[lastKey] : i.value();
        }
    }
    return last();
}

OptimizeNode* LaneMap::nearestSearch(OptimizeNode* node, bool remove, QSet<OptimizeNode*>& externNodes)
{
    if (empty())
        return nullptr;

    Lane& lane = nearestLane(node->currentPos());
    if (!externNodes.empty())
        lane.intersect(externNodes);
    //lane.buildKdtree();
    OptimizeNode* dstNode = lane.nearestSearch(node, remove);
    if (remove)
    {
        this->removeNode(dstNode);
    }
    return dstNode;
}

