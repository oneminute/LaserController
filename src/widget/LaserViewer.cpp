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
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));

    if (StateControllerInst.isInState(StateControllerInst.documentIdleState()))
    {
        qLogD << "SelectedEditing paint";
    }
    //selectioin
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectingState()))
    {
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawRect(QRectF(m_selectionStartPoint, m_selectionEndPoint));
        //qLogD << "drawing: " << m_selectionStartPoint << ", " << m_selectionEndPoint;
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedEditingState())) {
        qDebug() << "SelectedEditing paint";
		switch (m_curSelectedHandleIndex) {
			case 0:{
				QPointF lastPos = m_selectedHandleList[0].center();
				QPointF offsetPos = m_mousePoint - lastPos;
				QPointF curHandleTopLeft = m_selectedHandleList[0].topLeft() + offsetPos;
				m_selectedHandleList[0] = QRectF(curHandleTopLeft.x(), curHandleTopLeft.y(), m_selectedHandleList[0].width(), m_selectedHandleList[0].height());
				for (int i = 0; i < m_scene->selectedItems().size(); i++) {

					QGraphicsItem* item = m_scene->selectedItems()[i];
					QMatrix matrix;
					matrix.translate(offsetPos.x(), offsetPos.y());
					//item->setTransform(QTransform(matrix), true);
					item->setPos(item->pos() + offsetPos);
				}
				//painter.drawRect(m_scene->selectedItems()[0]->boundingRect());
				break;
			}
			case 1: 
			case 4:
			case 7:
			case 10:
			{
				selectedHandleScale(painter);
				break;
			}
		}
		
    }
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
        qDebug() << "documentSelectedState paint: " << m_scene->selectedPrimitives();
        paintSelectedState(painter);
    }
    //Rect
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveRectCreatingState())) {
        painter.drawRect(QRectF(mapFromScene(m_creatingRectStartPoint), mapFromScene(m_creatingRectEndPoint)));
    }
    //Ellipse
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
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
        painter.drawLine(mapFromScene(m_creatingLineStartPoint), mapFromScene(m_creatingLineEndPoint));
    }
    //Polygon
    else if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonCreatingState())) {
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
}

void LaserViewer::paintSelectedState(QPainter& painter)
{
    QList<QGraphicsItem*> items = m_scene->selectedItems();
    if (items.size() <= 0) {
        return;
    }
    qreal left, right, top, bottom;
    for (int i = 0; i < items.size(); i++) {
		LaserPrimitive* item = (LaserPrimitive*)items[i];
        QRectF boundingRect = item->sceneBoundingRect();
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
	m_selectedRect = QRectF(QPointF(left, top), QPointF(right, bottom));
    qLogD << "viewer painter transform: " << painter.transform();
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
	selectionPath.addRect(QRectF(_startPoint, _endPoint));
	m_scene->setSelectionArea(mapToScene(selectionPath));
	//right select
	if (_endPoint.x() < _startPoint.x()) {
		m_scene->setSelectionArea(mapToScene(selectionPath), Qt::ItemSelectionOperation::ReplaceSelection, Qt::ItemSelectionMode::IntersectsItemShape);
	}
	//left selection
	else if (_endPoint.x() > _startPoint.x()) {
		m_scene->setSelectionArea(mapToScene(selectionPath), Qt::ItemSelectionOperation::ReplaceSelection, Qt::ItemSelectionMode::ContainsItemShape);
	}
}

void LaserViewer::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        //qreal wheelZoomValue = qPow(1.2, event->delta() / 240.0);
        qreal wheelZoomValue = 1 + event->delta() / 120.0 * 0.1;
        //qLogD << "wheelZoomValue: " << wheelZoomValue << ", delta: " << event->delta();
        zoomBy(wheelZoomValue);
    }
    else
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
                emit beginSelectedEditing();
            }
            else
            {
                emit cancelSelected();
                m_selectionStartPoint = event->pos();
                m_selectionEndPoint = m_selectionStartPoint;
                qDebug() << "begin to select";
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
        //qLogD << "moving update: ";
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
        qLogD << "documentSelectedEditingState move";
        QGraphicsView::mouseMoveEvent(event);
        this->viewport()->repaint();
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
    /*else
    {
        QGraphicsView::mouseMoveEvent(event);
    }*/
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
        emit endSelecting();
        m_selectedHandleList.clear();
        m_curSelectedHandleIndex = -1;
    }
    /*else if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
        m_selectedHandleList.clear();
        m_curSelectedHandleIndex = -1;
        qLogD << "end selected: " << m_scene->selectedPrimitives();
        emit cancelSelected();
    }*/
    else if (StateControllerInst.isInState(StateControllerInst.documentSelectedEditingState())) {
		//setSelectionArea(m_selectionStartPoint, m_selectionEndPoint);
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

void LaserViewer::selectedHandleScale(QPainter& painter)
{
	QPointF lastPos = m_selectedHandleList[m_curSelectedHandleIndex].center();
	qreal offset = (m_mousePoint - lastPos).manhattanLength();
	switch (m_curSelectedHandleIndex) {
		case 1: {
			if (m_mousePoint.x() < lastPos.x()) {
				offset = 1 + offset * 0.0025;
			}
			else if (m_mousePoint.x() > lastPos.x()) {
				offset = 1 - offset * 0.0025;
			}
			else {
				if (m_mousePoint.y() > lastPos.y()) {
					offset = 1 - offset * 0.0025;
				}
				else {
					offset = 1 + offset * 0.0025;
				}
			}
			break;
		}
		case 4: {
			if (m_mousePoint.x() > lastPos.x()) {
				offset = 1 + offset * 0.0025;
			}
			else if (m_mousePoint.x() < lastPos.x()) {
				offset = 1 - offset * 0.0025;
			}
			else {
				if (m_mousePoint.y() > lastPos.y()) {
					offset = 1 - offset * 0.0025;
				}
				else {
					offset = 1 + offset * 0.0025;
				}
			}
			break;
		}
		case 7: {
			if (m_mousePoint.x() > lastPos.x()) {
				offset = 1 + offset * 0.0025;
			}
			else if (m_mousePoint.x() < lastPos.x()) {
				offset = 1 - offset * 0.0025;
			}
			else {
				if (m_mousePoint.y() < lastPos.y()) {
					offset = 1 - offset * 0.0025;
				}
				else {
					offset = 1 + offset * 0.0025;
				}
			}
			break;
		}
		case 10: {
			if (m_mousePoint.x() < lastPos.x()) {
				offset = 1 + offset * 0.0025;
			}
			else if (m_mousePoint.x() < lastPos.x()) {
				offset = 1 - offset * 0.0025;
			}
			else {
				if (m_mousePoint.y() < lastPos.y()) {
					offset = 1 - offset * 0.0025;
				}
				else {
					offset = 1 + offset * 0.0025;
				}
			}
			break;
		}
	}
	
	QPointF offsetPos = m_mousePoint - lastPos;
	QPointF curHandleTopLeft = m_selectedHandleList[m_curSelectedHandleIndex].topLeft() + offsetPos;
	m_selectedHandleList[m_curSelectedHandleIndex] = QRectF(curHandleTopLeft.x(), curHandleTopLeft.y(), m_selectedHandleList[m_curSelectedHandleIndex].width(), m_selectedHandleList[m_curSelectedHandleIndex].height());
	for (int i = 0; i < m_scene->selectedItems().size(); i++) {

		LaserPrimitive* item = (LaserPrimitive*)m_scene->selectedItems()[i];
		QMatrix matrix;
		matrix.scale(item->scale() * offset, item->scale() * offset);
		//painter.device()-
		//matrix.scale(item->scale() * offset, item->scale() * offset);
		QPointF diff;
		if (m_curSelectedHandleIndex == 1) 
        {
            //计算该放大操作中的新原点位置
			QPointF newPoint = matrix.map(m_selectedRect.bottomRight());
			diff = m_selectedRect.bottomRight() - newPoint;
		}
		else if (m_curSelectedHandleIndex == 4) {
			QPointF newPoint = matrix.map(m_selectedRect.bottomLeft());
			diff = m_selectedRect.bottomLeft() - newPoint;
		}
		else if (m_curSelectedHandleIndex == 7) {
			QPointF newPoint = matrix.map(m_selectedRect.topLeft());
			diff = m_selectedRect.topLeft() - newPoint;
		}
		else if (m_curSelectedHandleIndex == 10) {
			QPointF newPoint = matrix.map(m_selectedRect.topRight());
			diff = m_selectedRect.topRight() - newPoint;
		}
		//item->setMatrix(matrix, true);

        
        // 平移原点
        matrix.translate(diff.x(), diff.y());
		item->setTransform(QTransform(matrix), true);
	}
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
