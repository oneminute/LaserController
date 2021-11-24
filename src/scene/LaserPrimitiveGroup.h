#ifndef LASERPRIMITIVEGROUP_H
#define LASERPRIMITIVEGROUP_H

#include <QGraphicsItemGroup>
#include <QGraphicsItem>

class LaserPrimitive;
class LaserPrimitiveGroupPrivate;
//class LaserPrimitiveGroup : public QObject, public QGraphicsItemGroup
class LaserPrimitiveGroup : public QObject, public QGraphicsItem
{
    Q_OBJECT
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
public: signals:
    void childrenChanged();
};

#endif // LASERPRIMITIVEGROUP_H