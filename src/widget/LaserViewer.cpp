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
    , m_mousePressed(false)
    , m_isKeyDelPress(false)
    , m_isKeyShiftPressed(false)
    , m_isMouseInStartRect(false)
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
	
    if (StateControllerInst.isInState(StateControllerInst.documentIdleState()))
    {
		QGraphicsView::paintEvent(event);
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        ///qLogD << "SelectedEditing paint";
    }
    //selectioin
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
		QGraphicsView::paintEvent(event);
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawRect(QRectF(m_selectionStartPoint, m_selectionEndPoint));
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedEditingState())) {
        //qDebug() << "SelectedEditing paint";
		QGraphicsView::paintEvent(event);
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
		QGraphicsView::paintEvent(event);
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
		QRect rect = m_scene->backgroundItem()->boundingRect().toRect();
		rect = QRect(mapFromScene(rect.topLeft()), mapFromScene(rect.bottomRight()));
		//painter.eraseRect(rect);
		paintSelectedState(painter);
    }
    //Rect
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectCreatingState())) {
		QGraphicsView::paintEvent(event);
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        painter.drawRect(QRectF(mapFromScene(m_creatingRectStartPoint), mapFromScene(m_creatingRectEndPoint)));
    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
		QGraphicsView::paintEvent(event);
		QPainter painter(viewport());
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
		QGraphicsView::paintEvent(event);
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        painter.drawLine(mapFromScene(m_creatingLineStartPoint), mapFromScene(m_creatingLineEndPoint));
    }
    //Polygon
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonCreatingState())) {
		QGraphicsView::paintEvent(event);
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        QRectF rect = QRectF(mapFromScene(m_polygonStartRect.topLeft()), mapFromScene(m_polygonStartRect.bottomRight()));
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
        painter.drawLine(mapFromScene(m_creatingPolygonPoints.at(m_creatingPolygonPoints.length() - 1)), mapFromScene(m_creatingPolygonEndPoint));
    }
    //Spline
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineCreatingState())) {
		QGraphicsView::paintEvent(event);
		QPainter painter(viewport());
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
			QGraphicsView::paintEvent(event);
        }
    }
	
	if (m_group && !m_group->isEmpty())
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
	}
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
void LaserViewer::resetSelectedItemsGroupRect(QRectF _sceneRect, qreal _xscale, qreal _yscale, int _state, int _transformType)
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
				switch (_state)
				{
					case SelectionOriginalTopLeft: {
						break;
					}
					case SelectionOriginalTopCenter: {
						break;
					}
					case SelectionOriginalTopRight: {
						break;
					}

					case SelectionOriginalLeftCenter: {
						break;
					}
					case SelectionOriginalCenter: {
						break;
					}
					case SelectionOriginalRightCenter: {
						break;
					}
					case SelectionOriginalLeftBottom: {
						break;
					}
					case SelectionOriginalBottomCenter: {
						break;
					}
					case SelectionOriginalBottomRight: {
						break;
					}
				}
				break;
			}
		}
		/*switch (_state) {
			case SelectionOriginalTopLeft: {//topLeft
				switch (_transformType) {
					case Transform_MOVE: {
						// move
						
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_SCALE: {
						QTransform t1;
						t1.scale(_xscale, _yscale);
						t = t * t1;
						QPointF origi = bounds.topLeft();
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
						//resize
						qreal rateX = width / bounds.width();
						qreal rateY = height / bounds.height();
						qLogD << "scale: " << rateX << ", " << rateY;
						QPointF original = bounds.topLeft();
						QTransform t1;
						t1.scale(rateX, rateY);
						t = t * t1;
						QPointF newOriginal = t1.map(bounds.topLeft());
						QPointF diff = original - newOriginal;
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_ROTATE: {
						break;
					}
				}
				break;
			}
			case SelectionOriginalTopCenter: {//topCenter
				switch (_transformType) {
					case Transform_MOVE: {
						QPointF diff = point - QPointF(bounds.center().x(), bounds.topLeft().y());
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_SCALE: {
						QTransform t1;
						t1.scale(_xscale, _yscale);
						t = t * t1;
						QPointF origi = QPointF(bounds.center().x(), bounds.top());
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
						//resize
						qreal rateX = width / bounds.width();
						qreal rateY = height / bounds.height();
						qLogD << "scale: " << rateX << ", " << rateY;
						QPointF original = QPointF(bounds.center().x(), bounds.topLeft().y());
						QTransform t1;
						t1.scale(rateX, rateY);
						t = t * t1;
						QPointF newOriginal = t1.map(original);
						QPointF diff = original - newOriginal;
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_ROTATE: {
						break;
					}
				}
				break;
			}
			case SelectionOriginalTopRight: {//topRight
				switch (_transformType) {
					case Transform_MOVE: {
						QPointF diff = point - QPointF(bounds.bottomRight().x(), bounds.topLeft().y());
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_SCALE: {
						QTransform t1;
						t1.scale(_xscale, _yscale);
						t = t * t1;
						QPointF origi = QPointF(bounds.right(), bounds.top());
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
						//resize
						qreal rateX = width / bounds.width();
						qreal rateY = height / bounds.height();
						qLogD << "scale: " << rateX << ", " << rateY;
						QPointF original = bounds.topRight();
						QTransform t1;
						t1.scale(rateX, rateY);
						t = t * t1;
						QPointF newOriginal = t1.map(original);
						QPointF diff = original - newOriginal;
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_ROTATE: {
						break;
					}
				}
				break;
			}
			case SelectionOriginalLeftCenter: {//leftCenter
				switch (_transformType) {
					case Transform_MOVE: {
						QPointF diff = point - QPointF(bounds.topLeft().x(), bounds.center().y());
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_SCALE: {
						QTransform t1;
						t1.scale(_xscale, _yscale);
						t = t * t1;
						QPointF origi = QPointF(bounds.left(), bounds.center().y());
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
						//resize
						qreal rateX = width / bounds.width();
						qreal rateY = height / bounds.height();
						qLogD << "scale: " << rateX << ", " << rateY;
						QPointF original = QPointF(bounds.topLeft().x(), bounds.center().y());
						QTransform t1;
						t1.scale(rateX, rateY);
						t = t * t1;
						QPointF newOriginal = t1.map(original);
						QPointF diff = original - newOriginal;
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_ROTATE: {
						break;
					}
				}
				break;
			}
			case SelectionOriginalCenter: {//center
				switch (_transformType) {
					case Transform_MOVE: {
						// move
						QPointF diff = point - bounds.center();
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31()+diff.x(), t.m32()+ diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_SCALE: {
						QTransform t1;
						t1.scale(_xscale, _yscale);
						t = t * t1;
						QPointF origi = bounds.center();
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
						//resize
						qreal rateX = width / bounds.width();
						qreal rateY = height / bounds.height();
						QPointF original = bounds.center();
						QTransform t1;
						t = t * t1.scale(rateX, rateY);
						QPointF newOriginal = original * t1;
						QPointF diff = original - newOriginal;
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31()+diff.x(), t.m32()+diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_ROTATE: {
						break;
					}
				}
				break;
			}
			case SelectionOriginalRightCenter: {//rightCenter
				switch (_transformType) {
					case Transform_MOVE: {
						QPointF diff = point - QPointF(bounds.bottomRight().x(), bounds.center().y());
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_SCALE: {
						QTransform t1;
						t1.scale(_xscale, _yscale);
						t = t * t1;
						QPointF origi = QPointF(bounds.right(), bounds.center().y());
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
						//resize
						qreal rateX = width / bounds.width();
						qreal rateY = height / bounds.height();
						qLogD << "scale: " << rateX << ", " << rateY;
						QPointF original = QPointF(bounds.bottomRight().x(), bounds.center().y());
						QTransform t1;
						t1.scale(rateX, rateY);
						t = t * t1;
						QPointF newOriginal = t1.map(original);
						QPointF diff = original - newOriginal;
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
					
						break;
					}
					case Transform_ROTATE: {
						break;
					}
				}
				break;
			}
			case SelectionOriginalLeftBottom: {//leftBottom
				switch (_transformType) {
					case Transform_MOVE: {
						QPointF diff = point - QPointF(bounds.topLeft().x(), bounds.bottomRight().y());
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_SCALE: {
						QTransform t1;
						t1.scale(_xscale, _yscale);
						t = t * t1;
						QPointF origi = QPointF(bounds.left(), bounds.bottom());
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
						//resize
						qreal rateX = width / bounds.width();
						qreal rateY = height / bounds.height();
						qLogD << "scale: " << rateX << ", " << rateY;
						QPointF original = QPointF(bounds.topLeft().x(), bounds.bottomRight().y());
						QTransform t1;
						t1.scale(rateX, rateY);
						t = t * t1;
						QPointF newOriginal = t1.map(original);
						QPointF diff = original - newOriginal;
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_ROTATE: {
						break;
					}
				}
				break;
			}
			case SelectionOriginalBottomCenter: {//bottomCenter
				switch (_transformType) {
					case Transform_MOVE: {
						QPointF diff = point - QPointF(bounds.center().x(), bounds.bottomRight().y());
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_SCALE: {
						QTransform t1;
						t1.scale(_xscale, _yscale);
						t = t * t1;
						QPointF origi = QPointF(bounds.center().x(), bounds.bottom());
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
						//resize
						qreal rateX = width / bounds.width();
						qreal rateY = height / bounds.height();
						qLogD << "scale: " << rateX << ", " << rateY;
						QPointF original = QPointF(bounds.center().x(), bounds.bottom());
						QTransform t1;
						t1.scale(rateX, rateY);
						t = t * t1;
						QPointF newOriginal = t1.map(original);
						QPointF diff = original - newOriginal;
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_ROTATE: {
						break;
					}
				}
				break;
			}
			case SelectionOriginalBottomRight: {//rightBottom
				switch (_transformType) {
					case Transform_MOVE: {
						QPointF diff = point - bounds.bottomRight();
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_SCALE: {
						QTransform t1;
						t1.scale(_xscale, _yscale);
						t = t * t1;
						QPointF origi = bounds.bottomRight();
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
						//resize
						qreal rateX = width / bounds.width();
						qreal rateY = height / bounds.height();
						qLogD << "scale: " << rateX << ", " << rateY;
						QPointF original = bounds.bottomRight();
						QTransform t1;
						t1.scale(rateX, rateY);
						t = t * t1;
						QPointF newOriginal = t1.map(original);
						QPointF diff = original - newOriginal;
						t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31() + diff.x(), t.m32() + diff.y(), t.m33());
						m_group->setTransform(t);
						break;
					}
					case Transform_ROTATE: {
						break;
					}
				}
				break;
			}
		}*/
		
		
		//t.scale(rateX, )
	}
}
void LaserViewer::paintSelectedState(QPainter& painter)
{
	
    qreal left, right, top, bottom;
	/*QRectF rect;
	if (m_group && !m_group->isEmpty()) {
		rect = m_group->sceneBoundingRect();
		qDebug() << "paintSelectedState group" ;
	}
	else {
		rect = selectedItemsSceneBoundingRect();
		qDebug() << "paintSelectedState primi";
	}*/
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

void LaserViewer::setSelectionArea(const QPointF& _startPoint, const QPointF& _endPoint)
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
}

void LaserViewer::wheelEvent(QWheelEvent* event)
{
    //qreal wheelZoomValue = qPow(1.2, event->delta() / 240.0);
    qreal wheelZoomValue = 1 + event->delta() / 120.0 * 0.1;
    //qLogD << "wheelZoomValue: " << wheelZoomValue << ", delta: " << event->delta();
    zoomBy(wheelZoomValue);
    QGraphicsView::wheelEvent(event);
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
    // 若事件已被处理，则跳过
    //if (event->isAccepted())
        //return;

// 处理鼠标左键
    if (event->button() == Qt::LeftButton) {
        // 若在DocumentIdle状态下，开始进入选择流程
        if (StateControllerInst.isInState(StateControllerInst.documentIdleState()))
        {
            QGraphicsView::mousePressEvent(event);

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
				if (!m_group->isEmpty())
				{
					const auto items = m_group->childItems();
					for (QGraphicsItem *item : items) {
						LaserPrimitive* p_item = qgraphicsitem_cast<LaserPrimitive*>(item);
						m_group->removeFromGroup(p_item);
						p_item->reShape();
					}
						
				}
				if (m_group->isSelected()) {
					m_group->setSelected(false);
				}
				
				//m_scene->removeItem(m_group);
				//delete m_group;
				//m_scene->destroyItemGroup(m_group);
				m_group->setMatrix(QMatrix());
                emit cancelSelected();
                m_selectionStartPoint = event->pos();
                m_selectionEndPoint = m_selectionStartPoint;
                emit beginSelecting();
            }
        }

        //Rect
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectReadyState()))
        {
            m_creatingRectStartPoint = mapToScene(event->pos());
            m_creatingRectEndPoint = m_creatingRectStartPoint;
            emit creatingRectangle();
        }
        //Ellipse
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseReadyState()))
        {
            m_creatingEllipseStartPoint = mapToScene(event->pos());
            m_creatingEllipseStartInitPoint = m_creatingEllipseStartPoint;
            m_creatingEllipseEndPoint = m_creatingEllipseStartPoint;
            emit creatingEllipse();
        }
        //Line
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineReadyState()))
        {
            m_creatingLineStartPoint = mapToScene(event->pos());
            m_creatingLineEndPoint = m_creatingLineStartPoint;
            emit creatingLine();
        }
        //Polygon Ready
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonReadyState())) {
            //clear
            m_creatingPolygonPoints.clear();
            emit creatingPolygonStartRect();
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
    //shift keyboard
    if (event->modifiers() & Qt::ShiftModifier) {
        m_isKeyShiftPressed = true;
    }
    else {
        m_isKeyShiftPressed = false;
    }

    // 当在DocumentSelecting状态时
    if (StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
        m_selectionEndPoint = m_mousePoint;
        QGraphicsView::mouseMoveEvent(event);
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
		QGraphicsView::mouseMoveEvent(event);
        
    }
    //Rect
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectCreatingState())) {
        m_creatingRectEndPoint = mapToScene(m_mousePoint);
        return;

    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
        m_creatingEllipseEndPoint = mapToScene(m_mousePoint);

        return;
    }
    //Line
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineCreatingState())) {
        m_creatingLineEndPoint = mapToScene(m_mousePoint);
        if (m_isKeyShiftPressed) {
            qreal angle = QLineF(m_creatingLineStartPoint, m_creatingLineEndPoint).angle();
            if ((angle >= 0 && angle <= 45) ||
                (angle >= 315 && angle <= 360) ||
                (angle >= 135 && angle <= 180) ||
                (angle > 180 && angle <= 225)) {
                m_creatingLineEndPoint = QPointF(m_creatingLineEndPoint.x(), m_creatingLineStartPoint.y());
            }
            else if ((angle > 45 && angle <= 90) ||
                (angle > 90 && angle < 135) ||
                (angle > 225 && angle <= 270) ||
                (angle > 270 && angle < 315)) {
                m_creatingLineEndPoint = QPointF(m_creatingLineStartPoint.x(), m_creatingLineEndPoint.y());
            }
        }
        return;
    }
    //Polygon
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonCreatingState())) {

        m_isMouseInStartRect = m_polygonStartRect.contains(mapToScene(event->pos()));
        m_creatingPolygonEndPoint = mapToScene(m_mousePoint);
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
    //select
    if (StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
        qreal distance = (m_selectionEndPoint - m_selectionStartPoint).manhattanLength();
        qDebug() << "distance:" << distance;
        if (distance <= 0.000001f)
        {
            m_scene->clearSelection();
            emit cancelSelecting();
            QGraphicsView::mouseReleaseEvent(event);
            m_mousePressed = false;
            m_isKeyShiftPressed = false;
            viewport()->repaint();
            return;
        }
		setSelectionArea(m_selectionStartPoint, m_selectionEndPoint);
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
			qDebug() << m_scene->selectedPrimitives().length();
			m_group = m_scene->createItemGroup(m_scene->selectedPrimitives());
			qDebug() << m_scene->selectedPrimitives().length();
			m_group->setFlag(QGraphicsItem::ItemIsSelectable, true);
			m_group->setSelected(true);
			
		}
		m_scene->addItem(m_group);
		
        emit endSelecting();
        m_selectedHandleList.clear();
        m_curSelectedHandleIndex = -1;
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedEditingState())) {
		
		m_selectedEditCount = 0;
		m_radians = 0;
        emit endSelectedEditing();
    }

    //Rect
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectCreatingState())) {
        QRectF rect(m_creatingRectStartPoint, m_creatingRectEndPoint);
        LaserRect* rectItem = new LaserRect(rect, m_scene->document());
        m_scene->addLaserPrimitive(rectItem);
        emit readyRectangle();
    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
        QRectF rect(m_creatingEllipseStartPoint, m_EllipseEndPoint);
        LaserEllipse* ellipseItem = new LaserEllipse(rect, m_scene->document());
        m_scene->addLaserPrimitive(ellipseItem);
        emit readyEllipse();
    }
    //Line
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineCreatingState())) {
        if (m_creatingLineStartPoint != m_creatingLineEndPoint) {

            QLineF line(m_creatingLineStartPoint, m_creatingLineEndPoint);
            LaserLine* lineItem = new LaserLine(line, m_scene->document());
            m_scene->addLaserPrimitive(lineItem);
        }
        emit readyLine();

    }
    //Polygon Start
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonStartRectState())) {
        //init
        m_creatingPolygonStartPoint = mapToScene(event->pos());
        m_creatingPolygonEndPoint = m_creatingPolygonStartPoint;
        m_creatingPolygonPoints.append(m_creatingPolygonStartPoint);

        qreal width = 5;
        qreal halfWidth = width * 0.5;
        m_polygonStartRect = QRectF(m_creatingPolygonStartPoint.x() - halfWidth, m_creatingPolygonStartPoint.y() - halfWidth, width, width);

        emit creatingPolygon();
    }
    //Polygon Creating
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonCreatingState())) {
        m_creatingPolygonEndPoint = mapToScene(event->pos());
        if (m_polygonStartRect.contains(m_creatingPolygonEndPoint)) {
            m_creatingPolygonEndPoint = m_creatingPolygonStartPoint;
            if (m_creatingPolygonPoints.length() > 1) {
                m_creatingPolygonPoints.append(m_creatingPolygonEndPoint);
                for (int i = 0; i < m_creatingPolygonPoints.length(); i++) {
                    m_creatingPolygonPoints[i] = m_creatingPolygonPoints[i];
                }
                QPolygonF qPolygon(m_creatingPolygonPoints);
                LaserPolygon* polygon = new LaserPolygon(qPolygon, m_scene->document());
                m_scene->addLaserPrimitive(polygon);
                emit readyPolygon();
            }
        }
        else {
            m_creatingPolygonPoints.append(m_creatingPolygonEndPoint);
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
        QGraphicsView::mouseReleaseEvent(event);
    }

    m_mousePressed = false;
    m_isKeyShiftPressed = false;
    this->viewport()->repaint();
}

void LaserViewer::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Shift:
        m_isKeyShiftPressed = true;
        break;
    case Qt::Key_Delete:
        m_isKeyDelPress = true;
        break;
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
    case Qt::Key_Shift:
        m_isKeyShiftPressed = false;
    case Qt::Key_Delete:
        m_isKeyDelPress = false;
        break;
        break;
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
    ADD_TRANSITION(documentPrimitivePolygonStartRectState, documentPrimitivePolygonCreatingState, this, SIGNAL(creatingPolygon()));
    ADD_TRANSITION(documentPrimitivePolygonCreatingState, documentPrimitivePolygonReadyState, this, SIGNAL(readyPolygon()));
    ADD_TRANSITION(documentPrimitivePolygonReadyState, documentPrimitivePolygonStartRectState, this, SIGNAL(creatingPolygonStartRect()));
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
			QPointF diff = m_mousePoint - m_lastPos;
			t = m_group->transform();
			t.setMatrix(t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), diff.x()+ t.dx(), diff.y()+ t.dy(), t.m33());
			m_group->setTransform(t);
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
