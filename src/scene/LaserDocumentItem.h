#ifndef LASERDOCUMENTITEM_H
#define LASERDOCUMENTITEM_H

#include <QObject>
#include "common/common.h"

class OptimizeNode;
class ILaserDocumentItem;

class ILaserDocumentItemPrivate
{
    Q_DECLARE_PUBLIC(ILaserDocumentItem)
public:
    ILaserDocumentItemPrivate(ILaserDocumentItem* ptr, LaserNodeType _type);

    ILaserDocumentItem* q_ptr;
    LaserNodeType type;
    QString name;
    QString id;
    OptimizeNode* optimizeNode;
};

class ILaserDocumentItem
{
public:
    explicit ILaserDocumentItem(LaserNodeType type, ILaserDocumentItemPrivate* ptr);
    ~ILaserDocumentItem();

    QString name() const;
    void setName(const QString& name);

    LaserNodeType type() const;

    QString id() const;

    OptimizeNode* optimizeNode() const;

protected:
    void setId(const QString& id);
    QScopedPointer<ILaserDocumentItemPrivate> d_ptr;

    Q_DECLARE_PRIVATE(ILaserDocumentItem)
    Q_DISABLE_COPY(ILaserDocumentItem) 

    friend class LaserPrimitive;
};



#endif // LASERDOCUMENTITEM_H
