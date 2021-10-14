#include <QList>
class QuadTreeNode {
private:
    QRectF m_region;
    QuadTreeNode* m_node1;
    QuadTreeNode* m_node2;
    QuadTreeNode* m_node3;
    QuadTreeNode* m_node4;
    bool m_isLeaf;
    QList<QuadTreeNode*> m_targetLeafs;
    QList<QuadTreeNode*> m_candidateNodes;
    int m_depth;//Ϊ0˵��Ϊ�ӽڵ�

    void searchCandidateNodes();
public:
    QuadTreeNode(QRectF region, int depth = 1);
    ~QuadTreeNode();
    QList<QuadTreeNode*>* search(QRectF selection);
    bool isLeaf();
    
};