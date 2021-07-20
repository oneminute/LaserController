#include "LaserScene.h"

#include "LaserDocument.h"
#include "LaserPrimitive.h"
#include "LaserLayer.h"
#include "LaserPrimitiveGroup.h"
#include "widget/LaserViewer.h"

#include<QGraphicsSceneMouseEvent>
#include<QGraphicsSceneWheelEvent>

LaserScene::LaserScene(QObject* parent)
    : QGraphicsScene(parent)
    , m_doc(nullptr)
	, m_background(nullptr)
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
    }
    m_doc = doc;

    m_doc->setParent(this);

    qDebug() << "page bounds in pixel:" << m_doc->pageBounds();
    m_background = new LaserBackgroundItem(m_doc->pageBounds());
	addItem(m_background);
	//setSceneRect(m_doc->pageBounds());
	setSceneRect(QRectF(QPointF(-DBL_MAX, -DBL_MAX), QPointF(DBL_MAX, DBL_MAX)));
	//setSceneRect(QRectF(QPointF(0, 0), QPointF(0, 0)));
	
    QMap<QString, LaserPrimitive*> items = m_doc->primitives();
    for (QMap<QString, LaserPrimitive*>::iterator i = items.begin(); i != items.end(); i++)
    {
        this->addItem(i.value());
    }
	
}

void LaserScene::clearDocument(bool delDoc)
{
	m_background = nullptr;
    this->clear();
	LaserViewer* viewer = qobject_cast<LaserViewer*>(views()[0]);
	viewer->clearGroup();

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
}

QList<LaserPrimitive*> LaserScene::selectedPrimitives() const
{
    QList<LaserPrimitive*> primitives;
	qDebug() << "selectedItems() size: " << selectedItems().size();
    for (QGraphicsItem* item : selectedItems())
    {
        LaserPrimitive* primitive = dynamic_cast<LaserPrimitive*>(item);
        if (primitive)
        {
            primitives.append(primitive);
        }
    }
	qDebug() << "selectedItems() size: " << selectedItems().size();
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
	QString name = watched->metaObject()->className();
	//qDebug() << name;
	qDebug() << event->type();
	if (event->type() == QEvent::GraphicsSceneMousePress) {
		
		if (name == "LaserBitmap") {
			m_mousePressBlock = true;
			return false;
		}
		
	}
	else if (event->type() == QEvent::GraphicsSceneMouseMove) {
		if (name == "LaserBitmap") {
			m_mouseMoveBlock = true;
			return false;//scene 没有过滤掉此事件，继续向item传递， true则过滤掉了
		}
	}
	
	
	return QGraphicsScene::eventFilter(watched, event);
}

bool LaserScene::mousePressBlock()
{
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
	//selection lines
	QVector<QLineF> selectionEdges;
	selectionEdges.append(QLineF(selection.topLeft(), selection.topRight()));
	selectionEdges.append(QLineF(selection.topRight(), selection.bottomRight()));
	selectionEdges.append(QLineF(selection.bottomLeft(), selection.bottomRight()));
	selectionEdges.append(QLineF(selection.topLeft(), selection.bottomLeft()));
	//items
	QMap<QString, LaserPrimitive*> list = this->document()->primitives();
	
	for (int i = 0; i < list.size(); i++) {
		bool breakdown = false;
		LaserPrimitive* item = list.values()[i];
		if (!(item->flags() & QGraphicsItem::ItemIsSelectable)) {
			continue;
		}
		if (item->isSelected()) {
			continue;
		}
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
					break;
				}
			}
			if (breakdown)
				break;
		}
		/*QString name = item->metaObject()->className();
		if (name == "LaserRect") {
			LaserRect* rect = qgraphicsitem_cast<LaserRect*>(item);
			
		}*/
	}
}


