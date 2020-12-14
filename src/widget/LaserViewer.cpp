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

#include "scene/LaserPrimitive.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include "state/StateController.h"
#include "widget/SplineNode.h"

LaserViewer::LaserViewer(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new LaserScene)
    , m_rubberBandActive(false)
    , m_mousePressed(false)
	, m_ruller(this)
	, m_isKeyShiftPressed(false)
	, m_isMouseInStartRect(false)
	, m_splineNodeDrawWidth(3)
	, m_splineHandlerWidth(5)
	, m_splineNodeEditWidth(5)
	, m_handlingSpline(SplineStruct())
	, m_lastTime(0)
	, m_curTime(0)

{
    setScene(m_scene.data());
    init();
}

LaserViewer::~LaserViewer()
{
}

void LaserViewer::paintEvent(QPaintEvent * event)
{
    QGraphicsView::paintEvent(event);
	//RULLER
	m_ruller.draw();
	//painter
	QPainter painter(viewport());
	painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
	//selectioin
	if (StateControllerInst.onState(StateControllerInst.documentSelectingState()))
    {       
		painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawRect(QRectF(m_selectionStartPoint, m_selectionEndPoint));
	}
	//Rect
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveRectCreatingState())) {
		painter.drawRect(QRectF(mapFromScene(m_creatingRectStartPoint), mapFromScene(m_creatingRectEndPoint)));
	}
	//Ellipse
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
		painter.drawEllipse(QRectF(mapFromScene(m_creatingEllipseStartPoint), mapFromScene(m_creatingEllipseEndPoint)));
	}
	//Line
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveLineCreatingState())) {
		painter.drawLine(mapFromScene(m_creatingLineStartPoint), mapFromScene(m_creatingLineEndPoint));
	}
	//Polygon
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitivePolygonCreatingState())) {
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
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveSplineCreatingState())) {
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
		if (StateControllerInst.onState(StateControllerInst.documentPrimitiveTextCreatingState())) {
			
			m_curTime = m_time.elapsed();
			if (m_curTime>0 && m_curTime <= 1000) {
				
				painter.setPen(QPen(Qt::black, 3, Qt::SolidLine));
			}
			else if(m_curTime>1000 && m_curTime <=2000){
				painter.setPen(QPen(Qt::white, 3, Qt::SolidLine));
			}
			else if(m_curTime > 2000){
				m_time.restart();
			}
			
			QPointF point = mapFromScene(m_textInputPoint);
			painter.drawLine(QPointF(point.x(), point.y()-10), QPointF(point.x(), point.y() + 10));
		}
	}
}

void LaserViewer::wheelEvent(QWheelEvent * event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        zoomBy(qPow(1.2, event->delta() / 240.0));
    }
    else
        QGraphicsView::wheelEvent(event);
}

void LaserViewer::zoomBy(qreal factor)
{
    const qreal currentZoom = zoomFactor();
    if ((factor < 1 && currentZoom < 0.01) || (factor > 1 && currentZoom > 10))
        return;
    scale(factor, factor);
    m_scene->document()->setScale(factor);
    emit zoomChanged();
}

void LaserViewer::mousePressEvent(QMouseEvent * event)
{
    QGraphicsView::mousePressEvent(event);
    if (event->isAccepted())
        return;
    QList<QAbstractState*> states;
    states << StateControllerInst.documentIdleState()
        << StateControllerInst.documentSelectedState();
	//Left Button
	if (event->button() == Qt::LeftButton) {
		if (StateControllerInst.anyState(states))
		{
			
			m_selectionStartPoint = event->pos();
			m_selectionEndPoint = m_selectionStartPoint;
			emit beginSelecting();
			qDebug() << "begin to select";			
		}
		//Rect
		else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveRectReadyState())) {
			
			m_creatingRectStartPoint = mapToScene(event->pos());
			m_creatingRectEndPoint = m_creatingRectStartPoint;
			emit creatingRectangle();
		}
		//Ellipse
		else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveEllipseReadyState())) {
			
			m_creatingEllipseStartPoint = mapToScene(event->pos());
			m_creatingEllipseStartInitPoint = m_creatingEllipseStartPoint;
			m_creatingEllipseEndPoint = m_creatingEllipseStartPoint;
			emit creatingEllipse();
		}
		//Line
		else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveLineReadyState())) {
			
			m_creatingLineStartPoint = mapToScene(event->pos());
			m_creatingLineEndPoint = m_creatingLineStartPoint;
			emit creatingLine();
		}
		//Polygon Ready
		else if (StateControllerInst.onState(StateControllerInst.documentPrimitivePolygonReadyState())) {
			//clear
			m_creatingPolygonPoints.clear();
			emit creatingPolygonStartRect();
		}
		//Spline
		else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveSplineReadyState())) {
			initSpline();
			emit creatingSpline();
		}
		//Text
		else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveTextReadyState())) {
			m_textInputPoint = mapToScene(event->pos());
			m_time.start();
			emit creatingText();
		}
	}
    
}

void LaserViewer::mouseMoveEvent(QMouseEvent * event)
{
    QPoint point = event->pos();
	//shift keyboard
	if (event->modifiers() & Qt::ShiftModifier) {
		m_isKeyShiftPressed = true;
	}
	else {
		m_isKeyShiftPressed = false;
	}
    if (StateControllerInst.onState(StateControllerInst.documentSelectingState()))
    {
        m_selectionEndPoint = point;
        QPainterPath selectionPath;
        selectionPath.addRect(QRectF(m_selectionStartPoint, m_selectionEndPoint));
        
        m_scene->setSelectionArea(mapToScene(selectionPath));
        return;
    }
	//Rect
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveRectCreatingState())) {
		m_creatingRectEndPoint = mapToScene(point);
		return;

	}
	//Ellipse
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
		m_creatingEllipseEndPoint = mapToScene(point);
		if (m_isKeyShiftPressed) {
			qreal w = m_creatingEllipseEndPoint.x() - m_creatingEllipseStartPoint.x();
			qreal h = m_creatingEllipseEndPoint.y() - m_creatingEllipseStartPoint.y();
			if (qAbs(w) < qAbs(h)) {
				m_creatingEllipseEndPoint = QPointF(m_creatingEllipseStartPoint.x()+w, m_creatingEllipseStartPoint.y()+w);
			}
			else {
				m_creatingEllipseEndPoint = QPointF(m_creatingEllipseStartPoint.x()+h, m_creatingEllipseStartPoint.y()+h);
			}
		}
		return;
	}
	//Line
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveLineCreatingState())) {
		m_creatingLineEndPoint = mapToScene(point);
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
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitivePolygonCreatingState())) {
		
		m_isMouseInStartRect = m_polygonStartRect.contains(mapToScene(event->pos()));
		m_creatingPolygonEndPoint = mapToScene(point);
	}
	//Spline
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveSplineCreatingState())) {
		m_creatingSplineMousePos = event->pos();
		
	}
	
    else
    {
        QGraphicsView::mouseMoveEvent(event);
    }
    QPointF pos = mapToScene(point);
    emit mouseMoved(pos);
}

void LaserViewer::mouseReleaseEvent(QMouseEvent * event)
{
	
    if (StateControllerInst.onState(StateControllerInst.documentSelectingState()))
    {
        qreal distance = (m_selectionEndPoint - m_selectionStartPoint).manhattanLength();
        qDebug() << "distance:" << distance;
        if (distance <= 0.000001f)
        {
            m_scene->clearSelection();
            emit cancelSelecting();
            QGraphicsView::mouseReleaseEvent(event);
        }
        else
        {
            emit endSelecting();
        }
	}
	//Rect
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveRectCreatingState())) {
        QRectF rect(m_creatingRectStartPoint, m_creatingRectEndPoint);
		LaserRect *rectItem = new LaserRect(rect, m_scene->document());
		m_scene->addLaserPrimitive(rectItem);
		emit readyRectangle();
	}
	//Ellipse
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {		
		QRectF rect(m_creatingEllipseStartPoint, m_creatingEllipseEndPoint);
		LaserEllipse *ellipseItem = new LaserEllipse(rect, m_scene->document());
		m_scene->addLaserPrimitive(ellipseItem);
		emit readyEllipse();
	}
	//Line
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveLineCreatingState())) {
		if (m_creatingLineStartPoint != m_creatingLineEndPoint) {

			QLineF line(m_creatingLineStartPoint, m_creatingLineEndPoint);
			LaserLine *lineItem = new LaserLine(line, m_scene->document());
			m_scene->addLaserPrimitive(lineItem);
		}
			emit readyLine();
		
	}
	//Polygon Start
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitivePolygonStartRectState())) {
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
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitivePolygonCreatingState())) {
		m_creatingPolygonEndPoint = mapToScene(event->pos());
		if (m_polygonStartRect.contains(m_creatingPolygonEndPoint)) {
			m_creatingPolygonEndPoint = m_creatingPolygonStartPoint;
			if (m_creatingPolygonPoints.length() > 1) {
				m_creatingPolygonPoints.append(m_creatingPolygonEndPoint);
				for (int i = 0; i < m_creatingPolygonPoints.length(); i++) {
					m_creatingPolygonPoints[i] = m_creatingPolygonPoints[i];
				}
				QPolygonF qPolygon(m_creatingPolygonPoints);
				LaserPolygon *polygon = new LaserPolygon(qPolygon, m_scene->document());
				m_scene->addLaserPrimitive(polygon);
				emit readyPolygon();
			}
		}
		else {
			m_creatingPolygonPoints.append(m_creatingPolygonEndPoint);
		}
	}
	//Spline
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveSplineCreatingState())) {
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
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveSplineEditState())) {
		if (m_scene->selectedItems().length() == 1) {
			LaserPath *path = (LaserPath*)m_scene->selectedItems()[0];
			path->setVisible(false);
		}
	}
    else
    {
        QGraphicsView::mouseReleaseEvent(event);
    }

    m_mousePressed = false;
	m_isKeyShiftPressed = false;
}

void LaserViewer::keyPressEvent(QKeyEvent * event)
{
	switch (event->key())
	{
		case Qt::SHIFT:

			break;

	}
}

void LaserViewer::keyReleaseEvent(QKeyEvent * event)
{
	qDebug() << event->key() <<":"<< Qt::Key_Enter;
	
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
			if (StateControllerInst.onState(StateControllerInst.documentPrimitiveTextCreatingState())) {
				emit readyText();
			}
			break;

	}
}

qreal LaserViewer::zoomFactor() const
{
    return transform().m11();
}

void LaserViewer::init()
{
    setRubberBandSelectionMode(Qt::ItemSelectionMode::IntersectsItemBoundingRect);
    setInteractive(true);
    setMouseTracking(true);

    /*QComboBox* comboBoxScale = new QComboBox;
    comboBoxScale->addItem("100%", 1.0);
    comboBoxScale->addItem("200%", 2.0);
    comboBoxScale->addItem("300%", 3.0);
    comboBoxScale->addItem("400%", 4.0);
    comboBoxScale->addItem("500%", 5.0);
    comboBoxScale->addItem("1000%", 10.0);
*/
    //QLayout* layout = horizontalScrollBar()->layout();
    //QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    //horizontalScrollBar()->setLayout(layout);
    //layout->insertWidget(0, comboBoxScale);
    //layout->addWidget(comboBoxScale);

    //QScrollBar* newBar = new QScrollBar(Qt::Horizontal);
    //QPalette::ColorRole role = newBar->foregroundRole();
    //QPalette palette;
    //palette.setColor(role, Qt::red);
    //newBar->setPalette(palette);
    //layout->addWidget(comboBoxScale);
    //layout->addWidget(newBar);
    //newBar->setLayout(layout);
    //newBar->setFixedSize(100, 30);
    //newBar->show();
    //setHorizontalScrollBar(newBar);

    ADD_TRANSITION(documentIdleState, documentSelectingState, this, &LaserViewer::beginSelecting);
    ADD_TRANSITION(documentSelectingState, documentSelectedState, this, &LaserViewer::endSelecting);
    ADD_TRANSITION(documentSelectedState, documentSelectingState, this, &LaserViewer::beginSelecting);
    ADD_TRANSITION(documentSelectedState, documentIdleState, this, &LaserViewer::cancelSelecting);
    ADD_TRANSITION(documentSelectingState, documentIdleState, this, &LaserViewer::cancelSelecting);
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

	LaserPath *laserPath = new LaserPath(path, m_scene->document());
	m_scene->addLaserPrimitive(laserPath);
	m_handlingSpline.objectName = laserPath->objectName();
	m_splineList.append(m_handlingSpline);
	initSpline();
	qDebug() << "m_splineList length: " << m_splineList.length();
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
    if (!qFuzzyCompare(zoomFactor(), qreal(1))) {
        resetTransform();
        emit zoomChanged();
    }
}
