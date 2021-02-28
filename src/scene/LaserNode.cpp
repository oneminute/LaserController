#include "LaserNode.h"

LaserNode::LaserNode(LaserNodePrivate* dPtr, LaserNodeType nodeType)
    : d_ptr(dPtr)
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
