#ifndef LASERBACKGROUNDITEM_H
#define LASERBACKGROUNDITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsItemGroup>
class LaserBackgroundItem : public QGraphicsItemGroup
{
public:
	explicit LaserBackgroundItem(QGraphicsItem *parent = nullptr);
	LaserBackgroundItem(const QRectF &rect, QGraphicsItem *parent = nullptr);
	~LaserBackgroundItem();

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	void onChangeGrids();//���¼���nodeֵ
	void drawGrids(QPainter& painter);
	bool detectGridNode(QPointF& point, QPointF & mousePoint);//������ʱ�������յ��Ƿ�Ӧ�ñ���Ϊ�����е�node��
	QRectF rect();
private:
	QList<qreal> m_gridNodeXList;
	QList<qreal> m_gridNodeYList;
	QList<qreal> m_gridSecondNodeXList;
	QList<qreal> m_gridSecondNodeYList;
	QGraphicsRectItem* m_rectItem;

};
#endif // LASERBACKGROUNDITEM_H