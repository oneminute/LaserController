#ifndef SPLINENODE_H
#define SPLINENODE_H
#include <QGraphicsRectItem> 
class SplineNode : public QGraphicsRectItem
{
	using QGraphicsRectItem::QGraphicsRectItem;
public:
	//explicit SplineNode(QGraphicsItem* parent = nullptr);
	~SplineNode();
protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // SPLINENODE_H

