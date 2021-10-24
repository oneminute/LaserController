#include "LaserPrimitiveGroup.h"

#include "LaserPrimitive.h"
#include "widget/LaserViewer.h"
#include "ui/LaserControllerWindow.h"
#include "scene/LaserScene.h"
#include"LaserApplication.h"
#include <QGraphicsScene>


//class QGraphicsItem;
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

void LaserPrimitiveGroup::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	//����group
	//QGraphicsItemGroup::paint(painter, option, widget);
}

/*QVariant LaserPrimitiveGroup::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value)
{
    
    int size = this->childItems().size();
    //qDebug() << size;
    LaserScene* s = qobject_cast<LaserScene*>(scene());
    if (!s) {
        return QGraphicsItemGroup::itemChange(change, value);
    }
    LaserViewer* view = qobject_cast<LaserViewer*>(s->views()[0]);
    LaserControllerWindow* window = LaserApplication::mainWindow;   
    if (window) {
        window->onLaserPrimitiveGroupItemChanged();
    }
    

    return QGraphicsItemGroup::itemChange(change, value);
}*/

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

