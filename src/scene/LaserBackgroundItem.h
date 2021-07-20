#ifndef LASERBACKGROUNDITEM_H
#define LASERBACKGROUNDITEM_H

#include <QGraphicsRectItem>
class LaserBackgroundItem : public QGraphicsRectItem
{
public:
	explicit LaserBackgroundItem(QGraphicsItem *parent = nullptr);
	LaserBackgroundItem(const QRectF &rect, QGraphicsItem *parent = nullptr);
	~LaserBackgroundItem();

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	void onChangeGrids();//重新计算node值
	void drawGrids(QPainter& painter);
	bool detectGridNode(QPointF& point);//检测绘制时的起点或终点是否应该被设为网格中的node点
private:
	QList<qreal> m_gridNodeXList;
	QList<qreal> m_gridNodeYList;
	QList<qreal> m_gridSecondNodeXList;
	QList<qreal> m_gridSecondNodeYList;

};
#endif // LASERBACKGROUNDITEM_H