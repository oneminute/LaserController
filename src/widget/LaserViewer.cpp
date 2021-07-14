#include "LaserViewer.h"

#include <QBoxLayout>
#include <QGraphicsProxyWidget>
#include <QComboBox>
#include <QPainterPath>
#include <QPalette>
#include <QtMath>
#include <QWheelEvent>
#include <QScrollBar>
#include <QPainterPath>
#include <QLabel> 
#include <QImage>
#include <QVector3D>

#include "scene/LaserPrimitiveGroup.h"
#include "scene/LaserPrimitive.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include "state/StateController.h"
#include "widget/RulerWidget.h"

LaserViewer::LaserViewer(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new LaserScene)
    , m_rubberBandActive(false)
    //, m_mousePressed(false)
    , m_isKeyDelPress(false)
    , m_isKeyShiftPressed(false)
    //, m_isMouseInStartRect(false)
    , m_splineNodeDrawWidth(3)
    , m_splineHandlerWidth(5)
    , m_splineNodeEditWidth(5)
    , m_handlingSpline(SplineStruct())
    , m_lastTime(0)
    , m_curTime(0)
    , m_horizontalRuler(nullptr)
    , m_verticalRuler(nullptr)
	, m_radians(0)
{
    setScene(m_scene.data());
    init();

    Global::dpiX = logicalDpiX();
    Global::dpiY = logicalDpiY();
}

LaserViewer::~LaserViewer()
{
}

void LaserViewer::paintEvent(QPaintEvent* event)
{
	QGraphicsView::paintEvent(event);
	QPainter painter(viewport());
	painter.setRenderHint(QPainter::Antialiasing);
	
	//网格
	drawGrids(painter);
    if (StateControllerInst.isInState(StateControllerInst.documentIdleState()))
    {
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
		
        ///qLogD << "SelectedEditing paint";
    }
	
    //selectioin
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
		if (m_isLeftSelecting) {
			painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
		}
		else {
			painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
		}
        
        painter.drawRect(QRectF(m_selectionStartPoint, m_selectionEndPoint));
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedEditingState())) {
        //qDebug() << "SelectedEditing paint";
		//painter.setRenderHint(QPainter::Antialiasing);
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
		//QGraphicsView::paintEvent(event);
		//QPainter painter(viewport());
		//painter.setRenderHint(QPainter::Antialiasing);
		//painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
		//QRect rect = m_scene->backgroundItem()->boundingRect().toRect();
		//rect = QRect(mapFromScene(rect.topLeft()), mapFromScene(rect.bottomRight()));
		paintSelectedState(painter);
    }
    //Rect
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectCreatingState())) {
		//painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
		if (m_isKeyShiftPressed) {
			qreal width = m_creatingRectEndPoint.x() - m_creatingRectStartPoint.x();
			qreal height = m_creatingRectEndPoint.y() - m_creatingRectStartPoint.y();
			qreal absWidth = qAbs(width);
			qreal absHeight = qAbs(height);
			QPoint point;
			if (absWidth > absHeight) {
				point.setX(m_creatingRectEndPoint.x());
				if (height > 0) {
					point.setY(m_creatingRectStartPoint.y() + absWidth);
				}
				else {
					point.setY(m_creatingRectStartPoint.y() - absWidth);
				}
			}
			else {
				point.setY(m_creatingRectEndPoint.y());
				if (width > 0) {
					point.setX(m_creatingRectStartPoint.x() + absHeight);
				}
				else {
					point.setX(m_creatingRectStartPoint.x() - absHeight);
				}
			}
			m_creatingRectEndPoint = point;
		}
		
        painter.drawRect(QRectF(mapFromScene(m_creatingRectStartPoint), mapFromScene(m_creatingRectEndPoint)));
    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        m_EllipseEndPoint = m_creatingEllipseEndPoint;
        if (m_isKeyShiftPressed) {
            qreal w = m_EllipseEndPoint.x() - m_creatingEllipseStartPoint.x();
            qreal h = m_EllipseEndPoint.y() - m_creatingEllipseStartPoint.y();
            if (qAbs(w) < qAbs(h)) {
                m_EllipseEndPoint = QPointF(m_creatingEllipseStartPoint.x() + w, m_creatingEllipseStartPoint.y() + w);
            }
            else {
                m_EllipseEndPoint = QPointF(m_creatingEllipseStartPoint.x() + h, m_creatingEllipseStartPoint.y() + h);
            }

        }

        painter.drawEllipse(QRectF(mapFromScene(m_creatingEllipseStartPoint), mapFromScene(m_EllipseEndPoint)));
    }
    //Line
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineCreatingState())) {
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        painter.drawLine(mapFromScene(m_creatingLineStartPoint), mapFromScene(m_creatingLineEndPoint));
    }
    //Polygon
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonCreatingState())) {
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
		int size = m_creatingPolygonPoints.size();
		if (size == 0) {
			return;
		}
		else if (size > 1) {
			for (int i = 0; i < size; i++) {
				if (i == size - 1) {
					break;
				}
				painter.drawLine(mapFromScene(m_creatingPolygonPoints[i]), mapFromScene(m_creatingPolygonPoints[i+1]));
			}			
		}
		painter.drawLine(mapFromScene(m_creatingPolygonPoints[size - 1]), mapFromScene(m_creatingPolygonEndPoint));
        /*QRectF rect = QRectF(mapFromScene(m_polygonStartRect.topLeft()), mapFromScene(m_polygonStartRect.bottomRight()));
        if (m_isMouseInStartRect) {
            painter.fillRect(rect, QBrush(Qt::red, Qt::SolidPattern));
        }
        else {
            painter.fillRect(rect, QBrush(Qt::black, Qt::SolidPattern));
        }
        painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        for (int i = 0; i < m_creatingPolygonPoints.length(); i++) {
            QPointF start = mapFromScene(m_creatingPolygonPoints.at(i));
            if (i < m_creatingPolygonPoints.length() - 1) {
                QPointF end = mapFromScene(m_creatingPolygonPoints.at(i + 1));
                painter.drawLine(start, end);
            }
        }
        painter.drawLine(mapFromScene(m_creatingPolygonPoints.at(m_creatingPolygonPoints.length() - 1)), mapFromScene(m_creatingPolygonEndPoint));*/
    }
    //Spline
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineCreatingState())) {
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        QList<SplineNodeStruct>& nodeList = m_handlingSpline.nodeList;
        int nodesLength = nodeList.length();
        for (int i = 0; i < nodesLength; i++) {

            qreal halfWidth = m_splineNodeDrawWidth * 0.5;
            QPointF start(nodeList[i].node.x() - halfWidth, nodeList[i].node.y() - halfWidth);
            QPointF end(nodeList[i].node.x() + halfWidth, nodeList[i].node.y() + halfWidth);

            QRectF rect = QRectF(mapFromScene(start), mapFromScene(end));
            if (rect.contains(m_creatingSplineMousePos)) {
                painter.fillRect(rect, QBrush(Qt::red, Qt::SolidPattern));
                m_mouseHoverRect = rect;
            }
            else {
                painter.fillRect(rect, QBrush(Qt::black, Qt::SolidPattern));
            }
            //line
            if (i < nodesLength - 1) {
                QPointF nextStart(nodeList[i + 1].node.x() - halfWidth, nodeList[i + 1].node.y() - halfWidth);
                QPointF nextEnd(nodeList[i + 1].node.x() + halfWidth, nodeList[i + 1].node.y() + halfWidth);
                QRectF nextRect = QRectF(nextStart.toPoint(), nextEnd.toPoint());
                painter.drawLine(rect.center(), mapFromScene(nextRect.center()));
            }
            else {
                painter.drawLine(rect.center(), m_creatingSplineMousePos);
            }
        }
    }
    //Text
    else {
        if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {
        }
    }
	
	/*if (m_group && !m_group->isEmpty())
	{
		QPainter painter(viewport());
		painter.setPen(QPen(Qt::red, 5, Qt::SolidLine));
		QPointF groupOrigin = this->mapFromScene(m_origin);
		painter.drawEllipse(groupOrigin, 5, 5);
		painter.setPen(QPen(Qt::blue, 5, Qt::SolidLine));
		groupOrigin = this->mapFromScene(m_newOrigin);
		painter.drawEllipse(groupOrigin, 5, 5);

		painter.setPen(QPen(Qt::red, 1, Qt::SolidLine));
		QPolygonF boundingRect = this->mapFromScene(selectedItemsSceneBoundingRect());
		painter.drawPolygon(boundingRect);
		painter.setPen(QPen(Qt::green, 1, Qt::SolidLine));
		boundingRect = this->mapFromScene(m_selectedRect);
		painter.drawPolygon(boundingRect);
		painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
		boundingRect = this->mapFromScene(selectedItemsSceneBoundingRect());
		painter.drawPolygon(boundingRect);
	}*/
}
void LaserViewer::detectRect(LaserPrimitive& item, int i, qreal& left, qreal& right, qreal& top, qreal& bottom) {
	QRectF boundingRect = item.sceneBoundingRect();
	if (i == 0) {
		left = boundingRect.left();
		right = boundingRect.right();
		top = boundingRect.top();
		bottom = boundingRect.bottom();
	}
	else {
		qreal curLeft = boundingRect.left();
		qreal curRight = boundingRect.right();
		qreal curTop = boundingRect.top();
		qreal curBottom = boundingRect.bottom();
		if (curLeft < left) {
			left = curLeft;
		}
		if (curRight > right) {
			right = curRight;
		}
		if (curTop < top) {
			top = curTop;
		}
		if (curBottom > bottom) {
			bottom = curBottom;
		}
	}
}
QRectF LaserViewer::selectedItemsSceneBoundingRect() {
	//QList<QGraphicsItem*> items = m_scene->selectedItems();
	QRectF rect;
	
	QList<LaserPrimitive*> items = m_scene->selectedPrimitives();
	qreal left = 0;
	qreal right = 0;
	qreal top = 0;
	qreal bottom = 0;
	
	if (items.size() == 0 && m_group) {
		QList<QGraphicsItem*> group_items = m_group->childItems();
		if (group_items.size() == 0) {
			return rect;
		}
		for (int i = 0; i < group_items.size(); i++) {
			LaserPrimitive* item = static_cast<LaserPrimitive*>(group_items[i]);
			detectRect(*item, i, left, right, top, bottom);
		}
		rect = QRectF(left, top, right - left, bottom - top);
		return rect;
	}
	if (items.size() == 0 || !m_group) {
		return rect;
	}
	for (int i = 0; i < items.size(); i++) {
		LaserPrimitive* item = items[i];
		detectRect(*item, i, left, right, top, bottom);
	}
	rect = QRectF(left, top, right - left, bottom - top);
	return rect;
}
void LaserViewer::resetSelectedItemsGroupRect(QRectF _sceneRect, qreal _xscale, qreal _yscale, qreal rotate, int _state, int _transformType)
{
	if (m_group && !m_group->isEmpty()) {
		QRectF bounds = selectedItemsSceneBoundingRect();
		QPointF point = _sceneRect.topLeft();
		qreal width = _sceneRect.width();
		qreal height = _sceneRect.height();
		QTransform t = m_group->transform();
		switch (_transformType) {
			case Transform_MOVE: {
				QPointF diff;
				switch (_state)
				{
					case SelectionOriginalTopLeft: {
						diff = point - bounds.topLeft();
						break;
					}
					case SelectionOriginalTopCenter: {
						diff = point - QPointF(bounds.center().x(), bounds.topLeft().y());
						break;
					}
					case SelectionOriginalTopRight: {
						diff = point - QPointF(bounds.right(), bounds.top());
						break;
					}
													
					case SelectionOriginalLeftCenter: {
						diff = point - QPointF(bounds.left(), bounds.center().y());
						break;
					}
					case SelectionOriginalCenter: {
						diff = point - bounds.center();
						break;
					}
					case SelectionOriginalRightCenter: {
						diff = point - QPointF(bounds.right(), bounds.center().y());
						break;
					}
					case SelectionOriginalLeftBottom: {
						diff = point - QPointF(bounds.left(), bounds.bottom());
						break;
					}
					case SelectionOriginalBottomCenter: {
						diff = point - QPointF(bounds.center().x(), bounds.bottom());
						break;
					}
					case SelectionOriginalBottomRight: {
						diff = point - QPointF(bounds.right(), bounds.bottom());
						break;
					}
				}
				t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
				m_group->setTransform(t);
				break;
			}
			case Transform_SCALE: {
				QTransform t1;
				t1.scale(_xscale, _yscale);
				t = t * t1;
				QPointF origi;
				
				switch (_state)
				{
					case SelectionOriginalTopLeft: {
						origi = bounds.topLeft();
						break;
					}
					case SelectionOriginalTopCenter: {
						origi = QPointF(bounds.left(), bounds.top());
						break;
					}
					case SelectionOriginalTopRight: {
						origi = QPointF(bounds.right(), bounds.top());
						break;
					}

					case SelectionOriginalLeftCenter: {
						origi = QPointF(bounds.left(), bounds.center().y());
						break;
					}
					case SelectionOriginalCenter: {
						origi = bounds.center();
						break;
					}
					case SelectionOriginalRightCenter: {
						origi = QPointF(bounds.right(), bounds.center().y());
						break;
					}
					case SelectionOriginalLeftBottom: {
						origi = QPointF(bounds.left(), bounds.bottom());
						break;
					}
					case SelectionOriginalBottomCenter: {
						origi = QPointF(bounds.center().x(), bounds.bottom());
						break;
					}
					case SelectionOriginalBottomRight: {
						origi = bounds.bottomRight();
						break;
					}
				}
				QPointF newOrigi = t1.map(origi);
				QPointF diff = origi - newOrigi;
				QTransform t2;
				t2.translate(diff.x(), diff.y());
				t = t * t2;
				m_group->setTransform(t);
				emit selectedChange();
				break;
			}
			case Transform_RESIZE: {
				qreal rateX = width / bounds.width();
				qreal rateY = height / bounds.height();
				QTransform t1;
				t1.scale(rateX, rateY);
				t = t * t1;
				QPointF original;
				QPointF diff;
				switch (_state)
				{
					case SelectionOriginalTopLeft: {
						original = bounds.topLeft();
						break;
					}
					case SelectionOriginalTopCenter: {
						original = QPointF(bounds.center().x(), bounds.top());
						break;
					}
					case SelectionOriginalTopRight: {
						original = QPointF(bounds.right(), bounds.top());
						break;
					}

					case SelectionOriginalLeftCenter: {
						original = QPointF(bounds.left(), bounds.center().y());
						break;
					}
					case SelectionOriginalCenter: {
						original = bounds.center();
						break;
					}
					case SelectionOriginalRightCenter: {
						original = QPointF(bounds.right(), bounds.center().y());
						break;
					}
					case SelectionOriginalLeftBottom: {
						original = QPointF(bounds.left(), bounds.bottom());
						break;
					}
					case SelectionOriginalBottomCenter: {
						original = QPointF(bounds.center().x(), bounds.bottom());
						break;
					}
					case SelectionOriginalBottomRight: {
						original = bounds.bottomRight();
						break;
					}
				}
				QPointF newOriginal = t1.map(original);
				diff = original - newOriginal;
				t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
				m_group->setTransform(t);
				break;
			}
			case Transform_ROTATE: {
				QTransform t1;
				QTransform t2;
				QPointF original, newOriginal;
				switch (_state)
				{
					case SelectionOriginalTopLeft: {
						original = bounds.topLeft();
						break;
					}
					case SelectionOriginalTopCenter: {
						original = QPointF(bounds.center().x(), bounds.top());
						break;
					}
					case SelectionOriginalTopRight: {
						original = QPointF(bounds.right(), bounds.top());
						break;
					}

					case SelectionOriginalLeftCenter: {
						original = QPointF(bounds.left(), bounds.center().y());
						break;
					}
					case SelectionOriginalCenter: {
						original = bounds.center();
						break;
					}
					case SelectionOriginalRightCenter: {
						original = QPointF(bounds.right(), bounds.center().y());
						break;
					}
					case SelectionOriginalLeftBottom: {
						original = QPointF(bounds.left(), bounds.bottom());
						break;
					}
					case SelectionOriginalBottomCenter: {
						original = QPointF(bounds.center().x(), bounds.bottom());
						break;
					}
					case SelectionOriginalBottomRight: {
						original = bounds.bottomRight();
						break;
					}
				}
				t1.setMatrix(qCos(rotate), qSin(rotate), t1.m13(), -qSin(rotate), qCos(rotate), t.m23(), t1.m31(), t1.m32(), t1.m33());
				newOriginal = t1.map(original);
				QPointF diff = original - newOriginal;
				t2.translate(diff.x(), diff.y());
				t = t * t1 * t2;
				m_group->setTransform(t);
				break;
			}
		}
		
	}
}
void LaserViewer::paintSelectedState(QPainter& painter)
{
	
    qreal left, right, top, bottom;
	QRectF rect = selectedItemsSceneBoundingRect();
	qDebug() << "paintSelectedState: "<<rect;
	left = rect.left();
	right = rect.right();
	top = rect.top();
	bottom = rect.bottom();
    //qLogD << "viewer painter transform: " << painter.transform();
    painter.setPen(QPen(Qt::gray, 0, Qt::SolidLine));
    painter.setBrush(QBrush(Qt::gray));
    QPointF centerPoint(mapFromScene((right - left) * 0.5 + left, (bottom - top) * 0.5 + top));
    qreal halfSize = m_handleRectPixel * 0.5;
    //center(0)
    m_selectedHandleList.clear();
    m_selectedHandleList.append(QRectF(QPointF(centerPoint.x() - halfSize, centerPoint.y() - halfSize), QPointF(centerPoint.x() + halfSize, centerPoint.y() + halfSize)));
    painter.drawRect(m_selectedHandleList[0]);
    QPointF leftTop(mapFromScene(left, top));
    QPointF rightBottom(mapFromScene(right, bottom));
	//m_selectedRect = QRectF(leftTop, rightBottom);
    leftTop.setX(leftTop.x() - 2);
    leftTop.setY(leftTop.y() - 2);
    rightBottom.setX(rightBottom.x() + 2);
    rightBottom.setY(rightBottom.y() + 2);
    //left_top(1,2)
    m_selectedHandleList.append(QRectF(leftTop.x() - m_handleRectPixel, leftTop.y() - m_handleRectPixel, m_handleRectPixel, m_handleRectPixel));
    painter.drawRect(m_selectedHandleList[1]);
    painter.setPen(QPen(Qt::gray, 3, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(leftTop.x() - 15, leftTop.y() - 17, 8, 8));//rotate
                                                                                  //painter.drawRect(m_selectedHandleList[2]);
    painter.drawArc(QRectF(QPointF(leftTop.x() - 17, leftTop.y() - 17), QPointF(leftTop.x() + 10, leftTop.y() + 10)), 90 * 16, 95 * 16);
    painter.setPen(QPen(Qt::gray, 2, Qt::SolidLine));
    painter.drawLine(QPointF(leftTop.x() - 1, leftTop.y() - 17), QPointF(leftTop.x() - 9, leftTop.y() - 19));
    painter.drawLine(QPointF(leftTop.x() - 18, leftTop.y() - 5), QPointF(leftTop.x() - 19, leftTop.y() - 9));
    //top_center(3)
    painter.setPen(QPen(Qt::gray, 0, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(leftTop.x() + (rightBottom.x() - leftTop.x()) * 0.5 - m_handleRectPixel * 0.5, leftTop.y() - m_handleRectPixel, m_handleRectPixel, m_handleRectPixel));
    painter.drawRect(m_selectedHandleList[3]);
    //right_top(4,5)
    painter.setPen(QPen(Qt::gray, 0, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(rightBottom.x(), leftTop.y() - m_handleRectPixel, m_handleRectPixel, m_handleRectPixel));
    painter.drawRect(m_selectedHandleList[4]);
    painter.setPen(QPen(Qt::gray, 3, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(rightBottom.x() + 13, leftTop.y() - 17, 8, 8));//rotate
                                                                                      //painter.drawRect(m_selectedHandleList[5]);
    painter.drawArc(QRectF(QPointF(rightBottom.x() - 10, leftTop.y() - 17), QPointF(rightBottom.x() + 17, leftTop.y() + 10)), 90 * 16, -95 * 16);
    painter.setPen(QPen(Qt::gray, 2, Qt::SolidLine));
    painter.drawLine(QPointF(rightBottom.x() + 4, leftTop.y() - 18), QPointF(rightBottom.x() + 4 + qCos(M_PI / 180 * 13) * 6, leftTop.y() - 18 - qSin(M_PI / 180 * 13) * 6));
    painter.drawLine(QPointF(rightBottom.x() + 18, leftTop.y() - 3), QPointF(rightBottom.x() + 18 + qSin(M_PI / 180 * 13) * 6, leftTop.y() - 3 - qCos(M_PI / 180 * 13) * 6));
    //right_center(6)
    painter.setPen(QPen(Qt::gray, 0, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(rightBottom.x(), leftTop.y() + (rightBottom.y() - leftTop.y()) * 0.5 - m_handleRectPixel * 0.5, m_handleRectPixel, m_handleRectPixel));
    painter.drawRect(m_selectedHandleList[6]);
    //right_bottom(7,8)
    painter.setPen(QPen(Qt::gray, 0, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(rightBottom.x(), rightBottom.y(), m_handleRectPixel, m_handleRectPixel));
    painter.drawRect(m_selectedHandleList[7]);
    painter.setPen(QPen(Qt::gray, 3, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(rightBottom.x() + 12, rightBottom.y() + 12, 8, 8));//rotateת
                                                                                          //painter.drawRect(m_selectedHandleList[8]);
    painter.drawArc(QRectF(QPointF(rightBottom.x() - 10, rightBottom.y() - 10), QPointF(rightBottom.x() + 17, rightBottom.y() + 17)), -90 * 16, 95 * 16);
    painter.drawLine(QPointF(rightBottom.x() + 4, rightBottom.y() + 18), QPointF(rightBottom.x() + 4 + qCos(M_PI / 180 * 13) * 6, rightBottom.y() + 18 + qSin(M_PI / 180 * 13) * 6));
    painter.drawLine(QPointF(rightBottom.x() + 18, rightBottom.y() + 1), QPointF(rightBottom.x() + 19 + qSin(M_PI / 180 * 13) * 7, rightBottom.y() + 1 + qCos(M_PI / 180 * 13) * 7));
    //bottom_center(9)
    painter.setPen(QPen(Qt::gray, 0, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(leftTop.x() + (rightBottom.x() - leftTop.x()) * 0.5 - m_handleRectPixel * 0.5, rightBottom.y(), m_handleRectPixel, m_handleRectPixel));
    painter.drawRect(m_selectedHandleList[9]);
    //left_bottom(10,11)
    painter.setPen(QPen(Qt::gray, 0, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(leftTop.x() - m_handleRectPixel, rightBottom.y(), m_handleRectPixel, m_handleRectPixel));
    painter.drawRect(m_selectedHandleList[10]);
    painter.setPen(QPen(Qt::gray, 3, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(leftTop.x() - 19, rightBottom.y() + 12, 8, 8));//rotate
                                                                                      //painter.drawRect(m_selectedHandleList[11]);
    painter.drawArc(QRectF(QPointF(leftTop.x() - 17, rightBottom.y() + 17), QPointF(leftTop.x() + 10, rightBottom.y() - 10)), -90 * 16, -95 * 16);
    painter.setPen(QPen(Qt::gray, 2, Qt::SolidLine));
    painter.drawLine(QPointF(leftTop.x() - 11, rightBottom.y() + 19), QPointF(leftTop.x() - 11 + qCos(M_PI / 180 * 13) * 6, rightBottom.y() + 19 - qSin(M_PI / 180 * 13) * 6));
    painter.drawLine(QPointF(leftTop.x() - 18, rightBottom.y() + 2), QPointF(leftTop.x() - 18 - qSin(M_PI / 180 * 13) * 7, rightBottom.y() + 2 + qCos(M_PI / 180 * 13) * 7));
    //left_center(12)
    painter.setPen(QPen(Qt::gray, 0, Qt::SolidLine));
    m_selectedHandleList.append(QRectF(leftTop.x() - m_handleRectPixel, leftTop.y() + (rightBottom.y() - leftTop.y()) * 0.5 - m_handleRectPixel * 0.5, m_handleRectPixel, m_handleRectPixel));
    painter.drawRect(m_selectedHandleList[12]);
}

int LaserViewer::setSelectionArea(const QPointF& _startPoint, const QPointF& _endPoint)
{
	//m_selectionStartPoint, m_selectionEndPoint
	QPainterPath selectionPath;
	//QRectF rect = QRectF(mapToScene(_startPoint.toPoint()), mapToScene(_endPoint.toPoint()));
	selectionPath.addRect(QRectF(_startPoint, _endPoint));
	//selectionPath.addRect(rect);
	//m_scene->setSelectionArea(mapToScene(selectionPath));
	qDebug() << "_startPoint: " << _startPoint;
	qDebug() << "_endPoint: " << _endPoint;
	//right select
	if (_endPoint.x() < _startPoint.x()) {
		m_scene->setSelectionArea(mapToScene(selectionPath), Qt::ItemSelectionMode::IntersectsItemShape);
	}
	//left selection
	else if (_endPoint.x() >= _startPoint.x()) {
		m_scene->setSelectionArea(mapToScene(selectionPath), Qt::ItemSelectionMode::ContainsItemBoundingRect);
	}
	return m_scene->selectedPrimitives().size();
}

void LaserViewer::wheelEvent(QWheelEvent* event)
{
	QGraphicsView::wheelEvent(event);
	if (!m_scene->document()) {
		return;
	}
    //qreal wheelZoomValue = qPow(1.2, event->delta() / 240.0);
    qreal wheelZoomValue = 1 + event->delta() / 120.0 * 0.1;
    //qLogD << "wheelZoomValue: " << wheelZoomValue << ", delta: " << event->delta();
    zoomBy(wheelZoomValue);
	onChangeGrids();
}

void LaserViewer::zoomBy(qreal factor)
{
    const qreal currentZoom = zoomValue();
    if ((factor < 1 && currentZoom < 0.01) || (factor > 1 && currentZoom > 10))
        return;
    scale(factor, factor);
    //m_scene->document()->setScale(factor);
    //qDebug() << "scale:" << m_scene->document()->scale();
    emit zoomChanged(mapFromScene(m_scene->backgroundItem()->pos()));
    emit scaleChanged(zoomValue());
}

void LaserViewer::leaveEvent(QEvent* event)
{
    m_horizontalRuler->setIsMarkMouse(false);
    m_verticalRuler->setIsMarkMouse(false);
    m_horizontalRuler->repaint();
    m_verticalRuler->repaint();
}

void LaserViewer::enterEvent(QEvent* event)
{
    m_horizontalRuler->setIsMarkMouse(true);
    m_verticalRuler->setIsMarkMouse(true);
    m_horizontalRuler->repaint();
    m_verticalRuler->repaint();
}

void LaserViewer::mousePressEvent(QMouseEvent* event)
{
// 处理鼠标左键
    if (event->button() == Qt::LeftButton) {
        // 若在DocumentIdle状态下，开始进入选择流程
        if (StateControllerInst.isInState(StateControllerInst.documentIdleState()))
        {
			QGraphicsView::mousePressEvent(event);
            //事件被Item截断
			if (m_scene->mousePressBlock()) {
				return;
			}
            // 获取选框起点
            m_selectionStartPoint = event->pos();
            m_selectionEndPoint = m_selectionStartPoint;
            qDebug() << "begin to select";
            emit beginSelecting();
        }
        // 若在DocumentSelected状态下
        else if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
			
            // 判断是鼠标是否按在选框控制柄上了
            int handlerIndex;
            if (isOnControllHandlers(event->pos(), handlerIndex))
            {
				//m_selectedRect = m_group->sceneBoundingRect();
				m_selectedRect = selectedItemsSceneBoundingRect();
				m_oldTransform = m_group->transform();
				m_rate = 1;
				//QRectF boundRect = m_group->sceneBoundingRect();
				QRectF boundRect = selectedItemsSceneBoundingRect();
				qLogD << "group bounding rect: " << boundRect;
				switch (m_curSelectedHandleIndex)
				{
					case 0: {
						m_origin = m_mousePoint;
					}
					//scale
					case 1: {
						m_origin = boundRect.bottomRight();
						break;
					}
					
					case 4: {
						m_origin = boundRect.bottomLeft();
						break;
					}
					case 7: {
						m_origin = boundRect.topLeft();
						break;
					}
					case 10: {
						m_origin = boundRect.topRight();
						break;
						
					}
					//stecth
					case 3: {//top
						m_origin = boundRect.bottomLeft();
						break;
					}
					case 6: {//right
						m_origin = boundRect.topLeft();
						break;
					}
					case 9: {//bottom
						m_origin = boundRect.topLeft();
						break;
					}
					case 12: {//left
						m_origin = boundRect.topRight();
						break;
					}
					//rotate
					case 2:
					case 5:
					case 8:
					case 11: {
						m_origin = boundRect.center();
						m_newOrigin = m_group->mapFromScene(m_origin);
						break;
					}

				}
				qLogD << "origin: " << m_origin;
				qLogD << "transform: " << m_group->transform();
                emit beginSelectedEditing();
            }
            else
            {
				QGraphicsView::mousePressEvent(event);
				m_selectionStartPoint = event->pos();
				m_selectionEndPoint = m_selectionStartPoint;
				onCancelSelected();
				// 获取选框起点
				
				//viewport()->repaint();
                //m_selectionStartPoint = event->pos();
				//m_selectionEndPoint = m_selectionStartPoint;
                //emit beginSelecting();
            }
        }

        //Rect
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectReadyState()))
        {
			QPointF point = mapToScene(event->pos());
			detectGridNode(point);
            m_creatingRectStartPoint = point;
            m_creatingRectEndPoint = m_creatingRectStartPoint;
            emit creatingRectangle();
        }
        //Ellipse
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseReadyState()))
        {
            m_creatingEllipseStartPoint = mapToScene(event->pos());
            m_creatingEllipseStartInitPoint = m_creatingEllipseStartPoint;
            m_creatingEllipseEndPoint = m_creatingEllipseStartPoint;
			m_EllipseEndPoint = m_creatingEllipseEndPoint;
            emit creatingEllipse();
        }
        //Line
        /*else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineReadyState()))
        {
            m_creatingLineStartPoint = mapToScene(event->pos());
            m_creatingLineEndPoint = m_creatingLineStartPoint;
            
        }*/
        //Polygon Ready
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonReadyState())) {
            //clear
            //m_creatingPolygonPoints.clear();
			//m_creatingPolygonLines.clear();

            //emit creatingPolygonStartRect();
        }
        //Spline
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineReadyState())) {
            initSpline();
            emit creatingSpline();
        }
        //Text
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextReadyState())) {
            m_textInputPoint = mapToScene(event->pos()).toPoint();
            creatTextEdit();
            emit creatingText();
        }
    }
}

void LaserViewer::mouseMoveEvent(QMouseEvent* event)
{
	m_mousePoint = event->pos();
    //ruler
    m_horizontalRuler->setMousePoint(m_mousePoint);
    m_verticalRuler->setMousePoint(m_mousePoint);
    m_horizontalRuler->repaint();
    m_verticalRuler->repaint();
	QGraphicsView::mouseMoveEvent(event);
    
    // 当在DocumentSelecting状态时
    if (StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
		m_selectionEndPoint = m_mousePoint;
		//start right
		if (m_selectionEndPoint.x() < m_selectionStartPoint.x()) {
			m_isLeftSelecting = false;
		}
		else {
			m_isLeftSelecting = true;
		}
		viewport()->repaint();
        return;
    }
    // 当在DocumentSelected状态时
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
		
        int handlerIndex;
        QRectF handlerRect;
        if (isOnControllHandlers(event->pos(), handlerIndex, handlerRect))
        {
            if (handlerRect.contains(m_mousePoint)) {
                m_curSelectedHandleIndex = handlerIndex;
                switch (handlerIndex)
                {
                case 0:
                    this->setCursor(Qt::SizeAllCursor);
                    break;
                case 1:
                case 7: {
                    this->setCursor(Qt::SizeFDiagCursor);
                    break;
                }
                case 4:
                case 10: {
                    this->setCursor(Qt::SizeBDiagCursor);
                    break;
                }
                case 3:
                case 9: {
                    this->setCursor(Qt::SizeVerCursor);
                    break;
                }
                case 6:
                case 12: {
                    this->setCursor(Qt::SizeHorCursor);
                    break;
                }
                case 2: {
                    //QMatrix matrix;
                    //matrix.rotate(-90);
                    QPixmap cMap(":/ui/icons/images/handle_rotate.png");
                    this->setCursor(cMap.scaled(16, 16, Qt::KeepAspectRatio));
                    break;
                }

                case 5: {
                    QMatrix matrix;
                    matrix.rotate(90);
                    QPixmap cMap(":/ui/icons/images/handle_rotate.png");
                    this->setCursor(cMap.scaled(16, 16, Qt::KeepAspectRatio).transformed(matrix));
                    break;
                }
                case 8: {
                    QMatrix matrix;
                    matrix.rotate(180);
                    QPixmap cMap(":/ui/icons/images/handle_rotate.png");
                    this->setCursor(cMap.scaled(16, 16, Qt::KeepAspectRatio).transformed(matrix));
                    break;
                }
                case 11: {
                    QMatrix matrix;
                    matrix.rotate(270);
                    QPixmap cMap(":/ui/icons/images/handle_rotate.png");
                    this->setCursor(cMap.scaled(16, 16, Qt::KeepAspectRatio).transformed(matrix));
                    break;
                }
                default:
                    break;
                }
            }
        }
        else
        {
            m_curSelectedHandleIndex = -1;
            this->setCursor(Qt::ArrowCursor);
        }
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedEditingState())) {
		switch (m_curSelectedHandleIndex) {
			/*case 0: {
				QPointF lastPos = m_selectedHandleList[0].center();
				QPointF offsetPos = m_mousePoint - lastPos;
				//offsetPos = QPoint(0.11, 0.1);
				qDebug() << "offsetPos: " << offsetPos;
				QPointF curHandleTopLeft = m_selectedHandleList[0].topLeft() + offsetPos;
				m_selectedHandleList[0] = QRectF(curHandleTopLeft.x(), curHandleTopLeft.y(), m_selectedHandleList[0].width(), m_selectedHandleList[0].height());
				m_group->setPos(m_group->pos() + offsetPos);
				
				break;
			}*/
			case 0:
			case 1:
			case 3:
			case 4:
			case 6:
			case 7:
			case 9:
			case 10:
			case 12:

			case 2:
			case 5:
			case 8:
			case 11:
			{
				selectedHandleScale();
				emit selectedChange();
				break;
			}
			
		}
		this->viewport()->repaint();
		//QGraphicsView::mouseMoveEvent(event);
        
    }
    //Rect
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectCreatingState())) {
        m_creatingRectEndPoint = mapToScene(m_mousePoint);
		m_creatingRectBeforeShiftPoint = m_creatingRectEndPoint;
		this->viewport()->repaint();
		//QGraphicsView::mouseMoveEvent(event);
        return;

    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
        m_creatingEllipseEndPoint = mapToScene(m_mousePoint);
		this->viewport()->repaint();
        return;
    }
    //Line
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineCreatingState())) {
        m_creatingLineEndPoint = mapToScene(m_mousePoint);
		qreal yLength = qAbs(m_creatingLineStartPoint.x() - m_creatingLineEndPoint.x()) * qTan(45);
        if (m_isKeyShiftPressed) {
            qreal angle = QLineF(m_creatingLineStartPoint, m_creatingLineEndPoint).angle();
            if ((angle >= 0 && angle <= 30) ||
                (angle >= 330 && angle <= 360) ||
                (angle >= 150 && angle <= 210)) {
                m_creatingLineEndPoint = QPointF(m_creatingLineEndPoint.x(), m_creatingLineStartPoint.y());
            }
            else if ((angle > 60 && angle <= 120) ||
                (angle > 240 && angle <= 300)) {
                m_creatingLineEndPoint = QPointF(m_creatingLineStartPoint.x(), m_creatingLineEndPoint.y());
			}
			else if ((angle >= 30 && angle <= 60) || (angle >= 120 && angle <= 150)) {
				m_creatingLineEndPoint = QPointF(m_creatingLineEndPoint.x(), m_creatingLineStartPoint.y()-yLength);
			}
			else if ((angle >= 210 && angle <= 240) || (angle >= 300 && angle <= 330)) {
				m_creatingLineEndPoint = QPointF(m_creatingLineEndPoint.x(), m_creatingLineStartPoint.y() + yLength);
			}
        }
		this->viewport()->repaint();
        return;
    }
    //Polygon
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonCreatingState())) {

        //m_isMouseInStartRect = m_polygonStartRect.contains(mapToScene(event->pos()));
        m_creatingPolygonEndPoint = mapToScene(m_mousePoint);

		if (detectPoint(m_creatingPolygonPoints, m_creatingPolygonLines, m_creatingPolygonEndPoint)) {
			this->setCursor(Qt::CrossCursor);
		}
		else {
			this->setCursor(Qt::ArrowCursor);
			//detectLine(m_creatingPolygonLines, m_creatingPolygonEndPoint);

		}
		//;
		
		this->viewport()->repaint();
		return;
    }
    //Spline
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineCreatingState())) {
        m_creatingSplineMousePos = event->pos();

    }
  
    QPointF pos = mapToScene(m_mousePoint);
    emit mouseMoved(pos);
}

void LaserViewer::mouseReleaseEvent(QMouseEvent* event)
{
	QGraphicsView::mouseReleaseEvent(event);
    //select
    if (StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
		
        if (checkTwoPointEqueal(m_selectionStartPoint, m_selectionEndPoint))
        {
            m_scene->clearSelection();
            emit cancelSelecting();
            m_isKeyShiftPressed = false;
            viewport()->repaint();
            return;
        }
		int selectedCount = setSelectionArea(m_selectionStartPoint, m_selectionEndPoint);
		if (selectedCount <= 0) {
			m_scene->clearSelection();
			emit cancelSelecting();
			m_isKeyShiftPressed = false;
			viewport()->repaint();
			return;
		}
		onEndSelecting();
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedEditingState())) {
		
		m_selectedEditCount = 0;
		m_radians = 0;
        emit endSelectedEditing();
    }

    //Rect
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectCreatingState())) {
		if (checkTwoPointEqueal(m_creatingRectStartPoint, m_creatingRectEndPoint)) {
			emit readyRectangle();
			return;
		}
        QRectF rect(m_creatingRectStartPoint, m_creatingRectEndPoint);
        LaserRect* rectItem = new LaserRect(rect, m_scene->document());
        m_scene->addLaserPrimitive(rectItem);
		//m_scene->addItem(rectItem);
		onReplaceGroup(rectItem);
		viewport()->repaint();
        emit readyRectangle();
    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
		if (checkTwoPointEqueal(m_creatingEllipseStartPoint, m_EllipseEndPoint)) {
			emit readyEllipse();
			return;
		}
        QRectF rect(m_creatingEllipseStartPoint, m_EllipseEndPoint);
        LaserEllipse* ellipseItem = new LaserEllipse(rect, m_scene->document());
        m_scene->addLaserPrimitive(ellipseItem);
		onReplaceGroup(ellipseItem);
        emit readyEllipse();
    }
    //Line
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineReadyState()))
	{
		m_creatingLineStartPoint = mapToScene(event->pos());
		m_creatingLineEndPoint = m_creatingLineStartPoint;

		emit creatingLine();

	}
    
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineCreatingState())) {
		if (m_creatingLineStartPoint != m_creatingLineEndPoint) {
			if (event->button() == Qt::LeftButton) {
				QLineF line(m_creatingLineStartPoint, m_creatingLineEndPoint);
				LaserLine* lineItem = new LaserLine(line, m_scene->document());
				m_scene->addLaserPrimitive(lineItem);
				onReplaceGroup(lineItem);
				emit readyLine();
			}
			else if (event->button() == Qt::RightButton) {
				emit readyLine();
			}
		}
	}
    //Polygon Start
    /*else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonStartRectState())) {
        //init
        m_creatingPolygonStartPoint = mapToScene(event->pos());
        m_creatingPolygonEndPoint = m_creatingPolygonStartPoint;
        m_creatingPolygonPoints.append(m_creatingPolygonStartPoint);

        qreal width = 5;
        qreal halfWidth = width * 0.5;
        m_polygonStartRect = QRectF(m_creatingPolygonStartPoint.x() - halfWidth, m_creatingPolygonStartPoint.y() - halfWidth, width, width);

        emit creatingPolygon();
    }*/
    //Polygon
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonReadyState())) {
		if (event->button() == Qt::LeftButton) {
			m_creatingPolygonStartPoint = mapToScene(event->pos());
			m_creatingPolygonEndPoint = m_creatingPolygonStartPoint;
			m_creatingPolygonPoints.append(m_creatingPolygonStartPoint);
			
			emit creatingPolygon();
		}
		else if (event->button() == Qt::RightButton) {
			emit readyPolygon();
		}
	}
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonCreatingState())) {
		
		if (event->button() == Qt::LeftButton) {
			//if()
			m_creatingPolygonLines.append(QLineF(m_creatingPolygonPoints[m_creatingPolygonPoints.size() - 1], m_creatingPolygonEndPoint));
			m_creatingPolygonPoints.append(m_creatingPolygonEndPoint);
			
			if (!isAllPolygonStartPoint()) {
				if (m_creatingPolygonEndPoint == m_creatingPolygonStartPoint) {
					LaserPolygon* polygon = new LaserPolygon(QPolygonF(m_creatingPolygonPoints), m_scene->document());
					m_scene->addLaserPrimitive(polygon);
					onReplaceGroup(polygon);
					m_creatingPolygonPoints.clear();
					setCursor(Qt::ArrowCursor);
					emit readyPolygon();
				}
			}
			
			
		}
		else if (event->button() == Qt::RightButton) {
			if (!isAllPolygonStartPoint()) {
				LaserPolyline* polyLine = new LaserPolyline(QPolygonF(m_creatingPolygonPoints), m_scene->document());
				m_scene->addLaserPrimitive(polyLine);
				onReplaceGroup(polyLine);
			}
			
			m_creatingPolygonPoints.clear();
			setCursor(Qt::ArrowCursor);
			emit readyPolygon();
		}
    }
    //Spline
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineCreatingState())) {
        if (event->button() == Qt::LeftButton) {
            m_creatingSplineMousePos = event->pos();
            if (m_mouseHoverRect.contains(m_creatingSplineMousePos)) {
                m_creatingSplineMousePos = m_mouseHoverRect.center();
            }

            m_handlingSpline.nodeList.append(SplineNodeStruct(mapToScene(m_creatingSplineMousePos.toPoint())));
        }
        else if (event->button() == Qt::RightButton) {
            createSpline();
            emit readySpline();
        }

    }
    //Spline Eidt
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineEditState())) {
        if (m_scene->selectedItems().length() == 1) {
            LaserPath* path = (LaserPath*)m_scene->selectedItems()[0];
            path->setVisible(false);
        }
    }
    else
    {
        
    }
	
    //m_mousePressed = false;
    m_isKeyShiftPressed = false;
    this->viewport()->repaint();
	//QGraphicsView::mouseReleaseEvent(event);
}

void LaserViewer::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
		case Qt::Key_Shift: {
			m_isKeyShiftPressed = true;
			viewport()->repaint();
			break;
		}
        
		case Qt::Key_Delete: {
			m_isKeyDelPress = true;
			break;
		}
        
    }
    QGraphicsView::keyPressEvent(event);
}

void LaserViewer::keyReleaseEvent(QKeyEvent* event)
{

    switch (event->key())
    {
		case Qt::Key_Escape:
			if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {

				releaseTextEdit();
				emit readyText();
			}
			break;
		case Qt::Key_Shift: {
			m_isKeyShiftPressed = false;
			m_creatingRectEndPoint = m_creatingRectBeforeShiftPoint;
			viewport()->repaint();
			break;
		}
        
		case Qt::Key_Delete: {
			m_isKeyDelPress = false;
			break;
		}
        
    }
    QGraphicsView::keyReleaseEvent(event);
}

void LaserViewer::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    if (m_horizontalRuler != nullptr) {
        m_horizontalRuler->repaint();
    }
    if (m_verticalRuler != nullptr) {
        m_verticalRuler->repaint();
    }
}

bool LaserViewer::isOnControllHandlers(const QPoint& point, int& handlerIndex, QRectF& handlerRect)
{
    bool isIn = false;
    for (int i = 0; i < m_selectedHandleList.size(); i++) {
        if (m_selectedHandleList[i].contains(point)) {
            handlerIndex = i;
            handlerRect = m_selectedHandleList[i];
            isIn = true;
            break;
        }
    }
    return isIn;
}

void LaserViewer::clearGroup()
{
	m_group = nullptr;
}

bool LaserViewer::checkTwoPointEqueal(const QPointF & point1, const QPointF & point2)
{
	qreal distance = (point2 - point1).manhattanLength();
	if (distance <= 0.000001f)
	{
		return true;
	}
	return false;
}

bool LaserViewer::detectPoint(QVector<QPointF> points, QList<QLineF> lines, QPointF& point)
{
	for each (QPointF p in points)
	{
		QRectF rect(QPointF(p.x() - 2, p.y() - 2), QPointF(p.x() + 2, p.y() + 2));
		if (rect.contains(point)) {
			point = p;
			return true;
		}
	}
	for each (QLineF line in lines)
	{
		QRectF rect(QPointF(line.center().x() - 3, line.center().y() - 3), QPointF(line.center().x() + 3, line.center().y() + 3));
		if (rect.contains(point)) {
			point = line.center();
			return true;
		}
	}
	return false;
}

bool LaserViewer::detectLine(QList<QLineF> lines, QPointF startPoint, QPointF point)
{
	for each(QLineF line in lines) {
		
		QPointF p1(line.p1());
		QPointF p2(line.p2());
		QVector2D standarVec(p2 - p1);
		standarVec.normalize();
		QVector2D vector(point - startPoint);
		vector.normalize();
		//QRectF rect(;
		if (standarVec == vector) {
			setCursor(Qt::IBeamCursor);
			return true;
		}
	}
	setCursor(Qt::ArrowCursor);
	return false;
}

bool LaserViewer::isAllPolygonStartPoint()
{
	bool bl = true;
	for (int i = 0; i < m_creatingPolygonPoints.size(); i++)
	{
		if (m_creatingPolygonPoints[i] != m_creatingPolygonStartPoint) {
			bl = false;
		}
	}
	return bl;
}

void LaserViewer::onChangeGrids()
{
	QGraphicsRectItem* backgroudItem = this->scene()->backgroundItem();
	if (!backgroudItem) {
		return;
	}
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
	qreal intervalH = Global::mm2PixelsYF(10);
	qreal intervalV = Global::mm2PixelsXF(10);
	qreal width = backgroudItem->boundingRect().width();
	qreal height = backgroudItem->boundingRect().height();
	int sizeH = height / intervalH;
	int sizeV = width / intervalV;
	int count = 10;
	for (int i = 0; i <= sizeH; i++) {
		//painter.setPen(QPen(QColor(210, 210, 210), 1, Qt::SolidLine));
		qreal startY = intervalH * i;
		m_gridNodeYList.append(startY);

		if (zoomValue() > 2 && i < sizeH) {
			//painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::SolidLine));
			for (int ic = 1; ic < count; ic++) {
				qreal intervalH_1 = intervalH / count;
				qreal y_1 = startY + intervalH_1 * ic;
				m_gridSecondNodeYList.append(y_1);
			}

		}
	}
	for (int j = 0; j <= sizeV; j++) {
		qreal startX = intervalV * j;
		m_gridNodeXList.append(startX);
		
		if (zoomValue() > 2 && j < sizeV) {
			//painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::SolidLine));
			for (int jc = 0; jc < count; jc++) {
				qreal intervalV_1 = intervalV / count;
				qreal x_1 = startX + intervalV_1 * jc;
				m_gridSecondNodeXList.append(x_1);
			}
		}
	}
}

void LaserViewer::drawGrids(QPainter& painter)
{
	QGraphicsRectItem* backgroudItem = this->scene()->backgroundItem();
	if (!backgroudItem) {
		return;
	}
	qreal width = backgroudItem->boundingRect().width();
	qreal height = backgroudItem->boundingRect().height();
	painter.setPen(QPen(QColor(210, 210, 210), 1, Qt::SolidLine));
	for (int i = 1; i < m_gridNodeYList.size(); i++) {
		QPointF p1H = mapFromScene(backgroudItem->mapToScene(QPointF(0, m_gridNodeYList[i])));
		QPointF p2H = mapFromScene(backgroudItem->mapToScene(QPointF(width, m_gridNodeYList[i])));
		painter.drawLine(p1H, p2H);
	}
	for (int j = 1; j < m_gridNodeXList.size(); j++) {
		QPointF p1V = mapFromScene(backgroudItem->mapToScene(QPointF(m_gridNodeXList[j], 0)));
		QPointF p2V = mapFromScene(backgroudItem->mapToScene(QPointF(m_gridNodeXList[j], height)));
		painter.drawLine(p1V, p2V);
	}
	//2级网格
	painter.setPen(QPen(QColor(238, 238, 238), 1, Qt::SolidLine));
	for (int i1 = 1; i1 < m_gridSecondNodeYList.size(); i1++) {
		QPointF p1SecondH = mapFromScene(backgroudItem->mapToScene(QPointF(0, m_gridSecondNodeYList[i1])));
		QPointF p2SecondH = mapFromScene(backgroudItem->mapToScene(QPointF(width, m_gridSecondNodeYList[i1])));
		painter.drawLine(p1SecondH, p2SecondH);
	}
	for (int j1 = 1; j1 < m_gridSecondNodeXList.size(); j1++) {
		QPointF p1SecondV = mapFromScene(backgroudItem->mapToScene(QPointF(m_gridSecondNodeXList[j1], 0)));
		QPointF p2SecondV = mapFromScene(backgroudItem->mapToScene(QPointF(m_gridSecondNodeXList[j1], height)));
		painter.drawLine(p1SecondV, p2SecondV);
	}
}

bool LaserViewer::detectGridNode(QPointF & point)
{
	if (m_gridNodeYList.isEmpty() || m_gridNodeXList.isEmpty()) {
		return false;
	}
	//pont从view到document转换
	QGraphicsRectItem* backgroundItem = m_scene->backgroundItem();
	if (!backgroundItem) {
		return false;
	}
	QPointF documentPoint = backgroundItem->mapFromScene(point);
	qreal valueX = 5 / zoomValue();//5个像素
	qreal valueY = 5 / zoomValue();
	for (int i = 0; i < m_gridNodeXList.size(); i++) {
		for (int j = 0; j < m_gridNodeYList.size(); j++) {
			QPointF node = QPointF(m_gridNodeXList[i], m_gridNodeYList[j]);
			qreal absX = qAbs(node.x() - documentPoint.x());
			qreal absY = qAbs(node.y() - documentPoint.y());
			if (absX < valueX && absY < valueY) {
				//node 从document转换到view
				point = backgroundItem->mapToScene(node);
				return true;
			}
		}
		for (int j2 = 0; j2 < m_gridSecondNodeYList.size(); j2++) {
			QPointF node = QPointF(m_gridNodeXList[i], m_gridSecondNodeYList[j2]);
			qreal absX = qAbs(node.x() - documentPoint.x());
			qreal absY = qAbs(node.y() - documentPoint.y());
			if (absX < valueX && absY < valueY) {
				//node 从document转换到view
				point = backgroundItem->mapToScene(node);
				return true;
			}
		}
	}

	for (int i2 = 0; i2 < m_gridSecondNodeXList.size(); i2++) {
		for (int j = 0; j < m_gridNodeYList.size(); j++) {
			QPointF node = QPointF(m_gridSecondNodeXList[i2], m_gridNodeYList[j]);
			qreal absX = qAbs(node.x() - documentPoint.x());
			qreal absY = qAbs(node.y() - documentPoint.y());
			if (absX < valueX && absY < valueY) {
				//node 从document转换到view
				point = backgroundItem->mapToScene(node);
				return true;
			}
		}
		for (int j2 = 0; j2 < m_gridSecondNodeYList.size(); j2++) {
			QPointF node = QPointF(m_gridSecondNodeXList[i2], m_gridSecondNodeYList[j2]);
			qreal absX = qAbs(node.x() - documentPoint.x());
			qreal absY = qAbs(node.y() - documentPoint.y());
			if (absX < valueX && absY < valueY) {
				//node 从document转换到view
				point = backgroundItem->mapToScene(node);
				return true;
			}
		}
	}

	return false;
}

qreal LaserViewer::zoomValue() const
{
    return transform().m11();
}

void LaserViewer::setZoomValue(qreal zoomValue)
{
    scale(1 / transform().m11(), 1 / transform().m22());
    scale(zoomValue, zoomValue);
    emit scaleChanged(zoomValue);
}

void LaserViewer::init()
{
    setRubberBandSelectionMode(Qt::ItemSelectionMode::IntersectsItemBoundingRect);
    setInteractive(true);
    setMouseTracking(true);

    ADD_TRANSITION(documentIdleState, documentSelectingState, this, &LaserViewer::beginSelecting);

    ADD_TRANSITION(documentSelectionState, documentIdleState, this, &LaserViewer::cancelSelecting);
    ADD_TRANSITION(documentSelectionState, documentIdleState, this, &LaserViewer::cancelSelected);

    ADD_TRANSITION(documentSelectingState, documentSelectedState, this, &LaserViewer::endSelecting);

    ADD_TRANSITION(documentSelectedState, documentSelectingState, this, &LaserViewer::beginSelecting);
    ADD_TRANSITION(documentSelectedState, documentSelectedEditingState, this, &LaserViewer::beginSelectedEditing);
	ADD_TRANSITION(documentIdleState, documentSelectedState, this, &LaserViewer::endSelecting);

    ADD_TRANSITION(documentSelectedEditingState, documentSelectedState, this, &LaserViewer::endSelectedEditing);

    //ADD_TRANSITION(documentSelectedState, documentIdleState, this, &LaserViewer::cancelSelecting);
    //ADD_TRANSITION(documentSelectingState, documentIdleState, this, &LaserViewer::cancelSelecting);
    //ADD_TRANSITION(documentSelectedState, documentIdleState, this, &LaserViewer::cancelSelected);

    ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveRectCreatingState, this, SIGNAL(creatingRectangle()));
    ADD_TRANSITION(documentPrimitiveRectCreatingState, documentPrimitiveRectReadyState, this, SIGNAL(readyRectangle()));
    ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveEllipseCreatingState, this, SIGNAL(creatingEllipse()));
    ADD_TRANSITION(documentPrimitiveEllipseCreatingState, documentPrimitiveEllipseReadyState, this, SIGNAL(readyEllipse()));
    ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveLineCreatingState, this, SIGNAL(creatingLine()));
    ADD_TRANSITION(documentPrimitiveLineCreatingState, documentPrimitiveLineReadyState, this, SIGNAL(readyLine()));
    ADD_TRANSITION(documentPrimitivePolygonReadyState, documentPrimitivePolygonCreatingState, this, SIGNAL(creatingPolygon()));
    ADD_TRANSITION(documentPrimitivePolygonCreatingState, documentPrimitivePolygonReadyState, this, SIGNAL(readyPolygon()));
    //ADD_TRANSITION(documentPrimitivePolygonReadyState, documentPrimitivePolygonStartRectState, this, SIGNAL(creatingPolygonStartRect()));
    ADD_TRANSITION(documentPrimitiveSplineReadyState, documentPrimitiveSplineCreatingState, this, SIGNAL(creatingSpline()));
    ADD_TRANSITION(documentPrimitiveSplineCreatingState, documentPrimitiveSplineReadyState, this, SIGNAL(readySpline()));
    ADD_TRANSITION(documentPrimitiveTextReadyState, documentPrimitiveTextCreatingState, this, SIGNAL(creatingText()));
    ADD_TRANSITION(documentPrimitiveTextCreatingState, documentPrimitiveTextReadyState, this, SIGNAL(readyText()));

	m_group = nullptr;
}

void LaserViewer::initSpline()
{
    m_handlingSpline = SplineStruct();
    m_mouseHoverRect = QRectF();
}

void LaserViewer::creatTextEdit()
{
    m_textEdit = new QTextEdit();
    QPoint startPoint = QPoint(m_textInputPoint.x(), m_textInputPoint.y());
    QGraphicsProxyWidget* proxy = m_scene->addWidget(m_textEdit);
    proxy->setPos(startPoint);
    m_textEdit->setFocus();
    m_textEdit->resize(24, 42);
    m_textEdit->setStyleSheet("background-color : rgb(0,255,0,0%);");
    m_textEdit->setFrameShape(QFrame::NoFrame);
    m_textEdit->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_textEdit->document(), SIGNAL(contentsChanged()), SLOT(textAreaChanged()));
}

void LaserViewer::releaseTextEdit()
{
    QPoint end(m_textInputPoint.x() + m_textEdit->document()->size().width(), m_textInputPoint.y() + m_textEdit->document()->size().height());
    QRect rect(m_textInputPoint, end);
    LaserText* text = new LaserText(rect, m_textEdit->toHtml(), m_scene->document(), LaserPrimitiveType::LPT_TEXT);
    m_scene->addLaserPrimitive(text);
    delete m_textEdit;
}

void LaserViewer::selectedHandleRotate() {

}
void LaserViewer::onEndSelecting() {
	onSelectedFillGroup();
	emit endSelecting();

}
void LaserViewer::onSelectedFillGroup()
{
	if (m_scene->selectedItems().size() == 0) {
		m_scene->clearSelection();
		emit cancelSelecting();
		m_isKeyShiftPressed = false;
		viewport()->repaint();
		return;
	}
	if (m_group)
	{
		for (LaserPrimitive* item : m_scene->selectedPrimitives())
		{
			if (m_group->childItems().contains(item))
				continue;
			m_group->addToGroup(item);
		}
		m_group->setSelected(true);
	}
	else
	{
		m_group = m_scene->createItemGroup(m_scene->selectedPrimitives());
		m_group->setFlag(QGraphicsItem::ItemIsSelectable, true);
		m_group->setSelected(true);

	}
	//m_scene->addItem(m_group);
	//绘制操作柄之前先清理一下
	m_selectedHandleList.clear();
	m_curSelectedHandleIndex = -1;
}

void LaserViewer::onReplaceGroup(LaserPrimitive* item)
{
	
	onCancelSelected();
	item->setSelected(true);
	//onSelectedFillGroup();
	onEndSelecting();
}

void LaserViewer::selectedHandleScale()
{
	if (m_selectedEditCount == 0) {
		m_lastPos = m_mousePoint;
	}
	m_selectedEditCount++;
	qreal rate = 1;
	qreal xRate = 1;
	qreal yRate = 1;
	qreal scaleSpeed = 0.0000015;
	switch (m_curSelectedHandleIndex) {
		case 0: {
			break;
		}
		//scale
		case 1: {
			QPointF lastVect = m_lastPos - mapFromScene(m_selectedRect.bottomRight());
			QPointF currVect = m_mousePoint - mapFromScene(m_selectedRect.bottomRight());
			
			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			rate = v2.length() / v1.length();
			break;
		}
		case 4: {
			QPointF lastVect = m_lastPos - mapFromScene(m_selectedRect.bottomLeft());
			QPointF currVect = m_mousePoint - mapFromScene(m_selectedRect.bottomLeft());

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			rate = v2.length() / v1.length();
			break;
		}
		case 7: {
			QPointF lastVect = m_lastPos - m_selectedRect.topLeft();
			QPointF currVect = m_mousePoint - m_selectedRect.topLeft();
			
			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			rate = v2.length() / v1.length();
			
			break;
		}
		case 10: {
			QPointF lastVect = m_lastPos - mapFromScene(m_selectedRect.topRight());
			QPointF currVect = m_mousePoint - mapFromScene(m_selectedRect.topRight());

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			rate = v2.length() / v1.length();
			break;
		}
		//stretch
		case 3: {
			qreal bottom = mapFromScene(m_selectedRect.bottomLeft()).y();
			QPointF lastVect = m_lastPos - QPointF(m_lastPos.x(), bottom);
			QPointF currVect = m_mousePoint - QPointF(m_mousePoint.x(), bottom);

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			rate = v2.length() / v1.length();
			break;
		}
		case 9: {
			qreal top = mapFromScene(m_selectedRect.topLeft()).y();
			QPointF lastVect = m_lastPos - QPointF(m_lastPos.x(), top);
			QPointF currVect = m_mousePoint - QPointF(m_mousePoint.x(), top);

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			rate = v2.length() / v1.length();
			break;
		}
		case 6: {
			qreal left = mapFromScene(m_selectedRect.topLeft()).x();
			QPointF lastVect = m_lastPos - QPointF(left, m_lastPos.y());
			QPointF currVect = m_mousePoint - QPointF(left, m_mousePoint.y());

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			rate = v2.length() / v1.length();
			break;
		}
		case 12: {
			qreal right = mapFromScene(m_selectedRect.topRight()).x();
			QPointF lastVect = m_lastPos - QPointF(right, m_lastPos.y());
			QPointF currVect = m_mousePoint - QPointF(right, m_mousePoint.y());

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			rate = v2.length() / v1.length();
			break;
		}
		case 2:
		case 5:
		case 8:
		case 11: {

			break;
		}
	}
	//qreal oldScaleX = m_oldTransform.m11();
	//qreal oldScaleY = m_oldTransform.m22();
	qreal oldDx = m_oldTransform.dx();
	qreal oldDy = m_oldTransform.dy();

	QTransform t;
	switch (m_curSelectedHandleIndex) {
		case 0: {
			//QPointF diff = m_group->mapFromScene(this->mapToScene(QPointF(m_mousePoint - m_origin).toPoint()));
			QPointF diff = (m_mousePoint - m_lastPos) ;
			t = m_group->transform();
			QTransform t1;
			t1.translate(diff.x() / zoomValue(), diff.y() / zoomValue());
			//t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), diff.x()+ t.dx(), diff.y()+ t.dy(), t.m33());
			m_group->setTransform(t * t1);
			break;
		}
		case 1:
		case 4:
		case 7:
		case 10: {
			m_rate *= rate;
			m_newOrigin = m_origin * m_rate;
			QPointF diff = m_origin - m_newOrigin;
			t.scale(m_rate, m_rate);
			t = m_oldTransform * t;
			t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), diff.x() + oldDx * m_rate, diff.y() + oldDy * m_rate, t.m33());
			m_group->setTransform(t);
			break;
		}
		case 3: 
		case 9: {
			m_rate *= rate;
			m_newOrigin = m_origin * m_rate;
			QPointF diff = m_origin - m_newOrigin;
			t.scale(1, m_rate);
			t = m_oldTransform * t;
			t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), oldDx * 1, diff.y() + oldDy * m_rate, t.m33());
			m_group->setTransform(t);
			break;
		}
		case 12:
		case 6: {
			m_rate *= rate;
			m_newOrigin = m_origin * m_rate;
			QPointF diff = m_origin - m_newOrigin;
			t.scale(m_rate, 1);
			t = m_oldTransform * t;
			t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), diff.x() + oldDx * m_rate, oldDy * 1, t.m33());
			m_group->setTransform(t);
			break;
		}
		case 2:
		case 5:
		case 8:
		case 11: {
			//QPointF vec2 = m_mousePoint - mapFromScene(m_group->sceneBoundingRect().center());
			//QPointF vec1 = m_lastPos - mapFromScene(m_group->sceneBoundingRect().center());
			QPointF vec2 = m_mousePoint - mapFromScene(selectedItemsSceneBoundingRect().center());
			QPointF vec1 = m_lastPos - mapFromScene(selectedItemsSceneBoundingRect().center());
			QVector2D v1(vec1);
			QVector2D v2(vec2);
			v1.normalize();
			v2.normalize();
			qreal radians = qAcos(QVector2D::dotProduct(v1, v2));
			if (QVector3D::crossProduct(QVector3D(v1, 0), QVector3D(v2, 0)).z() < 0) {
				radians = -radians;
			}
			m_radians += radians;
			//QPointF diff = m_origin - m_group->sceneBoundingRect().center();
			//t.setMatrix(qCos(radians)+t.m11(), qSin(radians) + t.m12(), t.m13(), t.m21()-qSin(radians), t.m21()+qCos(radians), t.m23(), t.m31()+ diff.x(), t.m32()+ diff.y(), t.m33());
			qDebug() << "m_radians: " << m_radians;
			t = m_group->transform();
			QTransform t1;
			t1.setMatrix(qCos(radians), qSin(radians), t1.m13(), -qSin(radians), qCos(radians), t.m23(), t1.m31(), t1.m32(), t1.m33());
			m_newOrigin = m_origin * t1;
			//m_group->setTransform(t * t1);
			QTransform t2;
			//m_newOrigin = m_group->sceneBoundingRect().center();
			QPointF diff = m_origin - m_newOrigin;
			//QPointF diff = QPointF(0.1, 0.1);
			t2.setMatrix(t2.m11(), t2.m12(), t2.m13(), t2.m21(), t2.m22(), t2.m23(), diff.x(), diff.y(), t2.m33());
			m_group->setTransform(t * t1 * t2);
			m_origin = m_origin * t1 * t2;
			break;
		}
	}
	
	qLogD << "rate: " << rate << ", transform: " << t;
	m_lastPos = m_mousePoint;
	this->repaint();
}

void LaserViewer::createSpline()
{
    //draw
    qreal length = m_handlingSpline.nodeList.length();
    if (length <= 1) {
        if (m_splineList.length() > 0) {
            m_handlingSpline = m_splineList[m_splineList.length() - 1];
        }
        return;
    }

    QPainterPath path(m_handlingSpline.nodeList[0].node.toPoint());
    for (int i = 0; i < length; i++) {
        SplineNodeStruct nodeStruct = m_handlingSpline.nodeList[i];
        path.lineTo(nodeStruct.node.toPoint());
    }

    LaserPath* laserPath = new LaserPath(path, m_scene->document());
    m_scene->addLaserPrimitive(laserPath);
    m_handlingSpline.objectName = laserPath->objectName();
    m_splineList.append(m_handlingSpline);
    initSpline();
    qDebug() << "m_splineList length: " << m_splineList.length();
}

LaserScene* LaserViewer::scene()
{
    return this->m_scene.data();
}

void LaserViewer::setHorizontalRuler(RulerWidget* _r)
{
    m_horizontalRuler = _r;
}

void LaserViewer::setVerticalRuler(RulerWidget* _r)
{
    m_verticalRuler = _r;
}

LaserPrimitiveGroup* LaserViewer::group()
{
	return m_group;
}

void LaserViewer::zoomIn()
{
    zoomBy(2);
}

void LaserViewer::zoomOut()
{
    zoomBy(0.5);
}

void LaserViewer::resetZoom()
{
    if (!qFuzzyCompare(zoomValue(), qreal(1))) {
        resetTransform();
        emit zoomChanged(mapFromScene(m_scene->backgroundItem()->pos()));
    }
}

void LaserViewer::textAreaChanged()
{
    int newwidth = m_textEdit->document()->size().width();
    int newheight = m_textEdit->document()->size().height();
    m_textEdit->resize(newwidth, newheight);
}

void LaserViewer::onDocumentIdle()
{
	if (m_group && m_group->isSelected()) {
		QList<QGraphicsItem*> items = m_group->childItems();
		int size = items.size();
		if (items.size() > 0) {
			emit endSelecting();
			//
		}

	}
	//viewport()->repaint();
}

void LaserViewer::onCancelSelected()
{
	if (!m_group) {
		return;
	}
	if (!m_group->isEmpty())
	{
		/*if (m_group->isSelected()) {
			m_group->setSelected(false);
		}*/
		const auto items = m_group->childItems();
		for (QGraphicsItem *item : items) {
			LaserPrimitive* p_item = qgraphicsitem_cast<LaserPrimitive*>(item);
			
			m_group->removeFromGroup(p_item);
			if (p_item->isSelected()) {
				p_item->setSelected(false);
			}
			p_item->reShape();
		}
		
	}
	m_group->setTransform(QTransform());
	m_scene->clearSelection();
	//emit cancelSelected();
	
	emit beginSelecting();
	viewport()->repaint();
}
