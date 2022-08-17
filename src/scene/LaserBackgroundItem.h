#ifndef LASERBACKGROUNDITEM_H
#define LASERBACKGROUNDITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsItemGroup>
class LaserBackgroundItem : public QObject, public QGraphicsItemGroup
{
	Q_OBJECT
public:
	explicit LaserBackgroundItem(QGraphicsItem *parent = nullptr);
	~LaserBackgroundItem();

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	void onChangeGrids();//���¼���nodeֵ
	void drawGrids(QPainter& painter);
	bool detectGridNode(QPoint& point, QPoint & mousePoint);//������ʱ�������յ��Ƿ�Ӧ�ñ���Ϊ�����е�node��
	QRect rect();

protected:
	void onLayoutChanged(const QSizeF& size);

private:
	QList<int> m_gridNodeXList;
	QList<int> m_gridNodeYList;
	QList<int> m_gridSecondNodeXList;
	QList<int> m_gridSecondNodeYList;
	QGraphicsRectItem* m_rectItem;
    //QRect m_rect;

};

#endif // LASERBACKGROUNDITEM_H