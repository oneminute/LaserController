#ifndef QUADTREENODE_H
#define QUADTREENODE_H
#include <QList>
#include "scene/LaserPrimitive.h"
class QuadTreeNode {
private:
    QRectF m_region;
    QList<LaserPrimitive*> m_primitiveList;
    QuadTreeNode* m_nodeTopLeft;
    QuadTreeNode* m_nodeTopRight;
    QuadTreeNode* m_nodeBottomLeft;
    QuadTreeNode* m_nodeBottomRight;
    QRectF m_topLeftRegion;
    QRectF m_topRightRegion;
    QRectF m_bottomLeftRegion;
    QRectF m_bottomRightRegion;
    QList<QuadTreeNode*> m_targetNodes;
    int m_depth;//Ϊ0˵��Ϊ�ӽڵ�
    int m_maxDepth = 7;
public:
    QuadTreeNode(QRectF region, int depth = 0);
    ~QuadTreeNode();
    bool createChildrenNodes(LaserPrimitive* primitive);
    void createPrimitiveTreeNode(LaserPrimitive* primitive);
    void createNode(int type);
    QList<QuadTreeNode*>& search(QRectF selection);
    bool isLeaf();//�Ƿ���Ҷ�ӽڵ�
    void setPrimitiveList(QList<LaserPrimitive*> list);
    QList<LaserPrimitive*> primitiveList();
    void addPrimitive(LaserPrimitive* primitive);
    void removePrimitive(LaserPrimitive* primitive);
    QRectF region();
    QRectF topLeftRegion();
    QRectF topRightRegion();
    QRectF bottomLeftRegion();
    QRectF bottomRightRegion();
    QuadTreeNode* nodeTopLeft();
    QuadTreeNode* nodeTopRight();
    QuadTreeNode* nodeBottomLeft();
    QuadTreeNode* nodeBottomRight();
    QList<QuadTreeNode*> children();
    QList<QRectF> targetRegions();
    void upDatePrimitive(LaserPrimitive* primitive);
    void clearAllNodes();
};
#endif // QUADTREENODE_H