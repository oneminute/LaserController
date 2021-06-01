#ifndef LASERNODE_H
#define LASERNODE_H

#include <QObject>
#include <QScopedPointer>
#include <QVector2D>

enum LaserNodeType
{
    LNT_UNKNOWN,
    LNT_DOCUMENT,
    LNT_LAYER,
    LNT_PRIMITIVE,
    LNT_VIRTUAL,
};

class LaserNodePrivate;

class LaserNode
{
protected:
    LaserNode(LaserNodePrivate* dPtr, LaserNodeType nodeType);
public:
    LaserNode(LaserNodeType nodeType);
    virtual ~LaserNode();

    QList<LaserNode*>& childNodes();
    LaserNodeType nodeType() const;

    QString nodeName() const;
    void setNodeName(const QString& name);

    void addChildNode(LaserNode* node);
    void removeChildNode(LaserNode* node);
    void clearChildren();
    bool hasChildren() const;
    int childCount() const;

    QPointF position() const;
    void setPosition(QPointF& value);

    virtual bool isAvailable() const;
    bool isVirtual() const;
    bool isDocument() const;
    bool isLayer() const;
    bool isPrimitive() const;
    bool isUnknown() const;

    LaserNode* parentNode() const;
    void setParentNode(LaserNode* parent);

    QList<LaserNode*> findAllLeaves(LaserNode* exclude = nullptr);

protected:
    QScopedPointer<LaserNodePrivate> d_ptr;

    Q_DECLARE_PRIVATE(LaserNode)
    Q_DISABLE_COPY(LaserNode)
};

#endif //LASERNODE_H