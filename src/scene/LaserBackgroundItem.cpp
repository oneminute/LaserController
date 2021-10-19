#include "LaserBackgroundItem.h"
#include <QPainter>
#include <QDebug>
#include <QList>
#include <QGraphicsItemGroup>
#include <QtCore/qmath.h>  
#include <QTransform>
#include <QStyleOptionGraphicsItem>
#include "scene/LaserPrimitive.h"

#include "widget//LaserViewer.h"
#include "common/common.h"
#include "common/Config.h"
LaserBackgroundItem::LaserBackgroundItem(QGraphicsItem *parent) 
	:QGraphicsItemGroup(parent)
{
	QGraphicsItemGroup::setFlag(ItemIsSelectable, true);
}
LaserBackgroundItem::LaserBackgroundItem(const QRectF & rect, QGraphicsItem * parent)
	: QGraphicsItemGroup(parent)
{
	m_rectItem = new QGraphicsRectItem(rect);
	QPen pen(Qt::black, 1.0f, Qt::SolidLine);
	pen.setCosmetic(true);
	m_rectItem->setPen(pen);
	addToGroup(m_rectItem);
	QGraphicsItemGroup::setFlag(ItemIsSelectable, true);

    QPen pen1(Qt::black, 1.0f, Qt::SolidLine);
    pen1.setCosmetic(true);
    qreal maxSize = Global::mm2PixelsYF(3000);

    QRectF m_maxRegion = QRectF(rect.left(), rect.top(), maxSize, maxSize);
    
}
LaserBackgroundItem::~LaserBackgroundItem()
{
}

void LaserBackgroundItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	//QGraphicsItemGroup::paint(painter, option, widget);
    

	QPen pen(Qt::black, 1.0f, Qt::SolidLine);
	//this->setBrush(QBrush(Qt::white));
	pen.setCosmetic(true);
    QColor red;
    red.setRed(177);
    painter->setPen(QPen(Qt::red));
    painter->drawRect(m_maxRegion);
	painter->setPen(pen);
	//painter->drawRect(this->rect());
	//QGraphicsItemGroup::setSelected(true);
	//����
	drawGrids(*painter);
	//QGraphicsItemGroup::paint(painter, option, widget);
	/*QList<QGraphicsItem*> list = QGraphicsItemGroup::childItems();
	for (int i = 0; i < list.size(); i++) {
		LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>( list[i]);
		QString name = primitive->metaObject()->className();
		if (name == "LaserRect") {
			primitive->paint(painter, option, widget);
			primitive->setSelected(true);
		}
		
	}*/
	
	
	
}
//���¼���nodeֵ
void LaserBackgroundItem::onChangeGrids()
{
	QGraphicsScene* scene = this->QGraphicsItemGroup::scene();
	if (!scene) {
		return;
	}
	LaserViewer* view = qobject_cast<LaserViewer*>(scene->views()[0]);
	if (!view) {
		return;
	}
	qreal zoomValue = view->zoomValue();
	if (!m_gridNodeXList.isEmpty()) {
		m_gridNodeXList.clear();
	}
	if (!m_gridNodeYList.isEmpty()) {
		m_gridNodeYList.clear();
	}
	if (!m_gridSecondNodeXList.isEmpty()) {
		m_gridSecondNodeXList.clear();
	}
	if (!m_gridSecondNodeYList.isEmpty()) {
		m_gridSecondNodeYList.clear();
	}
	qreal spacing = Config::Ui::visualGridSpacing();//mm
	qreal intervalH = Global::mm2PixelsYF(spacing);
	qreal intervalV = Global::mm2PixelsXF(spacing);
	qreal width = this->rect().width();
	qreal height = this->rect().height();
	qreal sH = height / intervalH;
	qreal sV = width / intervalV;

	int sizeH = qCeil(sH);
	int sizeV = qCeil(sV);
	//�����ֵ�Ƿ񳬹�2������
	qreal diffH = (sH - qFloor(sH)) * intervalH * zoomValue;
	qreal diffV = (sV - qFloor(sV)) * intervalV * zoomValue;
	if (diffH < 2 && diffH > 0) {
		sizeH -= 1;
	}
	if (diffV < 2 && diffV > 0) {
		sizeV -= 1;
	}
	//qDebug() <<"diffH: " << diffH ;
	//qDebug() <<"diffV: " << diffV ;
	int count = 10;
	//���������С
	//qreal intervalH_1 = intervalH * 0.1;
	//qreal intervalV_1 = intervalV * 0.1;
	for (int i = 0; i <= sizeH; i++) {
		//painter.setPen(QPen(QColor(210, 210, 210), 1, Qt::SolidLine));
		qreal startY = intervalH * i;
		m_gridNodeYList.append(startY);
		
		if (zoomValue > 2 && i < sizeH) {
			//painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::SolidLine));
			//int count = 
			for (int ic = 0; ic < count; ic++) {
				qreal intervalH_1 = intervalH / count;
				qreal y_1 = startY + intervalH_1 * ic;
				m_gridSecondNodeYList.append(y_1);
			}

		}
	}
	for (int j = 0; j <= sizeV; j++) {
		qreal startX = intervalV * j;
		m_gridNodeXList.append(startX);

		if (zoomValue > 2 && j < sizeV) {
			//painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::SolidLine));
			for (int jc = 0; jc < count; jc++) {
				qreal intervalV_1 = intervalV / count;
				qreal x_1 = startX + intervalV_1 * jc;
				m_gridSecondNodeXList.append(x_1);
			}
		}
	}
}

void LaserBackgroundItem::drawGrids(QPainter& painter)
{
	qreal width = this->rect().width();
	qreal height = this->rect().height();
	int contrastValue = Config::Ui::gridContrast();
	//qDebug() << "contrastValue: " << contrastValue;
	bool isShow = true;
	//һ������
	QPen pen1(QColor(0, 0, 0), 1, Qt::SolidLine);
	pen1.setCosmetic(true);
	//��������
	QPen pen2(QColor(0, 0, 0), 1, Qt::SolidLine);
	pen2.setCosmetic(true);
	switch (contrastValue) {
		case 0: {
			isShow = false;
			break;
		}
		case 1: {
			pen1.setColor(QColor(Global::lowPen1, Global::lowPen1, Global::lowPen1));
			pen2.setColor(QColor(Global::lowPen2, Global::lowPen2, Global::lowPen2));
			break;
		}
		case 2: {
			pen1.setColor(QColor(Global::mediumPen1, Global::mediumPen1, Global::mediumPen1));
			pen2.setColor(QColor(Global::mediumPen2, Global::mediumPen2, Global::mediumPen2));
			break;
		}
		case 3: {
			pen1.setColor(QColor(Global::highPen1, Global::highPen1, Global::highPen1));
			pen2.setColor(QColor(Global::highPen2, Global::highPen2, Global::highPen2));
			break;
		}

	}
	if (!isShow) {
		return;
	}
	//2������
	painter.setPen(pen2);
	for (int i1 = 0; i1 < m_gridSecondNodeYList.size(); i1++) {
		//QPointF p1SecondH = mapFromScene(mapToScene(QPointF(0, m_gridSecondNodeYList[i1])));
		//QPointF p2SecondH = mapFromScene(mapToScene(QPointF(m_width, m_gridSecondNodeYList[i1])));
		QPointF p1SecondH = QPointF(0, m_gridSecondNodeYList[i1]);
		QPointF p2SecondH = QPointF(width, m_gridSecondNodeYList[i1]);
		painter.drawLine(p1SecondH, p2SecondH);
	}
	for (int j1 = 0; j1 < m_gridSecondNodeXList.size(); j1++) {
		//QPointF p1SecondV = mapFromScene(mapToScene(QPointF(m_gridSecondNodeXList[j1], 0)));
		//QPointF p2SecondV = mapFromScene(mapToScene(QPointF(m_gridSecondNodeXList[j1], m_height)));
		QPointF p1SecondV = QPointF(m_gridSecondNodeXList[j1], 0);
		QPointF p2SecondV = QPointF(m_gridSecondNodeXList[j1], height);
		painter.drawLine(p1SecondV, p2SecondV);
	}
	//1������
	painter.setPen(pen1);
	for (int i = 1; i < m_gridNodeYList.size()-1; i++) {
		//QPointF p1H = mapFromScene(mapToScene(QPointF(0, m_gridNodeYList[i])));
		//QPointF p2H = mapFromScene(mapToScene(QPointF(m_width, m_gridNodeYList[i])));
		QPointF p1H = QPointF(0, m_gridNodeYList[i]);
		QPointF p2H = QPointF(width, m_gridNodeYList[i]);
		painter.drawLine(p1H, p2H);
	}
	for (int j = 1; j < m_gridNodeXList.size()-1; j++) {
		//QPointF p1V = mapFromScene(mapToScene(QPointF(m_gridNodeXList[j], 0)));
		//QPointF p2V = mapFromScene(mapToScene(QPointF(m_gridNodeXList[j], m_height)));
		QPointF p1V = QPointF(m_gridNodeXList[j], 0);
		QPointF p2V = QPointF(m_gridNodeXList[j], height);
		painter.drawLine(p1V, p2V);
	}
	
}
bool LaserBackgroundItem::detectGridNode(QPointF & point, QPointF & mousePoint)
{
	QGraphicsScene* scene = this->scene();
	if (!scene) {
		return false;
	}
	LaserViewer* view = qobject_cast<LaserViewer*>(scene->views()[0]);
	if (!view) {
		return false;
	}
	if (m_gridNodeYList.isEmpty() || m_gridNodeXList.isEmpty()) {
		return false;
	}
	//point��scene��documentת��
	QPointF documentPoint = mapFromScene(mousePoint);
	qreal distance = Config::Ui::gridShapeDistance();
	qreal valueX = distance / view->zoomValue();//5������
	qreal valueY = distance / view->zoomValue();
	for (int i = 0; i < m_gridNodeXList.size(); i++) {
		for (int j = 0; j < m_gridNodeYList.size(); j++) {
			QPointF node = QPointF(m_gridNodeXList[i], m_gridNodeYList[j]);
			qreal absX = qAbs(node.x() - documentPoint.x());
			qreal absY = qAbs(node.y() - documentPoint.y());
			if (absX < valueX && absY < valueY) {
				//node ��documentת����view
				point = mapToScene(node);
				return true;
			}
		}
		for (int j2 = 0; j2 < m_gridSecondNodeYList.size(); j2++) {
			QPointF node = QPointF(m_gridNodeXList[i], m_gridSecondNodeYList[j2]);
			qreal absX = qAbs(node.x() - documentPoint.x());
			qreal absY = qAbs(node.y() - documentPoint.y());
			if (absX < valueX && absY < valueY) {
				//node ��documentת����view
				point = mapToScene(node);
				return true;
			}
		}
	}

	/*for (int i2 = 0; i2 < m_gridSecondNodeXList.size(); i2++) {
		for (int j = 0; j < m_gridNodeYList.size(); j++) {
			QPointF node = QPointF(m_gridSecondNodeXList[i2], m_gridNodeYList[j]);
			qreal absX = qAbs(node.x() - documentPoint.x());
			qreal absY = qAbs(node.y() - documentPoint.y());
			if (absX < valueX && absY < valueY) {
				//node ��documentת����scene
				point = mapToScene(node);
				return true;
			}
		}
		for (int j2 = 0; j2 < m_gridSecondNodeYList.size(); j2++) {
			QPointF node = QPointF(m_gridSecondNodeXList[i2], m_gridSecondNodeYList[j2]);
			qreal absX = qAbs(node.x() - documentPoint.x());
			qreal absY = qAbs(node.y() - documentPoint.y());
			if (absX < valueX && absY < valueY) {
				//node ��documentת����scene
				point = mapToScene(node);
				return true;
			}
		}
	}*/

	return false;
}

QRectF LaserBackgroundItem::rect()
{
	return m_rectItem->rect();
}
