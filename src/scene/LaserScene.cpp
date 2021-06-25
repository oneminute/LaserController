#include "LaserScene.h"

#include "LaserDocument.h"
#include "LaserPrimitive.h"
#include "LaserLayer.h"
#include "LaserPrimitiveGroup.h"

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
    m_background = addRect(m_doc->pageBounds(), QPen(Qt::black, 1.0f, Qt::SolidLine), QBrush(Qt::white));
	setSceneRect(m_doc->pageBounds());
    QMap<QString, LaserPrimitive*> items = m_doc->primitives();
    for (QMap<QString, LaserPrimitive*>::iterator i = items.begin(); i != items.end(); i++)
    {
        this->addItem(i.value());
    }
}

void LaserScene::clearDocument(bool delDoc)
{
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

