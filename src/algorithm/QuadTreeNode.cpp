#include "QuadTreeNode.h"
#include <QList>
#include <QRectF>
#include <QStack>

QuadTreeNode::QuadTreeNode(QRectF region, int depth)
    :m_nodeTopLeft(nullptr)
    , m_nodeTopRight(nullptr)
    , m_nodeBottomLeft(nullptr)
    , m_nodeBottomRight(nullptr)
{
    m_region = region;
    m_depth = depth;
    //子节点区域
    qreal widthHalf = region.width() * 0.5;
    qreal heightHalf = region.height() * 0.5;
    qreal left = region.left();
    qreal top = region.top();

    m_region = QRectF(left, top, region.width(), region.height());
    m_topLeftRegion = QRectF(left, top, widthHalf, heightHalf);
    m_topRightRegion = QRectF(left + widthHalf, top, widthHalf, heightHalf);
    m_bottomLeftRegion = QRectF(left, top + heightHalf, widthHalf, heightHalf);
    m_bottomRightRegion = QRectF(left + widthHalf, top + heightHalf, widthHalf, heightHalf);
    //m_node1 = new QuadTreeNode(QRectF(left, top, widthHalf, heightHalf), m_depth);
    //m_node2 = new QuadTreeNode(QRectF(left + widthHalf, top, widthHalf, heightHalf), m_depth);
}

QuadTreeNode::~QuadTreeNode()
{
}
bool QuadTreeNode::createChildrenNodes(LaserPrimitive* primitive)
{
    if (m_depth > m_maxDepth) {
        addPrimitive(primitive);
        return false;
    }
    createPrimitiveTreeNode(primitive);
    return true;
}
void QuadTreeNode::createPrimitiveTreeNode(LaserPrimitive* primitive)
{
    QRectF bound = primitive->sceneBoundingRect();   
    //区域包含图元
    if (m_topLeftRegion.contains(bound)) {
        createNode(Qt::TopLeftCorner);
        //m_nodeTopLeft->addPrimitive(primitive);
        if (!m_nodeTopLeft->createChildrenNodes(primitive)) {
            return;
        }

    }
    else if (m_topRightRegion.contains(bound)) {
        createNode(Qt::TopRightCorner);
        //m_nodeTopRight->addPrimitive(primitive);
        if (!m_nodeTopRight->createChildrenNodes(primitive)) {
            return;
        }
    }
    else if (m_bottomRightRegion.contains(bound)) {
        createNode(Qt::BottomRightCorner);
        //m_nodeBottomRight->addPrimitive(primitive);
        if (!m_nodeBottomRight->createChildrenNodes(primitive)) {
            return;
        }
    }
    else if (m_bottomLeftRegion.contains(bound)) {
        createNode(Qt::BottomLeftCorner);
        //m_nodeBottomLeft->addPrimitive(primitive);
        if (!m_nodeBottomLeft->createChildrenNodes(primitive)) {
            return;
        }
    }
    else {
        if (bound.contains(m_topLeftRegion)) {
            //图元包含节点区域
            createNode(Qt::TopLeftCorner);
            m_nodeTopLeft->addPrimitive(primitive);
        }
        else if (m_topLeftRegion.intersects(bound)) {
            //区域与图元相交
            createNode(Qt::TopLeftCorner);
            //m_nodeTopLeft->addPrimitive(primitive);
            if (!m_nodeTopLeft->createChildrenNodes(primitive)) {
                return;
            }
        }
        
        if (bound.contains(m_topRightRegion)) {
            //图元包含节点区域
            createNode(Qt::TopRightCorner);
            m_nodeTopRight->addPrimitive(primitive);
        }else if (m_topRightRegion.intersects(bound)) {
            //区域与图元相交
            createNode(Qt::TopRightCorner);
            //m_nodeTopRight->addPrimitive(primitive);
            if (!m_nodeTopRight->createChildrenNodes(primitive)) {
                return;
            }
        }
        
        if (bound.contains(m_bottomRightRegion)) {
            //图元包含节点区域
            createNode(Qt::BottomRightCorner);
            m_nodeBottomRight->addPrimitive(primitive);
        }else if (m_bottomRightRegion.intersects(bound)) {
            //区域与图元相交
            createNode(Qt::BottomRightCorner);
            //m_nodeBottomRight->addPrimitive(primitive);
            if (!m_nodeBottomRight->createChildrenNodes(primitive)) {
                return;
            }
        }
        
        if (bound.contains(m_bottomLeftRegion)) {
            //图元包含节点区域
            createNode(Qt::BottomLeftCorner);
            m_nodeBottomLeft->addPrimitive(primitive);
        }else if (m_bottomLeftRegion.intersects(bound)) {
            //区域与图元相交
            createNode(Qt::BottomLeftCorner);
            //m_nodeBottomLeft->addPrimitive(primitive);
            if (!m_nodeBottomLeft->createChildrenNodes(primitive)) {
                return;
            }
        }       
    }
}
void QuadTreeNode::createNode(int type)
{
    switch (type) {
        case Qt::TopLeftCorner: {
            if (!m_nodeTopLeft) {
                m_nodeTopLeft = new QuadTreeNode(m_topLeftRegion, m_depth + 1);
            }
            break;
        }
        case Qt::TopRightCorner: {
            if (!m_nodeTopRight) {
                m_nodeTopRight = new QuadTreeNode(m_topRightRegion, m_depth + 1);
            }
            break;
        }
        case Qt::BottomLeftCorner: {
            if (!m_nodeBottomLeft) {
                m_nodeBottomLeft = new QuadTreeNode(m_bottomLeftRegion, m_depth + 1);
            }
            break;
        }
        case Qt::BottomRightCorner: {
            if (!m_nodeBottomRight) {
                m_nodeBottomRight = new QuadTreeNode(m_bottomRightRegion, m_depth + 1);
            }
            break;
        }
    }
}
void QuadTreeNode::searchCandidateNodes(QRectF selection)
{
    QStack<QuadTreeNode*> stack;
    stack.push(this);
    while (!stack.isEmpty()) {
        QuadTreeNode* node = stack.pop();
        if (!node) {
            continue;
        }
        if (selection.contains(node->region())) {
            m_targetNodes.append(node);
        }
        else if (node->region().contains(selection)) {
            if (node->isLeaf()) {
                m_targetNodes.append(node);
            }
            else {
                //查找子节点
                for (QuadTreeNode* child : node->children()) {
                    if (child) {
                        stack.push(child);
                    }
                    
                }
            }
            
        }
        else if (node->region().intersects(selection)) {
            if (node->isLeaf()) {
                m_targetNodes.append(node);
            }
            else {
                //查找子节点
                for (QuadTreeNode* child : node->children()) {
                    if (child) {
                        stack.push(child);
                    }

                }
            }
        }
    }
    
    //m_candidateNodes;
    
}
QList<QuadTreeNode*>& QuadTreeNode::search(QRectF selection)
{
    /*if (m_candidateNodes.isEmpty()) {
        searchCandidateNodes(selection);
    }*/
    searchCandidateNodes(selection);
    return m_targetNodes;
}

bool QuadTreeNode::isLeaf()
{
    if (m_nodeTopLeft || m_nodeTopRight || m_nodeBottomLeft || m_nodeBottomRight) {
        return false;
    }
    else {
        return true;
    }
}

void QuadTreeNode::setPrimitiveList(QList<LaserPrimitive*> list)
{
    m_primitiveList = list;
}

QList<LaserPrimitive*>& QuadTreeNode::primitiveList()
{
    return m_primitiveList;
}

void QuadTreeNode::addPrimitive(LaserPrimitive * primitive)
{
    if (!primitive) {
        return;
    }
    for (LaserPrimitive* p : m_primitiveList) {
        if (p == primitive) {
            return;
        }
    }
    m_primitiveList.append(primitive);
    primitive->addTreeNode(this);
}

void QuadTreeNode::removePrimitive(LaserPrimitive * primitive)
{
    if (primitive && !m_primitiveList.isEmpty()) {
        if (m_primitiveList.removeOne(primitive)) {
            qLogW << "the remove primitive not exit in the TreeNode";
        }
    }
}

QRectF QuadTreeNode::region()
{
    return m_region;
}

QRectF QuadTreeNode::topLeftRegion()
{
    return m_topLeftRegion;
}

QRectF QuadTreeNode::topRightRegion()
{
    return m_topRightRegion;
}

QRectF QuadTreeNode::bottomLeftRegion()
{
    return m_bottomLeftRegion;
}

QRectF QuadTreeNode::bottomRightRegion()
{
    return m_bottomRightRegion;
}

QList<QuadTreeNode*> QuadTreeNode::children()
{
    QList<QuadTreeNode*> list;
    if (m_nodeTopLeft) {
        list.append(m_nodeTopLeft);
    }
    if (m_nodeTopRight) {
        list.append(m_nodeTopRight);
    }
    if (m_nodeBottomLeft) {
        list.append(m_nodeBottomLeft);
    }
    if (m_nodeBottomRight) {
        list.append(m_nodeBottomRight);
    }
    return list;
}

QList<QRectF> QuadTreeNode::targetRegions()
{
    QList<QRectF> list;
    for (QuadTreeNode* node : m_targetNodes) {
        if (node) {
            list.append(node->region());
        }
    }
    return list;
}



