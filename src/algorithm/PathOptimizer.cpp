#include "PathOptimizer.h"
#include <opencv2/opencv.hpp>

#include "common/Config.h"
#include "algorithm/OptimizeNode.h"
#include "algorithm/OptimizeEdge.h"
#include "scene/LaserLayer.h"
#include "scene/LaserPrimitive.h"
#include "laser/LaserPoint.h"
#include "laser/LaserPointList.h"
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
//#include <omp.h>

class AntPrivate
{
    Q_DECLARE_PUBLIC(Ant)
public:
    AntPrivate(Ant* ant)
        : q_ptr(ant)
        , currentNode(nullptr)
    {}

    QMap<OptimizeNode*, int> arrivedNodes;
    QSet<OptimizeEdge*> pastEdges;
    QQueue<OptimizeNode*> path;
    OptimizeNode* currentNode;
    int antIndex;
    int currentPointIndex;
    double totalLength;
    PathOptimizer* optimizer;
    Ant* q_ptr;
};

Ant::Ant(int antIndex, PathOptimizer* optimizer)
    : m_ptr(new AntPrivate(this))
{
    Q_D(Ant);
    d->antIndex = antIndex;
    d->optimizer = optimizer;
}

void Ant::initialize()
{
    Q_D(Ant);
    d->arrivedNodes.clear();
    d->path.clear();
    d->pastEdges.clear();
    d->currentNode = nullptr;
    d->totalLength = 0.;
}

void Ant::arrived(OptimizeNode* node, const LaserPoint& lastPos)
{
    Q_D(Ant);
    float dist;
    node->nearestPoint(lastPos);
    
    d->arrivedNodes.insert(node, d->currentPointIndex);
    d->path.push_back(node);
    d->currentNode = node;
    d->totalLength += dist;
    //qLogD << "ant " << antIndex() << " arrived at " << currentNode()->nodeName() << " by walking through " << dist << " units";
}

OptimizeNode* Ant::currentNode() const
{
    Q_D(const Ant);
    return d->currentNode;
}

LaserPoint Ant::currentPos() const
{
    Q_D(const Ant);
    LaserPoint target;
    if (d->currentNode->isClosed())
    {
        target = d->currentNode->point(d->currentPointIndex);
    }
    else
    {
        if (d->currentPointIndex == 0)
        {
            target = d->currentNode->tailPoint();
        }
        else
        {
            target = d->currentNode->headPoint();
        }
    }
    return target;
}

bool Ant::moveForward()
{
    Q_D(Ant);
    double maxProb = 0.f;
    OptimizeEdge* maxEdge = nullptr;
    QRandomGenerator* random = QRandomGenerator::global();
    qreal minLength = d->currentNode->edges().first()->length();
    for (OptimizeEdge* edge : d->currentNode->edges())
    {
        if (d->arrivedNodes.contains(edge->b()))
            continue;

        if (edge->length() < minLength)
        {
            minLength = edge->length();
            maxEdge = edge;
        }
    }

    if (maxEdge == nullptr)
    {
        // 从这里开始访问父节点
        if (!d->currentNode->outEdge())
        {
            return false;
        }
        //qLogD << "    ant " << d->antIndex << " walked through all nodes within this group.";
        arrived(d->currentNode->outEdge()->b(), currentPos());
        return true;
    }
    else
    {
        //qLogD << "the best edge: " << maxEdge->a()->nodeName() << " --> " << maxEdge->b()->nodeName()
            //<< ", maxProb: " << maxProb << ", length: " << maxEdge->length();
    }
    arrived(maxEdge->b(), currentPos());
    d->pastEdges.insert(maxEdge);
    return true;
}

int Ant::antIndex() const
{
    Q_D(const Ant);
    return d->antIndex;
}

double Ant::totalLength() const
{
    Q_D(const Ant);
    return d->totalLength;
}

void drawPath(cv::Mat& canvas, const QQueue<OptimizeNode*>& path, const QMap<OptimizeNode*, int>& arrivedNodes)
{
    cv::Point2f lastPos(0, 0);
    QRandomGenerator random(QDateTime::currentDateTime().toMSecsSinceEpoch());
    for (int i = 0; i < path.length(); i++)
    {
        OptimizeNode* node = path[i];
        if (i != 0)
            continue;
        node->debugDraw(canvas);
        int pointIndex = arrivedNodes[node];
        cv::Point2f startPos = typeUtils::qtPointF2CVPoint2f(node->point(pointIndex).toPointF());
        cv::circle(canvas, startPos, 20, cv::Scalar(0, 0, 255), 5);
        if (i != 0)
        {
            cv::line(canvas, lastPos, startPos, cv::Scalar(0, 0, 255), 3);
        }
        LaserPointList points = node->startingPoints();
        cv::Mat pointsMat(points.count(), 2, CV_32S, points.toPoints().data());
        pointsMat.convertTo(pointsMat, CV_32S);
        cv::polylines(canvas, pointsMat, false, cv::Scalar(random.bounded(256), random.bounded(256), random.bounded(256)), 5);
        lastPos = startPos;
        if (!node->isClosed())
        {
            if (pointIndex == 0)
            {
                lastPos = typeUtils::qtPointF2CVPoint2f(node->tailPoint().toPointF());
            }
            else
            {
                lastPos = typeUtils::qtPointF2CVPoint2f(node->headPoint().toPointF());
            }
            cv::circle(canvas, lastPos, 18, cv::Scalar(0, 255, 0), 6);
            cv::line(canvas, lastPos, startPos, cv::Scalar(0, 0, 255), 3);
        }
        //qLogD << "  " << node->nodeName();
    }
}

void Ant::drawPath(cv::Mat& canvas, int iteration)
{
    Q_D(Ant);

    ::drawPath(canvas, d->path, d->arrivedNodes);

    QString filename = QString("tmp/path_%1_%2.png").arg(iteration).arg(d->antIndex);
    cv::imwrite(filename.toStdString(), canvas);
    canvas.release();
}

QQueue<OptimizeNode*> Ant::path() const
{
    Q_D(const Ant);
    return d->path;
}

QMap<OptimizeNode*, int> Ant::arrivedNodes() const
{
    Q_D(const Ant);
    return d->arrivedNodes;
}

class PathOptimizerPrivate
{
    Q_DECLARE_PUBLIC(PathOptimizer)
public:
    PathOptimizerPrivate(PathOptimizer* ptr)
        : q_ptr(ptr)
        , canvas(nullptr)
        , totalNodes(0)
        , arrivedNodes(0)
        , progress(0)
    {}

    PathOptimizer* q_ptr;
    OptimizeNode* root;
    OptimizeNode* currentNode;

    int totalNodes;
    int arrivedNodes;
    QList<OptimizeEdge*> edges;
    QList<OptimizeNode*> nodes;

    qreal progress;

    PathOptimizer::Path optimizedPath;
    cv::Mat* canvas;
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
    //qDeleteAll(d->nodes);
    qDeleteAll(d->edges);
}

void PathOptimizer::optimize(int canvasWidth, int canvasHeight)
{
    Q_D(PathOptimizer);

    emit titleUpdated(tr("Initializing optimizer..."));
    d->progress = 0;
    emit progressUpdated(d->progress);

    d->arrivedNodes = 0;
    d->currentNode = d->root;
    d->currentNode->update();
    d->optimizedPath.clear();

    for (OptimizeNode* layerNode : d->root->childNodes())
    {
        optimizeLayer(layerNode);
    }

    d->progress = 90;
    emit messageUpdated(tr("Optimizing ended."));
    for (OptimizeNode* node : d->optimizedPath)
    {
        LaserPointList points = node->arrangeMachiningPoints();
        emit drawPath(points.toPainterPath(), QPen(Qt::red, 2));
        d->progress += 1.0 * 9 / d->totalNodes;
        emit messageUpdated(tr("Arranged machining points of node %1.").arg(node->nodeName()));
    }

    emit progressUpdated(d->progress);
    d->progress = 100;
    emit progressUpdated(100);
    emit titleUpdated(tr("Done."));
    emit finished();
}

QList<OptimizeEdge*> PathOptimizer::edges() const
{
    Q_D(const PathOptimizer);
    return d->edges;
}

int PathOptimizer::edgesCount() const
{
    Q_D(const PathOptimizer);
    return d->edges.size();
}

PathOptimizer::Path PathOptimizer::optimizedPath() const
{
    Q_D(const PathOptimizer);
    return d->optimizedPath;
}

void PathOptimizer::setCanvas(cv::Mat& canvas)
{
    Q_D(PathOptimizer);
    d->canvas = &canvas;
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
        node->update();
        // 如果当前节点为一个图元
        if (node->nodeType() == LNT_PRIMITIVE)
        {
            // 先更新它的加工点集
            LaserPrimitive* primitive = static_cast<LaserPrimitive*>(node->documentItem());
            emit drawPath(primitive->toMachiningPath(), QPen(Qt::blue, 2), primitive->name());
            emit titleUpdated(tr("Generating machining points of node %1").arg(primitive->name()));
            d->progress += 1.0 * 0.9 * 90 / d->totalNodes;
            emit progressUpdated(d->progress);
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
            //emit drawLine(edge->toLine(), QPen(Qt::lightGray, 1, Qt::DashLine), label);
            d->edges.append(edge);
        }

        // find all child nodes from its siblings
        QList<OptimizeNode*> siblingChildren = node->parentNode()->findAllLeaves(node);
        for (OptimizeNode* leafNode : siblingChildren)
        {
            OptimizeEdge* edge = new OptimizeEdge(node, leafNode);
            node->addEdge(edge);
            QString label = QString("%1 -> %2").arg(node->nodeName()).arg(leafNode->nodeName());
            //emit drawLine(edge->toLine(), QPen(Qt::magenta, 1, Qt::DashLine), label);
            d->edges.append(edge);
        }

        for (OptimizeNode* childNode : node->childNodes())
        {
            stack.push(childNode);
        }
    }

    for (OptimizeNode* node : leaves)
    {
        //root->leavesPoints().addOptimizeNode(node);
        OptimizeEdge* edge = new OptimizeEdge(d->currentNode, node);
        d->currentNode->addEdge(edge);
        QString label = QString("%1 -> %2").arg(root->nodeName()).arg(node->nodeName());
        //emit drawLine(edge->toLine(), QPen(Qt::green, 1, Qt::DashLine), label);
        d->edges.append(edge);
    }
    //root->leavesPoints().buildKdtree();

    // 开始图遍历
    travelled.clear();
    while (true)
    {
        // 访问该节点，添加到访问列表中
        travelled.insert(d->currentNode);
        qLogD << "arrived node " << d->currentNode->nodeName();

        if (d->currentNode->nodeType() == LNT_PRIMITIVE)
        {
            emit titleUpdated(tr("Arrived node %1").arg(d->currentNode->nodeName()));
            d->progress += 1.0 * 0.1 * 90 / d->totalNodes;
            emit progressUpdated(d->progress);
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
            emit drawLine(QLineF(d->currentNode->currentPos().toPointF(), 
                point.toPointF()), QPen(Qt::black, 2));

            d->currentNode = parentNode;
        }
        else
        {
            // 构建kdtree
            siblingPoints.buildKdtree();
            OptimizeNode* candidate = siblingPoints.nearestSearch(d->currentNode);
            emit drawLine(QLineF(d->currentNode->currentPos().toPointF(), 
                candidate->currentPos().toPointF()), QPen(Qt::black, 2));
            d->currentNode = candidate;
        }
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

OptimizerController::OptimizerController(OptimizeNode* root, int totalNodes, QObject* parent)
    : QObject(parent)
    , m_previewWindow(new PreviewWindow)
    , m_optimizer(new PathOptimizer(
        root,
        totalNodes))
{
    qLogD << "OptimizerController";
    m_optimizer->moveToThread(&m_thread);
    //connect(&m_thread, &QThread::finished, m_optimizer.data(), &QObject::deleteLater);
    connect(this, &OptimizerController::start, m_optimizer.data(), &PathOptimizer::optimize);
    connect(m_optimizer.data(), &PathOptimizer::finished, this, &OptimizerController::finished);
    connect(m_optimizer.data(), &PathOptimizer::finished, m_previewWindow.data(), &PreviewWindow::finished);
    connect(m_optimizer.data(), &PathOptimizer::messageUpdated, m_previewWindow.data(), &PreviewWindow::addMessage);
    connect(m_optimizer.data(), &PathOptimizer::progressUpdated, m_previewWindow.data(), &PreviewWindow::setProgress);
    connect(m_optimizer.data(), &PathOptimizer::titleUpdated, m_previewWindow.data(), &PreviewWindow::setTitle);
    connect(m_optimizer.data(), &PathOptimizer::drawPrimitive, m_previewWindow.data(), &PreviewWindow::addPrimitive);
    connect(m_optimizer.data(), &PathOptimizer::drawPath, m_previewWindow.data(), &PreviewWindow::addPath);
    connect(m_optimizer.data(), &PathOptimizer::drawLine, m_previewWindow.data(), &PreviewWindow::addLine);

    m_thread.start();
}

OptimizerController::~OptimizerController()
{
}

PathOptimizer::Path OptimizerController::optimize(float pageWidth, float pageHeight, cv::Mat& canvas)
{
    m_optimizer->setCanvas(canvas);
    emit start(pageWidth, pageHeight);
    //m_dialog->exec();
    m_previewWindow->setWindowModality(Qt::ApplicationModal);
    m_previewWindow->exec();
    qLogD << "optimized";
    m_thread.wait();
    return m_optimizer->optimizedPath();
}

void OptimizerController::finished()
{
    m_previewWindow->setProgress(100);
    m_thread.quit();
}
