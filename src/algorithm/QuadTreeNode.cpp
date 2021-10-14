#include "QuadTreeNode.h"



QuadTreeNode::QuadTreeNode(QRectF region, int depth)
{
    m_region = region;
    m_depth = depth;
    //创建子节点
    if (depth > 0) {
        qreal widthHalf = region.width() * 0.5;
        qreal heightHalf = region.height() * 0.5;
        qreal left = region.left();
        qreal top = region.top();
        m_node1 = new QuadTreeNode(QRectF(left, top, widthHalf, heightHalf), m_depth - 1);
        m_node2 = new QuadTreeNode(QRectF(left + widthHalf, top, widthHalf, heightHalf), m_depth - 1);
    }
    
}

QuadTreeNode::~QuadTreeNode()
{
}
void QuadTreeNode::searchCandidateNodes()
{


}
QList<QuadTreeNode*>* QuadTreeNode::search(QRectF selection)
{
    if (m_candidateNodes.isEmpty()) {
        searchCandidateNodes();
    }
    
    return nullptr;
}

bool QuadTreeNode::isLeaf()
{
    if (m_depth > 0) {
        return false;
    }
    else {
        return true;
    }
    
}


