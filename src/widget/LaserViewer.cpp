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
#include <QMouseEvent>
#include <QUndoStack>
#include <QGLWidget>
#include <QMessageBox>

#include <LaserApplication.h>
#include "laser/LaserDevice.h"
#include "scene/LaserPrimitiveGroup.h"
#include "scene/LaserPrimitive.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include "ui/LaserControllerWindow.h"

#include "state/StateController.h"
#include "widget/RulerWidget.h"
#include "common/Config.h"
#include "widget/UndoCommand.h";
#include "util/utils.h"
#include "widget/OverstepMessageBoxWarn.h"

LaserViewer::LaserViewer(QWidget* parent)
	: QGraphicsView(parent)
	, m_scene(new LaserScene)
	, m_rubberBandActive(false)
	, m_isKeyShiftPressed(false)
	, m_isKeyCtrlPress(false)
	, m_isItemEdge(false)
	, m_isItemEdgeCenter(false)
	//, m_isMouseInStartRect(false)
	, m_splineNodeDrawWidth(3)
	, m_splineHandlerWidth(5)
	, m_splineNodeEditWidth(5)
	, m_handlingSpline(SplineStruct())
	//, m_lastTime(0)
	//, m_curTime(0)
	, m_horizontalRuler(nullptr)
	, m_verticalRuler(nullptr)
	, m_radians(0)
	, m_mousePressState(nullptr)
	, m_isPrimitiveInteractPoint(false)
	, m_isGridNode(false)
	, m_curLayerIndex(1)
    , m_editingText(nullptr)
    , m_isCapsLock(false)
    , m_insertIndex(-1)
    , m_textAlighH(Qt::AlignLeft)
    , m_textAlighV(Qt::AlignBottom)
    , m_isTextMessageBoxShow(false)
	
{
    setScene(m_scene.data());
    init();
    //m_maxRegion = QRectF(0, 0, maxSize, maxSize);

    Global::dpiX = logicalDpiX();
    Global::dpiY = logicalDpiY();
}

LaserViewer::~LaserViewer()
{
	m_undoStack = nullptr;
	m_horizontalRuler = m_verticalRuler = nullptr;
    m_editingText = nullptr;
	//delete m_copyedList;
	//m_copyedList = nullptr;
}

void LaserViewer::paintEvent(QPaintEvent* event)
{
	if (m_isFirstPaint) {
		//m_fitInRect = rect();
		//fitInView(m_fitInRect, Qt::KeepAspectRatio);
		qDebug() << "m_fitInRect: " << rect();
		m_isFirstPaint = false;
	}
	
	QGraphicsView::paintEvent(event);	
	QPainter painter(viewport());
    
	painter.setRenderHint(QPainter::Antialiasing);
    
    LaserBackgroundItem* backgroundItem = m_scene->backgroundItem();
    if (backgroundItem) {
        painter.setPen(QPen(Qt::red, 1, Qt::DashDotDotLine));
        QRectF sceneMaxRegion = m_scene->maxRegion();
        painter.drawPolygon(mapFromScene(sceneMaxRegion));
    }

	if (Config::Ui::showDocumentBoundingRect() && scene()->document())
	{
		QRectF rect = scene()->document()->docBoundingRect();
		if (rect.isValid())
		{
			painter.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
            QPolygonF gridBounds = mapFromScene(rect);
			painter.drawPolygon(gridBounds);            
		}
        
        painter.setPen(QPen(Qt::darkGreen));
        QPointF origin = mapFromScene(scene()->document()->docOrigin());
        QRectF originRect(origin - QPointF(2, 2), origin + QPointF(2, 2));
        painter.drawRect(originRect);

		painter.setPen(QPen(Qt::red, 2));
		QPointF deviceOrigin = mapFromScene(LaserApplication::device->deviceOrigin());
		QRectF deviceOriginRect(deviceOrigin - QPointF(2, 2), deviceOrigin + QPointF(2, 2));
		painter.drawRect(deviceOriginRect);

		if (scene()->document()->enablePrintAndCut())
		{
			for (const PointPair& pair : scene()->document()->printAndCutPointPairs())
			{
				QPointF canvasPoint(Global::mm2PixelsXF(pair.second.x()), Global::mm2PixelsYF(pair.second.y()));
                painter.setPen(QPen(Qt::red, 2));
                canvasPoint = mapFromScene(canvasPoint);
                QRectF canvasPointRect(canvasPoint - QPointF(2, 2), canvasPoint + QPointF(2, 2));
                painter.drawRect(canvasPointRect);
			}
		}
	}

	//painter.setPen(QPen(Qt::red, 1, Qt::SolidLine));
	//painter.drawPolygon (testRect);
	//painter.drawPolygon(testBoundinRect);
	
    if (StateControllerInst.isInState(StateControllerInst.documentIdleState()))
    {
		//painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    }
	
    //selectioin
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
		//painter.setRenderHint(QPainter::Antialiasing);
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
        QRectF rect = QRectF(mapFromScene(m_creatingRectStartPoint), mapFromScene(m_creatingRectEndPoint));
        if (rect.width() != 0 && rect.height() != 0) {
            painter.drawRect(rect);
        }       
    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
		//painter.setRenderHint(QPainter::Antialiasing);
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
		//painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        painter.drawLine(mapFromScene(m_creatingLineStartPoint), mapFromScene(m_creatingLineEndPoint));
    }
    //Polygon
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonCreatingState())) {
		//painter.setRenderHint(QPainter::Antialiasing);
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
				//painter.drawLine(mapFromScene(m_creatingPolygonPoints[i]), mapFromScene(m_creatingPolygonPoints[i+1]));
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
		//painter.setRenderHint(QPainter::Antialiasing);
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
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {
        painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        QLineF l = QLineF(mapFromScene(m_textCursorLine.p1()), mapFromScene(m_textCursorLine.p2()));       
        painter.drawLine(l);
        /*painter.drawLine(QPoint(point.x(), point.y() - Global::mm2PixelsYF(10) * zoomValue()),
            QPoint(point.x(), point.y() + Global::mm2PixelsYF(10)* zoomValue()));*/
        
    }
    else {
        
    }
	painter.setPen(QPen(Qt::red, 5, Qt::SolidLine));
    //painter.drawRect(this->rect());
	//painter.drawPoint(testPoint);
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
	
	//QList<LaserPrimitive*> items = m_scene->selectedPrimitives();
	//QList<LaserPrimitive*> items = m_;
	if (!m_group) {
		return rect;
	}

	qreal left = 0;
	qreal right = 0;
	qreal top = 0;
	qreal bottom = 0;

	QList<QGraphicsItem*> group_items = m_group->childItems();
	if (group_items.size() == 0) {
		return rect;
	}
	for (int i = 0; i < group_items.size(); i++) {
		LaserPrimitive* item = qgraphicsitem_cast<LaserPrimitive*>(group_items[i]);
		detectRect(*item, i, left, right, top, bottom);
	}
	rect = QRectF(left, top, right - left, bottom - top);
	return rect;
	
}
QRectF LaserViewer::AllItemsSceneBoundingRect()
{
    QList<LaserPrimitive*> list = m_scene->document()->primitives().values();
    if (list.isEmpty()) {
        return QRectF();
    }
    QRectF firstBound = list[0]->sceneBoundingRect();
    qreal top = firstBound.top();
    qreal left = firstBound.left();
    qreal bottom = firstBound.bottom();
    qreal right = firstBound.right();
    for (LaserPrimitive* primitive : list) {
        QRectF bound = primitive->sceneBoundingRect();
        if (top > bound.top()) {
            top = bound.top();
        }
        if (left > bound.left()) {
            left = bound.left();
        }
        if (bottom < bound.bottom()) {
            bottom = bound.bottom();
        }
        if (right < bound.right()) {
            right = bound.right();
        }
    }
    return QRectF(QPointF(left, top), QPointF(right, bottom));
}
void LaserViewer::resetSelectedItemsGroupRect(QRectF _sceneRect, qreal _xscale, qreal _yscale, qreal rotate, 
    int _state, int _transformType, int _pp, bool _unitIsMM)
{
	if (m_group && !m_group->isEmpty()) {
		QRectF bounds = selectedItemsSceneBoundingRect();
		QPointF point = _sceneRect.topLeft();
		qreal widthReal = _sceneRect.width();
		qreal heightReal = _sceneRect.height();
        LaserControllerWindow* window = LaserApplication::mainWindow;
       
		QTransform t = m_group->transform();
        
        bool isLockRatio = window->lockEqualRatio();
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
                qreal scaleX = _xscale;
                qreal scaleY = _yscale;
                if (_pp == PrimitiveProperty::PP_ScaleX && isLockRatio) {
                    scaleY = scaleX;
                }else if (_pp == PrimitiveProperty::PP_ScaleY && isLockRatio) {
                    scaleX = scaleY;
                }
				t1.scale(scaleX, scaleY);
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
				//emit selectedChange();
				break;
			}
			case Transform_RESIZE: {
                qDebug() << bounds.width();
				qreal rateX = widthReal / bounds.width();
				qreal rateY = heightReal / bounds.height();
                if (_pp == PrimitiveProperty::PP_Height && isLockRatio) {
                    rateX = rateY;
                    if (_unitIsMM) {
                        qreal v = Global::pixelsF2mmY(bounds.width() * rateY);
                        window->widthBox()->setValue(v);
                    }
                    
                }
                if (_pp == PrimitiveProperty::PP_Width && isLockRatio) {
                    rateY = rateX;
                    if (_unitIsMM) {
                        qreal v = Global::pixelsF2mmY(bounds.height() * rateY);
                        window->heightBox()->setValue(v);
                    }
                }

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
void LaserViewer::setAnchorPoint(QPointF point)
{
	m_anchorPoint = point;
}
bool LaserViewer::detectIntersectionByMouse(QPointF& result, QPointF mousePoint, bool& isSpecialPoint)
{
	isSpecialPoint = false;
	qreal delta = Config::Ui::objectShapeDistance();
	if (delta <= 0) {
		return false;
	}
	LaserPrimitive *primitive = nullptr;
	//返回的是在scene中的坐标
	QLineF edge = detectItemEdge(primitive, mapFromScene(mousePoint), delta);
	if (!primitive) {
		return false;
	}
	/*else {
		qDebug() << "true";
	}*/
	//QPointF point = mapToScene(mousePoint.toPoint());
	QPointF point = mousePoint;
	QVector2D v1 = QVector2D(point - edge.p1());
	QVector2D v2 = QVector2D(edge.p2() - edge.p1()).normalized();
	float projectorDistance = QVector2D::dotProduct(v1, v2);
	
	/*if (projectorDistance < 0) {
		float dis1 = QVector2D(point - edge.p1()).lengthSquared();
		float dis2 = QVector2D(point - edge.p2()).lengthSquared();
		if (dis1 < dis2) {
			result = edge.p1();
			isSpecialPoint = true;
		}
		else {
			result = edge.p2();
			isSpecialPoint = true;
		}

	}*/
	QVector2D projectorV = v2 * projectorDistance;
	result = edge.p1() + projectorV.toPointF();
	QString className = primitive->metaObject()->className();
	if (className == "LaserEllipse") {
		LaserEllipse* ellipse = qgraphicsitem_cast<LaserEllipse*>(primitive);
		QPolygonF rect = ellipse->sceneTransform().map(ellipse->boundingRect());
		QLineF top = QLineF(rect[0], rect[1]);
		QLineF bottom = QLineF(rect[1], rect[2]);
		QLineF left = QLineF(rect[2], rect[3]);
		QLineF right = QLineF(rect[3], rect[0]);
		if (utils::checkTwoPointEqueal(top.center(), result, 5.0f)) {
			result = top.center();
			isSpecialPoint = true;
		}
		else if (utils::checkTwoPointEqueal(left.center(), result, delta)) {
			result = left.center();
			isSpecialPoint = true;
		}
		else if (utils::checkTwoPointEqueal(bottom.center(), result, delta)) {
			result = bottom.center();
			isSpecialPoint = true;
		}
		else if (utils::checkTwoPointEqueal(right.center(), result, delta)) {
			result = right.center();
			isSpecialPoint = true;
		}
	}
	else {
		if (utils::checkTwoPointEqueal(edge.center(), result, 5.0f)) {
			result = edge.center();
			isSpecialPoint = true;
		}
		else if (utils::checkTwoPointEqueal(edge.p1(), result, delta)) {
			result = edge.p1();
			isSpecialPoint = true;
		}
		else if (utils::checkTwoPointEqueal(edge.p2(), result, delta)) {
			result = edge.p2();
			isSpecialPoint = true;
		}
	}
	
	return true;
}
QLineF LaserViewer::detectItemEdge(LaserPrimitive *& result, QPointF mousePoint, float scop)
{

	QLine line;
	QList <LaserPrimitive*> list = m_scene->document()->primitives().values();
	//先遍历新添加的
	for(QList<LaserPrimitive*>::Iterator i = list.end()-1; i != list.begin()-1; i--){
	//for each(LaserPrimitive* primitive in list) {
		LaserPrimitive* primitive = *i;
		QVector<QLineF> edgeList = primitive->edges();
		//先判断边框
		QPolygonF bounding = mapFromScene(primitive->sceneOriginalBoundingPolygon(scop));
		//testBoundinRect = bounding;
		if (bounding.containsPoint(mousePoint, Qt::OddEvenFill)) {

			//然后判断边
			for each(QLineF edge in edgeList) {
				QVector2D vec(QPointF(edge.p1() - edge.p2()));

				//edges's vertical vector
				QVector2D verticalV1(-vec.y(), vec.x());
				verticalV1 = verticalV1.normalized() * scop;
				QVector2D verticalV2(vec.y(), -vec.x());
				verticalV2 = verticalV2.normalized() * scop;
				QVector2D vecNormal = vec.normalized();

				QPointF newP1 = edge.p1() + (vecNormal * scop).toPointF();
				QPointF newP2 = edge.p2() - (vecNormal * scop).toPointF();
				QPointF newP1_1 = newP1 + verticalV1.toPointF();
				QPointF newP1_2 = newP1 + verticalV2.toPointF();
				QPointF newP2_1 = newP2 + verticalV1.toPointF();
				QPointF newP2_2 = newP2 + verticalV2.toPointF();

				//QPainterPath path;
				//QPolygonF polygon = path.toFillPolygon;
				QVector<QPointF> polVector;
				polVector.append(mapFromScene(newP1_1));
				polVector.append(mapFromScene(newP1_2));
				polVector.append(mapFromScene(newP2_2));
				polVector.append(mapFromScene(newP2_1));
				QPolygonF polygon(polVector);
				//qDebug() << "polygon: " << polygon;
				//qDebug() << "mousePoint: " << mousePoint;
				//QRectF rect(mapFromScene(newP1_1), mapFromScene(newP2_2));

				if (polygon.containsPoint(mousePoint, Qt::OddEvenFill)) {
					result = primitive;
					//testRect = polygon;

					return edge;
				}
			}
		}
	}

	return line;
}
bool LaserViewer::detectItemByMouse(LaserPrimitive*& result, QPointF mousePoint)
{
	
	qreal delta = Config::Ui::clickSelectionTolerance();
	if (delta <= 0) {
		return false;
	}
	result = nullptr;
	detectItemEdge(result, mousePoint, delta);
	if (result != nullptr) {
		return true;
	}
	return false;
}
bool LaserViewer::detectBitmapByMouse(LaserBitmap *& result, QPointF mousePoint)
{
	QList<QGraphicsItem*> items = m_scene->items(mapToScene(mousePoint.toPoint()));
	for each(QGraphicsItem* item in items) {
		LaserPrimitive* primitive = qobject_cast<LaserPrimitive*> (item->toGraphicsObject());
		if (primitive != nullptr) {
			LaserBitmap* bitmap = qobject_cast<LaserBitmap*> (primitive);
			if (bitmap) {
				result = bitmap;
				return true;
			}
		}
	}
	
	return false;
}
 bool LaserViewer::detectTextInsertPosition(QPointF insertPoint, LaserText*& laserText)
{
    QPoint global = QCursor::pos();
    QPointF mousePoint = mapToScene(mapFromGlobal(global));
    bool isInAllPathBound = false;
    qreal extend = Global::mm2PixelsXF(10.0);
    QFontMetrics fontMetrics(m_textFont);
    
    
    //先遍历整个外框
    for (LaserPrimitive* primitive : m_scene->document()->primitives().values()) {
        QString name = primitive->metaObject()->className();
        if ( name != "LaserText") {
            continue;
        }
        LaserText* text = qgraphicsitem_cast<LaserText*>(primitive);
        QRectF rect = text->originalBoundingRect(extend);
        //如果只有一个空格
        //if (rowSize == 1 && text->subPathList()[0].subRowPathlist()[0].isEmpty()) {
        qDebug() << text->content();
        qDebug() << text->content().length();
        laserText = text;
        if (text->content().trimmed().isEmpty()) {
            m_scene->removeLaserPrimitive(laserText);
            laserText = nullptr;
            m_insertIndex = -1;
            return false;
        }
        
        isInAllPathBound = (text->sceneTransform().map(rect)).containsPoint(mousePoint, Qt::OddEvenFill);
        if (!isInAllPathBound) {
            continue;
        }
        
        QPainterPath lastPath;
        int rowSize = text->subPathList().size();

        
        //再遍历每一行的外包框
        for (int i = 0; i < rowSize; i++) {
            LaserTextRowPath rowPathStruct = text->subPathList()[i];
            
            QList<QPainterPath> subRowPathlist = rowPathStruct.subRowPathlist();
            QList<QRectF> subRowBoundList = rowPathStruct.subRowBoundList();
            int subPathSize = subRowPathlist.size();
            //qreal worldSpacing = m_textFont.wordSpacing();
            QRectF rowPathBoundingRect = rowPathStruct.path().boundingRect();
            qreal halfWTopSpacing = 0;
            qreal halfWBottomSpacing = 0;
            //遍历每一行的外包框,只有一行直接遍历字符path
            bool isInRowPathBound = false;
            QRectF extendRowRect;
            if (rowSize > 1) {
                if (i + 1 < rowSize) {
                    QRectF nextRowPathBoundingRect = text->subPathList()[i + 1].path().boundingRect();
                    halfWBottomSpacing = (nextRowPathBoundingRect.top() - rowPathBoundingRect.bottom()) * 0.5;
                    if (halfWBottomSpacing < 0) {
                        halfWBottomSpacing = 0;
                    }
                }
                if (i - 1 >= 0) {
                    QRectF lastRowPathBoundingRect = text->subPathList()[i - 1].path().boundingRect();
                    halfWTopSpacing = (rowPathBoundingRect.top() - lastRowPathBoundingRect.bottom()) * 0.5;
                    if (halfWTopSpacing < 0) {
                        halfWTopSpacing = 0;
                    }
                }
                if (i == 0) {
                    extendRowRect = QRectF(rowPathBoundingRect.topLeft() + QPointF(-extend, -extend),
                        rowPathBoundingRect.bottomRight() + QPointF(extend, halfWBottomSpacing));
                }
                else if (i == rowSize - 1) {
                    
                    extendRowRect = QRectF(rowPathBoundingRect.topLeft() + QPointF(-extend, -halfWTopSpacing),
                        rowPathBoundingRect.bottomRight() + QPointF(extend, extend));
                }
                else {
                    extendRowRect = QRectF(rowPathBoundingRect.topLeft() + QPointF(-extend, -halfWTopSpacing),
                        rowPathBoundingRect.bottomRight() + QPointF(extend, halfWBottomSpacing));
                }
                if (text->sceneTransform().map(extendRowRect).containsPoint(insertPoint, Qt::OddEvenFill)) {
                    isInRowPathBound = true;
                }

            }
            else {
                extendRowRect = QRectF(rowPathBoundingRect.topLeft() + QPointF(-extend, -extend),
                    rowPathBoundingRect.bottomRight() + QPointF(extend, extend));
                isInRowPathBound = true;
            }
            if (!isInRowPathBound) {
                continue;
            }
            //遍历这一行的字符
            for (int j = 0; j < subPathSize; j++) {
                QPainterPath subPath = subRowPathlist[j];
                QRectF rect = subRowBoundList[j];
                //QRectF extendSubRect;
                qreal halfLLeftSpacing = 0;
                qreal halfLRightSpacing = 0;
                qreal halfWidth = rect.width() * 0.5;
                QRectF frontRect, backRect;
                if (j + 1 < subPathSize) {
                    QRectF nextRect = subRowBoundList[j + 1];
                    halfLRightSpacing = nextRect.left() - rect.right();
                    if (halfLRightSpacing < 0) {
                        halfLRightSpacing = 0;
                    }
                }
                if (j - 1 >= 0) {
                    QRectF lastRect = subRowBoundList[j - 1];
                    halfLLeftSpacing = rect.left() - lastRect.right();
                    if (halfLLeftSpacing < 0) {
                        halfLLeftSpacing = 0;
                    }
                }
                if (subPathSize == 1) {
                    
                    qreal cx = extendRowRect.left()+ extendRowRect.width() * 0.5;
                    frontRect = QRectF(extendRowRect.topLeft(),  QPointF(cx, extendRowRect.bottom()));
                    backRect = QRectF(QPointF(cx, extendRowRect.top()), extendRowRect.bottomRight());
                    
                }
                else {
                    qreal cx = rect.left() + rect.width()*0.5;
                    if (j == 0) {
                        frontRect = QRectF(extendRowRect.topLeft(), QPointF(cx, extendRowRect.bottom()));
                        backRect = QRectF(QPointF(cx, extendRowRect.top()), 
                            QPointF(rect.right() + halfLRightSpacing, extendRowRect.bottom()));
                    }
                    else if (j == subPathSize - 1) {
                        frontRect = QRectF(QPointF(extendRowRect.left() - halfLLeftSpacing, extendRowRect.top()),
                            QPointF(QPointF(cx, extendRowRect.bottom())));
                        backRect = QRectF(QPointF(cx, extendRowRect.top()), extendRowRect.bottomRight());
                    }
                    else {
                        frontRect = QRectF(QPointF(extendRowRect.left() - halfLLeftSpacing, extendRowRect.top()),
                            QPointF(QPointF(cx, extendRowRect.bottom())));
                        backRect = QRectF(QPointF(cx, extendRowRect.top()),
                            QPointF(rect.right() + halfLRightSpacing, extendRowRect.bottom()));
                    }
                }
                //前面
                if (text->sceneTransform().map(frontRect).containsPoint(insertPoint, Qt::OddEvenFill)) {
                    int index = i-1;
                    m_insertIndex = 0;
                    while (index >= 0) {
                        m_insertIndex += text->subPathList()[index].subRowPathlist().size()+1;
                        index--;
                    }
                    m_insertIndex += j;

                    return true;
                }
                //后面
                else if(text->sceneTransform().map(backRect).containsPoint(insertPoint, Qt::OddEvenFill)){
                    int index = i-1;
                    m_insertIndex = 0;
                    while (index >= 0) {
                        m_insertIndex += text->subPathList()[index].subRowPathlist().size()+1;
                        index--;
                    }
                    m_insertIndex += (j + 1);

                    return true;
                    
                }
            }
        }  
    }
    m_insertIndex = -1;

    laserText = nullptr;
    return false;  
}
QState* LaserViewer::currentState()
{
	QState* currentState = nullptr;
	if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectState())) {
		currentState = StateControllerInst.documentPrimitiveRectState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseState())) {
		currentState = StateControllerInst.documentPrimitiveEllipseState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineState())) {
		currentState = StateControllerInst.documentPrimitiveLineState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonState())) {
		currentState = StateControllerInst.documentPrimitivePolygonState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineState())) {
		currentState = StateControllerInst.documentPrimitiveSplineState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineEditState())) {
		currentState = StateControllerInst.documentPrimitiveSplineEditState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextState())) {
		currentState = StateControllerInst.documentPrimitiveTextState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
		currentState = StateControllerInst.documentIdleState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentSelectionState())) {
		currentState = StateControllerInst.documentIdleState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonReadyState())) {
		currentState = StateControllerInst.documentPrimitivePolygonReadyState();
	}
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonState())) {
		currentState = StateControllerInst.documentPrimitivePolygonState();
	}
	
	return currentState;
}
QUndoStack * LaserViewer::undoStack()
{
	return m_undoStack;
}
QMap<LaserPrimitive*, QTransform>& LaserViewer::copyedList()
{
	return m_copyedList;
}
QMap<QString, QList<LaserPrimitive*>>& LaserViewer::groupedMap()
{
	return m_groupedMap;
}
QList<QString>& LaserViewer::selectedGroupedList()
{
	return m_selectedGroupedList;
}
int LaserViewer::curLayerIndex()
{
	return m_curLayerIndex;
}
void LaserViewer::setCurLayerIndex(int index)
{
	m_curLayerIndex = index;
}
QLineF LaserViewer::modifyTextCursor()
{
    if (m_editingText && m_editingText->subPathList().length() > 0) {
        int index = m_insertIndex;
        for (int i = 0; i < m_editingText->subPathList().size(); i++) {
            LaserTextRowPath rowPathstruct = m_editingText->subPathList()[i];
            QPointF rowLeftTop = rowPathstruct.leftTopPosition();
            int size = rowPathstruct.subRowPathlist().size();
            if (index > size) {
                index -= (size + 1);
            }
            else {
                qreal height = m_editingText->font().pixelSize();
                qreal bottom = rowLeftTop.y();
                qreal top = bottom - height;
                qreal x;
                if (index > 0) {
                    //QRectF rect = rowPathstruct.subRowPathlist()[index - 1].boundingRect();
                    QRectF rect = rowPathstruct.subRowBoundList()[index - 1];
                    x = rect.right();
                }
                else if (index == 0) {
                    x = rowLeftTop.x();
                }
                m_textCursorLine = m_editingText->sceneTransform().map(QLineF(QPointF(x, top), QPointF(x, bottom)));
                return m_textCursorLine;
            }
        }
    }
    else {
        QPointF p1 = mapToScene(m_textMousePressPos.toPoint());
        qreal height = m_textFont.pixelSize();
        switch (m_textAlighV) {
        case Qt::AlignTop: {
            QPointF p2(p1.x(), p1.y() + height);
            m_textCursorLine = QLineF(QLineF(p1, p2));
            break;
        }
        case Qt::AlignBottom: {
            QPointF p2(p1.x(), p1.y() - height);
            m_textCursorLine = QLineF(QLineF(p1, p2));
            break;
        }
        case Qt::AlignVCenter: {
            QPointF p2(p1.x(), p1.y() + height * 0.5);
            m_textCursorLine = QLineF(QLineF(QPointF(p1.x(), p1.y() - height * 0.5), p2));
            break;
        }
        }
    }
    return m_textCursorLine;
    
}
QFont* LaserViewer::textFont()
{
    return &m_textFont;
}
LaserText * LaserViewer::editingText()
{
    return m_editingText;
}
void LaserViewer::setEditingText(LaserText* text)
{
    m_editingText = text;
}
void LaserViewer::paintSelectedState(QPainter& painter)
{
	
    qreal left, right, top, bottom;
	QRectF rect = selectedItemsSceneBoundingRect();
	qDebug() << "paintSelectedState: "<<rect;
	if (rect.width() == 0 && rect.height() == 0) {
		return;
	}
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
	QRectF rect = QRectF(mapToScene(_startPoint.toPoint()), mapToScene(_endPoint.toPoint()));
	selectionPath.addRect(rect);
	//selectionPath.addRect(rect);
	//m_scene->setSelectionArea(mapToScene(selectionPath));
	qDebug() << "_startPoint: " << _startPoint;
	qDebug() << "_endPoint: " << _endPoint;
	m_scene->blockSignals(true);
	//right select
	if (_endPoint.x() < _startPoint.x()) {
		//m_scene->setSelectionArea(selectionPath, Qt::ItemSelectionMode::ContainsItemBoundingRect);
		//m_scene->findSelectedByBoundingRect(rect);
		m_scene->findSelectedByLine(rect);
		
	}
	//left selection
	else if (_endPoint.x() >= _startPoint.x()) {
		//m_scene->setSelectionArea(selectionPath, Qt::ItemSelectionMode::ContainsItemBoundingRect);
		m_scene->findSelectedByBoundingRect(rect);
	}
	m_scene->blockSignals(false);
	viewport()->repaint();
	emit m_scene->selectionChanged();
	return m_scene->document()->selectedPrimitives().size();
}

void LaserViewer::wheelEvent(QWheelEvent* event)
{
	
	QGraphicsView::wheelEvent(event);
	//
	if (!m_scene) {
		return;
	}
	if (!m_scene->document()) {
		return;
	}
    qreal wheelZoomValue = 1 + event->delta() / 120.0 * 0.1;
	LaserBackgroundItem* backgroundItem = m_scene->backgroundItem();
	if (!backgroundItem) {
		return;
	}
	
	QPointF mousePos = mapFromGlobal(QCursor::pos());
	zoomBy(wheelZoomValue, mousePos);
	this->viewport()->repaint();
}
//输入的点zoomAnchor是view的widget为坐标系
bool LaserViewer::zoomBy(qreal factor, QPointF zoomAnchor, bool zoomAnchorCenter)
{

    const qreal currentZoom = zoomValue();
    if ((factor < 1 && currentZoom < 0.01) || (factor > 1 && currentZoom > 58))
        return false;
	LaserBackgroundItem* backgroundItem = m_scene->backgroundItem();
	if (!backgroundItem) {
		return false;
	}
	//QPointF point = mapFromGlobal(QCursor::pos());
	QTransform t = transform();
	QTransform t1;
	t1.scale(factor, factor);
	QTransform t2;
	QPointF diff;

	QPointF newZoomAnchor = zoomAnchor - m_anchorPoint;//映射到以m_anchorPoint为（0， 0）点的坐标系
	QPointF scaled = t1.map(newZoomAnchor);
	QPointF newPoint = newZoomAnchor;
	if (zoomAnchorCenter) {
		
		newPoint =  this->rect().center();
		scaled += m_anchorPoint;
	}
	diff = newPoint - scaled;
	t2.translate(diff.x(), diff.y());
	setTransform(t*t1*t2);
	//更新网格
	backgroundItem->onChangeGrids();
    emit zoomChanged(mapFromScene(m_scene->backgroundItem()->QGraphicsItemGroup::pos()));
    emit scaleChanged(zoomValue());
    return true;
}

void LaserViewer::resizeEvent(QResizeEvent * event)
{
    if (!m_scene->document()) {
        return;
    }
	LaserBackgroundItem* backgroundItem = m_scene->backgroundItem();
	if (!backgroundItem) {
		return;
	}
	//QPointF fitInRectTopLeft = mapFromScene(backgroundItem->pos());
	QGraphicsView::resizeEvent(event);
    qreal scale = adapterViewScale();
    zoomBy(scale/zoomValue(), mapTo(this,mapFromScene(backgroundItem->rect().center().toPoint())), true);
    viewport()->repaint();
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
		m_mousePressState = nullptr;
		//qDebug() << mapFromGlobal(QCursor::pos());
		//qDebug() << mapToScene(mapFromGlobal(QCursor::pos()));
		//附近的图元交点
		/*if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveState())) {
			bool isEdgeSpecailPoint = false;
			if (detectIntersectionByMouse(intersePoint, point, isEdgeSpecailPoint)) {
				point = intersePoint;
			}
		}*/
			
        // 若在DocumentIdle状态下，开始进入选择流程
        if (StateControllerInst.isInState(StateControllerInst.documentIdleState()))
        {
			m_mousePressState = StateControllerInst.documentIdleState();
			m_detectedPrimitive = nullptr;
			m_detectedBitmap = nullptr;
			//点击选中图元
			if (detectItemByMouse(m_detectedPrimitive, event->pos())) {
				//undo
				//qDebug()<<m_group->childItems();
				selectionUndoStackPushBefore();
				
				//clearGroupSelection();
				m_detectedPrimitive->setSelected(true);
				if (onSelectedFillGroup()) {
					m_curSelectedHandleIndex = 13;
					emit beginIdelEditing();
				}
				//undo before
				transformUndoStackPushBefore();
				//undo redo
				selectionUndoStackPush();
                //选取区域的属性面板
                LaserApplication::mainWindow->onLaserPrimitiveGroupItemChanged();
				return;
			}
			else {
				//detectBitmapByMouse(m_detectedBitmap, event->pos());
				//QGraphicsView::mousePressEvent(event);
				//点击选中图片
				if (detectBitmapByMouse(m_detectedBitmap, event->pos())) {
					//clearGroupSelection();
					//undo
					selectionUndoStackPushBefore();
					//transformUndoStackPushBefore();
					m_detectedBitmap->setSelected(true);
					if (onSelectedFillGroup()) {
						m_curSelectedHandleIndex = 14;
						emit beginIdelEditing();
					}
					//undo before
					transformUndoStackPushBefore();
					//undo redo
					selectionUndoStackPush();
                    //选取区域的属性面板
                    LaserApplication::mainWindow->onLaserPrimitiveGroupItemChanged();
					return;
				}
				// 获取选框起点
				m_selectionStartPoint = event->pos();
				m_selectionEndPoint = m_selectionStartPoint;
				qDebug() << "begin to select";
				emit beginSelecting();
			}
            
        }
        // 若在DocumentSelected状态下
        else if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
			m_mousePressState = StateControllerInst.documentSelectedState();
			
			//
			m_detectedPrimitive = nullptr;
			m_detectedBitmap = nullptr;
            // 判断是鼠标是否按在选框控制柄上了
            int handlerIndex;
            if (isOnControllHandlers(event->pos(), handlerIndex))
            {
				m_selectedRect = selectedItemsSceneBoundingRect();
				m_oldTransform = m_group->transform();
				m_rate = 1;
				m_isItemScaleChangeX = true;
				m_isItemScaleChangeY = true;
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
				//undo
				//m_undoTransform = m_group->sceneTransform();
				transformUndoStackPushBefore();
                emit beginSelectedEditing();
			}
			else if(detectItemByMouse(m_detectedPrimitive, event->pos())){
				//undo before
				selectionUndoStackPushBefore();
				pointSelectWhenSelectedState(13, m_detectedPrimitive);
				//undo redo
				selectionUndoStackPush();
				//undo before
				if (m_group->isAncestorOf(m_detectedPrimitive)) {
					transformUndoStackPushBefore();
				}
				else {
					transformUndoStackPushBefore(m_detectedPrimitive);
				}
			}
			
            else
            {
				m_detectedBitmap = nullptr;
				
				//QGraphicsView::mousePressEvent(event);
				//事件被Item截断 图片点选
				if (detectBitmapByMouse(m_detectedBitmap, event->pos())) {
					//undo before
					selectionUndoStackPushBefore();
					pointSelectWhenSelectedState(14, m_detectedBitmap);
					//undo redo
					selectionUndoStackPush();
					//undo before
					if (m_group->isAncestorOf(m_detectedBitmap)) {
						transformUndoStackPushBefore();
					}
					else {
						transformUndoStackPushBefore(m_detectedBitmap);
					}
					return;
				}
				m_selectionStartPoint = event->pos();
				m_selectionEndPoint = m_selectionStartPoint;
				emit beginSelecting();
            }
        }
		//View Drag Ready
		else if (StateControllerInst.isInState(StateControllerInst.documentViewDragReadyState())) {
			m_mousePressState = StateControllerInst.documentViewDragReadyState();
			QPixmap cMap(":/ui/icons/images/dragging_hand.png");
			this->setCursor(cMap.scaled(30, 30, Qt::KeepAspectRatio));
			m_lastViewDragPoint = event->pos();
			
			emit beginViewDraging();
		}
        //Rect
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectReadyState()))
        {
			m_mousePressState = StateControllerInst.documentPrimitiveRectReadyState();
			QPointF point = mapToScene(event->pos());
            m_creatingRectStartPoint = point;
			//
			if (m_isPrimitiveInteractPoint) {
				m_creatingRectStartPoint = m_primitiveInteractPoint;
			}
			//是否网格点
			if (m_isGridNode) {
				m_creatingRectStartPoint = m_gridNode;
			}
            m_creatingRectEndPoint = m_creatingRectStartPoint;
            emit creatingRectangle();
        }
        //Ellipse
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseReadyState()))
        {
			m_mousePressState = StateControllerInst.documentPrimitiveEllipseReadyState();
            m_creatingEllipseStartPoint = mapToScene(event->pos());
			if (m_isPrimitiveInteractPoint) {
				m_creatingEllipseStartPoint = m_primitiveInteractPoint;
			}
			//是否网格点
			if (m_isGridNode) {
				m_creatingEllipseStartPoint = m_gridNode;
			}
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
			m_mousePressState = StateControllerInst.documentPrimitivePolygonReadyState();
			m_lastPolygon = nullptr;
			m_creatingPolygonPoints.clear();
            //clear
            //m_creatingPolygonPoints.clear();
			//m_creatingPolygonLines.clear();

            //emit creatingPolygonStartRect();
        }
        //Spline
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineReadyState())) {
			m_mousePressState = StateControllerInst.documentPrimitiveSplineReadyState();
            initSpline();
            emit creatingSpline();
        }
        //Text
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextReadyState())) {
			m_mousePressState = StateControllerInst.documentPrimitiveTextReadyState();
            //addText();
            //emit creatingText();
            //m_textInputPoint = mapToScene(event->pos()).toPoint();
			/*if (m_isPrimitiveInteractPoint) {
				m_textInputPoint = m_primitiveInteractPoint.toPoint();
			}
			//是否网格点
			if (m_isGridNode) {
				m_textInputPoint = m_gridNode.toPoint();
			}*/
            //creatTextEdit();
            
        }
        //Text
        else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {
            onEndText();
            emit readyText();
            viewport()->repaint();
        }
		else {
			m_mousePressState = nullptr;
			//QGraphicsView::mousePressEvent(event); 会使画线时出现Bug
			//setInteractive(false);
		}
    }
    else if (event->button() == Qt::RightButton) {
        m_mirrorLine = nullptr;
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
	m_isPrimitiveInteractPoint = false;
	m_isGridNode = false;
	//所有的图元draw
	if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveState())) {
		//获取鼠标附近图元的最近点
		bool isSpecailPoint = false;
		m_isPrimitiveInteractPoint = detectIntersectionByMouse(m_primitiveInteractPoint, mapToScene(event->pos()), isSpecailPoint);
		//testPoint = mapFromScene(m_primitiveInteractPoint);
		viewport()->repaint();
		if (m_isPrimitiveInteractPoint) {
			if (!isSpecailPoint) {
				QPixmap cMap(":/ui/icons/images/lineCursor.png");
				this->setCursor(cMap.scaled(20, 20, Qt::KeepAspectRatio));
			}
			else {
				QPixmap cMap(":/ui/icons/images/center.png");
				this->setCursor(cMap.scaled(20, 20, Qt::KeepAspectRatio));
			}
			
		}
		else {
			this->setCursor(Qt::ArrowCursor);
			//是否为网格上的点
			//网格点吸附判断
			LaserBackgroundItem* backgroundItem = m_scene->backgroundItem();
			if (backgroundItem) {
				m_isGridNode = backgroundItem->detectGridNode(m_gridNode, mapToScene(event->pos()));
			}
		}
		
	}
    // 当在DocumentSelecting状态时
	if (StateControllerInst.isInState(StateControllerInst.documentIdleState())){
		
		m_detectedPrimitive = nullptr;
        if (event->button() == Qt::NoButton) {
            if (detectItemByMouse(m_detectedPrimitive, event->pos())) {
                QPixmap cMap(":/ui/icons/images/arrow.png");
                this->setCursor(cMap.scaled(25, 25, Qt::KeepAspectRatio));
            }
            else {
                setCursor(Qt::ArrowCursor);
            }
        }
		

	}else if(StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
		//事件被Item截断
		/*if (m_scene->mouseMoveBlock()) {
			return;
		}*/
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
		m_detectedPrimitive = nullptr;
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
        }else if (detectItemByMouse(m_detectedPrimitive, event->pos()) && event->button() == Qt::NoButton) {
			//when mousepress, detect again
			m_curSelectedHandleIndex = -1;
			QPixmap cMap(":/ui/icons/images/arrow.png");
			this->setCursor(cMap.scaled(25, 25, Qt::KeepAspectRatio));
		}
        else
        {
            m_curSelectedHandleIndex = -1;
            this->setCursor(Qt::ArrowCursor);
        }
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedEditingState())) {
		
		switch (m_curSelectedHandleIndex) {
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
			case 13://点选
			case 14://点选图片
			{
				selectedHandleScale();
				emit selectedChange();
				this->viewport()->repaint();
				return;
			}
			
		}
		//QGraphicsView::mouseMoveEvent(event);
		//事件被Item截断
		if (m_scene->mouseMoveBlock()) {
			this->viewport()->repaint();
			return;
		}
		
		//QGraphicsView::mouseMoveEvent(event);
        
    }
	//View Draging
	else if (StateControllerInst.isInState(StateControllerInst.documentViewDragingState())) {
		//QGraphicsView::mouseMoveEvent(event);
		//QPixmap cMap(":/ui/icons/images/dragging_hand.png");
		//this->setCursor(cMap.scaled(32, 32, Qt::KeepAspectRatio));
		QPointF viewDragPoint = event->pos();
		QPointF diff = viewDragPoint - m_lastViewDragPoint;
		QTransform t = transform();
		QTransform t1;
		t1.translate(diff.x(), diff.y());
		setTransform(t * t1);
		m_lastViewDragPoint = viewDragPoint;
	}
    //Rect
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectCreatingState())) {
        m_creatingRectEndPoint = mapToScene(m_mousePoint);
		if (m_isPrimitiveInteractPoint) {
			m_creatingRectEndPoint = m_primitiveInteractPoint;
		}
		//是否网格点
		if (m_isGridNode) {
			m_creatingRectEndPoint = m_gridNode;
		}
		m_creatingRectBeforeShiftPoint = m_creatingRectEndPoint;
		this->viewport()->repaint();
		//QGraphicsView::mouseMoveEvent(event);
        return;

    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
        m_creatingEllipseEndPoint = mapToScene(m_mousePoint);
		if (m_isPrimitiveInteractPoint) {
			m_creatingEllipseEndPoint = m_primitiveInteractPoint;
		}
		//是否网格点
		if (m_isGridNode) {
			m_creatingEllipseEndPoint = m_gridNode;
		}
		this->viewport()->repaint();
        return;
    }
    //Line
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineCreatingState())) {
        m_creatingLineEndPoint = mapToScene(m_mousePoint);
		if (utils::checkTwoPointEqueal(m_creatingLineEndPoint, m_creatingLineStartPoint)) {
			return;
		}
		if (m_isPrimitiveInteractPoint) {
			m_creatingLineEndPoint = m_primitiveInteractPoint;
		}
		//是否网格点
		if (m_isGridNode) {
			m_creatingLineEndPoint = m_gridNode;
		}
		qreal yLength = qAbs(m_creatingLineStartPoint.x() - m_creatingLineEndPoint.x()) * qTan(M_PI *0.25);
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
        m_creatingPolygonEndPoint = mapToScene(m_mousePoint);
		if (m_isPrimitiveInteractPoint) {
			m_creatingPolygonEndPoint = m_primitiveInteractPoint;
		}
		//是否网格点
		if (m_isGridNode) {
			m_creatingPolygonEndPoint = m_gridNode;
		}
		/*if (detectPoint(m_creatingPolygonPoints, m_creatingPolygonLines, m_creatingPolygonEndPoint)) {
			this->setCursor(Qt::CrossCursor);
		}
		else {
			this->setCursor(Qt::ArrowCursor);
		}*/
		this->viewport()->repaint();
		return;
    }
    //Spline
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveSplineCreatingState())) {
        m_creatingSplineMousePos = event->pos();
		if (m_isPrimitiveInteractPoint) {
			m_creatingSplineMousePos = m_primitiveInteractPoint;
		}
		//是否网格点
		if (m_isGridNode) {
			m_creatingSplineMousePos = m_gridNode;
		}
	}
	else {
		//setAnchorPoint(mapFromScene(QPointF(0, 0)));
	}
	
    QPointF pos = mapToScene(m_mousePoint);
    emit mouseMoved(pos);
}

void LaserViewer::mouseReleaseEvent(QMouseEvent* event)
{
	//QGraphicsView::mouseReleaseEvent(event);
	LaserBackgroundItem* backgroundItem = m_scene->backgroundItem();
	if (!backgroundItem) {
		
		return;
	}
    //select
    if (StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
		QGraphicsView::mouseReleaseEvent(event);
        
        if (utils::checkTwoPointEqueal(m_selectionStartPoint, m_selectionEndPoint))
        {
			//点中空白且press与release同一个点
			selectingReleaseInBlank();
            //选取区域的属性面板
            LaserApplication::mainWindow->onLaserPrimitiveGroupItemChanged();
			return;
        }
		//undo before
		selectionUndoStackPushBefore();
		//框选前先将item从group中移除
		//不然修改完group内的item的selected属性再从group中移除的话，
		//group子对象selected不论true/false都设为group的isSelected值
		resetGroup();
		//框选前被选中的
		QList<LaserPrimitive*> selectedList = m_scene->document()->selectedPrimitives();
		//框选区域内被选中的
		setSelectionArea(m_selectionStartPoint, m_selectionEndPoint);
		QList<LaserPrimitive*> newSelectedList = m_scene->document()->selectedPrimitives();
		//框选区域，分情况处理selectionUndo
		if (m_isKeyCtrlPress) {
            //update tree

			for each(LaserPrimitive* item in selectedList) {
				item->setSelected(true);
			}
			for each(LaserPrimitive* newItem in newSelectedList) {
                LaserPrimitive* p_newItem = qgraphicsitem_cast<LaserPrimitive*>(newItem);
				if (selectedList.contains(p_newItem)) {
                    p_newItem->blockSignals(true);
                    p_newItem->setSelected(false);
                    p_newItem->blockSignals(false);
				}
				else {
                    p_newItem->blockSignals(true);
                    p_newItem->setSelected(true);
                    p_newItem->blockSignals(false);
				}
			}
			onEndSelectionFillGroup();
			//undo redo
			selectionUndoStackPush();
		}
		else {
			if (selectedList == newSelectedList) {
				onEndSelectionFillGroup();
				return;
			}
			else {
				onEndSelectionFillGroup();
				//undo redo
				selectionUndoStackPush();
				
			}
		}
        //选取区域的属性面板
        LaserApplication::mainWindow->onLaserPrimitiveGroupItemChanged();
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedEditingState())) {
		
		m_selectedEditCount = 0;
		m_radians = 0;
		
		//group中没有被选中的item，返回idle状态
		//if (m_group->isEmpty()) {
        if (m_scene->selectedPrimitives().isEmpty()) {
			emit selectionToIdle();
		}
		//group中有被选中的item，返回selected状态
		else {
			emit endSelectedEditing();
		}
		//undo
		//如果是图元点选
		if (m_detectedPrimitive != nullptr) {
			if (m_group->isAncestorOf(m_detectedPrimitive)) {
				transformUndoStackPush();
			}
			else {
				transformUndoStackPush(m_detectedPrimitive);
			}
		}
		else if (m_detectedBitmap != nullptr) {
			if (m_group->isAncestorOf(m_detectedBitmap)) {
				transformUndoStackPush();
			}
			else {
				transformUndoStackPush(m_detectedBitmap);
			}
		}
		else {
			transformUndoStackPush();
		}
        //选取区域的属性面板
        LaserApplication::mainWindow->onLaserPrimitiveGroupItemChanged();
		this->viewport()->repaint();
		return;
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
        QList<LaserPrimitive*> pList = m_scene->document()->primitives().values();
        QList<LaserPrimitive*> selectedList = m_scene->document()->selectedPrimitives();
        LaserPrimitive* selectedOnlyLine = nullptr;
        if (selectedList.size() == 1) {
            LaserPrimitive* sp = selectedList[0];
            QString className = sp->metaObject()->className();
            if (className == "LaserLine") {
                selectedOnlyLine = sp;
            }
        }
        if (event->button() == Qt::RightButton && pList.size() > 1) {
            if (m_detectedPrimitive) {
                QString className = m_detectedPrimitive->metaObject()->className();
                if (className == "LaserLine" && !selectedOnlyLine) {
                    m_mirrorLine = m_detectedPrimitive;
                }
            }
           
        }
        
    }
	//View Drag Ready
	else if (StateControllerInst.isInState(StateControllerInst.documentViewDragingState())) {
		//setInteractive(true);
		QGraphicsView::mouseReleaseEvent(event);
		QPixmap cMap(":/ui/icons/images/drag_hand.png");
		this->setCursor(cMap.scaled(30, 30, Qt::KeepAspectRatio));
		emit endViewDraging();
	}
    //Rect
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectCreatingState())) {
		
		if (utils::checkTwoPointEqueal(m_creatingRectStartPoint, m_creatingRectEndPoint)) {
			emit readyRectangle();
			return;
		}
		
        QRectF rect(backgroundItem->QGraphicsItemGroup::mapFromScene(m_creatingRectStartPoint),
			backgroundItem->QGraphicsItemGroup::mapFromScene(m_creatingRectEndPoint));
		//QRectF rect(0, 0, 500, 300);
        if (rect.width() != 0 && rect.height() != 0) {
            LaserRect* rectItem = new LaserRect(rect, 0, m_scene->document(), QTransform(), m_curLayerIndex);
            //判断是否在4叉树的有效区域内
            if (m_scene->maxRegion().contains(QRectF(m_creatingRectStartPoint, m_creatingRectEndPoint))) {
                //undo 创建完后会执行redo
                QList<QGraphicsItem*> list;
                list.append(rectItem);
                AddDelUndoCommand* addCmd = new AddDelUndoCommand(m_scene.data(), list);
                m_undoStack->push(addCmd);
                //m_scene->addLaserPrimitive(rectItem);
                onReplaceGroup(rectItem);
            }
            else {
                QMessageBox::warning(this, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
                
            }
            
        }
        viewport()->repaint();
        emit readyRectangle();
    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
		if (utils::checkTwoPointEqueal(m_creatingEllipseStartPoint, m_EllipseEndPoint)) {
			emit readyEllipse();
			return;
		}
        QRectF rect(m_creatingEllipseStartPoint, m_EllipseEndPoint);
        LaserEllipse* ellipseItem = new LaserEllipse(rect, m_scene->document(), QTransform(), m_curLayerIndex);
        //m_scene->addLaserPrimitive(ellipseItem);
        //判断是否在4叉树的有效区域内
        if (m_scene->maxRegion().contains(rect)) {
            //undo 创建完后会执行redo
            QList<QGraphicsItem*> list;
            list.append(ellipseItem);
            AddDelUndoCommand* addCmd = new AddDelUndoCommand(m_scene.data(), list);
            m_undoStack->push(addCmd);
            onReplaceGroup(ellipseItem);
        }
        else {
            QMessageBox::warning(this, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
        }
        emit readyEllipse();
    }
	
	//Line
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineReadyState())) {
		if (event->button() == Qt::LeftButton) {
			m_creatingLineStartPoint = mapToScene(m_mousePoint);
			if (m_isPrimitiveInteractPoint) {
				m_creatingLineStartPoint = m_primitiveInteractPoint;
			}
			//是否网格点
			if (m_isGridNode) {
				m_creatingLineStartPoint = m_gridNode;
			}
			m_creatingLineEndPoint = m_creatingLineStartPoint;
			emit creatingLine();
		}
		
	}
	//Line
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveLineCreatingState())) {
		if (m_creatingLineStartPoint != m_creatingLineEndPoint) {
			if (event->button() == Qt::LeftButton) {
				QLineF line(m_creatingLineStartPoint, m_creatingLineEndPoint);
				LaserLine* lineItem = new LaserLine(line, m_scene->document(), QTransform(), m_curLayerIndex);
				//m_scene->addLaserPrimitive(lineItem);
                //判断是否在4叉树的有效区域内
                if (m_scene->maxRegion().contains(m_creatingLineStartPoint) && m_scene->maxRegion().contains(m_creatingLineEndPoint)) {
                    //undo 创建完后会执行redo
                    QList<QGraphicsItem*> list;
                    list.append(lineItem);
                    AddDelUndoCommand* addCmd = new AddDelUndoCommand(m_scene.data(), list);
                    m_undoStack->push(addCmd);
                    onReplaceGroup(lineItem);
                    
                }
                else {
                    QMessageBox::warning(this, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
                }
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

        qreal m_width = 5;
        qreal halfWidth = m_width * 0.5;
        m_polygonStartRect = QRectF(m_creatingPolygonStartPoint.x() - halfWidth, m_creatingPolygonStartPoint.y() - halfWidth, m_width, m_width);

        emit creatingPolygon();
    }*/
    //Polygon
	else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonReadyState())) {
		if (event->button() == Qt::LeftButton) {
			m_creatingPolygonStartPoint = mapToScene(event->pos());
			if (m_isPrimitiveInteractPoint) {
				m_creatingPolygonStartPoint = m_primitiveInteractPoint;
			}
			//是否网格点
			if (m_isGridNode) {
				m_creatingPolygonStartPoint = m_gridNode;
			}
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
			if (!isRepeatPoint()) {
				m_creatingPolygonLines.append(QLineF(m_creatingPolygonPoints[m_creatingPolygonPoints.size() - 1], m_creatingPolygonEndPoint));
				m_creatingPolygonPoints.append(m_creatingPolygonEndPoint);
				LaserPrimitive* primitive;
				LaserPolyline * curPolyline = new LaserPolyline(QPolygonF(m_creatingPolygonPoints), m_scene->document(),
					QTransform(), m_curLayerIndex);
				//m_scene->addLaserPrimitive(curPolyline);
				primitive = curPolyline;
				if (m_creatingPolygonEndPoint == m_creatingPolygonStartPoint) {
					LaserPolygon* polygon = new LaserPolygon(QPolygonF(m_creatingPolygonPoints), m_scene->document(),
						QTransform(), m_curLayerIndex);
					//m_scene->addLaserPrimitive(polygon);
					//onReplaceGroup(polygon);
					m_creatingPolygonPoints.clear();
					setCursor(Qt::ArrowCursor);
					emit readyPolygon();				
					primitive = polygon;
				}	
                //判断是否在4叉树的有效区域内
                if (m_scene->maxRegion().contains(m_creatingPolygonEndPoint)) {
                    //undo
                    PolygonUndoCommand* polyCmd = new PolygonUndoCommand(m_scene.data(), m_lastPolygon, primitive);
                    m_undoStack->push(polyCmd);
                    m_lastPolygon = primitive;
                }
                else {
                    QMessageBox::warning(this, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
                    m_creatingPolygonPoints.removeLast();
                }
				
			}
		}
		else if (event->button() == Qt::RightButton) {
			if ( m_creatingPolygonPoints.size() > 0) {
				LaserPolyline* polyLine = new LaserPolyline(QPolygonF(m_creatingPolygonPoints), m_scene->document(), 
					QTransform(), m_curLayerIndex);
				//m_scene->addLaserPrimitive(polyLine);
				//onReplaceGroup(polyLine);
				//undo
				PolygonUndoCommand* polyCmd = new PolygonUndoCommand(m_scene.data(), m_lastPolygon, polyLine);
				m_lastPolygon = polyLine;
				//m_undoStack->push(polyCmd);
			}
			m_creatingPolygonPoints.clear();
			setCursor(Qt::ArrowCursor);
			emit readyPolygon();

		}
    }
    //text
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextReadyState())) {
        m_textMousePressPos = event->pos();
        LaserText* text = nullptr;
        detectTextInsertPosition(mapToScene(m_textMousePressPos.toPoint()), text);
        m_editingText = text;
        modifyTextCursor();
        //m_textInsertPos = m_textMousePressPos;
        this->setAttribute(Qt::WA_InputMethodEnabled, true);
        this->setAttribute(Qt::WA_KeyCompression, true);
        this->setFocusPolicy(Qt::WheelFocus);
        emit creatingText();
        viewport()->repaint();
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
        if (m_scene->selectedPrimitives().length() == 1) {
            LaserPath* path = (LaserPath*)m_scene->selectedPrimitives()[0];
            path->setVisible(false);
        }
    }
    else
    {
		QGraphicsView::mouseReleaseEvent(event);
    }
	
    //m_mousePressed = false;
    //m_isKeyShiftPressed = false;
    this->viewport()->repaint();
	//QGraphicsView::mouseReleaseEvent(event);
}

void LaserViewer::dragEnterEvent(QDragEnterEvent * event)
{
	QGraphicsView::dragEnterEvent(event);
	qDebug() << event->pos();
}

void LaserViewer::dragLeaveEvent(QDragLeaveEvent * event)
{
	QGraphicsView::dragLeaveEvent(event);
}

void LaserViewer::dragMoveEvent(QDragMoveEvent * event)
{
	QGraphicsView::dragMoveEvent(event);
}

void LaserViewer::dropEvent(QDropEvent * event)
{
	QGraphicsView::dropEvent(event);
}

void LaserViewer::keyPressEvent(QKeyEvent* event)
{
    QGraphicsView::keyPressEvent(event);
    if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextState())) {
        this->setAttribute(Qt::WA_InputMethodEnabled, true);
    }
    qDebug() << event->key();
    switch (event->key())
    {
		case Qt::Key_Shift: {
            if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectState()) ||
                StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseState())) {
                m_isKeyShiftPressed = true;
                viewport()->repaint();
            }
            
			break;
		}
		case Qt::Key_Delete: {
			break;
		}
		case Qt::Key_Control: {
			m_isKeyCtrlPress = true;
			break;
		}
        case Qt::Key_CapsLock: {
            if (m_isCapsLock) {
                m_isCapsLock = false;
            }
            else {
                m_isCapsLock = true;
            }
            
            break;
        }
        //number
        case Qt::Key_0: {
            addTextByKeyInput("0");
            break;
        }
        case Qt::Key_1: {
            addTextByKeyInput("1");
            break;
        }
        case Qt::Key_2: {
            addTextByKeyInput("2");
            break;
        }
        case Qt::Key_3: {
            addTextByKeyInput("3");
            break;
        }
        case Qt::Key_4: {
            addTextByKeyInput("4");
            break;
        }
        case Qt::Key_5: {
            addTextByKeyInput("5");
            break;
        }
        case Qt::Key_6: {
            addTextByKeyInput("6");
            break;
        }
        case Qt::Key_7: {
            addTextByKeyInput("7");
            break;
        }
        case Qt::Key_8: {
            addTextByKeyInput("8");
            break;
        }
        case Qt::Key_9: {
            addTextByKeyInput("9");
            break;
        }
        case Qt::Key_AsciiTilde: {
            addTextByKeyInput("~");
            break;
        }
        case Qt::Key_Minus: {
            addTextByKeyInput("-");
            break;
        }
        case Qt::Key_Plus: {
            addTextByKeyInput("+");
            break;
        }
        case Qt::Key_Underscore: {
            addTextByKeyInput("_");
            break;
        }
        case Qt::Key_Exclam: {
            addTextByKeyInput("!");
            break;
        }
        case Qt::Key_QuoteDbl: {
            addTextByKeyInput(QString('"'));
            break;
        }
        case Qt::Key_NumberSign: {
            addTextByKeyInput("#");
            break;
        }
        case Qt::Key_Dollar: {
            addTextByKeyInput("$");
            break;
        }
        case Qt::Key_Percent: {
            addTextByKeyInput("%");
            break;
        }
        case Qt::Key_Ampersand: {
            addTextByKeyInput("&");
            break;
        }
        case Qt::Key_Apostrophe: {
            addTextByKeyInput("'");
            break;
        }
        case Qt::Key_ParenLeft: {
            addTextByKeyInput("(");
            break;
        }
        case Qt::Key_ParenRight: {
            addTextByKeyInput(")");
            break;
        }
        case Qt::Key_Asterisk: {
            addTextByKeyInput("*");
            break;
        }
        case Qt::Key_Period: {
            addTextByKeyInput(".");
            break;
        }
        case Qt::Key_Slash: {
            addTextByKeyInput("/");
            break;
        }
        case Qt::Key_Colon: {
            addTextByKeyInput(":");
            break;
        }
        case Qt::Key_Semicolon: {
            addTextByKeyInput(";");
            break;
        }case Qt::Key_Less: {
            addTextByKeyInput("<");
            break;
        }case Qt::Key_Equal: {
            addTextByKeyInput("=");
            break;
        }case Qt::Key_Greater: {
            addTextByKeyInput(">");
            break;
        }case Qt::Key_Question: {
            addTextByKeyInput("?");
            break;
        }
        case Qt::Key_At: {
            addTextByKeyInput("@");
            break;
        }
        case Qt::Key_BracketLeft: {
            addTextByKeyInput("[");
            break;
        }
        case Qt::Key_Backslash: {
            addTextByKeyInput("\\");
            break;
        }
        case Qt::Key_BracketRight: {
            addTextByKeyInput("]");
            break;
        }
        case Qt::Key_AsciiCircum: {
            addTextByKeyInput("^");
            break;
        }
        case Qt::Key_BraceLeft: {
            addTextByKeyInput("{");
            break;
        }
        case Qt::Key_QuoteLeft: {
            addTextByKeyInput("`");
            break;
        }
        case Qt::Key_Bar: {
            addTextByKeyInput("|");
            break;
        }
        case Qt::Key_BraceRight: {
            addTextByKeyInput("}");
            break;
        }
        case Qt::Key_Comma: {
            addTextByKeyInput(",");
            break;
        }
        
        //letter
        case Qt::Key_A: {

            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("a");
                }
                else {
                    addTextByKeyInput("A");
                }
                
            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("A");
                }
                else {
                    addTextByKeyInput("a");
                }
            }
            break;
        }
        case Qt::Key_B: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("b");
                }
                else {
                    addTextByKeyInput("B");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("B");
                }
                else {
                    addTextByKeyInput("b");
                }
            }
            break;
        }
        case Qt::Key_C: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("c");
                }
                else {
                    addTextByKeyInput("C");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("C");
                }
                else {
                    addTextByKeyInput("c");
                }
            }
            break;
        }
        case Qt::Key_D: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("d");
                }
                else {
                    addTextByKeyInput("D");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("D");
                }
                else {
                    addTextByKeyInput("d");
                }
            }
            break;
        }
        case Qt::Key_E: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("e");
                }
                else {
                    addTextByKeyInput("E");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("E");
                }
                else {
                    addTextByKeyInput("e");
                }
            }
            break;
        }
        case Qt::Key_F: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("f");
                }
                else {
                    addTextByKeyInput("F");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("F");
                }
                else {
                    addTextByKeyInput("f");
                }
            }
            break;
        }
        case Qt::Key_G: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("g");
                }
                else {
                    addTextByKeyInput("G");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("G");
                }
                else {
                    addTextByKeyInput("g");
                }
            }
            break;
        }
        case Qt::Key_H: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("h");
                }
                else {
                    addTextByKeyInput("H");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("H");
                }
                else {
                    addTextByKeyInput("h");
                }
            }
            break;
        }

        case Qt::Key_R: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("r");
                }
                else {
                    addTextByKeyInput("R");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("R");
                }
                else {
                    addTextByKeyInput("r");
                }
            }
            break;
        }
        case Qt::Key_J: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("j");
                }
                else {
                    addTextByKeyInput("J");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("J");
                }
                else {
                    addTextByKeyInput("j");
                }
            }
            break;
        }
        case Qt::Key_K: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("k");
                }
                else {
                    addTextByKeyInput("K");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("K");
                }
                else {
                    addTextByKeyInput("k");
                }
            }
            break;
        }
        case Qt::Key_L: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("l");
                }
                else {
                    addTextByKeyInput("L");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("L");
                }
                else {
                    addTextByKeyInput("l");
                }
            }
            break;
        }
        case Qt::Key_M: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("m");
                }
                else {
                    addTextByKeyInput("M");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("M");
                }
                else {
                    addTextByKeyInput("m");
                }
            }
            break;
        }
        case Qt::Key_N: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("n");
                }
                else {
                    addTextByKeyInput("N");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("N");
                }
                else {
                    addTextByKeyInput("n");
                }
            }
            break;
        }
        case Qt::Key_O: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("o");
                }
                else {
                    addTextByKeyInput("O");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("O");
                }
                else {
                    addTextByKeyInput("o");
                }
            }
            break;
        }
        case Qt::Key_P: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("p");
                }
                else {
                    addTextByKeyInput("P");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("P");
                }
                else {
                    addTextByKeyInput("p");
                }
            }
            break;
        }
        case Qt::Key_Q: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("q");
                }
                else {
                    addTextByKeyInput("Q");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("Q");
                }
                else {
                    addTextByKeyInput("q");
                }
            }
            break;
        }
        case Qt::Key_I: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("i");
                }
                else {
                    addTextByKeyInput("I");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("I");
                }
                else {
                    addTextByKeyInput("i");
                }
            }
            break;
        }
        case Qt::Key_S: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("s");
                }
                else {
                    addTextByKeyInput("S");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("S");
                }
                else {
                    addTextByKeyInput("s");
                }
            }
            break;
        }
        case Qt::Key_T: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("t");
                }
                else {
                    addTextByKeyInput("T");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("T");
                }
                else {
                    addTextByKeyInput("t");
                }
            }
            break;
        }
        case Qt::Key_U: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("u");
                }
                else {
                    addTextByKeyInput("U");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("U");
                }
                else {
                    addTextByKeyInput("u");
                }
            }
            break;
        }
        case Qt::Key_V: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("v");
                }
                else {
                    addTextByKeyInput("V");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("V");
                }
                else {
                    addTextByKeyInput("v");
                }
            }
            break;
        }
        case Qt::Key_W: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("w");
                }
                else {
                    addTextByKeyInput("W");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("W");
                }
                else {
                    addTextByKeyInput("w");
                }
            }
            break;
        }
        case Qt::Key_X: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("x");
                }
                else {
                    addTextByKeyInput("X");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("X");
                }
                else {
                    addTextByKeyInput("x");
                }
            }
            break;
        }
        case Qt::Key_Y: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("y");
                }
                else {
                    addTextByKeyInput("Y");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("Y");
                }
                else {
                    addTextByKeyInput("y");
                }
            }
            break;
        }
        case Qt::Key_Z: {
            if (event->modifiers() == Qt::ShiftModifier) {
                if (m_isCapsLock) {
                    addTextByKeyInput("z");
                }
                else {
                    addTextByKeyInput("Z");
                }

            }
            else {
                if (m_isCapsLock) {
                    addTextByKeyInput("Z");
                }
                else {
                    addTextByKeyInput("z");
                }
            }
            break;
        }
    }
    
}

void LaserViewer::keyReleaseEvent(QKeyEvent* event)
{
    QGraphicsView::keyReleaseEvent(event);
    if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextState())) {
        this->setAttribute(Qt::WA_InputMethodEnabled, true);
    }
    else {
        this->setAttribute(Qt::WA_InputMethodEnabled, false);
    }
    
    switch (event->key())
    {
		case Qt::Key_Escape:
			/*if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {

				addText("j");
				//emit readyText();
			}*/
			break;
		case Qt::Key_Shift: {
            if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectState()) ||
                StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseState())) {
                m_isKeyShiftPressed = false;
                m_creatingRectEndPoint = m_creatingRectBeforeShiftPoint;
                viewport()->repaint();
            }
            
			
			break;
		}
		case Qt::Key_Control: {
			m_isKeyCtrlPress = false;
			break;
		}
        //text
                              
        case Qt::Key_Return: {
            if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {
                addTextByKeyInput("\n");
            }
            break;
        }
        case Qt::Key_Enter: {
            if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {
                    addTextByKeyInput("\n");
                    //event->accept();
            }
            break;
        }
        case Qt::Key_Delete: {
            if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextState())) {
                removeBackText();
            }
            break;
        }
        case Qt::Key_Backspace: {
            removeFrontText();
            break;
        }
        case Qt::Key_Space: {
            int v = event->type();
            if (event->type() != QEvent::InputMethod) {
                addTextByKeyInput(" ");
            }
            
            break;
        }
        /*case Qt::Key_nobreakspace: {
            //addTextByKeyInput(" ");
            break;
        }*/
        
    }
    
}

void LaserViewer::inputMethodEvent(QInputMethodEvent * event)
{
    QWidget::inputMethodEvent(event);
    if (!event->commitString().isEmpty()) {
        addTextByKeyInput(event->commitString());
        
    }
    
}

void LaserViewer::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
	//setAnchorPoint(mapFromScene(0, 0));
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

QMap<QGraphicsItem*, QTransform> LaserViewer::clearGroupSelection()
{
	QMap<QGraphicsItem*, QTransform> selectedList;
    
	m_scene->blockSignals(true);
	if (!m_group) {
		return selectedList;
	}
	if (!m_group->isEmpty())
	{
		
        //清空group
		for (QGraphicsItem *item : m_group->childItems()) {
            selectedList.insert(item, item->sceneTransform());
			LaserPrimitive* p_item = qgraphicsitem_cast<LaserPrimitive*>(item);
			m_group->removeFromGroup(p_item);
			p_item->blockSignals(true);
			p_item->setSelected(false);
			p_item->blockSignals(false);
            
		}
        //m_scene->destroyItemGroup(m_group);
        //m_group = nullptr;
	}
    //清空选取区域
    for (QGraphicsItem *item : m_scene->document()->selectedPrimitives()) {
        LaserPrimitive* p_item = qgraphicsitem_cast<LaserPrimitive*>(item);
        p_item->blockSignals(true);
        p_item->setSelected(false);
        p_item->blockSignals(false);
        selectedList.insert(p_item, p_item->sceneTransform());
    }
	m_group->setTransform(QTransform());
    
	m_scene->clearSelection();
	m_scene->blockSignals(false);
    viewport()->repaint();
	emit m_scene->selectionChanged();
	return selectedList;
}

/*bool LaserViewer::detectPoint(QVector<QPointF> points, QList<QLineF> lines, QPointF& point)
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
		if (standarVec == vector) {
			setCursor(Qt::IBeamCursor);
			return true;
		}
	}
	setCursor(Qt::ArrowCursor);
	return false;
}*/

bool LaserViewer::isRepeatPoint()
{
	bool bl = true;
	if (m_creatingPolygonPoints[m_creatingPolygonPoints.size() - 1] != m_creatingPolygonEndPoint) {
		bl = false;
	}
	if (bl) {
		qDebug() << "==";
	}
	return bl;
}

bool LaserViewer::isStartPoint()
{
	if (m_creatingPolygonStartPoint != m_creatingPolygonEndPoint) {
		return false;
	}
	return true;
}

qreal LaserViewer::leftScaleMirror(qreal rate, qreal x)
{
	qreal xRate = rate;
	//判断左右反转
	if (m_isItemScaleChangeX) {
		if (x > 0) {
			xRate = -rate;
			m_isItemScaleChangeX = false;
		}
	}
	else {
		if (x < 0) {
			xRate = -rate;
			m_isItemScaleChangeX = true;
		}
	}
	return xRate;
}

qreal LaserViewer::rightScaleMirror(qreal rate, qreal x)
{
	qreal xRate = rate;
	//判断左右反转
	if (m_isItemScaleChangeX) {
		if (x < 0) {
			xRate = -rate;
			m_isItemScaleChangeX = false;
		}
	}
	else {
		if (x > 0) {
			xRate = -rate;
			m_isItemScaleChangeX = true;
		}
	}
	return xRate;
}

qreal LaserViewer::topScaleMirror(qreal rate, qreal y)
{
	qreal yRate = rate;
	//判断上下反转
	if (m_isItemScaleChangeY) {
		if (y > 0) {
			yRate = -rate;
			m_isItemScaleChangeY = false;
		}
	}
	else {
		if (y < 0) {
			yRate = -rate;
			m_isItemScaleChangeY = true;
		}
	}
	return yRate;
}

qreal LaserViewer::bottomScaleMirror(qreal rate, qreal y)
{
	qreal yRate = rate;
	//判断上下反转
	if (m_isItemScaleChangeY) {
		if (y < 0) {
			yRate = -rate;
			m_isItemScaleChangeY = false;
		}
	}
	else {
		if (y > 0) {
			yRate = -rate;
			m_isItemScaleChangeY = true;
		}
	}
	return yRate;
}

LaserPrimitive* LaserViewer::mirrorLine()
{
    return m_mirrorLine;
}

void LaserViewer::setMirrorLine(LaserPrimitive* l)
{
    m_mirrorLine = l;
}

void LaserViewer::setGroupNull()
{
	m_group = nullptr;
}

void LaserViewer::pointSelectWhenSelectedState(int handleIndex, LaserPrimitive * primitive)
{
	
	//因为图片被选中时需要描外框， 所以在LaserPrimitive的draw里会用类型判断是否描外框
	QString className = primitive->metaObject()->className();
	if (className == "LaserBitmap") {
		LaserBitmap* bitmap = qobject_cast<LaserBitmap*> (primitive);
		if (!m_group->isAncestorOf(bitmap)) {
			if (bitmap) {
				if (m_isKeyCtrlPress) {
						resetGroup();
				}
				else {
						clearGroupSelection();
				}

				bitmap->setSelected(true);
				if (!onSelectedFillGroup()) {
					emit selectionToIdle();
				}

			}
		}
		//selected
		else {
			if (bitmap) {
				if (m_isKeyCtrlPress) {
						resetGroup();
					if (m_scene->selectedPrimitives().size() == 1) {
						emit beginSelectedEditing();
						this->viewport()->repaint();
						m_curSelectedHandleIndex = handleIndex;//点选
					}
					bitmap->setSelected(false);
					if (!onSelectedFillGroup()) {
						this->viewport()->repaint();
						return;
					}
				}
			}
		}
		emit beginSelectedEditing();
		m_curSelectedHandleIndex = handleIndex;//点选
		this->viewport()->repaint();		
		return;
	}
	
	//if (!m_group->isAncestorOf(primitive)) {
    if (!scene()->selectedPrimitives().contains(primitive)) {
		if (primitive) {
			if (m_isKeyCtrlPress) {
					resetGroup();
			}
			else {
					clearGroupSelection();
                    
			}

			primitive->setSelected(true);
			if (!onSelectedFillGroup()) {
				emit selectionToIdle();
			}

		}
	}
	//selected
	else {
		if (primitive) {
			if (m_isKeyCtrlPress) {
					resetGroup();

				if (m_scene->selectedPrimitives().size() == 1) {
					emit beginSelectedEditing();
					this->viewport()->repaint();
					m_curSelectedHandleIndex = handleIndex;//点选
				}
				primitive->setSelected(false);
				if (!onSelectedFillGroup()) {
					this->viewport()->repaint();
					return;
				}
			}
		}
	}
	emit beginSelectedEditing();
	m_curSelectedHandleIndex = handleIndex;//点选
	this->viewport()->repaint();
}
//mouse press：idle或selected satate 
//mouse release：切换到了selecting state 
//点选或框选都为空白，没有选中图元
//框选，点选分不同的状态过来，做不同处理
void LaserViewer::selectingReleaseInBlank()
{
	//idle state press mouse
	if (m_mousePressState == StateControllerInst.documentIdleState()) {
        if (m_isKeyCtrlPress) {
            //选区不变
            onEndSelecting();
        }
        else {
            //undo
            selectionUndoStackPushBefore();
            for (QGraphicsItem* item : m_scene->selectedPrimitives()) {
                item->setSelected(false);
            }
            emit selectionToIdle();
            //undo redo
            selectionUndoStackPush();
        }

	}
	//selected satate press
	else {
		if (m_isKeyCtrlPress) {
			//选区不变
			onEndSelectionFillGroup();
		}
		else {
			//m_scene->clearSelection();
			if (!m_scene->selectedItems().isEmpty()) {
                
				//undo
				selectionUndoStackPushBefore();
				//清理group及所有被选中item
				clearGroupSelection();
				emit selectionToIdle();
				//undo redo
				selectionUndoStackPush();
                
			}
            
		}
	}
	
	viewport()->repaint();
}

void LaserViewer::selectionUndoStackPushBefore()
{
	m_undoSelectionList.clear();
	if (m_group) {
		for each(QGraphicsItem* item in m_scene->selectedPrimitives()) {
			m_undoSelectionList.insert(item, item->sceneTransform());
		}
	}
}

void LaserViewer::selectionUndoStackPush()
{
	QMap<QGraphicsItem*, QTransform> redoList;

	if (m_group) {
		for each(QGraphicsItem* item in m_scene->selectedPrimitives()) {
			redoList.insert(item, item->sceneTransform());
		}
		//redoList = m_group->childItems();
	}
	if (redoList == m_undoSelectionList) {
		return;
	}

	SelectionUndoCommand* selection = new SelectionUndoCommand(this,
		m_undoSelectionList, redoList);
	m_undoStack->push(selection);
}

void LaserViewer::transformUndoStackPushBefore(LaserPrimitive* item)
{
	
    if (item) {
        m_singleLastTransform = item->sceneTransform();
    }
    else {
        m_groupLastTransform = m_group->transform();
    }
    
}

void LaserViewer::transformUndoStackPush(LaserPrimitive* item)
{
    //判断是否在4叉树的有效区域内
    QRectF bounds;
    if (item) {
        bounds = item->sceneBoundingRect();
    }
    //group
    else {
        bounds = selectedItemsSceneBoundingRect();
    }
    //垂直或水平线，点的情况
    if (bounds.width() == 0 || bounds.height() == 0) {
        if (!m_scene->maxRegion().contains(bounds.topLeft()) || !m_scene->maxRegion().contains(bounds.bottomRight())) {
            QMessageBox::warning(this, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
            if (item) {
                item->setTransform(m_singleLastTransform);
            }
            //group
            else {
                m_group->setTransform(m_groupLastTransform);
            }
            return;
        }
    }
    else {
        if (!m_scene->maxRegion().contains(bounds)) {
            QMessageBox::warning(this, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
            if (item) {
                item->setTransform(m_singleLastTransform);
            }
            //group
            else {
                m_group->setTransform(m_groupLastTransform);
            }
            return;
        }
    }
    //TransformUndoCommand
    if (item) {
        
        SingleTransformUndoCommand* cmd = new SingleTransformUndoCommand(m_scene.data(), m_singleLastTransform, item->sceneTransform(), item);
        m_undoStack->push(cmd);
    }
    //group
    else {

        GroupTransformUndoCommand* cmd = new GroupTransformUndoCommand(m_scene.data(), m_groupLastTransform, m_group->transform());
        m_undoStack->push(cmd);
    }
    
}

int LaserViewer::textAlignH()
{
    return m_textAlighH;
}

int LaserViewer::textAlignV()
{
    return m_textAlighV;
}

void LaserViewer::setTextAlignH(int align)
{
    m_textAlighH = align;
}

void LaserViewer::setTextAlignV(int align)
{
    m_textAlighV = align;
}

void LaserViewer::updateGroupTreeNode()
{
    if (m_group && !m_group->isEmpty()) {
        for (QGraphicsItem* item : m_group->childItems()) {
            LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
            m_scene->quadTreeNode()->upDatePrimitive(primitive);
        }
    }
}

qreal LaserViewer::zoomValue() const
{
	
    return transform().m11();
}

void LaserViewer::setZoomValue(qreal zoomValue)
{
    //scale(1 / transform().m11(), 1 / transform().m22());
    //scale(zoomValue, zoomValue);
	zoomBy(zoomValue / this->zoomValue(), this->rect().center());
    //emit scaleChanged(zoomValue);
}

qreal LaserViewer::adapterViewScale()
{
    QRectF viewRect = this->rect();
    QRectF docRect = m_scene->backgroundItem()->rect();
    qreal wScale = viewRect.width() / docRect.width();
    qreal hScale = viewRect.height() / docRect.height();
    qreal scale = 1;
    if (wScale < hScale) {
        scale = (viewRect.width() - 20) / docRect.width();
    }
    else {
        scale = (viewRect.height() - 20) / docRect.height();
    }
    return scale;
}

void LaserViewer::init()
{
	//setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
	setMouseTracking(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontClipPainter | QGraphicsView::DontSavePainterState);	
	horizontalScrollBar()->setEnabled(false);
	verticalScrollBar()->setEnabled(false);
	//undo stack
	m_undoStack = new QUndoStack(this);
	m_copyedList = QMap<LaserPrimitive*, QTransform>();
	//arrange
	m_copyedList = QMap<LaserPrimitive*, QTransform>();
	m_groupedMap = QMap<QString, QList<LaserPrimitive*>>();
	m_selectedGroupedList = QList<QString>();
	
	
    ADD_TRANSITION(documentIdleState, documentSelectingState, this, &LaserViewer::beginSelecting);
	ADD_TRANSITION(documentIdleState, documentSelectedEditingState, this, &LaserViewer::beginIdelEditing);
	ADD_TRANSITION(documentIdleState, documentSelectedState, this, &LaserViewer::idleToSelected);

    ADD_TRANSITION(documentSelectionState, documentIdleState, this, &LaserViewer::selectionToIdle);
    ADD_TRANSITION(documentSelectionState, documentIdleState, this, &LaserViewer::cancelSelected);

    ADD_TRANSITION(documentSelectingState, documentSelectedState, this, &LaserViewer::endSelecting);

    ADD_TRANSITION(documentSelectedState, documentSelectingState, this, &LaserViewer::beginSelecting);
    ADD_TRANSITION(documentSelectedState, documentSelectedEditingState, this, &LaserViewer::beginSelectedEditing);
	ADD_TRANSITION(documentIdleState, documentSelectedState, this, &LaserViewer::endSelecting);

    ADD_TRANSITION(documentSelectedEditingState, documentSelectedState, this, &LaserViewer::endSelectedEditing);

    ADD_TRANSITION(documentPrimitiveRectState, documentPrimitiveRectCreatingState, this, SIGNAL(creatingRectangle()));
    ADD_TRANSITION(documentPrimitiveRectCreatingState, documentPrimitiveRectReadyState, this, SIGNAL(readyRectangle()));
    ADD_TRANSITION(documentPrimitiveEllipseState, documentPrimitiveEllipseCreatingState, this, SIGNAL(creatingEllipse()));
    ADD_TRANSITION(documentPrimitiveEllipseCreatingState, documentPrimitiveEllipseReadyState, this, SIGNAL(readyEllipse()));
    ADD_TRANSITION(documentPrimitiveLineState, documentPrimitiveLineCreatingState, this, SIGNAL(creatingLine()));
    ADD_TRANSITION(documentPrimitiveLineCreatingState, documentPrimitiveLineReadyState, this, SIGNAL(readyLine()));
    ADD_TRANSITION(documentPrimitivePolygonReadyState, documentPrimitivePolygonCreatingState, this, SIGNAL(creatingPolygon()));
    ADD_TRANSITION(documentPrimitivePolygonCreatingState, documentPrimitivePolygonReadyState, this, SIGNAL(readyPolygon()));
    ADD_TRANSITION(documentPrimitiveSplineReadyState, documentPrimitiveSplineCreatingState, this, SIGNAL(creatingSpline()));
    ADD_TRANSITION(documentPrimitiveSplineCreatingState, documentPrimitiveSplineReadyState, this, SIGNAL(readySpline()));
    ADD_TRANSITION(documentPrimitiveTextReadyState, documentPrimitiveTextCreatingState, this, SIGNAL(creatingText()));
    ADD_TRANSITION(documentPrimitiveTextCreatingState, documentPrimitiveTextReadyState, this, SIGNAL(readyText()));
	ADD_TRANSITION(documentViewDragReadyState, documentViewDragingState, this, SIGNAL(beginViewDraging()));
	ADD_TRANSITION(documentViewDragingState, documentViewDragReadyState, this, SIGNAL(endViewDraging()));

	connect(StateController::instance().documentViewDragState(), &QState::entered,this,  [=] {

		QPixmap cMap(":/ui/icons/images/drag_hand.png");
		this->setCursor(cMap.scaled(30, 30, Qt::KeepAspectRatio));

	});
	connect(StateController::instance().documentViewDragState(), &QState::exited,this, [=] {
		this->setCursor(Qt::ArrowCursor);
		
	});
    connect(StateController::instance().documentPrimitiveTextCreatingState(), &QState::exited, this, [=] {
        if (scene() && scene()->document()) {
            onEndText();
        }
        
    });
	m_group = nullptr;

}

void LaserViewer::initSpline()
{
    m_handlingSpline = SplineStruct();
    m_mouseHoverRect = QRectF();
}

/*void LaserViewer::creatTextEdit()
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
}*/

void LaserViewer::addText(QString str)
{
    
    if (!m_editingText) {
        m_insertIndex = 0;
        m_editingText = new LaserText(m_scene->document(), mapToScene(m_textMousePressPos.toPoint()),
            m_textFont, m_textAlighH, m_textAlighV,
            QTransform(),  m_curLayerIndex);
        
        m_editingText->addPath(str, m_insertIndex);
        m_insertIndex += str.size();
        m_scene->addLaserPrimitive(m_editingText);
    }
    else {
        
        m_editingText->addPath(str, m_insertIndex);
        m_insertIndex += str.size();
    }
    QLineF cursorLine = modifyTextCursor();
    //判断是否在4叉树的有效区域内
    if (!m_scene->maxRegion().contains(cursorLine.p1()) || !m_scene->maxRegion().contains(cursorLine.p2())) {
        QMessageBox::warning(this, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
        //OverstepMessageBoxWarn msgBox;
        //msgBox.exec();
        removeFrontText();
    }
    viewport()->repaint();
    
}

void LaserViewer::removeBackText()
{
    if (!m_editingText) {
        return;
    }
    int size = m_editingText->content().length();
    if (size > 0 && m_insertIndex >= 0 && m_insertIndex < size) {
        m_editingText->delPath(m_insertIndex);
        modifyTextCursor();
        if (m_editingText->path().isEmpty()) {
            m_scene->removeLaserPrimitive(m_editingText);
            m_editingText = nullptr;
        }
        viewport()->repaint();
        
    }
    
}

void LaserViewer::removeFrontText()
{
    if (!m_editingText) {
        return;
    }
    int size = m_editingText->content().length();
    if (size > 0 && m_insertIndex > 0 && m_insertIndex <= size) {
        m_editingText->delPath(m_insertIndex - 1);
        m_insertIndex -= 1;
        modifyTextCursor();
        if (m_editingText->path().isEmpty()) {
            m_scene->removeLaserPrimitive(m_editingText);
            m_editingText = nullptr;
        }
        viewport()->repaint();
        
    }
    
}

void LaserViewer::addTextByKeyInput(QString str)
{
    if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveTextCreatingState())) {
        addText(str);
    }
}

void LaserViewer::selectedHandleRotate() {

}
void LaserViewer::onEndSelecting() {
	if (m_scene->document()->selectedPrimitives().size() == 0) {
		m_scene->clearSelection();
		viewport()->repaint();
		emit selectionToIdle();
		
	}
	else {
		emit endSelecting();
	}
	viewport()->repaint();
}
void LaserViewer::onEndSelectionFillGroup()
{
	if (!onSelectedFillGroup()) {
		m_scene->clearSelection();
		viewport()->repaint();
		emit selectionToIdle();
	}
	else {
		emit endSelecting();
        
	}
    viewport()->repaint();
	
}
bool LaserViewer::onSelectedFillGroup()
{
	if (m_scene->document()->selectedPrimitives().size() == 0) {
		m_scene->clearSelection();
		viewport()->repaint();
		return false;
	}
	if (m_group)
	{
		
		for (LaserPrimitive* item : m_scene->document()->selectedPrimitives())
		{
			if (item->isLocked())
				continue;
			m_group->addToGroup(item);
		}
		m_group->setSelected(true);
	}
	else
	{
		QList<LaserPrimitive*>list = m_scene->document()->selectedPrimitives();
		if (list.isEmpty()) {
			return false;
		}
		m_group = m_scene->createItemGroup(list);
		m_group->setFlag(QGraphicsItem::ItemIsSelectable, true);
		m_group->setSelected(true);
		m_group->setZValue(1);
	}
	//绘制操作柄之前先清理一下
	m_selectedHandleList.clear();
	m_curSelectedHandleIndex = -1;
	return true;
}
//目前只在画完图元使用
QMap<QGraphicsItem*, QTransform> LaserViewer::onReplaceGroup(LaserPrimitive* item)
{
	//undo Before
	//selectionUndoStackPushBefore();

	QMap<QGraphicsItem*, QTransform> selectedListBeforeReplace = onCancelSelected();
	item->setSelected(true);
	onEndSelectionFillGroup();
	return selectedListBeforeReplace;
}

void LaserViewer::onEndText()
{
    if (m_editingText  && m_editingText->content().length() > 0) {

        clearGroupSelection();
        m_editingText->setSelected(true);
        onSelectedFillGroup();
        m_editingText = nullptr;
    }
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
	switch (m_curSelectedHandleIndex) {
		case 0: {
			break;
		}
		//scale
		//lefttop
		case 1: {
			QPointF lastVect = m_lastPos - mapFromScene(m_selectedRect.bottomRight());
			QPointF currVect = m_mousePoint - mapFromScene(m_selectedRect.bottomRight());
			
			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			if (v1.length() != 0 && v2.length() != 0) {
				rate = v2.length() / v1.length();
			}
			xRate = leftScaleMirror(rate, v2.x());
			yRate = topScaleMirror(rate, v2.y());
			break;
		}
		//righttop
		case 4: {
			QPointF lastVect = m_lastPos - mapFromScene(m_selectedRect.bottomLeft());
			QPointF currVect = m_mousePoint - mapFromScene(m_selectedRect.bottomLeft());

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			if (v1.length() != 0 && v2.length() != 0) {
				rate = v2.length() / v1.length();
			}
			xRate = rightScaleMirror(rate, v2.x());
			yRate = topScaleMirror(rate, v2.y());
			break;
		}
		//rightbottom
		case 7: {
			QPointF lastVect = m_lastPos - mapFromScene(m_selectedRect.topLeft());
			QPointF currVect = m_mousePoint - mapFromScene(m_selectedRect.topLeft());
			
			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			if (v1.length() != 0 && v2.length() != 0) {
				rate = v2.length() / v1.length();
			}
			xRate = rightScaleMirror(rate, v2.x());
			yRate = bottomScaleMirror(rate, v2.y());
			break;
		}
		//leftbottom
		case 10: {
			QPointF lastVect = m_lastPos - mapFromScene(m_selectedRect.topRight());
			QPointF currVect = m_mousePoint - mapFromScene(m_selectedRect.topRight());

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			if (v1.length() != 0 && v2.length() != 0) {
				rate = v2.length() / v1.length();
				
			}
			xRate = leftScaleMirror(rate, v2.x());
			yRate = bottomScaleMirror(rate, v2.y());
			
			break;
		}
		//stretch
		//topCenter
		case 3: {
			qreal bottom = mapFromScene(m_selectedRect.bottomLeft()).y();
			QPointF lastVect = m_lastPos - QPointF(m_lastPos.x(), bottom);
			QPointF currVect = m_mousePoint - QPointF(m_mousePoint.x(), bottom);

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			if (v1.length() != 0 && v2.length() != 0) {
				rate = v2.length() / v1.length();
			}
			xRate = 1;
			yRate = topScaleMirror(rate, v2.y());
			break;
		}
		//bottomCenter
		case 9: {
			qreal top = mapFromScene(m_selectedRect.topLeft()).y();
			QPointF lastVect = m_lastPos - QPointF(m_lastPos.x(), top);
			QPointF currVect = m_mousePoint - QPointF(m_mousePoint.x(), top);

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			if (v1.length() != 0 && v2.length() != 0) {
				rate = v2.length() / v1.length();
			}
			xRate = 1;
			yRate = bottomScaleMirror(rate, v2.y());
			break;
		}
		//rightCenter
		case 6: {
			qreal left = mapFromScene(m_selectedRect.topLeft()).x();
			QPointF lastVect = m_lastPos - QPointF(left, m_lastPos.y());
			QPointF currVect = m_mousePoint - QPointF(left, m_mousePoint.y());

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			if (v1.length() != 0 && v2.length() != 0) {
				rate = v2.length() / v1.length();
			}
			xRate = rightScaleMirror(rate, v2.x());
			yRate = 1;
			break;
		}
		//leftCenter
		case 12: {
			qreal right = mapFromScene(m_selectedRect.topRight()).x();
			QPointF lastVect = m_lastPos - QPointF(right, m_lastPos.y());
			QPointF currVect = m_mousePoint - QPointF(right, m_mousePoint.y());

			QVector2D v1(lastVect);
			QVector2D v2(currVect);
			if (v1.length() != 0 && v2.length() != 0) {
				rate = v2.length() / v1.length();
			}
			xRate = leftScaleMirror(rate, v2.x());
			yRate = 1;
			break;
		}
		case 2://lefttop_rotate
		case 5://righttop_rotate
		case 8://rightbottom_rotate
		case 11: //leftbottom_rotate
		{

			break;
		}
	}
	QTransform t;
	qDebug() << " rate: "<< rate;
	switch (m_curSelectedHandleIndex) {
		case 0: 
		{
			//QPointF diff = m_group->mapFromScene(this->mapToScene(QPointF(m_mousePoint - m_origin).toPoint()));
			QPointF diff = (m_mousePoint - m_lastPos) ;
			t = m_group->transform();
			QTransform t1;
			t1.translate(diff.x() / zoomValue(), diff.y() / zoomValue());
			m_group->setTransform(t * t1);
            
			break;
		}
		case 13://点选
		{
			QPointF diff = (m_mousePoint - m_lastPos);
			if (m_group->isAncestorOf(m_detectedPrimitive)) {
				t = m_group->transform();
				QTransform t1;
				t1.translate(diff.x() / zoomValue(), diff.y() / zoomValue());
				m_group->setTransform(t * t1);
			}
			else {
                if (!m_detectedPrimitive->isLocked()) {
                    t = m_detectedPrimitive->transform();
                    QTransform t1;
                    t1.translate(diff.x() / zoomValue(), diff.y() / zoomValue());
                    m_detectedPrimitive->setTransform(t * t1);
                }
				

			}
			
			break;
		}
		case 14://点选图片
		{
			QPointF diff = (m_mousePoint - m_lastPos);
			if (m_group->isAncestorOf(m_detectedBitmap)) {
				t = m_group->transform();
				QTransform t1;
				t1.translate(diff.x() / zoomValue(), diff.y() / zoomValue());
				m_group->setTransform(t * t1);
			}
			else {
                if (!m_detectedBitmap->isLocked()) {
                    t = m_detectedBitmap->transform();
                    QTransform t1;
                    t1.translate(diff.x() / zoomValue(), diff.y() / zoomValue());
                    m_detectedBitmap->setTransform(t * t1);
                }
				

			}

			break;
		}
		//scale
		case 1:
		case 4:
		case 7:
		case 10: 
		//stretch
		case 3:
		case 9:
		case 12:
		case 6:
		{
			QTransform t1;
			t1.scale(xRate, yRate);
			m_newOrigin = t1.map(m_origin);
			QPointF diff = m_origin - m_newOrigin;
			t = m_group->transform();
			QTransform t2;
			t2.translate(diff.x(), diff.y());
			m_group->setTransform(t * t1*t2);
			break;
		}
		case 2:
		case 5:
		case 8:
		case 11: {
			QPointF vec2 = m_mousePoint - mapFromScene(selectedItemsSceneBoundingRect().center());
			QPointF vec1 = m_lastPos - mapFromScene(selectedItemsSceneBoundingRect().center());
			QVector2D v1(vec1);
			QVector2D v2(vec2);
			qreal radians = 0;
			qDebug() << "vec1: " << vec1;
			qDebug() << "vec2: " << vec2;
			
			v1.normalize();
			v2.normalize();
			qreal cos = QVector2D::dotProduct(v1, v2);
			if (qAbs(cos) <= 1) {
				radians = qAcos(cos);
			}
			
			if (QVector3D::crossProduct(QVector3D(v1, 0), QVector3D(v2, 0)).z() < 0) {
				radians = -radians;
			}
			if (std::isnan(radians)) {
				radians = 0;
			}
			//qDebug() << "radians: " << radians;

			m_radians += radians;
			//qDebug() << "m_radians: " << m_radians;
			t = m_group->transform();
			QTransform t1;
			t1.setMatrix(qCos(radians), qSin(radians), t1.m13(), -qSin(radians), qCos(radians), t.m23(), t1.m31(), t1.m32(), t1.m33());
			m_newOrigin = m_origin * t1;
			QTransform t2;
			QPointF diff = m_origin - m_newOrigin;
			t2.setMatrix(t2.m11(), t2.m12(), t2.m13(), t2.m21(), t2.m22(), t2.m23(), diff.x(), diff.y(), t2.m33());
			m_group->setTransform(t * t1 * t2);
			m_origin = m_origin * t1 * t2;
			break;
		}
		
	}
	m_lastPos = m_mousePoint;
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

    LaserPath* laserPath = new LaserPath(path, m_scene->document(), QTransform(), m_curLayerIndex);
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
// by center
void LaserViewer::zoomIn()
{
	QRectF rect = this->rect();
	QPointF center = rect.center();
    zoomBy(1.1, center);
}
// by center
void LaserViewer::zoomOut()
{
	QRectF rect = this->rect();
	QPointF center = rect.center();
    zoomBy(0.9, center);
}

void LaserViewer::resetZoom()
{
    if (!qFuzzyCompare(zoomValue(), qreal(1))) {
        //resetTransform();
		setTransform(QTransform());
		setZoomValue(1);//zoomBy()中更新网格和标尺
        //emit zoomChanged(mapFromScene(m_scene->backgroundItem()->QGraphicsItemGroup::pos()));
    }
}

void LaserViewer::zoomToSelection()
{
	QRectF rect = selectedItemsSceneBoundingRect();
    if (rect.isEmpty()) {
        rect = AllItemsSceneBoundingRect();
    }
	if (rect.width() == 0 && rect.height() == 0) {
		return;
	}
    rect = QRectF(QPointF(mapFromScene(rect.topLeft())), QPointF(mapFromScene(rect.bottomRight())));
	QRectF viewRect = this->rect();
	QPointF center = rect.center();
	qreal width = rect.width();
	qreal height = rect.height();
	qreal viewWidth = viewRect.width();
	qreal viewHeight = viewRect.height();
	qreal scale = 1;
    qreal scaleW = (viewWidth - 20) / width;
    qreal scaleH = (viewHeight - 20) / height;
    if (scaleW < scaleH) {
        scale = scaleW;
    }
    else {
        scale = scaleH;
    }
    
    if (!zoomBy(scale, center, true)) {
        QPointF diff = mapFrom(this, this->rect().center())- center.toPoint();
        QTransform t = transform();
        QTransform t1;
        t1.translate(diff.x(), diff.y());
        
        this->setTransform(t*t1);
    }
}

/*void LaserViewer::textAreaChanged()
{
    int newwidth = m_textEdit->document()->size().m_width();
    int newheight = m_textEdit->document()->size().m_height();
    m_textEdit->resize(newwidth, newheight);
}*/

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

QMap<QGraphicsItem*, QTransform> LaserViewer::onCancelSelected()
{
	
	//emit cancelSelected();
	QMap<QGraphicsItem*, QTransform> selectedList = clearGroupSelection();
	emit beginSelecting();
	viewport()->repaint();
	return selectedList;
}

bool LaserViewer::resetGroup()
{
	if (!m_group) {
		return false;
	}
	if (m_group->isEmpty())
	{
		m_group->setTransform(QTransform());
		return false;
	}
	const auto items = m_group->childItems();
	for (QGraphicsItem *item : items) {
		LaserPrimitive* p_item = qgraphicsitem_cast<LaserPrimitive*>(item);

		m_group->removeFromGroup(p_item);
		
	}
	m_group->setTransform(QTransform());
	return true;
}
