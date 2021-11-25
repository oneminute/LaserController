#include "LaserBackgroundItem.h"
#include <QPainter>
#include <QDebug>
#include <QList>
#include <QGraphicsItemGroup>
#include <QtCore/qmath.h>  
#include <QTransform>
#include <QStyleOptionGraphicsItem>
#include "scene/LaserPrimitive.h"

#include "LaserApplication.h"
#include "widget//LaserViewer.h"
#include "common/common.h"
#include "common/Config.h"
#include "laser/LaserDevice.h"

LaserBackgroundItem::LaserBackgroundItem(QGraphicsItem * parent)
	: QGraphicsItemGroup(parent)
{
	//QRectF rect = LaserApplication::device->layoutRectInScene();
	QRectF rect = LaserApplication::device->layoutRectInDevice();
    qDebug() << "deivce bounds in device:" << rect;

	m_rectItem = new QGraphicsRectItem(rect);
	QGraphicsItemGroup::setFlag(ItemIsSelectable, true);

	QPen pen(Qt::black, 1.0f, Qt::SolidLine);
	pen.setCosmetic(true);
	m_rectItem->setPen(pen);
	addToGroup(m_rectItem);

    connect(LaserApplication::device, &LaserDevice::layoutChanged, this, &LaserBackgroundItem::onLayoutChanged);
}

LaserBackgroundItem::~LaserBackgroundItem()
{
}

void LaserBackgroundItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	QPen pen(Qt::black, 1.0f, Qt::SolidLine);
	//this->setBrush(QBrush(Qt::white));
	pen.setCosmetic(true);
    /*QColor red;
    red.setRed(177);
    painter->setPen(QPen(Qt::red));
    painter->drawRect(m_maxRegion);*/
	painter->setPen(pen);
	//grides
	drawGrids(*painter);
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

	QRect rect = LaserApplication::device->layoutRectInDevice();
	int spacing = Config::Ui::visualGridSpacing() * 1000; //um
	int width = rect.width();
	int height = rect.height();
	int sizeH = height / spacing;
	int sizeV = width / spacing;

	int left = rect.left();
	int right = rect.right();
	int top = rect.top();
	int bottom = rect.bottom();

	int count = 10;
	for (int n = top, i = 0; n <= bottom; n += spacing, i++) {
		int startY = top + spacing * i;
		m_gridNodeYList.append(startY);
		
		if (zoomValue > 0.02 && i < sizeH) {
			for (int ic = 0; ic < count; ic++) {
				int intervalH_1 = spacing / count;
				int y_1 = startY + intervalH_1 * ic;
				m_gridSecondNodeYList.append(y_1);
			}

		}
	}
	for (int n = left, j = 0; n <= right; n += spacing, j++) {
		int startX = left + spacing * j;
		m_gridNodeXList.append(startX);

		if (zoomValue > 0.02 && j < sizeV) {
			//painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::SolidLine));
			for (int jc = 0; jc < count; jc++) {
				int intervalV_1 = spacing / count;
				int x_1 = startX + intervalV_1 * jc;
				m_gridSecondNodeXList.append(x_1);
			}
		}
	}
}

void LaserBackgroundItem::drawGrids(QPainter& painter)
{
	QRect rect = LaserApplication::device->layoutRectInDevice();
	int left = rect.left();
	int right = rect.right();
	int top = rect.top();
	int bottom = rect.bottom();
	int width = rect.width();
	int height = rect.height();
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
		QPointF p1SecondH = mapFromScene(QPoint(left, m_gridSecondNodeYList[i1]));
		QPointF p2SecondH = mapFromScene(QPoint(right, m_gridSecondNodeYList[i1]));
		painter.drawLine(p1SecondH, p2SecondH);
	}
	for (int j1 = 0; j1 < m_gridSecondNodeXList.size(); j1++) {
		QPointF p1SecondV = mapFromScene(QPoint(m_gridSecondNodeXList[j1], top));
		QPointF p2SecondV = mapFromScene(QPoint(m_gridSecondNodeXList[j1], bottom));
		painter.drawLine(p1SecondV, p2SecondV);
	}
	//1������
	painter.setPen(pen1);
	for (int i = 1; i < m_gridNodeYList.size()-1; i++) {
		//QPointF p1H = mapFromScene(mapToScene(QPointF(0, m_gridNodeYList[i])));
		//QPointF p2H = mapFromScene(mapToScene(QPointF(m_width, m_gridNodeYList[i])));
		QPointF p1H = mapFromScene(QPoint(left, m_gridNodeYList[i]));
		QPointF p2H = mapFromScene(QPoint(right, m_gridNodeYList[i]));
		painter.drawLine(p1H, p2H);
	}
	for (int j = 1; j < m_gridNodeXList.size()-1; j++) {
		//QPointF p1V = mapFromScene(mapToScene(QPointF(m_gridNodeXList[j], 0)));
		//QPointF p2V = mapFromScene(mapToScene(QPointF(m_gridNodeXList[j], m_height)));
		QPointF p1V = mapFromScene(QPoint(m_gridNodeXList[j], top));
		QPointF p2V = mapFromScene(QPoint(m_gridNodeXList[j], bottom));
		painter.drawLine(p1V, p2V);
	}
	
}
bool LaserBackgroundItem::detectGridNode(QPoint & point, QPointF & mousePoint)
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
			QPoint node = QPoint(m_gridNodeXList[i], m_gridNodeYList[j]);
			qreal absX = node.x() - documentPoint.x();
			qreal absY = node.y() - documentPoint.y();
			if (absX < valueX && absY < valueY) {
				//node ��documentת����view
				point = mapToScene(node).toPoint();
				return true;
			}
		}
		for (int j2 = 0; j2 < m_gridSecondNodeYList.size(); j2++) {
			QPointF node = QPointF(m_gridNodeXList[i], m_gridSecondNodeYList[j2]);
			qreal absX = node.x() - documentPoint.x();
			qreal absY = node.y() - documentPoint.y();
			if (absX < valueX && absY < valueY) {
				//node ��documentת����view
				point = mapToScene(node).toPoint();
				return true;
			}
		}
	}

	return false;
}

QRectF LaserBackgroundItem::rect()
{
	return m_rectItem->rect();
}

void LaserBackgroundItem::onLayoutChanged(const QSizeF& size)
{
	//QRectF rect = LaserApplication::device->layoutRectInScene();
	QRectF rect = LaserApplication::device->layoutRectInDevice();
    qDebug() << "deivce bounds in device:" << rect;
	m_rectItem->setRect(rect);
}
