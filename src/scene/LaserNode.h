#ifndef LASERNODE_H
#define LASERNODE_H

#include <QObject>
#include <QScopedPointer>

enum LaserNodeType
{
    LNT_UNKNOWN,
    LNT_DOCUMENT,
    LNT_LAYER,
    LNT_PRIMITIVE
};

class LaserNodePrivate;

class LaserNode
{
public:
    LaserNode(LaserNodePrivate* dPtr, LaserNodeType nodeType);
    virtual ~LaserNode();

    QList<LaserNode*>& childNodes();
    LaserNodeType nodeType() const;

    QString nodeName() const;
    void setNodeName(const QString& name);

    void addChildNode(LaserNode* node);
    void removeChildNode(LaserNode* node);
    void clearChildren();
    bool hasChildren() const;

protected:
    QScopedPointer<LaserNodePrivate> d_ptr;

    Q_DECLARE_PRIVATE(LaserNode)
    Q_DISABLE_COPY(LaserNode)
};

class LaserNodePrivate
{
    Q_DECLARE_PUBLIC(LaserNode)
public:
    LaserNodePrivate(LaserNode* ptr)
        : q_ptr(ptr)
        , nodeType(LNT_UNKNOWN)
    {}

    QList<LaserNode*> childNodes;
    LaserNodeType nodeType;
    QString nodeName;
    LaserNode* q_ptr;
};

#endif //LASERNODE_H