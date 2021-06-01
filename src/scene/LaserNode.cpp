#include "LaserNode.h"

#include <QStack>

#include "LaserNodePrivate.h"

LaserNode::LaserNode(LaserNodePrivate* dPtr, LaserNodeType nodeType)
    : d_ptr(dPtr)
{
    Q_D(LaserNode);
    d->nodeType = nodeType;
}

LaserNode::LaserNode(LaserNodeType nodeType)
    : d_ptr(new LaserNodePrivate(this))
{
    Q_D(LaserNode);
    d->nodeType = nodeType;
}

LaserNode::~LaserNode()
{
}

QList<LaserNode*>& LaserNode::childNodes()
{
    Q_D(LaserNode);
    return d->childNodes;
}

LaserNodeType LaserNode::nodeType() const
{
    Q_D(const LaserNode);
    return d->nodeType;
}

QString LaserNode::nodeName() const
{
    Q_D(const LaserNode);
    return d->nodeName;
}

void LaserNode::setNodeName(const QString& name)
{
    Q_D(LaserNode);
    d->nodeName = name;
}

void LaserNode::addChildNode(LaserNode* node)
{
    Q_D(LaserNode);
    if (!d->childNodes.contains(node))
    {
        d->childNodes.append(node);
        node->setParentNode(this);
    }
}

void LaserNode::removeChildNode(LaserNode* node)
{
    Q_D(LaserNode);
    d->childNodes.removeOne(node);
}

void LaserNode::clearChildren()
{
    Q_D(LaserNode);
    d->childNodes.clear();
}

bool LaserNode::hasChildren() const
{
    Q_D(const LaserNode);
    return !d->childNodes.isEmpty();
}

int LaserNode::childCount() const
{
    Q_D(const LaserNode);
    return d->childNodes.count();
}

QPointF LaserNode::position() const
{
    Q_D(const LaserNode);
    return d->position;
}

void LaserNode::setPosition(QPointF& value)
{
    Q_D(LaserNode);
    d->position = value;
}

bool LaserNode::isAvailable() const
{
    return true;
}

bool LaserNode::isVirtual() const
{
    Q_D(const LaserNode);
    return d->nodeType == LaserNodeType::LNT_VIRTUAL;
}

bool LaserNode::isDocument() const
{
    Q_D(const LaserNode);
    return d->nodeType == LaserNodeType::LNT_DOCUMENT;
}

bool LaserNode::isLayer() const
{
    Q_D(const LaserNode);
    return d->nodeType == LaserNodeType::LNT_LAYER;
}

bool LaserNode::isPrimitive() const
{
    Q_D(const LaserNode);
    return d->nodeType == LaserNodeType::LNT_PRIMITIVE;
}

bool LaserNode::isUnknown() const
{
    Q_D(const LaserNode);
    return d->nodeType == LaserNodeType::LNT_UNKNOWN;
}

LaserNode* LaserNode::parentNode() const
{
    Q_D(const LaserNode);
    return d->parentNode;
}

void LaserNode::setParentNode(LaserNode* parent)
{
    Q_D(LaserNode);
    d->parentNode = parent;
}

QList<LaserNode*> LaserNode::findAllLeaves(LaserNode* exclude)
{
    QList<LaserNode*> leaves;
    QStack<LaserNode*> stack;

    stack.push(this);
    while (!stack.isEmpty())
    {
        LaserNode* laserNode = stack.pop();

        if (!laserNode->hasChildren())
        {
            leaves.append(laserNode);
            continue;
        }

        for (LaserNode* childNode : laserNode->childNodes())
        {
            if (childNode == exclude)
                continue;

            stack.push(childNode);
        }
    }
    return leaves;
}
