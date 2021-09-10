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

    void update();

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

    cv::Mat canvas;
};

OptimizeNodePrivate::~OptimizeNodePrivate()
{
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
        startingPoints = primitive->startingPoints();
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

QList<OptimizeNode*> OptimizeNode::findLeaves()
{
    QList<OptimizeNode*> leaves;
    for (OptimizeNode* childNode : childNodes())
    {
        if (childNode->hasChildren())
            continue;
        leaves.append(childNode);
    }
    return leaves;
}

QList<OptimizeNode*> OptimizeNode::findSiblings(bool onlyLeaves)
{
    QList<OptimizeNode*> siblings;

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
                siblings.append(node);
            }
        }
        else
        {
            siblings.append(node);
        }
    }
    return siblings;
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
        return Global::matrixToMM(SU_PX, 40, 40).map(primitive->position());
    }
    case LNT_VIRTUAL:
    {
        if (hasChildren())
            return d->childNodes.first()->machiningPosition();
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
        LaserPoint lastPoint;
        for (const LaserPoint& pt : primitive->machiningPoints())
        {
            if (i != 0)
            {
                cv::line(canvas, typeUtils::qtPointF2CVPoint2f(lastPoint.toPointF()), 
                    typeUtils::qtPointF2CVPoint2f(pt.toPointF()), cv::Scalar(color.red(), color.green(), color.blue()), 3);
            }

            lastPoint = pt;
            i++;
        }

        for (const LaserPoint& pt : primitive->startingPoints())
        {
            qreal lineLength = 20;
            cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt.toPointF() + QPointF(-lineLength, -lineLength)), 
                typeUtils::qtPointF2CVPoint2f(pt.toPointF() + QPointF(lineLength, lineLength)), cv::Scalar(0, 0, 255));
            cv::line(canvas, typeUtils::qtPointF2CVPoint2f(pt.toPointF() + QPointF(-lineLength, lineLength)), 
                typeUtils::qtPointF2CVPoint2f(pt.toPointF() + QPointF(lineLength, -lineLength)), cv::Scalar(0, 0, 255));
        }
    }
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

LaserPointList OptimizeNode::arrangeMachiningPoints(cv::Mat& canvas)
{
    Q_D(OptimizeNode);
    if (d->nodeType == LNT_PRIMITIVE)
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(d->documentItem);
        return primitive->arrangeMachiningPoints(d->lastPoint, d->index, canvas);
    }
    return LaserPointList();
}

LaserPointList OptimizeNode::arrangedPoints() const
{
    Q_D(const OptimizeNode);
    if (d->nodeType == LNT_PRIMITIVE)
    {
        LaserPrimitive* primitive = static_cast<LaserPrimitive*>(d->documentItem);
        return primitive->arrangedPoints();
    }
    return LaserPointList();
}
