#pragma once

#include "LaserPrimitive.h"
#include "scene/LaserDocument.h"
#include "scene/LaserDocumentItem.h"

class QuadTreeNode;
class LaserPrimitive;

class LaserPrimitivePrivate: public ILaserDocumentItemPrivate
{
    Q_DECLARE_PUBLIC(LaserPrimitive)
public:
    LaserPrimitivePrivate(LaserPrimitive* ptr)
        : ILaserDocumentItemPrivate(ptr, LNT_PRIMITIVE)
        , doc(nullptr)
        , layer(nullptr)
        , isEditing(false)
        , isHover(false)
        , primitiveType(LPT_UNKNOWN)
        , machiningCenter(0, 0)
        , isLocked(false)
        , exportable(true)
        , visible(true)
        , isJoinedGroup(false)
        , joinedGroupList(nullptr)
        , isAlignTarget(false)
    {}

    LaserDocument* doc;
    LaserLayer* layer;
	int layerIndex;
    QRect boundingRect;
    LaserPrimitiveType primitiveType;
    bool isEditing;
    bool isHover;
    QPainterPath outline;
    LaserPointListList machiningPointsList;
    LaserPointListList arrangedPointsList;
    QPoint machiningCenter;
    QList<int> startingIndices;
	QTransform allTransform;
	//QRect originalBoundingRect;
	QPainterPath path;
    bool isLocked;
    QList<QuadTreeNode*> treeNodes;
    bool exportable;
    bool visible;
    bool isJoinedGroup;
    bool isAlignTarget;
    QSet<LaserPrimitive*>* joinedGroupList;
    //bool stampIntaglio;//和印章相关的属性 阴刻，凹下
    //QPainterPath antiFakePath;
    //QRect variableBounds;//circleText，horizontalText，verticalText中使用，方便改变外包框
};




