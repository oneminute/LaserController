#include "LaserScene.h"

#include "LaserDocument.h"
#include "LaserPrimitive.h"
#include "LaserLayer.h"
#include "LaserPrimitiveGroup.h"
#include "widget/LaserViewer.h"
#include "laser/LaserDevice.h"
#include "LaserApplication.h"
#include "ui/LaserControllerWindow.h"

#include<QGraphicsSceneMouseEvent>
#include<QGraphicsSceneWheelEvent>
#include <QScrollBar>
#include <QInputMethodEvent>
#include <qmath.h>
#include <QMessageBox>
#include "util/Utils.h"
#include "common/Config.h"

LaserScene::LaserScene(QObject* parent)
    : QGraphicsScene(parent)
    , m_doc(nullptr)
	, m_background(nullptr)
	, m_imageBackground(nullptr)
    , m_quadTree(nullptr)
{
}

LaserScene::~LaserScene()
{
    clearSelection();
}

void LaserScene::setDocument(LaserDocument * doc)
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
    
	QRect rect = LaserApplication::device->layoutRect();
    m_background = new LaserBackgroundItem();
    m_background->setZValue(0);
	addItem(dynamic_cast<QGraphicsItemGroup*>(m_background));
	//setSceneRect(m_doc->pageBounds());
	//setSceneRect(QRectF(0, 0, 2000, 2000));
	setSceneRect(QRect(QPoint(-99000000000, -99000000000), QPoint(99000000000, 99000000000)));
	//views()[0]->horizontalScrollBar()->setSliderPosition(rect.center().x());
	//views()[0]->verticalScrollBar()->setSliderPosition(rect.center().y());
	views()[0]->setTransform(QTransform());
	views()[0]->centerOn(rect.center());
    //创建树
    if (!m_quadTree) {
        int maxSize = Config::Ui::validMaxRegion() * 1000;
        int top = -(maxSize - rect.height()) / 2;
        int left = -(maxSize - rect.width()) / 2;
        m_maxRegion = QRect(rect.left() + left, rect.top() + top, maxSize, maxSize);
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
            viewer->undoStack()->clear();
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

void LaserScene::addLaserPrimitive(LaserPrimitive * primitive, bool ignoreUpdateDocBounding)
{
    m_doc->addPrimitive(primitive);
	addItem(primitive);
    m_quadTree->createPrimitiveTreeNode(primitive);
    if (!ignoreUpdateDocBounding)
        m_doc->updateDocumentBounding();
}

void LaserScene::addLaserPrimitive(LaserPrimitive* primitive, LaserLayer* layer, bool ignoreUpdateDocBounding)
{
    m_doc->addPrimitive(primitive, layer);
	addItem(primitive);
    m_quadTree->createPrimitiveTreeNode(primitive);
    if (!ignoreUpdateDocBounding)
        m_doc->updateDocumentBounding();
}

void LaserScene::addLaserPrimitiveWithoutTreeNode(LaserPrimitive * primitive, bool ignoreUpdateDocBounding)
{
    m_doc->addPrimitive(primitive);
    addItem(primitive);
    if (!ignoreUpdateDocBounding)
        m_doc->updateDocumentBounding();
}

void LaserScene::addGroupItemsToTreeNode()
{
    LaserViewer* v = qobject_cast<LaserViewer*>( views()[0]);
    for each(QGraphicsItem* item in v->group()->childItems()) {
        LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
        m_quadTree->createPrimitiveTreeNode(primitive);
    }
}

void LaserScene::removeLaserPrimitive(LaserPrimitive * primitive, bool ignoreUpdateDocBounding)
{
	m_doc->removePrimitive(primitive, true);
    primitive->removeAllTreeNode();
	removeItem(primitive);
    if (!ignoreUpdateDocBounding)
        m_doc->updateDocumentBounding();
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
	for (LaserPrimitive *item : items)
	{
        if (item->isLocked())
            continue;
		group->addToGroup(item);
	}
    
    if (items.size() > 0) {
        LaserViewer* viewer = qobject_cast<LaserViewer*>(views()[0]);
        emit viewer->selectedSizeChanged();
    }
    
	return group;
}

void LaserScene::destroyItemGroup(LaserPrimitiveGroup * group)
{
	const auto items = group->childItems();
	for (QGraphicsItem *item : items)
		group->removeFromGroup(qgraphicsitem_cast<LaserPrimitive*>(item));
	removeItem(group);
	delete group;
}

bool LaserScene::eventFilter(QObject * watched, QEvent * event)
{
	
m_mousePressBlock = false;
m_mouseMoveBlock = false;
m_detectedFillSolid = nullptr;
QString name = watched->metaObject()->className();
//qDebug() << name;
qDebug() << event->type();
if (event->type() == QEvent::GraphicsSceneMousePress) {

    LaserViewer* viewer = qobject_cast<LaserViewer*>(views()[0]);
    if (viewer) {

        if (name == "LaserBitmap") {
            LaserBitmap* map = qobject_cast<LaserBitmap*>(watched);
            m_mousePressBlock = true;
            m_detectedFillSolid = map;
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
    QInputMethodEvent* e = reinterpret_cast<QInputMethodEvent*>(event);
    if (!e->commitString().isEmpty()) {
        qDebug() << e->commitString();
    }

}


return QGraphicsScene::eventFilter(watched, event);
}

bool LaserScene::mousePressDetectBitmap(LaserBitmap* & detected)
{
    detected = m_detectedFillSolid;
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

QSet<LaserPrimitive*> LaserScene::findPrimitivesByRect(const QRectF& rect)
{
    QSet<LaserPrimitive*> primitives;
    //tree 查找
    QList<QuadTreeNode*> nodes = m_quadTree->search(rect);
    for (QuadTreeNode* node : nodes) {
        for (LaserPrimitive* primitive : node->primitiveList()) {
            if (primitive->flags() & QGraphicsItem::ItemIsSelectable) {
                primitives.insert(primitive);
            }
        }
    }
    return primitives;
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
                int type = primitive->primitiveType();
                if (type == LPT_BITMAP ||
                    type == LPT_STAR ||
                    type == LPT_FRAME ||
                    type == LPT_RING ||
                    type == LPT_CIRCLETEXT ||
                    type == LPT_HORIZONTALTEXT ||
                    type == LPT_VERTICALTEXT){
                    //图元的path与selection的边交叉
                    selectedByRegion(selection, primitive);
                }
                else {
                    //图元的边与selection的边交叉
                    selectedByLine(selectionEdges, selection, primitive);
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
                QRect bounds;
                if (primitive->isJoinedGroup()) {
                    utils::boundingRect(*(primitive->joinedGroupList()), bounds, QRect(), false);
                    for (QSet<LaserPrimitive*>::iterator p = primitive->joinedGroupList()->begin();
                                    p != primitive->joinedGroupList()->end(); p++) {
                        selectedByBounds(bounds, selection, *p);
                    }
                }
                else {
                    bounds = primitive->sceneBoundingRect();
                    selectedByBounds(bounds, selection, primitive);
                }
                
                
            }           
        } 
    }
    
}
void LaserScene::findSelectedByRegion(QRectF selection)
{
    
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
void LaserScene::selectedByLine(QList<QLineF> selectionEdges, QRectF selection, LaserPrimitive* primitive)
{
    bool isIntersected = false;
    QVector<QLineF> edges = primitive->edges();
    for (QLineF selectionEdge : selectionEdges) {
        for (QLineF edge : edges) {
            QPointF p;
            if (selectionEdge.intersect(edge, &p) == QLineF::BoundedIntersection) {
                isIntersected = true;
                if (!primitive->isSelected()) {
                    primitive->setSelected(true);
                    if (primitive->isJoinedGroup()) {
                        for (QSet<LaserPrimitive*>::iterator p = primitive->joinedGroupList()->begin();
                            p != primitive->joinedGroupList()->end(); p++) {
                            (*p)->setSelected(true);
                        }
                    }

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
void LaserScene::selectedByRegion(QRectF selection, LaserPrimitive * primitive)
{
    bool isIntersected = primitive->getScenePath().intersects(selection);
    if (isIntersected) {
        primitive->setSelected(true);
        if (primitive->isJoinedGroup()) {
            for (QSet<LaserPrimitive*>::iterator p = primitive->joinedGroupList()->begin();
                p != primitive->joinedGroupList()->end(); p++) {
                (*p)->setSelected(true);
            }
        }

    }
}
QRect LaserScene::maxRegion()
{
    return m_maxRegion;
}

QuadTreeNode * LaserScene::quadTreeNode()
{
    return m_quadTree;
}

void LaserScene::updateValidMaxRegionRect()
{
    QRect rect = LaserApplication::device->layoutRect();
    int maxSize = Config::Ui::validMaxRegion() * 1000;
    int top = -(maxSize - rect.height()) / 2;
    int left = -(maxSize - rect.width()) / 2;
    m_maxRegion = QRect(rect.left() + left, rect.top() + top, maxSize, maxSize);
    views()[0]->viewport()->repaint();
}

void LaserScene::updataValidMaxRegion()
{
    if (!document())
        return;
    LaserViewer* view = qobject_cast<LaserViewer*>( views()[0]);
    //QRectF rect = Global::matrixFromUm().mapRect(LaserApplication::device->layoutRectInMech());
    QRect rect = LaserApplication::device->layoutRect();
    int maxSize = Config::Ui::validMaxRegion() * 1000;
    int top = -(maxSize - rect.height()) / 2;
    int left = -(maxSize - rect.width()) / 2;
    int lastMaxSize = m_maxRegion.width();
    m_maxRegion = QRect(rect.left() + left, rect.top() + top, maxSize, maxSize);
    view->viewport()->repaint();
    //所有图元bounds
    int bLeft = 0;
    int bRight = 0;
    int bTop = 0;
    int bBottom = 0;
    int i = 0;
    for (LaserPrimitive* primitive : document()->primitives()) {
        view->detectRect(*primitive, i, bLeft, bRight, bTop, bBottom);
        i++;
    }
    QRect bounds = QRect(bLeft, bTop, bRight - bLeft, bBottom - bTop);
    QPoint topLeft = bounds.topLeft();
    QPoint bottomRight(bounds.left() + bounds.width(), bounds.right() + bounds.height());
    //垂直或水平线，点的情况
    if (bounds.width() == 0 || bounds.height() == 0) {
        if (!maxRegion().contains(topLeft) || !maxRegion().contains(bottomRight)) {
            QMessageBox::warning(view, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
            
        }
    }
    else {
        if (!maxRegion().contains(bounds)) {
            QMessageBox::warning(view, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
            
        }
    }

    
}

void LaserScene::updateTree()
{
    if (document()) {
        //updata region
        updataValidMaxRegion();
        //clear tree
        delete m_quadTree;
        //rebuild tree
        m_quadTree = new QuadTreeNode(m_maxRegion);
        int s = document()->primitives().size();
        for (LaserPrimitive* p : document()->primitives()) {
            //clear primitive nodelist
            p->treeNodesList().clear();
            //create
            m_quadTree->createPrimitiveTreeNode(p);
        }
    }
}

QList<QSet<LaserPrimitive*>*>& LaserScene::joinedGroupList()
{
    return m_joinedGroupList;
}

void LaserScene::setImage(const QImage& image)
{
    if (m_imageBackground)
    {
        m_imageBackground->setPixmap(QPixmap::fromImage(image));
        m_imageBackground->show();
    }
    else
    {
        QRect layoutRect = LaserApplication::device->layoutRect();
        QSize resol = Config::Camera::resolution();
        qreal hFactor = layoutRect.width() * 1.0 / resol.width();
        qreal vFactor = layoutRect.height() * 1.0 / resol.height();
        m_imageBackground = addPixmap(QPixmap::fromImage(image));
        QPoint imagePos = LaserApplication::device->mapFromQuadToCurrent(QPoint(0, 0));
        QTransform ts = QTransform::fromScale(hFactor, vFactor);
        m_imageBackground->setTransform(ts);
        m_imageBackground->setPos(imagePos);
	    m_imageBackground->setFlag(QGraphicsItem::ItemIsSelectable, true);
        m_imageBackground->setZValue(-1);
    }
}

void LaserScene::clearImage()
{
    if (!m_imageBackground)
        return;

    m_imageBackground->hide();
}


