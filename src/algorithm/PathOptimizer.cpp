#include "PathOptimizer.h"
#include "scene/LaserLayer.h"
#include "scene/LaserNode.h"
#include "scene/LaserPrimitive.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QQueue>
#include <QRandomGenerator>
#include <QStack>
#include <QtMath>
#include <QVector>
#include <opencv2/opencv.hpp>
#include <opencv2/flann.hpp>

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
    NodePrivate(LaserPrimitive* _primitive, Node* ptr)
        : q_ptr(ptr)
        , primitive(_primitive)
        , outEdge(nullptr)
    {
        if (primitive == nullptr)
        {
            center = cv::Point2f(0, 0);
            isVirtual = true;
        }
        else
        {
            points = primitive->cuttingPoints();
            name = primitive->nodeName();
            center.x = center.y = 0;

            double distBetweenHeadAndTail = cv::norm(points[0] - points[points.size() - 1]);
            // check the primitive is whether closour
            if (qFuzzyCompare(distBetweenHeadAndTail, 0))
            {
                cv::Mat samples(points.size(), 2, CV_32F);
                int row = 0;
                for (cv::Point2f point : points)
                {
                    samples.ptr<float>(row)[0] = point.x;
                    samples.ptr<float>(row)[1] = point.y;

                    center += point;
                    row++;
                }
                center /= static_cast<int>(points.size());
                flannIndex.build(samples, cv::flann::KDTreeIndexParams(1));
                isClosour = true;
            }
            else
            {
                cv::Mat samples(2, 2, CV_32F);
                cv::Point2f head = points[0];
                cv::Point2f tail = points[points.size() - 1];
                samples.ptr<cv::Point2f>(0)[0] = head;
                samples.ptr<cv::Point2f>(1)[0] = tail;
                center = (head + tail) / 2;
                flannIndex.build(samples, cv::flann::KDTreeIndexParams(1));
                isClosour = false;
            }
            isVirtual = false;
        }
    }

    LaserPrimitive* primitive;

    QList<Edge*> edges;
    Edge* outEdge;
    cv::Point2f center;
    std::vector<cv::Point2f> points;
    cv::flann::Index flannIndex;
    bool isClosour;
    bool isVirtual;
    QString name;
    Node* q_ptr;
};

Node::Node(LaserPrimitive* primitive, const QString& name)
    : m_ptr(new NodePrivate(primitive, this))
{
    Q_D(Node);
    if (!name.isEmpty())
        d->name = name;
}

LaserPrimitive* Node::primitive()
{
    Q_D(Node);
    return d->primitive;
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

cv::Point2f Node::nearestPoint(const cv::Point2f& point, int& index, double& dist)
{
    Q_D(Node);

    if (d->isVirtual)
    {
        index = 0;
        dist = 0;
        return d->center;
    }
    else
    {
        dist = 0.f;
        cv::Mat indices, dists;
        cv::Mat query(1, 2, CV_32F);
        cv::Point2f& queryPoint = query.ptr<cv::Point2f>(0)[0];
        queryPoint = point;
        d->flannIndex.knnSearch(query, indices, dists, 1);

        index = indices.ptr<int>(0)[0];
        cv::Point2f target = d->points[index];
        dist = cv::norm(point - target);
        return target;
    }
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
    if (d->isVirtual)
    {
        return d->center;
    }
    return d->points[0];
}

cv::Point2f Node::tailPoint() const
{
    Q_D(const Node);
    if (d->isVirtual)
    {
        return d->center;
    }
    return d->points[d->points.size() - 1];
}

cv::Point2f Node::point(int index) const
{
    Q_D(const Node);
    if (d->isVirtual)
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
    int pointIndex;
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
    double dist;
    node->nearestPoint(lastPos, d->pointIndex, dist);
    d->arrivedNodes.insert(node, d->pointIndex);
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
    if (d->currentNode->isClosour())
    {
        target = d->currentNode->point(d->pointIndex);
    }
    else
    {
        if (d->pointIndex == 0)
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
    double alpha = 1.f;
    double beta = 2.f;
    QRandomGenerator* random = QRandomGenerator::global();
    QMap<Edge*, double> edgeWeights;
    for (Edge* edge : d->currentNode->edges())
    {
        if (d->arrivedNodes.contains(edge->b()))
            continue;

        double weight = edge->pheromones() / edge->length();
        edgeWeights.insert(edge, weight);
    }

    for (QMap<Edge*, double>::iterator i = edgeWeights.begin(); i != edgeWeights.end(); i++)
    {
        double rnd = random->bounded(1.0);
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
    double Q = 10.f * d->optimizer->avgEdgeLength() * d->optimizer->edgesCount();
    double deltaRho = Q / d->totalLength;

    qLogD << " ant " << d->antIndex << " deltaRho is " << deltaRho;
    for (QSet<Edge*>::iterator i = d->pastEdges.begin(); i != d->pastEdges.end(); i++)
    {
        Edge* edge = (*i);
        double pheromones = edge->pheromones();
        edge->setPheromones(pheromones + deltaRho);
        qLogD << "update ant " << d->antIndex << "'s pheromones of edge [" << edge->a()->nodeName() << " --> "
            << edge->b()->nodeName() << "] from " << pheromones << " to "
            << edge->pheromones();
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
        qLogD << "  " << node->nodeName();
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

    QList<QList<QList<Node*>>> layerNodes;
    QList<Edge*> edges;
    QMap<LaserPrimitive*, Node*> pritmive2NodeMap;
    bool containsLayers;
    QList<Node*> nodes;
    QList<Node*> leaves;
    double avgEdgeLength;
    Node* rootNode;
    PathOptimizer* q_ptr;
};

PathOptimizer::PathOptimizer(LaserNode* root, bool containsLayers, QObject* parent)
    : QObject(parent)
    , m_ptr(new PathOptimizerPrivate(this))
{
    Q_D(PathOptimizer);
    d->containsLayers = containsLayers;

    initializeByGroups(root);

    double pheromones = 1.0f;

    double initPheromones = 1 / (d->avgEdgeLength * d->edges.size());
    qLogD << "The init pheromones is " << initPheromones;
    for (int i = 0; i < d->edges.size(); i++)
    {
        d->edges[i]->setPheromones(initPheromones);
        /*qLogD << i << ". " << d->edges[i]->a()->nodeName()
            << " --> " << d->edges[i]->b()->nodeName()
            << ", length: " << d->edges[i]->length()
            << ", pheromones: " << d->edges[i]->pheromones();*/
    }
}

PathOptimizer::~PathOptimizer()
{
    Q_D(PathOptimizer);
    qDeleteAll(d->nodes);
    qDeleteAll(d->edges);
}

void PathOptimizer::optimize(int canvasWidth, int canvasHeight, Path& primitives)
{
    Q_D(PathOptimizer);
    int maxAnts = 10;
    int maxIteration = 2;
    const double rho = 0.65f;

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
    for (int iteration = 0; iteration < maxIteration; iteration++)
    {
        qLogD << "iteration " << iteration;
        for (int i = 0; i < maxAnts; i++)
        {
            Ant* ant = ants[i];
            ant->initialize();
            ant->arrived(d->rootNode, cv::Point2f(0, 0));
        }
        for (int iAnt = 0; iAnt < maxAnts; iAnt++)
        {
            Ant* ant = ants[iAnt];
            while (true)
            {
                if (!ant->moveForward())
                    break;
            }
            qLogD << "ant " << iAnt << " total length: " << ant->totalLength();
        }

        for (Edge* edge : d->edges)
        {
            edge->volatize(rho);
        }

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
            }
        }
        printNodeAndEdges();
    }

    primitives.clear();
    for (Node* node : minLengthPath)
    {
        QPair<LaserPrimitive*, int> pair;
        pair.first = node->primitive();
        pair.second = minLengthArrivedNodes[node];
        primitives.append(pair);
    }

    qLogD << "min length is " << minPathLength << " at iteration " << minIteration << " by ant " << minAnt->antIndex();

    cv::Mat canvas = cv::Mat(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(255, 255, 255));
    ::drawPath(canvas, minLengthPath, minLengthArrivedNodes);
    QString filename = QString("tmp/path.png");
    cv::imwrite(filename.toStdString(), canvas);
    canvas.release();
    qDeleteAll(ants);
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

void PathOptimizer::initializeByTopologyLayers(QList<LaserNode*> groups)
{
    Q_D(PathOptimizer);
    QList<LaserNode*> topGroups;
    if (d->containsLayers)
    {
        topGroups = groups;
    }
    else
    {
        topGroups.append(groups[0]->parentNode());
    }
    for (LaserNode* group : topGroups)
    {
        int layerIndex = d->layerNodes.length();
        d->layerNodes.append(QList<QList<Node*>>());
        QMap<LaserNode*, int> levelMap;
        QStack<LaserNode*> stack;
        stack.push(group);
        levelMap.insert(group, -1);
        int maxLevel = 0;
        while (!stack.isEmpty())
        {
            LaserNode* node = stack.pop();
            int level = levelMap[node];
            if (level > maxLevel)
                maxLevel = level;
            if (node->hasChildren())
            {
                for (LaserNode* childNode : node->childNodes())
                {
                    stack.push(childNode);
                    levelMap.insert(childNode, level + 1);
                }
            }
        }

        for (int i = 0; i <= maxLevel; i++)
        {
            d->layerNodes[layerIndex].append(QList<Node*>());
        }
        for (QMap<LaserNode*, int>::iterator i = levelMap.begin(); i != levelMap.end(); i++)
        {
            LaserNode* laserNode = i.key();
            int level = i.value();
            if (level < 0)
                continue;

            LaserPrimitive* primitive = dynamic_cast<LaserPrimitive*>(laserNode);
            Node* node = new Node(primitive);
            d->layerNodes[layerIndex][level].append(node);
            d->pritmive2NodeMap[primitive] = node;
        }

    }

    d->avgEdgeLength = 0;
    for (int l = 0; l < d->layerNodes.size(); l++)
    {
        for (int i = d->layerNodes[l].size() - 1; i >= 0; i--)
        {
            for (int j = 0; j < d->layerNodes[l][i].size(); j++)
            {
                for (int n = 0; n < d->layerNodes[l][i].size(); n++)
                {
                    if (n == j)
                        continue;

                    Edge* edge = new Edge(d->layerNodes[l][i][j], d->layerNodes[l][i][n]);
                    d->avgEdgeLength += edge->length();
                    d->layerNodes[l][i][j]->addEdge(edge);
                    d->edges.append(edge);
                }

                if (i > 0)
                {
                    LaserPrimitive* primitive = d->layerNodes[l][i][j]->primitive();
                    LaserPrimitive* parentPrimitive = dynamic_cast<LaserPrimitive*>(primitive->parentNode());
                    if (parentPrimitive)
                    {
                        Node* parentNode = d->pritmive2NodeMap[parentPrimitive];
                        Edge* edge = new Edge(d->layerNodes[l][i][j], parentNode);
                        d->avgEdgeLength += edge->length();
                        d->layerNodes[l][i][j]->setOutEdge(edge);
                        d->edges.append(edge);
                    }
                }
            }
        }
    }
    d->avgEdgeLength /= d->edges.size();
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
        LaserPrimitive* primitive = dynamic_cast<LaserPrimitive*>(laserNode);
        if (primitive)
        {
            Node* node = new Node(primitive);
            d->nodes.append(node);
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
    }

    stack.clear();
    for (Node* node : d->leaves)
    {
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
        
        if (laserNode->parentNode())
        {
            // find all child nodes from its siblings
            QList<LaserNode*> siblingChildren = laserNode->parentNode()->findAllLeaves(laserNode);

            Node* node = laserNodeMap[laserNode];
            for (LaserNode* leafNode : siblingChildren)
            {
                Edge* edge = new Edge(node, laserNodeMap[leafNode]);
                node->addEdge(edge);
                d->edges.append(edge);
            }

            Node* parentNode = laserNodeMap[laserNode->parentNode()];
            if (parentNode)
            {
                Edge* edge = new Edge(node, parentNode);
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

    printNodeAndEdges();

    d->avgEdgeLength /= d->edges.size();
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

