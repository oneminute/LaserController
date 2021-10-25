#include "LaserScene.h"

#include "LaserDocument.h"
#include "LaserPrimitive.h"
#include "LaserLayer.h"
#include "LaserPrimitiveGroup.h"
#include "widget/LaserViewer.h"

#include<QGraphicsSceneMouseEvent>
#include<QGraphicsSceneWheelEvent>
#include <QScrollBar>
#include <qmath.h>
#include "util/Utils.h"

LaserScene::LaserScene(QObject* parent)
    : QGraphicsScene(parent)
    , m_doc(nullptr)
	, m_background(nullptr)
    , m_quadTree(nullptr)
{
}

LaserScene::~LaserScene()
{
    clearSelection();
}

void LaserScene::updateDocument(LaserDocument * doc)
{
    if (m_doc == doc)
    {
        clearDocument(false);
    }
    else
    {
        clearDocument(true);
        //删除树
        if (m_quadTree) {
            delete m_quadTree;
            m_quadTree = nullptr;
        }
        
    }
    m_doc = doc;

    m_doc->setParent(this);
    

    qDebug() << "page bounds in pixel:" << m_doc->pageBounds();
    m_background = new LaserBackgroundItem(m_doc->pageBounds());
	addItem(dynamic_cast<QGraphicsItemGroup*>(m_background));
	//setSceneRect(m_doc->pageBounds());
	//setSceneRect(QRectF(0, 0, 2000, 2000));
	setSceneRect(QRectF(QPointF(-5000000, -5000000), QPointF(5000000, 5000000)));
	QRectF rect = m_doc->pageBounds();
	//views()[0]->horizontalScrollBar()->setSliderPosition(rect.center().x());
	//views()[0]->verticalScrollBar()->setSliderPosition(rect.center().y());
	views()[0]->setTransform(QTransform());
	views()[0]->centerOn(rect.center());
    //创建树
    if (!m_quadTree) {
        qreal maxSize = Global::mm2PixelsYF(3000);
        qreal top = -(maxSize - rect.height()) * 0.5;
        qreal left = -(maxSize - rect.width()) * 0.5;
        m_maxRegion = QRectF(left, top, maxSize, maxSize);
        m_quadTree = new QuadTreeNode(m_maxRegion);
    }
    QMap<QString, LaserPrimitive*> items = m_doc->primitives();
    for (QMap<QString, LaserPrimitive*>::iterator i = items.begin(); i != items.end(); i++)
    {
        this->addItem(i.value());
    }
}

void LaserScene::clearDocument(bool delDoc)
{
	LaserViewer* viewer = qobject_cast<LaserViewer*>(views()[0]);
	if (delDoc && m_doc)
	{
		if (viewer) {
			viewer->clearGroupSelection();
			viewer->setGroupNull();
            viewer->setEditingText(nullptr);
		}
	}
	m_background = nullptr;	
    this->clear();
	
	if (delDoc && m_doc)
	{
		m_doc->close();
		m_doc = nullptr;
	}
	
    
}

void LaserScene::addLaserPrimitive(LaserPrimitive * primitive)
{
    m_doc->addPrimitive(primitive);
	addItem(primitive);
    m_quadTree->createPrimitiveTreeNode(primitive);

}

void LaserScene::removeLaserPrimitive(LaserPrimitive * primitive)
{
	m_doc->removePrimitive(primitive);
    primitive->removeAllTreeNode();
	removeItem(primitive);
}

QList<LaserPrimitive*> LaserScene::selectedPrimitives() const
{
    QList<LaserPrimitive*> primitives;
	QList<QGraphicsItem*> items = selectedItems();
    for (QGraphicsItem* item : items)
    {
        LaserPrimitive* primitive = dynamic_cast<LaserPrimitive*>(item);
        if (primitive)
        {
            primitives.append(primitive);
            
        }
    }
    return primitives;
}

LaserPrimitiveGroup * LaserScene::createItemGroup(const QList<LaserPrimitive*>& items)
{
	// Build a list of the first item's ancestors
	QList<QGraphicsItem *> ancestors;
	int n = 0;
	if (!items.isEmpty()) {
		QGraphicsItem *parent = items.at(n++);
		while ((parent = parent->parentItem()))
			ancestors.append(parent);
	}

	// Find the common ancestor for all items
	QGraphicsItem *commonAncestor = 0;
	if (!ancestors.isEmpty()) {
		while (n < items.size()) {
			int commonIndex = -1;
			QGraphicsItem *parent = items.at(n++);
			do {
				int index = ancestors.indexOf(parent, qMax(0, commonIndex));
				if (index != -1) {
					commonIndex = index;
					break;
				}
			} while ((parent = parent->parentItem()));

			if (commonIndex == -1) {
				commonAncestor = 0;
				break;
			}

			commonAncestor = ancestors.at(commonIndex);
		}
	}

	// Create a new group at that level
	LaserPrimitiveGroup *group = new LaserPrimitiveGroup(commonAncestor);
	if (!commonAncestor)
		addItem(group);
	for (QGraphicsItem *item : items)
	{
		group->addToGroup(qgraphicsitem_cast<LaserPrimitive*>(item));
	}
	return group;
}

void LaserScene::destroyItemGroup(LaserPrimitiveGroup * group)
{
	const auto items = group->QGraphicsItemGroup::childItems();
	for (QGraphicsItem *item : items)
		group->removeFromGroup(qgraphicsitem_cast<LaserPrimitive*>(item));
	removeItem(group);
	delete group;
}

bool LaserScene::eventFilter(QObject * watched, QEvent * event)
{
	
	m_mousePressBlock = false;
	m_mouseMoveBlock = false;
	m_detectedBitmap = nullptr;
	QString name = watched->metaObject()->className();
	//qDebug() << name;
	qDebug() << event->type();
	if (event->type() == QEvent::GraphicsSceneMousePress) {
		
		LaserViewer* viewer = qobject_cast<LaserViewer*>( views()[0]);
		if (viewer) {
			
			if (name == "LaserBitmap") {
				LaserBitmap* map = qobject_cast<LaserBitmap*>(watched);
				m_mousePressBlock = true;
				m_detectedBitmap = map;
				return false;
			}
		}
			
		
		
	}
	else if (event->type() == QEvent::GraphicsSceneMouseMove) {
		if (name == "LaserBitmap") {
			m_mouseMoveBlock = true;
			return false;//scene û�й��˵���¼���������item���ݣ� true����˵���
		}
    }
    else if (event->type() == QEvent::InputMethod) {
        QInputMethodEvent* e = static_cast<QInputMethodEvent*>(event);
        if (!e->commitString().isEmpty()) {
            qDebug() << e->commitString();
        }
        
    }
	
	
	return QGraphicsScene::eventFilter(watched, event);
}

bool LaserScene::mousePressDetectBitmap(LaserBitmap* & detected)
{
	detected = m_detectedBitmap;
	return m_mousePressBlock;
}
bool LaserScene::mouseMoveBlock()
{
	return m_mouseMoveBlock;
}
//rewrite mouse event
void LaserScene::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsScene::mousePressEvent(event);
}

void LaserScene::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsScene::mouseMoveEvent(event);
	
}

void LaserScene::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsScene::mouseReleaseEvent(event);
}

void LaserScene::findSelectedByLine(QRectF selection)
{
    //已经被选中的恢复正常状态
    for (LaserPrimitive* primitive : selectedPrimitives()) {
        primitive->setSelected(false);
    }
    QList<QLineF> selectionEdges;
    utils::rectEdges(selection, selectionEdges);
    //tree 查找
    QList<QuadTreeNode*> nodes = m_quadTree->search(selection);
    for (QuadTreeNode* node : nodes) {
        for (LaserPrimitive* primitive : node->primitiveList()) {
            
            if (primitive->flags() & QGraphicsItem::ItemIsSelectable) {
                QRectF bounds = primitive->sceneBoundingRect();
                //图元外包框整个被selection包裹
                selectedByBounds(bounds, selection, primitive);
                //图元的边与selection的边交叉
                bool isIntersected = false;
                QVector<QLineF> edges = primitive->edges();
                for (QLineF selectionEdge : selectionEdges) {
                    for (QLineF edge : edges) {
                        QPointF p;
                        if (selectionEdge.intersect(edge, &p) == QLineF::BoundedIntersection) {
                            isIntersected = true;
                            if (!primitive->isSelected()) {
                                primitive->setSelected(true);
                                //primitive->blockSignals(true);
                            }
                            break;
                        }
                        
                    }
                    if (isIntersected) {
                        isIntersected = false;
                        break;
                    }
                }
                
            }

        }
    }

	//items
	/*
    //selection lines
    QVector<QLineF> selectionEdges;
    selectionEdges.append(QLineF(selection.topLeft(), selection.topRight()));
    selectionEdges.append(QLineF(selection.topRight(), selection.bottomRight()));
    selectionEdges.append(QLineF(selection.bottomLeft(), selection.bottomRight()));
    selectionEdges.append(QLineF(selection.topLeft(), selection.bottomLeft()));
    QMap<QString, LaserPrimitive*> list = this->document()->primitives();
	
	for (int i = 0; i < list.size(); i++) {
		bool breakdown = false;
		LaserPrimitive* item = list.values()[i];
		if (!(item->flags() & QGraphicsItem::ItemIsSelectable)) {
			continue;
		}
		item->blockSignals(true);
		//如果已经被选中则设为false
		if (item->isSelected()) {
			item->setSelected(false);
		}
		//先判断是否在整个rect里面
		if (selection.contains(item->sceneBoundingRect())) {
			item->setSelected(true);
		}
		//按边判断
		QVector<QLineF> edges = item->edges();
		if (edges.size() <= 0) {
			continue;
		}
		for (int si = 0; si < selectionEdges.size(); si++) {
			QLineF sEdge = selectionEdges[si];
			for (int ei = 0; ei < edges.size(); ei++) {
				QLineF edge = edges[ei];
				QPointF point;
				if (sEdge.intersect(edge, &point) == QLineF::BoundedIntersection) {
					item->setSelected(true);
					breakdown = true;
					qDebug() << item;
					break;
				}
			}
			if (breakdown)
				break;
		}
		item->blockSignals(false);
	}*/
}

void LaserScene::findSelectedByBoundingRect(QRectF selection)
{
	//items遍历查找
    /*QList<LaserPrimitive*> list = this->document()->primitives().values();
	
	for(LaserPrimitive* item : list) {
		if (!(item->flags() & QGraphicsItem::ItemIsSelectable)) {
			continue;
		}
		item->blockSignals(true);
		//如果已经被选中则设为false
		if (item->isSelected()) {
			item->setSelected(false);
		}
		if (selection.contains(item->sceneBoundingRect())) {
			item->setSelected(true);
		}
		item->blockSignals(false);
	}*/
    //QT自带
    /*QPainterPath path;
    path.addRect(selection);
    setSelectionArea(path, Qt::ItemSelectionMode::ContainsItemShape);
    QList<LaserPrimitive*>list = selectedPrimitives();*/
    //已经被选中的恢复正常状态
    for (LaserPrimitive* primitive : selectedPrimitives()) {
        primitive->setSelected(false);
        //primitive->blockSignals(true);
    }
    //tree 查找
    QList<QuadTreeNode*> nodes = m_quadTree->search(selection);
    for (QuadTreeNode* node : nodes) {
        for (LaserPrimitive* primitive : node->primitiveList()) {
            if (primitive->flags() & QGraphicsItem::ItemIsSelectable) {
                QRectF bounds = primitive->sceneBoundingRect();
                selectedByBounds(bounds, selection, primitive);
            }           
        } 
    }
    
}

void LaserScene::selectedByBounds(QRectF bounds, QRectF selection, LaserPrimitive* primitive)
{  
    if (selection.contains(bounds)) {
        if (!primitive->isSelected()) {
            primitive->setSelected(true);
        }

    }
    //point
    else if (bounds.width() == 0 && bounds.height() == 0) {
        QPointF p = bounds.topLeft();
        if (selection.contains(p)) {
            if (!primitive->isSelected()) {
                primitive->setSelected(true);
            }
        }
    }
    // line
    else if (bounds.width() == 0 || bounds.height() == 0) {
        QPointF p1 = bounds.topLeft();
        QPointF p2 = bounds.bottomRight();
        if (selection.contains(p1) && selection.contains(p2)) {
            if (!primitive->isSelected()) {
                primitive->setSelected(true);
            }
        }
    }
}

QRectF LaserScene::maxRegion()
{
    return m_maxRegion;
}

QuadTreeNode * LaserScene::quadTreeNode()
{
    return m_quadTree;
}


