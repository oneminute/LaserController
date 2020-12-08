#include "LaserViewer.h"

#include <QBoxLayout>
#include <QGraphicsProxyWidget>
#include <QComboBox>
#include <QPainterPath>
#include <QPalette>
#include <QtMath>
#include <QWheelEvent>
#include <QScrollBar>

#include "scene/LaserPrimitive.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include "state/StateController.h"

LaserViewer::LaserViewer(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new LaserScene)
    , m_rubberBandActive(false)
    , m_mousePressed(false)
	, m_ruller(this)
	, m_isKeyShiftPressed(false)
	
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
		painter.drawRect(QRectF(m_creatingRectStartPoint, m_creatingRectEndPoint));
	}
	//Ellipse
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
		painter.drawEllipse(QRectF(m_creatingEllipseStartPoint, m_creatingEllipseEndPoint));
	}
	//Line
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveLineCreatingState())) {
		painter.drawLine(m_creatingLineStartPoint, m_creatingLineEndPoint);
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
    if (StateControllerInst.anyState(states))
    {
        if (event->button() == Qt::LeftButton)
        {
            m_selectionStartPoint = event->pos();
            m_selectionEndPoint = m_selectionStartPoint;
            emit beginSelecting();
            qDebug() << "begin to select";
        }
	}
	//Rect
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveRectReadyState())) {
		if (event->button() == Qt::LeftButton) {
			m_creatingRectStartPoint = event->pos();
			m_creatingRectEndPoint = m_creatingRectStartPoint;
			emit creatingRectangle();
		}

	}
	//Ellipse
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveEllipseReadyState())) {
		if (event->button() == Qt::LeftButton) {
			m_creatingEllipseStartPoint = event->pos();
			m_creatingEllipseStartInitPoint = m_creatingEllipseStartPoint;
			m_creatingEllipseEndPoint = m_creatingEllipseStartPoint;
			emit creatingEllipse();
		}
	}
	//Line
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveLineReadyState())) {
		if (event->button() == Qt::LeftButton) {
			m_creatingLineStartPoint = event->pos();
			m_creatingLineEndPoint = m_creatingLineStartPoint;
			emit creatingLine();
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
		m_creatingRectEndPoint = point;
		return;

	}
	//Ellipse
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {
		m_creatingEllipseEndPoint = point;
		if (m_isKeyShiftPressed) {
			QPointF end(m_creatingEllipseEndPoint.x(), m_creatingEllipseStartInitPoint.y());
			qreal radius = (m_creatingEllipseStartInitPoint - end).manhattanLength() * 0.5;
			QPointF delt((m_creatingEllipseEndPoint.x() - m_creatingEllipseStartInitPoint.x())*0.5, (m_creatingEllipseEndPoint.y() - m_creatingEllipseStartInitPoint.y())*0.5);
			QPointF center = m_creatingEllipseStartInitPoint + delt;
			m_creatingEllipseStartPoint = QPointF(center.x() - radius, center.y() - radius);
			m_creatingEllipseEndPoint = QPointF(center.x() + radius, center.y() + radius);
		}
		return;
	}
	//Line
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveLineCreatingState())) {
		m_creatingLineEndPoint = point;
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
        QRectF rect(mapToScene(m_creatingRectStartPoint.toPoint()), mapToScene(m_creatingRectEndPoint.toPoint()));
		LaserRect *rectItem = new LaserRect(rect, m_scene->document());
		m_scene->addLaserPrimitive(rectItem);
		emit readyRectangle();
	}
	//Ellipse
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveEllipseCreatingState())) {		
		QRectF rect(mapToScene(m_creatingEllipseStartPoint.toPoint()), mapToScene(m_creatingEllipseEndPoint.toPoint()));
		LaserEllipse *ellipseItem = new LaserEllipse(rect, m_scene->document());
		m_scene->addLaserPrimitive(ellipseItem);
		emit readyEllipse();
	}
	//Line
	else if (StateControllerInst.onState(StateControllerInst.documentPrimitiveLineCreatingState())) {
		QLineF line(mapToScene(m_creatingLineStartPoint.toPoint()), mapToScene(m_creatingLineEndPoint.toPoint()));
		LaserLine *lineItem = new LaserLine(line, m_scene->document());
		m_scene->addLaserPrimitive(lineItem);
		emit readyLine();
	}
    else
    {
        QGraphicsView::mouseReleaseEvent(event);
    }

    m_mousePressed = false;
	m_isKeyShiftPressed = false;
}

qreal LaserViewer::zoomFactor() const
{
    return transform().m11();
}

qreal LaserViewer::distanceTwoPoints(QPointF _start, QPointF _end)
{
	qreal x = _start.x() - _end.x();
	qreal y = _start.y() - _end.y();
	qreal distance = qSqrt(x*x + y * y);
	return distance;
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
