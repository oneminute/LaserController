#include "OptimizeNode.h"

#include "OptimizeEdge.h"
#include "scene/LaserPrimitive.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "util/TypeUtils.h"
#include <QRandomGenerator>
#include <QStack>

#include <flann/flann.hpp>

class OptimizeNodePrivate
{
    Q_DECLARE_PUBLIC(OptimizeNode)
public:
    OptimizeNodePrivate(OptimizeNode* ptr)
        : q_ptr(ptr)
        , nodeType(LNT_VIRTUAL)
        , outEdge(nullptr)
        , kdTree(nullptr)
        , currentPoint(0, 0)
        , isClosed(false)
    {}

    ~OptimizeNodePrivate();

    void update();

    bool isVirtual() const;

    OptimizeNode* q_ptr;

    OptimizeNode* parentNode;
    QList<OptimizeNode*> childNodes;
    LaserNodeType nodeType;
    QString nodeName;
    ILaserDocumentItem* documentItem;

    QList<OptimizeEdge*> edges;
    OptimizeEdge* outEdge;
    QPointF currentPoint;
    QVector<QPointF> startingPoints;
    flann::KDTreeSingleIndex<flann::L2_Simple<qreal>>* kdTree;
    //flann::LinearIndex<flann::L2_Simple<float>>* kdTree;
    bool isClosed;
    QString name;

    cv::Mat canvas;
};

OptimizeNodePrivate::~OptimizeNodePrivate()
{
    if (kdTree)
        delete kdTree;
    qLogD << "Node " << name << " destroyed.";
}

void OptimizeNodePrivate::update()
{
    Q_Q(OptimizeNode);

    if (nodeType == LNT_PRIMITIVE)
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(documentItem);
        name = primitive->name();
        primitive->updateMachiningPoints(canvas);
        isClosed = primitive->isClosed();
        startingPoints = primitive->startingPoints();
        flann::Matrix<qreal> samplePoints = flann::Matrix<qreal>((qreal*)(startingPoints.data()),
            primitive->startingIndices().size(), 2);

        flann::KDTreeSingleIndexParams singleIndexParams = flann::KDTreeSingleIndexParams(4);
        kdTree = new flann::KDTreeSingleIndex<flann::L2_Simple<qreal>>(samplePoints, singleIndexParams);

        //flann::LinearIndexParams linearIndexParams = flann::LinearIndexParams();
        //kdTree = new flann::LinearIndex<flann::L2_Simple<float>>(samplePoints, linearIndexParams);

        kdTree->buildIndex();
        currentPoint = primitive->firstStartingPoint();
    }
}

bool OptimizeNodePrivate::isVirtual() const
{
    return nodeType == LNT_VIRTUAL;
}

OptimizeNode::OptimizeNode(LaserNodeType nodeType, ILaserDocumentItem* item)
    : d_ptr(new OptimizeNodePrivate(this))
{
    Q_D(OptimizeNode);
    d->nodeType = nodeType;
    d->documentItem = item;
}

OptimizeNode::~OptimizeNode()
{
}

QList<OptimizeNode*>& OptimizeNode::childNodes()
{
    Q_D(OptimizeNode);
    return d->childNodes;
}

LaserNodeType OptimizeNode::nodeType() const
{
    Q_D(const OptimizeNode);
    return d->nodeType;
}

QString OptimizeNode::nodeName() const
{
    Q_D(const OptimizeNode);
    QString nodeName;
    switch (d->nodeType)
    {
    case LNT_DOCUMENT:
    case LNT_LAYER:
    case LNT_PRIMITIVE:
        nodeName = d->documentItem->name();
        break;
    case LNT_VIRTUAL:
        nodeName = d->nodeName;
        break;
    }
    return nodeName;
}

void OptimizeNode::setNodeName(const QString& name)
{
    Q_D(OptimizeNode);
    d->nodeName = name;
}

void OptimizeNode::addChildNode(OptimizeNode* node)
{
    Q_D(OptimizeNode);
    if (!d->childNodes.contains(node))
    {
        d->childNodes.append(node);
        node->setParentNode(this);
    }
}

void OptimizeNode::addChildNodes(const QList<OptimizeNode*>& nodes)
{
    Q_D(OptimizeNode);
    for (OptimizeNode* node : nodes)
    {
        addChildNode(node);
    }
}

void OptimizeNode::removeChildNode(OptimizeNode* node)
{
    Q_D(OptimizeNode);
    d->childNodes.removeOne(node);
}

void OptimizeNode::clearChildren()
{
    Q_D(OptimizeNode);
    d->childNodes.clear();
}

bool OptimizeNode::hasChildren() const
{
    Q_D(const OptimizeNode);
    return !d->childNodes.isEmpty();
}

int OptimizeNode::childCount() const
{
    Q_D(const OptimizeNode);
    return d->childNodes.count();
}

//QPointF OptimizeNode::position() const
//{
//    Q_D(const OptimizeNode);
//    return d->position;
//}
//
//void OptimizeNode::setPosition(QPointF& value)
//{
//    Q_D(OptimizeNode);
//    d->position = value;
//}

bool OptimizeNode::isVirtual() const
{
    Q_D(const OptimizeNode);
    return d->nodeType == LaserNodeType::LNT_VIRTUAL;
}

bool OptimizeNode::isDocument() const
{
    Q_D(const OptimizeNode);
    return d->nodeType == LaserNodeType::LNT_DOCUMENT;
}

bool OptimizeNode::isLayer() const
{
    Q_D(const OptimizeNode);
    return d->nodeType == LaserNodeType::LNT_LAYER;
}

bool OptimizeNode::isPrimitive() const
{
    Q_D(const OptimizeNode);
    return d->nodeType == LaserNodeType::LNT_PRIMITIVE;
}

OptimizeNode* OptimizeNode::parentNode() const
{
    Q_D(const OptimizeNode);
    return d->parentNode;
}

void OptimizeNode::setParentNode(OptimizeNode* parent)
{
    Q_D(OptimizeNode);
    d->parentNode = parent;
}

QList<OptimizeNode*> OptimizeNode::findAllLeaves(OptimizeNode* exclude)
{
    QList<OptimizeNode*> leaves;
    QStack<OptimizeNode*> stack;

    stack.push(this);
    while (!stack.isEmpty())
    {
        OptimizeNode* laserNode = stack.pop();

        if (!laserNode->hasChildren())
        {
            leaves.append(laserNode);
            continue;
        }

        for (OptimizeNode* childNode : laserNode->childNodes())
        {
            if (childNode == exclude)
                continue;

            stack.push(childNode);
        }
    }
    return leaves;
}

QPointF OptimizeNode::position() const
{
    Q_D(const OptimizeNode);
    switch (d->nodeType)
    {
    case LNT_DOCUMENT:
    {
        LaserDocument* doc = static_cast<LaserDocument*>(d->documentItem);
        return doc->docOrigin();
    }
    case LNT_LAYER:
    {
        LaserLayer* layer = static_cast<LaserLayer*>(d->documentItem);
        return layer->position();
    }
    case LNT_PRIMITIVE:
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(d->documentItem);
        return primitive->position();
    }
    case LNT_VIRTUAL:
    {
        if (hasChildren())
            return d->childNodes.first()->position();
    }
    }
    
    return QPointF(0, 0);
}

void OptimizeNode::update()
{
    Q_D(OptimizeNode);
    d->update();
}

ILaserDocumentItem* OptimizeNode::documentItem() const
{
    Q_D(const OptimizeNode);
    return d->documentItem;
}

void OptimizeNode::addEdge(OptimizeEdge* edge)
{
    Q_D(OptimizeNode);
    d->edges.append(edge);
}

void OptimizeNode::setOutEdge(OptimizeEdge* edge)
{
    Q_D(OptimizeNode);
    d->outEdge = edge;
}

QList<OptimizeEdge*> OptimizeNode::edges() const
{
    Q_D(const OptimizeNode);
    return d->edges;
}

OptimizeEdge* OptimizeNode::outEdge() const
{
    Q_D(const OptimizeNode);
    return d->outEdge;
}

QPointF OptimizeNode::startPos() const
{
    Q_D(const OptimizeNode);
    if (d->startingPoints.isEmpty())
    {
        return d->currentPoint;
    }
    return d->startingPoints.first();
}

QPointF OptimizeNode::nearestPoint(const QPointF& point, int& index, float& dist)
{
    Q_D(OptimizeNode);

    if (d->isVirtual())
    {
        index = 0;
        dist = 0;
        d->currentPoint = point;
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
        qreal* dists = new qreal[1];
        QPointF target(0, 0);
        {
            flann::Matrix<int> indicesMatrix(indices, 1, 1);
            flann::Matrix<qreal> distsMatrix(dists, 1, 1);
            qreal data[2];
            data[0] = point.x();
            data[1] = point.y();
            flann::Matrix<qreal> queryPoint = flann::Matrix<qreal>(data, 1, 2);
            d->kdTree->knnSearch(queryPoint, indicesMatrix, distsMatrix, 1, searchParams);

            index = indicesMatrix.ptr()[0];
            if (index < 0 || index >= d->startingPoints.size())
            {
                index = 0;
            }
            target = d->startingPoints[index];
            dist = QVector2D(target - point).length();
            //qLogD << "flann dist: " << dists[0] << ", " << qSqrt(dists[0]) << ", " << distsMatrix.ptr()[0] << ", dist: " << dist;
        }
        //qLogD << d->name << " " << index << " " << dist << " " << dist << " " << target.x << ", " << target.y;
        //qLogD;
        delete[] indices;
        delete[] dists;
        return target;
    }
}

QPointF OptimizeNode::currentPos() const
{
    Q_D(const OptimizeNode);
    return d->currentPoint;
}

QVector<QPointF> OptimizeNode::startingPoints() const
{
    Q_D(const OptimizeNode);
    return d->startingPoints;
}

bool OptimizeNode::isClosed() const
{
    Q_D(const OptimizeNode);
    return d->isClosed;
}

QPointF OptimizeNode::headPoint() const
{
    Q_D(const OptimizeNode);
    if (d->isVirtual())
    {
        return d->currentPoint;
    }
    return d->startingPoints.first();
}

QPointF OptimizeNode::tailPoint() const
{
    Q_D(const OptimizeNode);
    if (d->isVirtual())
    {
        return d->currentPoint;
    }
    return d->startingPoints.last();
}

QPointF OptimizeNode::point(int index) const
{
    Q_D(const OptimizeNode);
    if (d->isVirtual())
    {
        return d->currentPoint;
    }
    return d->startingPoints[index];
}

void OptimizeNode::debugDraw(cv::Mat& canvas)
{
    Q_D(OptimizeNode);
    static OptimizeNode* parentNode = nullptr;
    static QColor color;

    if (d->nodeType == LNT_PRIMITIVE)
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(d->documentItem);
        if (d->parentNode != parentNode)
        {
            parentNode = d->parentNode;
            color.setRed(QRandomGenerator::global()->bounded(192));
            color.setGreen(QRandomGenerator::global()->bounded(192));
            color.setBlue(QRandomGenerator::global()->bounded(192));
        }
        //qLogD << color;
        int i = 0;
        QPointF lastPoint;
        for (const QPointF& pt : primitive->machiningPoints())
        {
            if (i != 0)
            {
                cv::line(canvas, typeUtils::qtPointF2CVPoint2f(lastPoint), 
                    typeUtils::qtPointF2CVPoint2f(pt), cv::Scalar(color.red(), color.green(), color.blue()), 3);
            }

            lastPoint = pt;
            i++;
        }

        for (const QPointF& pt : primitive->startingPoints())
        {
            qreal lineLength = 20;
            cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt + QPointF(-lineLength, -lineLength)), typeUtils::qtPointF2CVPoint2f(pt + QPointF(lineLength, lineLength)), cv::Scalar(0, 0, 255));
            cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt + QPointF(-lineLength, lineLength)), typeUtils::qtPointF2CVPoint2f(pt + QPointF(lineLength, -lineLength)), cv::Scalar(0, 0, 255));
        }
    }
}
