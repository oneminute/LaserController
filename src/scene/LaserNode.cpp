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
