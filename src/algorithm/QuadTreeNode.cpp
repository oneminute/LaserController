#include "QuadTreeNode.h"

#include <QList>
#include <QRectF>
#include <QStack>

#include "primitive/LaserPrimitiveHeaders.h"
#include "util/utils.h"

QuadTreeNode::QuadTreeNode(QRectF region, int depth)
    :m_nodeTopLeft(nullptr)
    , m_nodeTopRight(nullptr)
    , m_nodeBottomLeft(nullptr)
    , m_nodeBottomRight(nullptr)
{
    m_region = region;
    m_depth = depth;
    //�ӽڵ�����
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
    m_nodeTopLeft = nullptr;
    m_nodeTopRight = nullptr;
    m_nodeBottomLeft = nullptr;
    m_nodeBottomRight = nullptr;
}

bool QuadTreeNode::createChildrenNodes(LaserPrimitive* primitive)
{
    if (m_depth > m_maxDepth) {
        addPrimitive(primitive);
        primitive->blockSignals(true);
        return false;
    }
    createPrimitiveTreeNode(primitive);
    return true;
}

void QuadTreeNode::createPrimitiveTreeNode(LaserPrimitive* primitive)
{
    QRectF bound = primitive->sceneBoundingRect();
    //qDebug() << bound;
    QLineF line1(QPointF(100, 0), QPointF(200, 10));
    QPointF p;
    int i = line1.intersect(line1, &p);
    //point
    if (bound.width() == 0 && bound.height() == 0) {
        QPointF p = bound.topLeft();
        //�������ͼԪ
        if (m_topLeftRegion.contains(p)) {
            createNode(Qt::TopLeftCorner);
            //m_nodeTopLeft->addPrimitive(primitive);
            m_nodeTopLeft->createChildrenNodes(primitive);
        }
        else if (m_topRightRegion.contains(p)) {
            createNode(Qt::TopRightCorner);
            //m_nodeTopRight->addPrimitive(primitive);
            m_nodeTopRight->createChildrenNodes(primitive);
        }
        else if (m_bottomRightRegion.contains(p)) {
            createNode(Qt::BottomRightCorner);
            //m_nodeBottomRight->addPrimitive(primitive);
            m_nodeBottomRight->createChildrenNodes(primitive);
        }
        else if (m_bottomLeftRegion.contains(p)) {
            createNode(Qt::BottomLeftCorner);
            //m_nodeBottomLeft->addPrimitive(primitive);
            m_nodeBottomLeft->createChildrenNodes(primitive);
        }
    }
    //line
    else if (bound.width() == 0 || bound.height() == 0) {
        QPointF p1 = bound.topLeft();
        QPointF p2 = QPoint(bound.left() + bound.width(), bound.top() + bound.height());
        QLineF line(p1, p2);
        //�������ͼԪ
        if (m_topLeftRegion.contains(p1) && m_topLeftRegion.contains(p2)) {
            createNode(Qt::TopLeftCorner);
            //m_nodeTopLeft->addPrimitive(primitive);
            m_nodeTopLeft->createChildrenNodes(primitive);
        }
        else if (m_topRightRegion.contains(p1) && m_topRightRegion.contains(p2)) {
            createNode(Qt::TopRightCorner);
            //m_nodeTopRight->addPrimitive(primitive);
            m_nodeTopRight->createChildrenNodes(primitive);
        }
        else if (m_bottomRightRegion.contains(p1) && m_bottomRightRegion.contains(p2)) {
            createNode(Qt::BottomRightCorner);
            //m_nodeBottomRight->addPrimitive(primitive);
            m_nodeBottomRight->createChildrenNodes(primitive);
        }
        else if (m_bottomLeftRegion.contains(p1) && m_bottomLeftRegion.contains(p2)) {
            createNode(Qt::BottomLeftCorner);
            //m_nodeBottomLeft->addPrimitive(primitive);
            m_nodeBottomLeft->createChildrenNodes(primitive);
        }
        else {
            QList<QLineF> topLeftRegionEdges;
            QList<QLineF> topRightRegionEdges;
            QList<QLineF> bottomLeftRegionEdges;
            QList<QLineF> bottomRightRegionEdges;
            utils::rectEdges(m_topLeftRegion, topLeftRegionEdges);
            utils::rectEdges(m_topRightRegion, topRightRegionEdges);
            utils::rectEdges(m_bottomLeftRegion, bottomLeftRegionEdges);
            utils::rectEdges(m_bottomRightRegion, bottomRightRegionEdges);
            for (QLineF lineTL : topLeftRegionEdges) {
                QPointF p;
                if (lineTL.intersect(line, &p)) {
                    createNode(Qt::TopLeftCorner);
                    m_nodeTopLeft->createChildrenNodes(primitive);
                    break;
                }
            }
            for (QLineF lineTR : topRightRegionEdges) {
                QPointF p;
                if (lineTR.intersect(line, &p)) {
                    createNode(Qt::TopRightCorner);
                    m_nodeTopRight->createChildrenNodes(primitive);
                    break;
                }
            }
            for (QLineF lineBL : bottomLeftRegionEdges) {
                QPointF p;
                if (lineBL.intersect(line, &p)) {
                    createNode(Qt::BottomLeftCorner);
                    m_nodeBottomLeft->createChildrenNodes(primitive);
                    break;
                }
            }
            for (QLineF lineBL : bottomRightRegionEdges) {
                QPointF p;
                if (lineBL.intersect(line, &p)) {
                    createNode(Qt::BottomRightCorner);
                    m_nodeBottomRight->createChildrenNodes(primitive);
                    break;
                }
            }
        }
    }
    //rect
    else {
        //�������ͼԪ
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
                //ͼԪ�����ڵ�����
                createNode(Qt::TopLeftCorner);
                m_nodeTopLeft->addPrimitive(primitive);
            }
            else if (m_topLeftRegion.intersects(bound)) {
                //������ͼԪ�ཻ
                createNode(Qt::TopLeftCorner);
                //m_nodeTopLeft->addPrimitive(primitive);
                m_nodeTopLeft->createChildrenNodes(primitive);
            }

            if (bound.contains(m_topRightRegion)) {
                //ͼԪ�����ڵ�����
                createNode(Qt::TopRightCorner);
                m_nodeTopRight->addPrimitive(primitive);
            }
            else if (m_topRightRegion.intersects(bound)) {
                //������ͼԪ�ཻ
                createNode(Qt::TopRightCorner);
                //m_nodeTopRight->addPrimitive(primitive);
                m_nodeTopRight->createChildrenNodes(primitive);
            }

            if (bound.contains(m_bottomRightRegion)) {
                //ͼԪ�����ڵ�����
                createNode(Qt::BottomRightCorner);
                m_nodeBottomRight->addPrimitive(primitive);
            }
            else if (m_bottomRightRegion.intersects(bound)) {
                //������ͼԪ�ཻ
                createNode(Qt::BottomRightCorner);
                //m_nodeBottomRight->addPrimitive(primitive);
                m_nodeBottomRight->createChildrenNodes(primitive);
            }

            if (bound.contains(m_bottomLeftRegion)) {
                //ͼԪ�����ڵ�����
                createNode(Qt::BottomLeftCorner);
                m_nodeBottomLeft->addPrimitive(primitive);
            }
            else if (m_bottomLeftRegion.intersects(bound)) {
                //������ͼԪ�ཻ
                createNode(Qt::BottomLeftCorner);
                //m_nodeBottomLeft->addPrimitive(primitive);
                m_nodeBottomLeft->createChildrenNodes(primitive);
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
QList<QuadTreeNode*>& QuadTreeNode::search(QRectF selection)
{
    m_targetNodes.clear();
    QStack<QuadTreeNode*> stack;
    stack.push(this);
    while (!stack.isEmpty()) {
        QuadTreeNode* node = stack.pop();
        if (!node) {
            continue;
        }
        if (selection.contains(node->region())) {
            if (!node->primitiveList().isEmpty()) {
                m_targetNodes.append(node);
            }
            //�����ӽڵ�
            if (node->m_nodeTopLeft) {
                stack.push(node->m_nodeTopLeft);
            }
            if (node->m_nodeTopRight) {
                stack.push(node->m_nodeTopRight);
            }
            if (node->m_nodeBottomLeft) {
                stack.push(node->m_nodeBottomLeft);
            }
            if (node->m_nodeBottomRight) {
                stack.push(node->m_nodeBottomRight);
            }
        }
        else if (node->region().contains(selection)) {
            if (node->isLeaf() && !node->primitiveList().isEmpty()) {
                m_targetNodes.append(node);
            }
            else {
                //�����ӽڵ�
                if (node->m_nodeTopLeft) {
                    stack.push(node->m_nodeTopLeft);
                }
                if (node->m_nodeTopRight) {
                    stack.push(node->m_nodeTopRight);
                }
                if (node->m_nodeBottomLeft) {
                    stack.push(node->m_nodeBottomLeft);
                }
                if (node->m_nodeBottomRight) {
                    stack.push(node->m_nodeBottomRight);
                }
            }

        }
        else if (node->region().intersects(selection)) {
            if (!node->primitiveList().isEmpty()) {
                m_targetNodes.append(node);

            }
            //�����ӽڵ�
            if (node->m_nodeTopLeft) {
                stack.push(node->m_nodeTopLeft);
            }
            if (node->m_nodeTopRight) {
                stack.push(node->m_nodeTopRight);
            }
            if (node->m_nodeBottomLeft) {
                stack.push(node->m_nodeBottomLeft);
            }
            if (node->m_nodeBottomRight) {
                stack.push(node->m_nodeBottomRight);
            }

        }

    }
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

QList<LaserPrimitive*> QuadTreeNode::primitiveList()
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
        if (!m_primitiveList.removeOne(primitive)) {
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

QuadTreeNode * QuadTreeNode::nodeTopLeft()
{
    return m_nodeTopLeft;
}

QuadTreeNode * QuadTreeNode::nodeTopRight()
{
    return m_nodeTopRight;
}

QuadTreeNode * QuadTreeNode::nodeBottomLeft()
{
    return m_nodeBottomLeft;
}

QuadTreeNode * QuadTreeNode::nodeBottomRight()
{
    return m_nodeBottomRight;
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

void QuadTreeNode::upDatePrimitive(LaserPrimitive * primitive)
{
    primitive->removeAllTreeNode();
    createPrimitiveTreeNode(primitive);
}

void QuadTreeNode::clearAllNodes()
{
    //clear all nodes
    QStack<QuadTreeNode*> stack;
    while(this)
    stack.push(this);
    while (!stack.isEmpty()) {
        QuadTreeNode* node = stack.pop();
        if (!node) {
            continue;
        }
        if (node->isLeaf()) {
            //clear primitive
            for (LaserPrimitive* primitive : node->primitiveList()) {
                primitive->removeOneTreeNode(node);
            }
        }
        else {
            stack.push(node);
            if (node->m_nodeTopLeft) {
                stack.push(node->nodeTopLeft());
            }
            if (node->m_nodeBottomLeft) {
                stack.push(node->nodeBottomLeft());
            }
            if (node->m_nodeTopRight) {
                stack.push(node->nodeTopRight());
            }
            if (node->m_nodeTopRight) {
                stack.push(node->nodeTopRight());
            }
        }
    }

}



