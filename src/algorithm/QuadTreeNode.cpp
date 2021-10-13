#include "QuadTreeNode.h"

void QuadTreeNode::searchCandidateNodes()
{
    
    
}

QuadTreeNode::QuadTreeNode(int depth)
{
}

QuadTreeNode::~QuadTreeNode()
{
}

QList<QuadTreeNode*>* QuadTreeNode::search(QRectF selection)
{
    if (m_candidateNodes.isEmpty()) {
        searchCandidateNodes();
    }
    
    return nullptr;
}


