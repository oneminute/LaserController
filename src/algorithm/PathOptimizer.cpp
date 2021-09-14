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
        , progress(0)
    {}

    PathOptimizer* q_ptr;
    OptimizeNode* root;
    OptimizeNode* currentNode;

    int totalNodes;
    int arrivedNodes;
    QList<OptimizeNode*> nodes;

    qreal progress;

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

void PathOptimizer::optimize()
{
    Q_D(PathOptimizer);

    //emit titleUpdated(tr("Initializing optimizer..."));
    LaserApplication::previewWindow->setTitle(tr("Initializing optimizer..."));
    //d->progress = 0;
    //emit progressUpdated(d->progress);

    d->arrivedNodes = 0;
    d->currentNode = d->root;
    d->currentNode->clearEdges();
    d->currentNode->update();
    d->optimizedPath.clear();

    for (OptimizeNode* layerNode : d->root->childNodes())
    {
        optimizeLayer(layerNode);
    }

    d->progress = 90;
    LaserApplication::previewWindow->addMessage(tr("Optimizing ended."));
    for (OptimizeNode* node : d->optimizedPath)
    {
        LaserPointList points = node->arrangeMachiningPoints();
        LaserApplication::previewWindow->addPath(points.toPainterPath(), QPen(Qt::red, 2));
        d->progress += 1.0 * 9 / d->totalNodes;
        LaserApplication::previewWindow->addProgress(this, 1.0 * 0.1 / d->totalNodes, tr("Arranged machining points of node %1.").arg(node->nodeName()));
    }

    emit finished();
}

PathOptimizer::Path PathOptimizer::optimizedPath() const
{
    Q_D(const PathOptimizer);
    return d->optimizedPath;
}

void PathOptimizer::optimizeLayer(OptimizeNode* root)
{
    Q_D(PathOptimizer);
    QStack<OptimizeNode*> stack;
    QList<OptimizeNode*> leaves;

    // 获取所有的叶节点
    for (OptimizeNode* node : root->childNodes())
    {
        stack.push(node);
    }

    while (!stack.isEmpty())
    {
        OptimizeNode* node = stack.pop();
        node->clearEdges();
        LaserApplication::previewWindow->addMessage(tr("Generating machining points of node %1").arg(node->nodeName()));
        node->update((quint32)this, 1.0 * 0.9 / d->totalNodes);
        // 如果当前节点为一个图元
        if (node->nodeType() == LNT_PRIMITIVE)
        {
            // 先更新它的加工点集
            LaserPrimitive* primitive = static_cast<LaserPrimitive*>(node->documentItem());
            LaserApplication::previewWindow->addPath(primitive->machiningPoints().toPainterPath(), QPen(Qt::blue, 2), primitive->name());
            LaserApplication::previewWindow->addMessage(tr("Generating machining points of node %1. Done.").arg(node->nodeName()));
        }
        d->nodes.append(node);

        if (node->hasChildren())
        {
            for (OptimizeNode* childNode : node->childNodes())
            {
                stack.push(childNode);
            }
        }
        else
        {
            leaves.append(node);
        }
    }

    QSet<OptimizeNode*> travelled;
    stack.clear();
    stack.push(root);
    while (!stack.isEmpty())
    {
        OptimizeNode* node = stack.pop();
        // 判断该点是否已经被处理过
        if (travelled.contains(node))
            continue;
        if (!node->isVirtual())
            travelled.insert(node);

        if (node->parentNode() && node->parentNode() != root && node->nodeType() != LNT_LAYER)
        {
            // 该节点有父节点，建立从子节点到父节点的边。
            OptimizeEdge* edge = new OptimizeEdge(node, node->parentNode());
            node->setOutEdge(edge);
            QString label = QString("%1 -> %2").arg(node->nodeName()).arg(node->parentNode()->nodeName());
        }

        // find all child nodes from its siblings
        QList<OptimizeNode*> siblingChildren = node->parentNode()->findAllLeaves(node);
        for (OptimizeNode* leafNode : siblingChildren)
        {
            OptimizeEdge* edge = new OptimizeEdge(node, leafNode);
            node->addEdge(edge);
            QString label = QString("%1 -> %2").arg(node->nodeName()).arg(leafNode->nodeName());
        }

        for (OptimizeNode* childNode : node->childNodes())
        {
            stack.push(childNode);
        }
    }

    for (OptimizeNode* node : leaves)
    {
        OptimizeEdge* edge = new OptimizeEdge(d->currentNode, node);
        d->currentNode->addEdge(edge);
        QString label = QString("%1 -> %2").arg(root->nodeName()).arg(node->nodeName());
    }

    // 开始图遍历
    travelled.clear();
    while (true)
    {
        // 访问该节点，添加到访问列表中
        travelled.insert(d->currentNode);
        qLogD << "arrived node " << d->currentNode->nodeName();

        if (d->currentNode->nodeType() == LNT_PRIMITIVE)
        {
            LaserApplication::previewWindow->addProgress(this, 0.9 * 0.1 / d->totalNodes, tr("Arrived node %1").arg(d->currentNode->nodeName()));
            d->optimizedPath.append(d->currentNode);
        }

        // 生成输出边的kdtree
        LaserPointList siblingPoints;
        for (OptimizeEdge* edge : d->currentNode->edges())
        {
            OptimizeNode* toNode = edge->b();
            // 如果还没遍历，就加入到kdtree中
            if (!travelled.contains(toNode))
            {
                siblingPoints.addOptimizeNode(toNode);
            }
        }
        if (siblingPoints.isEmpty())
        { 
            // 为空表示兄弟节点已经全部遍历完成，向父节点移动
            OptimizeNode* parentNode = d->currentNode->parentNode();
            if (parentNode->isVirtual())
                break;

            if (travelled.contains(parentNode))
            {
                d->currentNode = parentNode;
                continue;
            }

            LaserPoint point = parentNode->nearestPoint(d->currentNode);
            LaserApplication::previewWindow->addLine(QLineF(d->currentNode->currentPos().toPointF(), 
                point.toPointF()), QPen(Qt::black, 2));

            d->currentNode = parentNode;
        }
        else
        {
            // 构建kdtree
            siblingPoints.buildKdtree();
            OptimizeNode* candidate = siblingPoints.nearestSearch(d->currentNode);
            LaserApplication::previewWindow->addLine(QLineF(d->currentNode->currentPos().toPointF(), 
                candidate->currentPos().toPointF()), QPen(Qt::black, 2));
            d->currentNode = candidate;
        }
    }
    
    d->currentNode->clearChildren();
    d->currentNode->clearEdges();
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

OptimizerController::OptimizerController(OptimizeNode* root, int totalNodes, QObject* parent)
    : QObject(parent)
    , m_optimizer(new PathOptimizer(
        root,
        totalNodes))
{
    qLogD << "OptimizerController";
    LaserApplication::previewWindow->registerProgressCode(m_optimizer.data(), 0.9);
    m_optimizer->moveToThread(&m_thread);
    connect(this, &OptimizerController::start, m_optimizer.data(), &PathOptimizer::optimize);
    connect(m_optimizer.data(), &PathOptimizer::finished, this, &OptimizerController::finished);

    m_thread.start();
}

OptimizerController::~OptimizerController()
{
}

void OptimizerController::optimize()
{
    emit start();
}

PathOptimizer::Path OptimizerController::path()
{
    return m_optimizer->optimizedPath();
}

void OptimizerController::setFinishedCallback(FinishedCallback callback)
{
    m_finishedCallback = callback;
}

void OptimizerController::finished()
{
    LaserApplication::previewWindow->setProgress(this, 1);
    if (m_finishedCallback)
    {
        m_finishedCallback(this);
    }
    m_thread.quit();
    this->deleteLater();
}
