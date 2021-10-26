#include "LaserPrimitiveGroup.h"

#include "LaserPrimitive.h"
#include "widget/LaserViewer.h"
#include "ui/LaserControllerWindow.h"
#include "scene/LaserScene.h"
#include"LaserApplication.h"
#include <QGraphicsScene>
#include <QGraphicsItem>

//class QGraphicsItem;
/*class LaserPrimitiveGroupPrivate
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
	for (QGraphicsItem* item : QGraphicsItemGroup::childItems())
	{
		LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
		if (!primitive)
			continue;
		QTransform t = primitive->sceneTransform();
		QRectF boundingRect = primitive->boundingRect();
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
	return this->QGraphicsItemGroup::mapRectToScene(boundingRect());
}

bool LaserPrimitiveGroup::isEmpty() const
{
	return QGraphicsItemGroup::childItems().isEmpty();
}

void LaserPrimitiveGroup::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	//����group
	//QGraphicsItemGroup::paint(painter, option, widget);
}

QVariant LaserPrimitiveGroup::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value)
{

    //return QGraphicsItemGroup::itemChange(change, value);
    return value;
}*/

LaserPrimitiveGroup::LaserPrimitiveGroup(QGraphicsItem * parent)
    :QGraphicsItem(parent)
{
}

LaserPrimitiveGroup::~LaserPrimitiveGroup()
{
}

void LaserPrimitiveGroup::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
}

void LaserPrimitiveGroup::addToGroup(LaserPrimitive * primitive)
{
    primitive->setParentItem(this);
}

void LaserPrimitiveGroup::removeFromGroup(LaserPrimitive * primitive)
{
    QTransform transform = primitive->sceneTransform();
    primitive->setParentItem(0);
    primitive->setTransform(transform);
}

QRectF LaserPrimitiveGroup::boundingRect() const
{
    QRectF bounding = QRectF(0, 0, 0, 0);
    for (QGraphicsItem* item : childItems())
    {
        LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
        if (!primitive)
            continue;
        QTransform t = primitive->sceneTransform();
        QRectF boundingRect = primitive->boundingRect();
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

bool LaserPrimitiveGroup::isEmpty() const
{
    return childItems().isEmpty();
}
QRectF LaserPrimitiveGroup::sceneBoundingRect() const
{
    return this->mapRectToScene(boundingRect());
}
