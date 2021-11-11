#include "OptimizeEdge.h"
#include "LaserApplication.h"
#include "common/Config.h"
#include <laser/LaserDevice.h>
#include "scene/LaserPrimitive.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "util/TypeUtils.h"
#include <QRandomGenerator>
#include <QStack>

#include "OptimizeNode.h"

class OptimizeNodePrivate
{
    Q_DECLARE_PUBLIC(OptimizeNode)
public:
    OptimizeNodePrivate(OptimizeNode* ptr)
        : q_ptr(ptr)
        , parentNode(nullptr)
        , nodeType(LNT_VIRTUAL)
        , outEdge(nullptr)
        , currentPoint(0, 0, 0, 0)
        , index(0)
        , isClosed(false)
    {}

    ~OptimizeNodePrivate();

    void update(ProgressItem* parentProgress);

    //bool isVirtual() const;

    OptimizeNode* q_ptr;

    OptimizeNode* parentNode;
    QList<OptimizeNode*> childNodes;
    LaserNodeType nodeType;
    QString nodeName;
    ILaserDocumentItem* documentItem;

    QList<OptimizeEdge*> edges;
    OptimizeEdge* outEdge;
    LaserPoint currentPoint;

    LaserPointList startingPoints;
    //LaserPointList leavesPoints;
    //LaserPointList outerPoints;
    LaserPoint lastPoint;

    int index;

    bool isClosed;
    QString name;
};

OptimizeNodePrivate::~OptimizeNodePrivate()
{
    qLogD << "Node " << name << " destroyed.";
}

void OptimizeNodePrivate::update(ProgressItem* parentProgress)
{
    Q_Q(OptimizeNode);

    if (nodeType == LNT_PRIMITIVE)
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(documentItem);
        name = primitive->name();
        primitive->updateMachiningPoints(parentProgress);
        startingPoints = primitive->startingPoints();
        if (startingPoints.isEmpty())
            return;
        startingPoints.buildKdtree();
        currentPoint = primitive->firstStartingPoint();
    }
    else if (nodeType == LNT_DOCUMENT)
    {
        QPointF deviceOriginMM = LaserApplication::device->deviceOriginMachining();
        LaserDocument* document = static_cast<LaserDocument*>(documentItem);
        QPointF docOrigin = document->docOriginMachining();
        QLineF line(deviceOriginMM, docOrigin);
        qreal angle = 0;
        if (line.isNull())
        {
            switch (Config::SystemRegister::deviceOrigin())
            {
            case 0:
                angle = 315;
                break;
            case 1:
                angle = 225;
                break;
            case 2:
                angle = 135;
                break;
            case 4:
                angle = 45;
                break;
            }
        }
        else
        {
            angle = line.angle();
        }
        currentPoint = LaserPoint(docOrigin, angle, angle);
    }
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

void OptimizeNode::clearEdges()
{
    Q_D(OptimizeNode);
    if (!d->edges.isEmpty())
    {
        qDeleteAll(d->edges);
        d->edges.clear();
    }
}

bool OptimizeNode::hasChildren() const
{
    Q_D(const OptimizeNode);
    return !d->childNodes.isEmpty();
}

bool OptimizeNode::isLeaf() const
{
    return !hasChildren();
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

//bool OptimizeNode::isVirtual() const
//{
    //Q_D(const OptimizeNode);
    //return d->nodeType == LaserNodeType::LNT_VIRTUAL;
//}

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

bool OptimizeNode::exportable() const
{
    if (isDocument())
        return true;
    else if (isLayer())
        return layer()->exportable();
    else if (isPrimitive())
        return primitive()->exportable();
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

QSet<OptimizeNode*> OptimizeNode::findAllLeaves(const QSet<OptimizeNode*>& excludes)
{
    QSet<OptimizeNode*> leaves;
    QStack<OptimizeNode*> stack;

    stack.push(this);
    while (!stack.isEmpty())
    {
        OptimizeNode* laserNode = stack.pop();

        if (laserNode->isLeaf() && !excludes.contains(laserNode))
        {
            leaves.insert(laserNode);
            continue;
        }

        for (OptimizeNode* childNode : laserNode->childNodes())
        {
            stack.push(childNode);
        }
    }
    return leaves;
}

QSet<OptimizeNode*> OptimizeNode::findLeaves(const QSet<OptimizeNode*>& excludes)
{
    QSet<OptimizeNode*> leaves;
    for (OptimizeNode* childNode : childNodes())
    {
        if (excludes.contains(childNode))
            continue;

        if (childNode->isLeaf())
            leaves.insert(childNode);
    }
    return leaves;
}

QSet<OptimizeNode*> OptimizeNode::findSiblings(bool onlyLeaves)
{
    QSet<OptimizeNode*> siblings;

    OptimizeNode* parent = parentNode();
    if (!parent)
        return siblings;

    for (OptimizeNode* node : parent->childNodes())
    {
        if (node == this)
            continue;

        if (onlyLeaves)
        {
            if (!node->hasChildren())
            {
                siblings.insert(node);
            }
        }
        else
        {
            siblings.insert(node);
        }
    }
    return siblings;
}

void OptimizeNode::findSiblings(QSet<OptimizeNode*>& leaves, QSet<OptimizeNode*>& branches,
        const QSet<OptimizeNode*>& excludes)
{
    leaves.clear();
    branches.clear();

    OptimizeNode* parent = parentNode();
    if (!parent)
        return;

    for (OptimizeNode* node : parent->childNodes())
    {
        if (node == this)
            continue;

        if (excludes.contains(node))
            continue;

        if (node->hasChildren())
        {
            branches.insert(node);
        }
        else
        {
            leaves.insert(node);
        }
    }
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

QPointF OptimizeNode::machiningPosition() const
{
    Q_D(const OptimizeNode);
    switch (d->nodeType)
    {
    case LNT_DOCUMENT:
    {
        LaserDocument* doc = static_cast<LaserDocument*>(d->documentItem);
        return doc->docOriginMachining();
    }
    case LNT_LAYER:
    {
        LaserLayer* layer = static_cast<LaserLayer*>(d->documentItem);
        return layer->positionMachining();
    }
    case LNT_PRIMITIVE:
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(d->documentItem);
        return Global::matrixToMachining().map(primitive->position());
    }
    case LNT_VIRTUAL:
    {
        if (hasChildren())
            return d->childNodes.first()->machiningPosition();
    }
    }
    
    return QPointF(0, 0);
}

void OptimizeNode::update(ProgressItem* parentProgress)
{
    Q_D(OptimizeNode);
    d->update(parentProgress);
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

LaserPoint OptimizeNode::startPos() const
{
    Q_D(const OptimizeNode);
    if (d->startingPoints.isEmpty())
    {
        return d->currentPoint;
    }
    return d->startingPoints.first();
}

LaserPoint OptimizeNode::nearestPoint(const LaserPoint& point)
{
    Q_D(OptimizeNode);
    d->index = d->startingPoints.nearestSearch(point);
    d->currentPoint = d->startingPoints[d->index];
    d->lastPoint = point;
    return d->currentPoint;
}

LaserPoint OptimizeNode::nearestPoint(OptimizeNode* node)
{
    return nearestPoint(node->currentPos());
}

LaserPoint OptimizeNode::currentPos(const LaserPoint& hint) const
{
    Q_D(const OptimizeNode);
    if (d->nodeType == LNT_LAYER)
        return hint;
    return d->currentPoint;
}

void OptimizeNode::setCurrentPos(const LaserPoint& point)
{
    Q_D(OptimizeNode);
    d->currentPoint = point;
}

void OptimizeNode::setCurrentIndex(int index)
{
    Q_D(OptimizeNode);
    d->index = index;
    d->currentPoint = d->startingPoints[index];
}

LaserPoint OptimizeNode::lastPoint() const
{
    Q_D(const OptimizeNode);
    return d->lastPoint;
}

void OptimizeNode::setLastPoint(const LaserPoint& point)
{
    Q_D(OptimizeNode);
    d->lastPoint = point;
}

LaserPointList& OptimizeNode::startingPoints()
{
    Q_D(OptimizeNode);
    return d->startingPoints;
}

bool OptimizeNode::isClosed() const
{
    Q_D(const OptimizeNode);
    return d->isClosed;
}

LaserPoint OptimizeNode::headPoint() const
{
    Q_D(const OptimizeNode);
    return d->startingPoints.first();
}

LaserPoint OptimizeNode::tailPoint() const
{
    Q_D(const OptimizeNode);
    return d->startingPoints.last();
}

LaserPoint OptimizeNode::point(int index) const
{
    Q_D(const OptimizeNode);
    return d->startingPoints[index];
}

bool OptimizeNode::isVirtual() const
{
    switch (nodeType())
    {
    case LNT_DOCUMENT:
    case LNT_LAYER:
    case LNT_VIRTUAL:
        return true;
    case LNT_PRIMITIVE:
        return false;
    }
}

LaserPrimitive* OptimizeNode::primitive() const
{
    Q_D(const OptimizeNode);
    if (d->nodeType == LNT_PRIMITIVE)
        return static_cast<LaserPrimitive*>(d->documentItem);
    return nullptr;
}

LaserLayer* OptimizeNode::layer() const
{
    Q_D(const OptimizeNode);
    if (d->nodeType == LNT_LAYER)
        return static_cast<LaserLayer*>(d->documentItem);
    return nullptr;
}

LaserPointListList OptimizeNode::arrangeMachiningPoints()
{
    Q_D(OptimizeNode);
    if (d->nodeType == LNT_PRIMITIVE)
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(d->documentItem);
        // d->index是startingPoints的index，要换算为machiningPoints的index
        int index = primitive->startingIndices().at(d->index);
        return primitive->arrangeMachiningPoints(d->lastPoint, index);
    }
    return LaserPointListList();
}

LaserPointListList OptimizeNode::arrangedPoints() const
{
    Q_D(const OptimizeNode);
    if (d->nodeType == LNT_PRIMITIVE)
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(d->documentItem);
        return primitive->arrangedPoints();
    }
    return LaserPointListList();
}

LaserPoint OptimizeNode::arrangedStartingPoint() const
{
    Q_D(const OptimizeNode);
    if (d->nodeType == LNT_PRIMITIVE)
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(d->documentItem);
        return primitive->arrangedStartingPoint();
    }
    else if (d->nodeType == LNT_DOCUMENT)
    {
        return d->currentPoint;
    }
    return LaserPoint();
}

LaserPoint OptimizeNode::arrangedEndingPoint() const
{
    Q_D(const OptimizeNode);
    if (d->nodeType == LNT_PRIMITIVE)
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(d->documentItem);
        return primitive->arrangedEndingPoint();
    }
    else if (d->nodeType == LNT_DOCUMENT)
    {
        return d->currentPoint;
    }
    return LaserPoint();
}

QSet<int> OptimizeNode::laneIndices()
{
    Q_D(const OptimizeNode);
    QSet<int> indices;
    for (const LaserPoint& point : startingPoints())
    {
        indices.insert(point.laneIndex());
    }
    return indices;
}
