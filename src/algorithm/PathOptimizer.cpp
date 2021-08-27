#include "PathOptimizer.h"
#include <opencv2/opencv.hpp>

#include "common/Config.h"
#include "algorithm/OptimizeNode.h"
#include "algorithm/OptimizeEdge.h"
#include "scene/LaserLayer.h"
#include "scene/LaserPrimitive.h"
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

void Ant::arrived(OptimizeNode* node, const QPointF& lastPos)
{
    Q_D(Ant);
    float dist;
    node->nearestPoint(lastPos, d->currentPointIndex, dist);
    if (node->isVirtual())
    {
        // 对于虚拟父结点，搜索其它叶结点前，要先更新一下各条边。
        for (OptimizeEdge* edge : node->edges())
        {
            float edgeLength;
            int bPointIndex;
            edge->b()->nearestPoint(lastPos, bPointIndex, edgeLength);
            edge->setLength(edgeLength);
        }
    }
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

QPointF Ant::currentPos() const
{
    Q_D(const Ant);
    QPointF target;
    if (d->currentNode->isVirtual())
    {
        target = d->currentNode->currentPos();
    }
    else if (d->currentNode->isClosed())
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
        if (i != 0 && node->isVirtual())
            continue;
        node->debugDraw(canvas);
        int pointIndex = arrivedNodes[node];
        cv::Point2f startPos = typeUtils::qtPointF2CVPoint2f(node->point(pointIndex));
        cv::circle(canvas, startPos, 20, cv::Scalar(0, 0, 255), 5);
        if (i != 0)
        {
            cv::line(canvas, lastPos, startPos, cv::Scalar(0, 0, 255), 3);
        }
        QVector<QPointF> points = node->startingPoints();
        cv::Mat pointsMat(points.count(), 2, CV_32S, points.data());
        pointsMat.convertTo(pointsMat, CV_32S);
        cv::polylines(canvas, pointsMat, false, cv::Scalar(random.bounded(256), random.bounded(256), random.bounded(256)), 5);
        lastPos = startPos;
        if (!node->isClosed())
        {
            if (pointIndex == 0)
            {
                lastPos = typeUtils::qtPointF2CVPoint2f(node->tailPoint());
            }
            else
            {
                lastPos = typeUtils::qtPointF2CVPoint2f(node->headPoint());
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
    {}

    OptimizeNode* root;
    int totalNodes;
    QList<QList<QList<OptimizeNode*>>> layerNodes;
    QList<OptimizeEdge*> edges;
    QMap<LaserPrimitive*, OptimizeNode*> pritmive2NodeMap;
    bool containsLayers;
    QList<OptimizeNode*> nodes;
    QList<OptimizeNode*> leaves;
    OptimizeNode* rootNode;
    int maxAnts;
    int maxIterations;
    int maxTraverse;
    float volatileRate;
    bool useGreedyAlgorithm;
    PathOptimizer::Path optimizedPath;
    PathOptimizer* q_ptr;
    cv::Mat* canvas;
};

PathOptimizer::PathOptimizer(OptimizeNode* root, int totalNodes, int maxIterations,
    int maxAnts, int maxTravers, float volatileRate,
    bool useGreedyAlgorithm, bool containsLayers, QObject* parent)
    : QObject(parent)
    , m_ptr(new PathOptimizerPrivate(this))
{
    Q_D(PathOptimizer);
    qLogD << "PathOptimizer";
    d->containsLayers = containsLayers;
    d->maxAnts = maxAnts;
    d->maxIterations = maxIterations;
    d->maxTraverse = maxTravers;
    d->volatileRate = volatileRate;
    d->useGreedyAlgorithm = useGreedyAlgorithm;
    d->root = root;
    d->totalNodes = totalNodes;
}

PathOptimizer::~PathOptimizer()
{
    Q_D(PathOptimizer);
    qDeleteAll(d->nodes);
    qDeleteAll(d->edges);
}

void PathOptimizer::optimize(int canvasWidth, int canvasHeight)
{
    Q_D(PathOptimizer);

    emit titleUpdated(tr("Initializing optimizer..."));
    emit progressUpdated(0);

    initializeByGroups(d->root);

    emit titleUpdated(tr("Begin optimizing..."));
    emit progressUpdated(10);

    Ant* ant = new Ant(0, this);
    int initNodesCount = d->leaves.size();

    double minPathLength = std::numeric_limits<double>::max();
    QQueue<OptimizeNode*> minLengthPath;
    QMap<OptimizeNode*, int> minLengthArrivedNodes;

    //int coreNumber = omp_get_num_procs();
    //omp_set_num_threads(coreNumber * 0.6);
    int badTimes = 0;

    ant->initialize();
    ant->arrived(d->rootNode, QPointF(0, 0));

    while (true)
    {
        if (!ant->moveForward())
            break;
    }


    minPathLength = ant->totalLength();
    minLengthPath = ant->path();
    minLengthArrivedNodes = ant->arrivedNodes();

    emit progressUpdated(10.f + 85);

    d->optimizedPath.clear();
    for (OptimizeNode* node : minLengthPath)
    {
        QPair<LaserPrimitive*, int> pair;
        pair.first = static_cast<LaserPrimitive*>(node->documentItem());
        pair.second = minLengthArrivedNodes[node];
        d->optimizedPath.append(pair);
    }
    emit messageUpdated(tr("Optimizing ended."));
    emit progressUpdated(95);
    emit titleUpdated(tr("Outputing canvas."));

    qLogD << "min length is " << minPathLength;

    if (Config::Debug::generatePathImage())
    {
        cv::Mat canvas = cv::Mat(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(255, 255, 255));
        ::drawPath(canvas, minLengthPath, minLengthArrivedNodes);
        QString filename = QString("tmp/path.png");
        cv::imwrite(filename.toStdString(), canvas);
        canvas.release();
    }
    delete ant;

    emit messageUpdated(tr("Generated path canvas at tmp/path.png."));
    emit progressUpdated(100);
    emit titleUpdated(tr("Done."));
    emit finished();
}

bool PathOptimizer::isContainsLayers() const
{
    Q_D(const PathOptimizer);
    return d->containsLayers;
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

bool PathOptimizer::useGreedyAlgorithm() const
{
    Q_D(const PathOptimizer);
    return d->useGreedyAlgorithm;
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

void PathOptimizer::initializeByGroups(OptimizeNode* root)
{
    Q_D(PathOptimizer);
    QStack<OptimizeNode*> stack;

    for (OptimizeNode* node : root->childNodes())
    {
        stack.push(node);
    }

    while (!stack.isEmpty())
    {
        OptimizeNode* node = stack.pop();
        d->nodes.append(node);
        QString msg = QString(tr("[%1/%2] Node %3 created.")).arg(d->nodes.length()).arg(d->totalNodes).arg(node->nodeName());
        emit messageUpdated(msg);
        float progress = 8.f * d->nodes.length() / d->totalNodes;
        emit progressUpdated(progress);

        if (node->hasChildren())
        {
            for (OptimizeNode* childNode : node->childNodes())
            {
                stack.push(childNode);
            }
        }
        else
        {
            d->leaves.append(node);
        }
    }

    stack.clear();
    for (OptimizeNode* node : d->leaves)
    {
        if (node->nodeType() == LNT_PRIMITIVE)
            stack.push(node);
    }

    QSet<OptimizeNode*> travelled;
    while (!stack.isEmpty())
    {
        OptimizeNode* node = stack.pop();
        if (travelled.contains(node))
            continue;
        travelled.insert(node);

        QString msg = QString(tr("[%1/%2] Generating edges for node %3.")).arg(travelled.size()).arg(d->totalNodes).arg(node->nodeName());
        emit messageUpdated(msg);
        float progress = 8 + 2 * travelled.size() / d->totalNodes;
        emit progressUpdated(progress);
        if (node->parentNode())
        {
            // find all child nodes from its siblings
            QList<OptimizeNode*> siblingChildren = node->parentNode()->findAllLeaves(node);

            for (OptimizeNode* leafNode : siblingChildren)
            {
                OptimizeEdge* edge = new OptimizeEdge(node, leafNode);
                /*QString msg = QString(tr("[%1/%2] Created edge from node %3 to node %4 with length is %5."))
                    .arg(travelled.size())
                    .arg(d->totalNodes)
                    .arg(node->nodeName())
                    .arg(edge->b()->nodeName())
                    .arg(edge->length());
                emit messageUpdated(msg);*/
                node->addEdge(edge);
                d->edges.append(edge);
            }

            OptimizeNode* parentNode = node->parentNode();
            if (parentNode)
            {
                OptimizeEdge* edge = new OptimizeEdge(node, parentNode);
                /*QString msg = QString(tr("[%1/%2] Created edge from node %3 to node %4 with length is %5."))
                    .arg(travelled.size())
                    .arg(d->totalNodes)
                    .arg(node->nodeName())
                    .arg(edge->b()->nodeName())
                    .arg(edge->length());
                emit messageUpdated(msg);*/
                node->setOutEdge(edge);
                d->edges.append(edge);
                stack.push(node->parentNode());
            }
        }
    }

    d->rootNode = root;
    d->nodes.append(d->rootNode);
    for (OptimizeNode* node : d->leaves)
    {
        OptimizeEdge* edge = new OptimizeEdge(d->rootNode, node);
        d->rootNode->addEdge(edge);
        d->edges.append(edge);
        qLogD << d->rootNode->nodeName() << " --> " << node->nodeName() << ", length = " << edge->length()
            << ", at " << node->currentPos();
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
        totalNodes,
        Config::PathOptimization::maxIterations(),
        Config::PathOptimization::maxAnts(),
        Config::PathOptimization::maxTraverse(),
        Config::PathOptimization::volatileRate(),
        Config::PathOptimization::useGreedyAlgorithm(),
        true))
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
