#include "PathOptimizer.h"
#include "common/Config.h"
#include "scene/LaserLayer.h"
#include "scene/LaserNode.h"
#include "scene/LaserPrimitive.h"

#include <flann/flann.hpp>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QQueue>
#include <QRandomGenerator>
#include <QStack>
#include <QtMath>
#include <QVector>
#include <opencv2/opencv.hpp>
#include <omp.h>
//#include <opencv2/flann.hpp>

double distance(const cv::Point2f& p1, const cv::Point2f& p2)
{
    double diffX = p2.x - p1.x;
    double diffY = p2.y - p1.y;
    return std::sqrtf(diffX * diffX + diffY * diffY);
}

class NodePrivate
{
    Q_DECLARE_PUBLIC(Node)
public:
    NodePrivate(LaserNode* _laserNode, Node* ptr)
        : q_ptr(ptr)
        , laserNode(_laserNode)
        , outEdge(nullptr)
        , kdTree(nullptr)
    {
        LaserPrimitive* primitive = dynamic_cast<LaserPrimitive*>(laserNode);

        if (laserNode)
        {
            name = laserNode->nodeName();
            QPointF c = laserNode->position();
            center = cv::Point2f(c.x(), c.y());
        }
        if (primitive)
        {
            points = primitive->cuttingPoints();
            //points = primitive->mechiningPoints();
            //if (points.empty())
                //points = primitive->cuttingPoints();
            center.x = center.y = 0;

            double distBetweenHeadAndTail = cv::norm(points[0] - points[points.size() - 1]);
            // check the primitive is whether closour
            if (qFuzzyCompare(distBetweenHeadAndTail, 0))
            {
                for (int i = 0; i < points.size(); i++)
                {
                    cv::Point2f point = points[i];
                    center += point;
                }
                center /= static_cast<int>(points.size());
                isClosour = true;
            }
            else
            {
                points = std::vector<cv::Point2f>(2);
                cv::Point2f head = points[0];
                cv::Point2f tail = points[points.size() - 1];
                points[0] = head;
                points[1] = tail;
                center = (head + tail) / 2;
                isClosour = false;
            }
            {
                cv::Point2f origin;
                origin.x = 0;
                origin.y = 0;

                flann::Matrix<float> samplePoints = flann::Matrix<float>(&points[0].x, points.size(), 2);

                flann::KDTreeSingleIndexParams indexParams = flann::KDTreeSingleIndexParams();
                kdTree = new flann::KDTreeSingleIndex<flann::L2<float>>(samplePoints, indexParams);
                kdTree->buildIndex();
            }
        }
    }

    ~NodePrivate()
    {
        if (kdTree)
            delete kdTree;
        qLogD << "Node " << name << " destroyed.";
    }

    bool isVirtual() const
    {
        if (!laserNode)
            return true;
        return laserNode->isVirtual();
    }

    LaserNode* laserNode;

    QList<Edge*> edges;
    Edge* outEdge;
    cv::Point2f center;
    std::vector<cv::Point2f> points;
    //cv::flann::Index* flannIndex;
    flann::KDTreeSingleIndex<flann::L2<float>>* kdTree;
    bool isClosour;
    QString name;
    Node* q_ptr;
};

Node::Node(LaserNode* laserNode, const QString& name)
    : m_ptr(new NodePrivate(laserNode, this))
{
    Q_D(Node);
    if (!name.isEmpty())
        d->name = name;
}

Node::~Node()
{

}

LaserPrimitive* Node::primitive()
{
    Q_D(Node);
    return dynamic_cast<LaserPrimitive*>(d->laserNode);;
}

void Node::addEdge(Edge* edge)
{
    Q_D(Node);
    d->edges.append(edge);
}

void Node::setOutEdge(Edge* edge)
{
    Q_D(Node);
    d->outEdge = edge;
}

QList<Edge*> Node::edges() const
{
    Q_D(const Node);
    return d->edges;
}

Edge* Node::outEdge() const
{
    Q_D(const Node);
    return d->outEdge;
}

cv::Point2f Node::startPos() const
{
    Q_D(const Node);
    return d->center;
}

cv::Point2f Node::nearestPoint(cv::Point2f point, int& index, float& dist)
{
    Q_D(Node);

    if (d->isVirtual())
    {
        index = 0;
        dist = 0;
        //return d->center;
        d->center = point;
        return point;
    }
    else
    {
        dist = 0.f;

        flann::SearchParams searchParams = flann::SearchParams();
        searchParams.checks = -1;
        searchParams.sorted = false;
        searchParams.use_heap = flann::FLANN_True;

        int* indices = new int[1];
        float* dists = new float[1];
        cv::Point2f target(0, 0);
        {
            flann::Matrix<int> indicesMatrix(indices, 1, 1);
            flann::Matrix<float> distsMatrix(dists, 1, 1);
            flann::Matrix<float> queryPoint = flann::Matrix<float>(&point.x, 1, 2);
            d->kdTree->knnSearch(queryPoint, indicesMatrix, distsMatrix, 1, searchParams);

            index = indicesMatrix.ptr()[0];
            target = d->points[index];
            dist = std::sqrt(distsMatrix.ptr()[0]);
        }
        //qLogD << d->name << " " << index << " " << dist << " " << dist << " " << target.x << ", " << target.y;
        //qLogD;
        delete[] indices;
        delete[] dists;
        return target;
    }
}

cv::Point2f Node::currentPos() const
{
    Q_D(const Node);
    return d->center;
}

std::vector<cv::Point2f> Node::points() const
{
    Q_D(const Node);
    return d->points;
}

bool Node::isClosour() const
{
    Q_D(const Node);
    return d->isClosour;
}

cv::Point2f Node::headPoint() const
{
    Q_D(const Node);
    if (d->isVirtual())
    {
        return d->center;
    }
    return d->points[0];
}

cv::Point2f Node::tailPoint() const
{
    Q_D(const Node);
    if (d->isVirtual())
    {
        return d->center;
    }
    return d->points[d->points.size() - 1];
}

cv::Point2f Node::point(int index) const
{
    Q_D(const Node);
    if (d->isVirtual())
    {
        return d->center;
    }
    return d->points[index];
}

QString Node::nodeName() const
{
    Q_D(const Node);
    return d->name;
}

bool Node::isVirtual() const
{
    Q_D(const Node);
    return d->isVirtual();
}

class EdgePrivate
{
    Q_DECLARE_PUBLIC(Edge)
public:
    EdgePrivate(Edge* ptr)
        : q_ptr(ptr)
        , a(nullptr)
        , b(nullptr)
        , length(0)
        , pheromones(0)
        , force(false)
        , forward(false)
    {
    }

    Node* a;
    Node* b;

    double length;
    double pheromones;

    bool force;
    bool forward;
    Edge* q_ptr;
};

Edge::Edge(Node* a, Node* b, bool force, bool forward)
    : m_ptr(new EdgePrivate(this))
{
    Q_D(Edge);
    d->a = a;
    d->b = b;
    d->force = force;
    d->forward = forward;

    d->length = cv::norm(a->startPos() - b->startPos());
}

void Edge::clear()
{

}

double Edge::length() const
{
    Q_D(const Edge);
    return d->length;
}

void Edge::setLength(double length)
{
    Q_D(Edge);
    d->length = length;
}

void Edge::setLength(const cv::Point2f& p1, const cv::Point2f& p2)
{
    setLength(distance(p1, p2));
}

double Edge::pheromones() const
{
    Q_D(const Edge);
    return d->pheromones;
}

void Edge::setPheromones(double pheromones)
{
    Q_D(Edge);
    d->pheromones = pheromones;
}

void Edge::volatize(double rho)
{
    Q_D(Edge);
    d->pheromones *= (1 - rho);
}

Node* Edge::a()
{
    Q_D(Edge);
    return d->a;
}

Node* Edge::b()
{
    Q_D(Edge);
    return d->b;
}

void Edge::print()
{
    Q_D(Edge);
    qLogD << "    " << d->a->nodeName() << " --> " << d->b->nodeName() << ", " << d->pheromones;
}

class AntPrivate
{
    Q_DECLARE_PUBLIC(Ant)
public:
    AntPrivate(Ant* ant)
        : q_ptr(ant)
        , currentNode(nullptr)
    {}

    QMap<Node*, int> arrivedNodes;
    QSet<Edge*> pastEdges;
    QQueue<Node*> path;
    Node* currentNode;
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

void Ant::arrived(Node* node, const cv::Point2f& lastPos)
{
    Q_D(Ant);
    float dist;
    node->nearestPoint(lastPos, d->currentPointIndex, dist);
    if (node->isVirtual())
    {
        for (Edge* edge : node->edges())
        {
            float edgeLength;
            int bPointIndex;
            edge->b()->nearestPoint(lastPos, bPointIndex, edgeLength);
            edge->setLength(edgeLength);
            edge->setPheromones(d->optimizer->initPheromones());
        }
    }
    d->arrivedNodes.insert(node, d->currentPointIndex);
    d->path.push_back(node);
    d->currentNode = node;
    d->totalLength += dist;
    //qLogD << "ant " << antIndex() << " arrived at " << currentNode()->nodeName() << " by walking through " << dist << " units";
}

Node* Ant::currentNode() const
{
    Q_D(const Ant);
    return d->currentNode;
}

cv::Point2f Ant::currentPos() const
{
    Q_D(const Ant);
    cv::Point2f target;
    if (d->currentNode->isVirtual())
    {
        target = d->currentNode->currentPos();
    }
    else if (d->currentNode->isClosour())
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
    Edge* maxEdge = nullptr;
    QRandomGenerator* random = QRandomGenerator::global();
    QMap<Edge*, double> edgeWeights;
    for (Edge* edge : d->currentNode->edges())
    {
        if (d->arrivedNodes.contains(edge->b()))
            continue;

        //double weight = edge->pheromones();// / edge->length();
        double weight = qPow(edge->pheromones(), 1) + qPow(1.0 / edge->length(), 0.5);
        if (d->optimizer->useGreedyAlgorithm())
            weight /= edge->length() * 10000000;
        edgeWeights.insert(edge, weight);
    }

    for (QMap<Edge*, double>::iterator i = edgeWeights.begin(); i != edgeWeights.end(); i++)
    {
        double rnd = random->bounded(1.0);
        if (d->optimizer->useGreedyAlgorithm())
            rnd = 1.0;
        double p = rnd * i.value();
        Edge* edge = i.key();

        //qLogD << "  ant " << d->antIndex << " [" << edge->a()->nodeName()
            //<< " -> " << edge->b()->nodeName() << qSetRealNumberPrecision(10) << "]"
            //<< " rnd = " << rnd << ", pheromones = " << edge->pheromones() << ", weight = "
            //<< i.value() << ", p = " << p;
        if (p > maxProb)
        {
            maxProb = p;
            maxEdge = edge;
        }
    }

    if (maxEdge == nullptr)
    {
        if (!d->currentNode->outEdge())
        {
            return false;
        }
        //qLogD << "    ant " << d->antIndex << " walked through all nodes within this group.";
        arrived(d->currentNode->outEdge()->b(), currentPos());
        return true;
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

void Ant::updatePheromones()
{
    Q_D(Ant);
    //double Q = 0.1f * d->optimizer->avgEdgeLength() * d->optimizer->edgesCount();
    double Q = 0.01;
    double deltaRho = Q / d->totalLength;

    //qLogD << " ant " << d->antIndex << " deltaRho is " << deltaRho;
    for (QSet<Edge*>::iterator i = d->pastEdges.begin(); i != d->pastEdges.end(); i++)
    {
        Edge* edge = (*i);
        double pheromones = edge->pheromones();
        edge->setPheromones(pheromones + deltaRho);
        //qLogD << "update ant " << d->antIndex << "'s pheromones of edge [" << edge->a()->nodeName() << " --> "
            //<< edge->b()->nodeName() << "] from " << pheromones << " to "
            //<< edge->pheromones();
    }
}

double Ant::totalLength() const
{
    Q_D(const Ant);
    return d->totalLength;
}

void drawPath(cv::Mat& canvas, const QQueue<Node*>& path, const QMap<Node*, int>& arrivedNodes)
{
    cv::Point2f lastPos(0, 0);
    QRandomGenerator random(QDateTime::currentDateTime().toMSecsSinceEpoch());
    for (int i = 0; i < path.length(); i++)
    {
        Node* node = path[i];
        if (i != 0 && node->isVirtual())
            continue;
        int pointIndex = arrivedNodes[node];
        cv::Point2f startPos = node->point(pointIndex);
        cv::circle(canvas, startPos, 20, cv::Scalar(0, 0, 255), 5);
        if (i != 0)
        {
            cv::line(canvas, lastPos, startPos, cv::Scalar(0, 0, 255), 3);
        }
        std::vector<cv::Point2f> points = node->points();
        cv::Mat pointsMat(points);
        pointsMat.convertTo(pointsMat, CV_32S);
        cv::polylines(canvas, pointsMat, false, cv::Scalar(random.bounded(256), random.bounded(256), random.bounded(256)), 5);
        lastPos = startPos;
        if (!node->isClosour())
        {
            if (pointIndex == 0)
            {
                lastPos = node->tailPoint();
            }
            else
            {
                lastPos = node->headPoint();
            }
            cv::circle(canvas, lastPos, 18, cv::Scalar(0, 255, 0), 6);
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

QQueue<Node*> Ant::path() const
{
    Q_D(const Ant);
    return d->path;
}

QMap<Node*, int> Ant::arrivedNodes() const
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
    {}

    LaserNode* root;
    int totalNodes;
    QList<QList<QList<Node*>>> layerNodes;
    QList<Edge*> edges;
    QMap<LaserPrimitive*, Node*> pritmive2NodeMap;
    bool containsLayers;
    QList<Node*> nodes;
    QList<Node*> leaves;
    double avgEdgeLength;
    double initPheromones;
    Node* rootNode;
    int maxAnts;
    int maxIterations;
    int maxTraverse;
    float volatileRate;
    bool useGreedyAlgorithm;
    PathOptimizer::Path optimizedPath;
    PathOptimizer* q_ptr;
};

PathOptimizer::PathOptimizer(LaserNode* root, int totalNodes, int maxIterations,
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

    d->initPheromones = 1 / (d->avgEdgeLength * d->nodes.size());
    qLogD << "The init pheromones is " << d->initPheromones;
    emit messageUpdated(QString(tr("The init pheromones is %1")).arg(d->initPheromones));
    for (int i = 0; i < d->edges.size(); i++)
    {
        d->edges[i]->setPheromones(d->initPheromones);
        /*qLogD << i << ". " << d->edges[i]->a()->nodeName()
            << " --> " << d->edges[i]->b()->nodeName()
            << ", length: " << d->edges[i]->length()
            << ", pheromones: " << d->edges[i]->pheromones();*/
    }

    //int maxAnts = std::min(static_cast<int>(d->leaves.size() * 1.5f), d->maxAnts);
    int maxAnts = d->leaves.size() * 1;
    int maxIteration = std::min(d->maxTraverse / maxAnts, d->maxIterations);

    if (d->useGreedyAlgorithm)
    {
        maxAnts = 1;
        maxIteration = 1;
    }

    emit messageUpdated(QString(tr("Count of ants is %1, count of iterations is %2")).arg(maxAnts).arg(maxIteration));
    qDebug() << "maxAnts: " << maxAnts << ", maxIteration: " << maxIteration;

    qDebug() << "maxAnts: " << maxAnts;
    qDebug() << "maxIteration: " << maxIteration;
    double rho = d->volatileRate;

    QList<Ant*> ants;
    ants.reserve(maxAnts);
    QRandomGenerator random(QDateTime::currentDateTime().toMSecsSinceEpoch());
    int initNodesCount = d->leaves.size();

    for (int i = 0; i < maxAnts; i++)
    {
        Ant* ant = new Ant(i, this);
        ants.append(ant);
    }

    double minPathLength = std::numeric_limits<double>::max();
    QQueue<Node*> minLengthPath;
    QMap<Node*, int> minLengthArrivedNodes;
    int minIteration = 0;
    Ant* minAnt = nullptr;

    int coreNumber = omp_get_num_procs();
    omp_set_num_threads(coreNumber * 0.6);
    int badTimes = 0;

    for (int iteration = 0; iteration < maxIteration; iteration++)
    {
        qLogD << "iteration " << iteration;
        emit messageUpdated(QString(tr("[%1/%2] Iteration %1 beginning...")).arg(iteration).arg(maxIteration));
#pragma omp parallel 
        {
#pragma omp for
            for (int i = 0; i < maxAnts; i++)
            {
                Ant* ant = ants[i];
                ant->initialize();
                ant->arrived(d->rootNode, cv::Point2f(0, 0));
            }
        }

#pragma omp parallel 
        {
#pragma omp for
            for (int iAnt = 0; iAnt < maxAnts; iAnt++)
            {
                Ant* ant = ants[iAnt];
                while (true)
                {
                    if (!ant->moveForward())
                        break;
                }
                //qLogD << "ant " << iAnt << " total length: " << ant->totalLength();
            }
        }

#pragma omp parallel 
        {
#pragma omp for
            for (int iEdge = 0; iEdge < d->edges.length(); iEdge++)
            {
                Edge* edge = d->edges[iEdge];
                edge->volatize(rho);
            }
        }

        bool updated = false;
        for (int iAnt = 0; iAnt < maxAnts; iAnt++)
        {
            Ant* ant = ants[iAnt];
            ant->updatePheromones();
            //cv::Mat canvas = cv::Mat(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(255, 255, 255));
            //ant->drawPath(canvas, iteration);
            if (ant->totalLength() < minPathLength)
            {
                minPathLength = ant->totalLength();
                minLengthPath = ant->path();
                minLengthArrivedNodes = ant->arrivedNodes();
                minIteration = iteration;
                minAnt = ant;
                updated = true;
            }
        }

        emit progressUpdated(10.f + iteration * 85.f / maxIteration);
        if (!updated)
            badTimes++;

        if (badTimes >= 5)
        {
            emit messageUpdated(QString(tr("The optimized path has been already generated, quit calculating.")));
            break;
        }
    }

    d->optimizedPath.clear();
    for (Node* node : minLengthPath)
    {
        QPair<LaserPrimitive*, int> pair;
        pair.first = node->primitive();
        pair.second = minLengthArrivedNodes[node];
        d->optimizedPath.append(pair);
    }
    emit messageUpdated(tr("Optimizing ended."));
    emit progressUpdated(95);
    emit titleUpdated(tr("Outputing canvas."));

    qLogD << "min length is " << minPathLength << " at iteration " << minIteration << " by ant " << minAnt->antIndex();

    cv::Mat canvas = cv::Mat(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(255, 255, 255));
    ::drawPath(canvas, minLengthPath, minLengthArrivedNodes);
    QString filename = QString("tmp/path.png");
    cv::imwrite(filename.toStdString(), canvas);
    canvas.release();
    qDeleteAll(ants);

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

double PathOptimizer::avgEdgeLength() const
{
    Q_D(const PathOptimizer);
    return d->avgEdgeLength;
}

double PathOptimizer::initPheromones() const
{
    Q_D(const PathOptimizer);
    return d->initPheromones;
}

QList<Edge*> PathOptimizer::edges() const
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

void PathOptimizer::initializeByGroups(LaserNode* root)
{
    Q_D(PathOptimizer);
    QStack<LaserNode*> stack;

    for (LaserNode* node : root->childNodes())
    {
        stack.push(node);
    }

    QMap<LaserNode*, Node*> laserNodeMap;
    while (!stack.isEmpty())
    {
        LaserNode* laserNode = stack.pop();
        //LaserPrimitive* primitive = dynamic_cast<LaserPrimitive*>(laserNode);
        Node* node = new Node(laserNode);
        d->nodes.append(node);
        QString msg = QString(tr("[%1/%2] Node %3 created.")).arg(d->nodes.length()).arg(d->totalNodes).arg(node->nodeName());
        emit messageUpdated(msg);
        float progress = 8.f * d->nodes.length() / d->totalNodes;
        emit progressUpdated(progress);
        laserNodeMap.insert(laserNode, node);

        if (laserNode->hasChildren())
        {
            for (LaserNode* childNode : laserNode->childNodes())
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
    for (Node* node : d->leaves)
    {
        if (node->primitive())
            stack.push(node->primitive());
    }

    d->avgEdgeLength = 0;
    QSet<LaserNode*> travelled;
    while (!stack.isEmpty())
    {
        LaserNode* laserNode = stack.pop();
        if (travelled.contains(laserNode))
            continue;
        travelled.insert(laserNode);

        QString msg = QString(tr("[%1/%2] Generating edges for node %3.")).arg(travelled.size()).arg(d->totalNodes).arg(laserNode->nodeName());
        emit messageUpdated(msg);
        float progress = 8 + 2 * travelled.size() / d->totalNodes;
        emit progressUpdated(progress);
        if (laserNode->parentNode())
        {
            // find all child nodes from its siblings
            QList<LaserNode*> siblingChildren = laserNode->parentNode()->findAllLeaves(laserNode);

            Node* node = laserNodeMap[laserNode];
            for (LaserNode* leafNode : siblingChildren)
            {
                Edge* edge = new Edge(node, laserNodeMap[leafNode]);
                QString msg = QString(tr("[%1/%2] Created edge from node %3 to node %4 with length is %5."))
                    .arg(travelled.size())
                    .arg(d->totalNodes)
                    .arg(node->nodeName())
                    .arg(edge->b()->nodeName())
                    .arg(edge->length());
                emit messageUpdated(msg);
                node->addEdge(edge);
                d->edges.append(edge);
            }

            Node* parentNode = laserNodeMap[laserNode->parentNode()];
            if (parentNode)
            {
                Edge* edge = new Edge(node, parentNode);
                QString msg = QString(tr("[%1/%2] Created edge from node %3 to node %4 with length is %5."))
                    .arg(travelled.size())
                    .arg(d->totalNodes)
                    .arg(node->nodeName())
                    .arg(edge->b()->nodeName())
                    .arg(edge->length());
                emit messageUpdated(msg);
                d->avgEdgeLength += edge->length();
                node->setOutEdge(edge);
                d->edges.append(edge);
                stack.push(laserNode->parentNode());
            }
        }
    }

    d->rootNode = new Node(nullptr, "Root");
    d->nodes.append(d->rootNode);
    for (Node* node : d->leaves)
    {
        Edge* edge = new Edge(d->rootNode, node);
        d->rootNode->addEdge(edge);
        d->edges.append(edge);
        d->avgEdgeLength += edge->length();
    }

    d->avgEdgeLength /= d->edges.size();
    emit messageUpdated(QString(tr("average edge length is %1")).arg(d->avgEdgeLength));
    //printNodeAndEdges();
}

void PathOptimizer::printNodeAndEdges()
{
    Q_D(PathOptimizer);
    for (Node* node : d->nodes)
    {
        qLogD << "node " << node->nodeName() << "'s edges:";
        for (Edge* edge : node->edges())
        {
            qLogD << "    " << edge->a()->nodeName() << " --> " << edge->b()->nodeName() << " " << edge->pheromones();
        }
        if (node->outEdge())
        {
            Edge* edge = node->outEdge();
            qLogD << "    " << edge->a()->nodeName() << " --> " << edge->b()->nodeName() << " " << edge->pheromones();
        }
    }
}

OptimizerController::OptimizerController(LaserNode* root, int totalNodes, QObject* parent)
    : QObject(parent)
    , m_dialog(new ProgressDialog)
    , m_optimizer(new PathOptimizer(
        root,
        totalNodes,
        Config::OptimizePathMaxIterations(),
        Config::OptimizePathMaxAnts(),
        Config::OptimizePathMaxTraverseCount(),
        Config::OptimizePathVolatileRate(),
        Config::OptimizePathUseGreedyAlgorithm(),
        true))
{
    qLogD << "OptimizerController";
    m_optimizer->moveToThread(&m_thread);
    //connect(&m_thread, &QThread::finished, m_optimizer.data(), &QObject::deleteLater);
    connect(this, &OptimizerController::start, m_optimizer.data(), &PathOptimizer::optimize);
    connect(m_optimizer.data(), &PathOptimizer::finished, this, &OptimizerController::finished);
    connect(m_optimizer.data(), &PathOptimizer::finished, m_dialog.data(), &ProgressDialog::finished);
    connect(m_optimizer.data(), &PathOptimizer::messageUpdated, m_dialog.data(), &ProgressDialog::addMessage);
    connect(m_optimizer.data(), &PathOptimizer::progressUpdated, m_dialog.data(), &ProgressDialog::setProgress);
    connect(m_optimizer.data(), &PathOptimizer::titleUpdated, m_dialog.data(), &ProgressDialog::setTitle);

    m_thread.start();
}

OptimizerController::~OptimizerController()
{
}

PathOptimizer::Path OptimizerController::optimize(float pageWidth, float pageHeight)
{
    emit start(pageWidth, pageHeight);
    m_dialog->exec();
    qLogD << "optimized";
    return m_optimizer->optimizedPath();
}

void OptimizerController::finished()
{
    m_dialog->setProgress(100);
}
