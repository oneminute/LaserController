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

	void onChangeGrids();//���¼���nodeֵ
	void drawGrids(QPainter& painter);
	bool detectGridNode(QPointF& point);//������ʱ�������յ��Ƿ�Ӧ�ñ���Ϊ�����е�node��
private:
	QList<qreal> m_gridNodeXList;
	QList<qreal> m_gridNodeYList;
	QList<qreal> m_gridSecondNodeXList;
	QList<qreal> m_gridSecondNodeYList;

};
#endif // LASERBACKGROUNDITEM_H