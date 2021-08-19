#ifndef LASERNODEPRIVATE_H
#define LASERNODEPRIVATE_H

#include <QObject>

class LaserNodePrivate
{
    Q_DECLARE_PUBLIC(LaserNode)
public:
    LaserNodePrivate(LaserNode* ptr)
        : q_ptr(ptr)
        , nodeType(LNT_UNKNOWN)
        //, position(0, 0)
    {}

    LaserNode* parentNode;
    QList<LaserNode*> childNodes;
    LaserNodeType nodeType;
    QString nodeName;
    //QPointF position;
    LaserNode* q_ptr;
};

#endif // LASERNODEPRIVATE_H