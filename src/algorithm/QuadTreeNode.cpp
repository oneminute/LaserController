#include "QuadTreeNode.h"
#include <QList>
#include <QRectF>
#include <QStack>

QuadTreeNode::QuadTreeNode(QRectF region, int depth, bool isLeaf)
{
    m_region = region;
    m_depth = depth;
    m_isLeaf = isLeaf;
    m_nodeTopLeft = nullptr;
    m_nodeTopRight = nullptr;
    m_nodeBottomLeft = nullptr;
    m_nodeBottomRight = nullptr;
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
void QuadTreeNode::createTreeNodes()
{
    if (m_depth > m_maxDepth) {
        return;
    }
    for (LaserPrimitive* primitive : m_primitiveList) {
        setPrimitiveTreeNode(primitive);
    }
}
void QuadTreeNode::setPrimitiveTreeNode(LaserPrimitive* primitive)
{
    QRectF bound = primitive->sceneBoundingRect();
    //图元包含节点区域
    if (bound.contains(m_topLeftRegion)) {
        createNode(Qt::TopLeftCorner, true);
        m_nodeTopLeft->addPrimitive(primitive);

    }
    else if (bound.contains(m_topRightRegion)) {
        createNode(Qt::TopRightCorner, true);
        m_nodeTopRight->addPrimitive(primitive);
    }
    else if (bound.contains(m_bottomRightRegion)) {
        createNode(Qt::BottomRightCorner, true);
        m_nodeBottomRight->addPrimitive(primitive);
    }
    else if (bound.contains(m_bottomLeftRegion)) {
        createNode(Qt::BottomLeftCorner, true);
        m_nodeBottomLeft->addPrimitive(primitive);
    }
    //区域包含图元
    if (m_topLeftRegion.contains(bound)) {
        createNode(Qt::TopLeftCorner, false);
        m_nodeTopLeft->addPrimitive(primitive);
        m_nodeTopLeft->createTreeNodes();
    }
    else if (m_topRightRegion.contains(bound)) {
        createNode(Qt::TopRightCorner, false);
        m_nodeTopRight->addPrimitive(primitive);
        m_nodeTopRight->createTreeNodes();
    }
    else if (m_bottomRightRegion.contains(bound)) {
        createNode(Qt::BottomRightCorner, false);
        m_nodeBottomRight->addPrimitive(primitive);
        m_nodeBottomRight->createTreeNodes();
    }
    else if (m_bottomLeftRegion.contains(bound)) {
        createNode(Qt::BottomLeftCorner, false);
        m_nodeBottomLeft->addPrimitive(primitive);
        m_nodeBottomLeft->createTreeNodes();
    }
    else {
        bool isInterset = false;
        //区域与图元相交
        if (m_topLeftRegion.intersects(bound)) {
            createNode(Qt::TopLeftCorner, false);
            m_nodeTopLeft->addPrimitive(primitive);
            m_nodeTopLeft->createTreeNodes();
            isInterset = true;
        }
        if (m_topRightRegion.intersects(bound)) {
            createNode(Qt::TopRightCorner, false);
            m_nodeTopRight->addPrimitive(primitive);
            m_nodeTopRight->createTreeNodes();
            isInterset = true;
        }
        if (m_bottomRightRegion.intersects(bound)) {
            createNode(Qt::BottomRightCorner, false);
            m_nodeBottomRight->addPrimitive(primitive);
            m_nodeBottomRight->createTreeNodes();
            isInterset = true;
        }
        if (m_bottomLeftRegion.intersects(bound)) {
            createNode(Qt::BottomLeftCorner, false);
            m_nodeBottomLeft->addPrimitive(primitive);
            m_nodeBottomLeft->createTreeNodes();
            isInterset = true;
        }
        //区域与图元没有任何关系
        if (!isInterset) {
            m_isLeaf = true;
        }
    }
}
void QuadTreeNode::createNode(int type, bool isLeaf)
{
    switch (type) {
        case Qt::TopLeftCorner: {
            if (!m_nodeTopLeft) {
                m_nodeTopLeft = new QuadTreeNode(m_topLeftRegion, m_depth + 1, isLeaf);
            }
            break;
        }
        case Qt::TopRightCorner: {
            if (!m_nodeTopRight) {
                m_nodeTopRight = new QuadTreeNode(m_topRightRegion, m_depth + 1, isLeaf);
            }
            break;
        }
        case Qt::BottomLeftCorner: {
            if (!m_nodeBottomLeft) {
                m_nodeBottomLeft = new QuadTreeNode(m_bottomLeftRegion, m_depth + 1, isLeaf);
            }
            break;
        }
        case Qt::BottomRightCorner: {
            if (!m_nodeBottomRight) {
                m_nodeBottomRight = new QuadTreeNode(m_bottomRightRegion, m_depth + 1, isLeaf);
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
    
    m_candidateNodes;
    
}
QList<QuadTreeNode*>* QuadTreeNode::search(QRectF selection)
{
    if (m_candidateNodes.isEmpty()) {
        searchCandidateNodes(selection);
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

void QuadTreeNode::setLeaf(bool isLeaf)
{
    m_isLeaf = isLeaf;
}

void QuadTreeNode::setPrimitiveList(QList<LaserPrimitive*> list)
{
    m_primitiveList = list;
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



