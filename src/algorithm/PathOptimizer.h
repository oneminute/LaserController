#ifndef PATHOPTIMIZER_H
#define PATHOPTIMIZER_H

#include <QObject>
#include <QVector2D>
#include "scene/LaserNode.h"
#include "scene/LaserPrimitive.h"

class Edge;
class NodePrivate;
class Node
{
public:
    explicit Node(LaserPrimitive* primitive);

    LaserPrimitive* primitive();

    void addEdge(Edge* edge);
    void setOutEdge(Edge* edge);

    QList<Edge*> edges() const;
    Edge* outEdge() const;

    cv::Point2f startPos() const;
    cv::Point2f nearestPoint(const cv::Point2f& point, int& index, float& dist);
    std::vector<cv::Point2f> points() const;
    bool isClosour() const;
    cv::Point2f headPoint() const;
    cv::Point2f tailPoint() const;
    cv::Point2f point(int index) const;

    //Node* parent();
    //void addChild(Node* node);
    //QList<Node*>& children();

private:
    QScopedPointer<NodePrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, Node)
    Q_DISABLE_COPY(Node)
};

class EdgePrivate;
class Edge
{
public:
    explicit Edge(Node* a, Node* b, bool force = false, bool forward = false);

    void clear();

    float length() const;
    void setLength(float length);
    void setLength(const cv::Point2f& p1, const cv::Point2f& p2);

    float pheromones() const;
    void setPheromones(float pheromones);

    Node* a();
    Node* b();

private:
    QScopedPointer<EdgePrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, Edge)
    Q_DISABLE_COPY(Edge)
};

class AntPrivate;
class Ant
{
public:
    explicit Ant(int antIndex);

    void initialize();
    void arrived(Node* node, const cv::Point2f& lastPos);
    Node* currentNode() const;
    cv::Point2f currentPos() const;
    bool moveForward();
    int antIndex() const;
    void updatePheromones();
    float totalLength() const;
    void drawPath(cv::Mat& canvas, int iteration);

private:
    QScopedPointer<AntPrivate> m_ptr;
    Q_DECLARE_PRIVATE_D(m_ptr, Ant)
    Q_DISABLE_COPY(Ant)
};

class PathOptimizerPrivate;
class PathOptimizer : public QObject
{
    Q_OBJECT
public:
    explicit PathOptimizer(LaserNode* root, bool containsLayers = true, QObject* parent = nullptr);
    virtual ~PathOptimizer();

    void optimize(int canvasWidth, int canvasHeight);
    bool isContainsLayers() const;

protected:
    float initializeByTopologyLayers(QList<LaserNode*> groups);
    float initializeByGroups(LaserNode* root);

private:
    QScopedPointer<PathOptimizerPrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, PathOptimizer)
    Q_DISABLE_COPY(PathOptimizer)
};

#endif // PATHOPTIMIZER_H