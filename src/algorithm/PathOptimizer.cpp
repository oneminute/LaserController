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

float distance(const cv::Point2f& p1, const cv::Point2f& p2)
{
    float diffX = p2.x - p1.x;
    float diffY = p2.y - p1.y;
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
        points = primitive->cuttingPoints();
        center.x = center.y = 0;

        float distBetweenHeadAndTail = cv::norm(points[0] - points[points.size() - 1]);
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
    }

    LaserPrimitive* primitive;

    QList<Edge*> edges;
    Edge* outEdge;
    cv::Point2f center;
    std::vector<cv::Point2f> points;
    cv::flann::Index flannIndex;
    bool isClosour;
    Node* q_ptr;
};

Node::Node(LaserPrimitive* primitive)
    : m_ptr(new NodePrivate(primitive, this))
{
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

cv::Point2f Node::nearestPoint(const cv::Point2f& point, int& index, float& dist)
{
    Q_D(Node);
    dist = 0.f;
    cv::Mat indices, dists;
    cv::Mat query(1, 2, CV_32F);
    cv::Point2f& queryPoint = query.ptr<cv::Point2f>(0)[0];
    queryPoint = point;
    d->flannIndex.knnSearch(query, indices, dists, 1);

    index = indices.ptr<int>(0)[0];
    //dist = std::sqrtf(dists.ptr<float>(0)[0]);
    cv::Point2f target = d->points[index];
    dist = cv::norm(point - target);
    return target;
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
    return d->points[0];
}

cv::Point2f Node::tailPoint() const
{
    Q_D(const Node);
    return d->points[d->points.size() - 1];
}

cv::Point2f Node::point(int index) const
{
    Q_D(const Node);
    return d->points[index];
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

    float length;
    float pheromones;

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

float Edge::length() const
{
    Q_D(const Edge);
    return d->length;
}

void Edge::setLength(float length)
{
    Q_D(Edge);
    d->length = length;
}

void Edge::setLength(const cv::Point2f& p1, const cv::Point2f& p2)
{
    setLength(distance(p1, p2));
}

float Edge::pheromones() const
{
    Q_D(const Edge);
    return d->pheromones;
}

void Edge::setPheromones(float pheromones)
{
    Q_D(Edge);
    d->pheromones = pheromones;
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
    QMap<Edge*, float> pastEdges;
    QQueue<Node*> nodesQueue;
    Node* currentNode;
    int antIndex;
    int pointIndex;
    float totalLength;
    Ant* q_ptr;
};

Ant::Ant(int antIndex)
    : m_ptr(new AntPrivate(this))
{
    Q_D(Ant);
    d->antIndex = antIndex;
}

void Ant::initialize()
{
    Q_D(Ant);
    d->arrivedNodes.clear();
    d->nodesQueue.clear();
    d->pastEdges.clear();
    d->currentNode = nullptr;
    d->totalLength = 0.f;
}

void Ant::arrived(Node* node, const cv::Point2f& lastPos)
{
    Q_D(Ant);
    float dist;
    node->nearestPoint(lastPos, d->pointIndex, dist);
    d->arrivedNodes.insert(node, d->pointIndex);
    d->nodesQueue.push_back(node);
    d->currentNode = node;
    d->totalLength += dist;
    qLogD << "ant " << antIndex() << " arrived at " << currentNode()->primitive()->nodeName() << " by walking through " << dist << " units";
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
    float maxProb = 0.f;
    Edge* maxEdge = nullptr;
    float alpha = 1.f;
    float beta = 2.f;
    //QMap<Node*, int> neighbourPointIndex;
    int maxIndex;
    float maxDistance;
    for (Edge* edge : d->currentNode->edges())
    {
        if (d->arrivedNodes.contains(edge->b()))
            continue;

        int index;
        float distance;
        cv::Point2f pointPos = currentPos();
        cv::Point2f pos = edge->b()->nearestPoint(pointPos, index, distance);
        //neighbourPointIndex.insert(edge->b(), index);
        float eta = 1.f / distance;
        float prob = std::powf(edge->pheromones(), alpha) + std::powf(eta, beta);
        if (prob > maxProb)
        {
            maxEdge = edge;
            maxProb = prob;
            maxIndex = index;
            maxDistance = distance;
        }
    }

    if (maxEdge == nullptr)
    {
        if (!d->currentNode->outEdge())
        {
            return false;
        }
        qLogD << "    ant " << d->antIndex << " walked through all nodes within this group.";
        arrived(d->currentNode->outEdge()->b(), currentPos());
        return true;
    }
    arrived(maxEdge->b(), currentPos());
    d->pastEdges.insert(maxEdge, maxDistance);
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
    const float Q = 1000.f;
    float deltaRho = Q / d->totalLength;

    for (QMap<Edge*, float>::iterator i = d->pastEdges.begin(); i != d->pastEdges.end(); i++)
    {
        float pheromones = i.key()->pheromones();
        i.key()->setPheromones(pheromones + deltaRho);
        qLogD << "update ant " << d->antIndex << "'s pheromones of edge [" << i.key()->a()->primitive()->nodeName() << " --> "
            << i.key()->b()->primitive()->nodeName() << "] from " << pheromones << " to "
            << i.key()->pheromones();
    }
}

float Ant::totalLength() const
{
    Q_D(const Ant);
    return d->totalLength;
}

void Ant::drawPath(cv::Mat& canvas, int iteration)
{
    Q_D(const Ant);
    cv::Point2f lastPos(0, 0);
    for (int i = 0; i < d->nodesQueue.length(); i++)
    {
        cv::circle(canvas, lastPos, 20, cv::Scalar(0, 0, 255), 5);
        Node* node = d->nodesQueue[i];
        int pointIndex = d->arrivedNodes[node];
        cv::Point2f startPos = node->point(pointIndex);
        cv::line(canvas, lastPos, startPos, cv::Scalar(0, 0, 255), 3);
        std::vector<cv::Point2f> points = node->points();
        cv::Mat pointsMat(points);
        pointsMat.convertTo(pointsMat, CV_32S);
        cv::polylines(canvas, pointsMat, false, cv::Scalar(255, 0, 0), 5);
        lastPos = startPos;
        if (!node->isClosour())
        {
            if (d->pointIndex == 0)
            {
                lastPos = node->tailPoint();
            }
            else
            {
                lastPos = node->headPoint();
            }
        }
        qLogD << "  " << node->primitive()->nodeName();
    }

    QString filename = QString("tmp/path_%1_%2.png").arg(iteration).arg(d->antIndex);
    cv::imwrite(filename.toStdString(), canvas);
    canvas.release();
}

class PathOptimizerPrivate
{
    Q_DECLARE_PUBLIC(PathOptimizer)
public:
    PathOptimizerPrivate(PathOptimizer* ptr)
        : q_ptr(ptr)
    {}

    QList<QList<QList<Node*>>> nodes;
    QList<Edge*> edges;
    QMap<LaserPrimitive*, Node*> pritmive2NodeMap;
    PathOptimizer* q_ptr;
};

PathOptimizer::PathOptimizer(QList<LaserLayer*> layers, QObject* parent)
    : QObject(parent)
    , m_ptr(new PathOptimizerPrivate(this))
{
    Q_D(PathOptimizer);

    for (LaserLayer* layer : layers)
    {
        if (!layer->isAvailable())
            continue;
        int layerIndex = d->nodes.length();
        d->nodes.append(QList<QList<Node*>>());
        QMap<LaserNode*, int> levelMap;
        QStack<LaserNode*> stack;
        stack.push(layer);
        levelMap.insert(layer, -1);
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
            d->nodes[layerIndex].append(QList<Node*>());
        }
        for (QMap<LaserNode*, int>::iterator i = levelMap.begin(); i != levelMap.end(); i++)
        {
            LaserNode* laserNode = i.key();
            int level = i.value();
            if (level < 0)
                continue;

            LaserPrimitive* primitive = dynamic_cast<LaserPrimitive*>(laserNode);
            Node* node = new Node(primitive);
            d->nodes[layerIndex][level].append(node);
            d->pritmive2NodeMap[primitive] = node;
        }

    }

    float avgEdgeLength = 0;
    for (int l = 0; l < d->nodes.size(); l++)
    {
        for (int i = d->nodes[l].size() - 1; i >= 0; i--)
        {
            for (int j = 0; j < d->nodes[l][i].size(); j++)
            {
                for (int n = 0; n < d->nodes[l][i].size(); n++)
                {
                    if (n == j)
                        continue;

                    Edge* edge = new Edge(d->nodes[l][i][j], d->nodes[l][i][n]);
                    avgEdgeLength += edge->length();
                    d->nodes[l][i][j]->addEdge(edge);
                    d->edges.append(edge);
                }

                if (i > 0)
                {
                    LaserPrimitive* primitive = d->nodes[l][i][j]->primitive();
                    LaserPrimitive* parentPrimitive = dynamic_cast<LaserPrimitive*>(primitive->parentNode());
                    if (parentPrimitive)
                    {
                        Node* parentNode = d->pritmive2NodeMap[parentPrimitive];
                        Edge* edge = new Edge(d->nodes[l][i][j], parentNode);
                        avgEdgeLength += edge->length();
                        d->nodes[l][i][j]->setOutEdge(edge);
                        d->edges.append(edge);
                    }
                }
            }
        }
    }
    avgEdgeLength /= d->edges.size();
    //float pheromones = 1.0f / (d->pritmive2NodeMap.size() * avgEdgeLength);
    float pheromones = 1.0f;

    for (int i = 0; i < d->edges.size(); i++)
    {
        d->edges[i]->setPheromones(pheromones);
        qLogD << i << ". " << d->edges[i]->a()->primitive()->nodeName()
            << " --> " << d->edges[i]->b()->primitive()->nodeName()
            << ", length: " << d->edges[i]->length()
            << ", pheromones: " << d->edges[i]->pheromones();
    }
}

PathOptimizer::~PathOptimizer()
{
}

void PathOptimizer::optimize(int canvasWidth, int canvasHeight)
{
    Q_D(PathOptimizer);
    int maxAnts = 3;
    int maxIteration = 100;
    const float rho = 0.5f;

    QList<Ant*> ants;
    ants.reserve(maxAnts);
    QRandomGenerator random(QDateTime::currentDateTime().toMSecsSinceEpoch());
    int initNodesCount = d->nodes[0][d->nodes[0].size() - 1].size();

    for (int i = 0; i < maxAnts; i++)
    {
        Ant* ant = new Ant(i);
        ants.append(ant);
    }

    for (int iteration = 0; iteration < maxIteration; iteration++)
    {
        qLogD << "iteration " << iteration;
        for (int i = 0; i < maxAnts; i++)
        {
            int nodeIndex = random.bounded(initNodesCount);
            Node* node = d->nodes[0][d->nodes[0].size() - 1][nodeIndex];
            Ant* ant = ants[i];
            ant->initialize();
            ant->arrived(node, cv::Point2f(0, 0));
            ants.append(ant);
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
            float pheromones = edge->pheromones();
            edge->setPheromones(pheromones * (1 - rho));
        }

        for (int iAnt = 0; iAnt < maxAnts; iAnt++)
        {
            Ant* ant = ants[iAnt];
            ant->updatePheromones();
            cv::Mat canvas = cv::Mat(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(255, 255, 255));
            ant->drawPath(canvas, iteration);
        }
        
    }
}

