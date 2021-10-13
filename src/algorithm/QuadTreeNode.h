#include <QList>
class QuadTreeNode {
private:
    QuadTreeNode* m_node1;
    QuadTreeNode* m_node2;
    QuadTreeNode* m_node3;
    QuadTreeNode* m_node4;
    bool m_isLeaf;
    QList<QuadTreeNode*> m_targetLeafs;
    QList<QuadTreeNode*> m_candidateNodes;
    int depth;

    void searchCandidateNodes();
public:
    QuadTreeNode(int depth = 1);
    ~QuadTreeNode();
    QList<QuadTreeNode*>* search(QRectF selection);
    
};