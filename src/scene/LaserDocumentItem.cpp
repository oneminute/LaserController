#include "LaserDocumentItem.h"
#include "util/Utils.h"
#include "algorithm/OptimizeNode.h"

ILaserDocumentItemPrivate::ILaserDocumentItemPrivate(ILaserDocumentItem* ptr, LaserNodeType _type)
    : q_ptr(ptr)
    , type(_type)
    , name(QObject::tr("no name"))
{
    id = utils::createUUID();
    optimizeNode = new OptimizeNode(type, q_ptr);
}

ILaserDocumentItem::ILaserDocumentItem(LaserNodeType type, ILaserDocumentItemPrivate* ptr)
    : d_ptr(ptr)
{
}

ILaserDocumentItem::~ILaserDocumentItem()
{
}

QString ILaserDocumentItem::name() const
{
    Q_D(const ILaserDocumentItem);
    return d->name;
}

void ILaserDocumentItem::setName(const QString& name)
{
    Q_D(ILaserDocumentItem);
    d->name = name;
}

LaserNodeType ILaserDocumentItem::type() const
{
    Q_D(const ILaserDocumentItem);
    return d->type;
}

QString ILaserDocumentItem::id() const
{
    Q_D(const ILaserDocumentItem);
    return d->id;
}

OptimizeNode* ILaserDocumentItem::optimizeNode() const
{
    Q_D(const ILaserDocumentItem);
    return d->optimizeNode;
}

void ILaserDocumentItem::setId(const QString& id)
{
    Q_D(ILaserDocumentItem);
    d->id = id;
}
