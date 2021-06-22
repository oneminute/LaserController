#include "LaserPrimitiveGroup.h"

#include "LaserPrimitive.h"

class LaserPrimitiveGroupPrivate
{
	Q_DECLARE_PUBLIC(LaserPrimitiveGroup)
public:
	LaserPrimitiveGroupPrivate(LaserPrimitiveGroup* ptr)
		: q_ptr(ptr),
		boundingRect(0, 0, 0, 0)
	{}

	//QSet<LaserPrimitive*> primitives;
	QRectF boundingRect;
	LaserPrimitiveGroup* q_ptr;
};

LaserPrimitiveGroup::LaserPrimitiveGroup(QGraphicsItem * parent)
	: QGraphicsItemGroup(parent)
	, m_ptr(new LaserPrimitiveGroupPrivate(this))
{
}

LaserPrimitiveGroup::~LaserPrimitiveGroup()
{
}

//void LaserPrimitiveGroup::addPrimitives(const QList<LaserPrimitive*>& primitives)
//{
//	for (LaserPrimitive* primitive : primitives)
//	{
//		addToGroup(primitive);
//	}
//}
//
//void LaserPrimitiveGroup::clearPrimitives()
//{
//	Q_D(LaserPrimitiveGroup);
//	
//	const auto items = childItems();
//	for (QGraphicsItem *item : items)
//	{
//		LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
//		if (primitive)
//			removeFromGroup(primitive);
//	}
//		
//	d->boundingRect = QRectF(0, 0, 0, 0);
//}

void LaserPrimitiveGroup::addToGroup(LaserPrimitive * primitive)
{
	Q_D(LaserPrimitiveGroup);
	QGraphicsItemGroup::addToGroup(primitive);
}

void LaserPrimitiveGroup::removeFromGroup(LaserPrimitive * primitive, bool updateBounding)
{
	Q_D(LaserPrimitiveGroup);
	QGraphicsItemGroup::removeFromGroup(primitive);
	//primitive->setSelected(false);
}

QRectF LaserPrimitiveGroup::boundingRect() const
{
	Q_D(const LaserPrimitiveGroup);
	QRectF bounding = QRectF(0, 0, 0, 0);
	for (QGraphicsItem* item : childItems())
	{
		LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
		if (!primitive)
			continue;
		QTransform t = primitive->sceneTransform();
		QRectF boundingRect = primitive->boundingRect();
		/*if (!t.isIdentity())
		{
			boundingRect = t.mapRect(boundingRect);
		}*/
		if (bounding.isEmpty())
		{
			bounding = boundingRect;
		}
		else
		{
			bounding = bounding.united(boundingRect);
		}
	}
	return bounding;
}

QRectF LaserPrimitiveGroup::sceneBoundingRect() const
{
	return this->mapRectToScene(boundingRect());
}

bool LaserPrimitiveGroup::isEmpty() const
{
	return childItems().isEmpty();
}

//QRectF LaserPrimitiveGroup::updateBoundingRect()
//{
//	Q_D(LaserPrimitiveGroup);
//	d->boundingRect = QRectF(0, 0, 0, 0);
//	for (LaserPrimitive* primitive : d->primitives)
//	{
//		if (d->boundingRect.isEmpty())
//		{
//			d->boundingRect = primitive->sceneBoundingRect();
//		}
//		else
//		{
//			d->boundingRect = d->boundingRect.united(primitive->sceneBoundingRect());
//		}
//	}
//	return d->boundingRect;
//}

