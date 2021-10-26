#ifndef LASERPRIMITIVEGROUP_H
#define LASERPRIMITIVEGROUP_H

#include <QGraphicsItemGroup>
#include <QGraphicsItem>

class LaserPrimitive;
class LaserPrimitiveGroupPrivate;
//class LaserPrimitiveGroup : public QObject, public QGraphicsItemGroup
class LaserPrimitiveGroup : public QGraphicsItem
{
    /*Q_OBJECT
public:
	LaserPrimitiveGroup(QGraphicsItem* parent = nullptr);
	virtual ~LaserPrimitiveGroup();
	void addToGroup(LaserPrimitive* primitive);
	void removeFromGroup(LaserPrimitive* primitive, bool updateBounding = false);
	virtual QRectF boundingRect() const override;
	QRectF sceneBoundingRect() const;
	bool isEmpty() const;

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

protected:

    virtual QVariant
        itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
private:
	QScopedPointer<LaserPrimitiveGroupPrivate> m_ptr;

	Q_DECLARE_PRIVATE_D(m_ptr, LaserPrimitiveGroup)
	Q_DISABLE_COPY(LaserPrimitiveGroup)*/
public:
    LaserPrimitiveGroup(QGraphicsItem *parent = nullptr);
    ~LaserPrimitiveGroup();
protected:
    virtual void
        paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
public:
    void addToGroup(LaserPrimitive* primitive);
    void removeFromGroup(LaserPrimitive* primitive);
    virtual QRectF boundingRect() const override;
    QRectF sceneBoundingRect() const;
    bool isEmpty() const;
};

#endif // LASERPRIMITIVEGROUP_H