#include "RulerWidget.h"
#include "LaserViewer.h"
#include "scene/LaserScene.h"
#include <QObject>
#include <QPainter>
#include <QDebug>
#include <QGraphicsRectItem>
#include <state/StateController.h>
#include "qmath.h"
RulerWidget::RulerWidget(QWidget * parent, bool _isVertical)
	:QWidget(parent), m_isVertical(_isVertical), m_mousePoint(0, 0)
{
	m_baseMillimeter = Global::mm2PixelsXF(1.0);
	if (m_isVertical) {
		m_baseMillimeter = Global::mm2PixelsYF(1.0);
	}
	m_millimeter = m_baseMillimeter;
	m_unit = m_millimeter;
	qLogD << "baseMillimeter: " << m_baseMillimeter;
	qLogD << "millimeter: " << m_millimeter;
	qLogD << "unit: " << m_unit;
	refresh();
}

RulerWidget::~RulerWidget()
{

}

void RulerWidget::setIsVertical(bool _bl)
{
	m_isVertical = _bl;
	
}

void RulerWidget::setViewer(LaserViewer* _v)
{
	m_viewer = _v;
}

void RulerWidget::setFactor(qreal _factor)
{
	m_factor = _factor;
}

void RulerWidget::setScale(qreal _scale)
{
	m_scale = _scale;
}

void RulerWidget::setIsMarkMouse(bool _bl)
{
	m_isMarkMouse = _bl;
}

void RulerWidget::setMousePoint(const QPoint& _point)
{
	m_mousePoint = _point;
}

void RulerWidget::refresh()
{
	
	if (m_isVertical) {
		this->setMinimumWidth(m_minWidthSize);
	}
	else {
		this->setMinimumHeight(m_minHeightSize);
	}
	
}



void RulerWidget::viewZoomChanged(qreal _factor, const QPointF& topleft) {
	m_factor = _factor;
	m_original = topleft;
	m_scale *= m_factor;
	repaint();
}

void RulerWidget::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);
	if (!StateControllerInst.onState(StateControllerInst.documentWorkingState())) {
		return;
	}
	QPainter painter(this);
	//mouse position
	if (m_isMarkMouse) {
		painter.setPen(QPen(Qt::red, 1.2, Qt::DotLine));
		if (m_isVertical) {
			painter.drawLine(QPoint(0, m_mousePoint.y()), QPoint(m_minWidthSize, m_mousePoint.y()));
		}
		else {
			painter.drawLine(QPoint(m_mousePoint.x(), 0), QPoint(m_mousePoint.x(), m_minHeightSize));
		}
		
		
	}
	
	//Ruller
	painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
	QFont font = painter.font();
	font.setPixelSize(10);
	painter.setFont(font);
	//Axes
	m_millimeter = m_baseMillimeter * m_scale;
	qreal minification = 1 / m_scale;
	int pow_10 = qLn(minification) / qLn(10);
	m_flag = qRound(qPow(10, pow_10));//1,10,100,1000...��С����
	m_original = m_viewer->mapFromScene(m_viewer->scene()->backgroundItem()->pos());
	int textCoef = 1 * m_flag;
	if (m_scale >= 1.0) {
		m_unit = m_millimeter;
		m_longUnit = 10 * m_millimeter;
		m_mediumUnit = 5 * m_millimeter;
	}
	else {
		qreal shrinkScale = 1 / m_scale;
		if (shrinkScale > 1*m_flag && shrinkScale < 1.2*m_flag) {
			m_unit = m_millimeter * m_flag;
			m_longUnit = 10 * m_millimeter * m_flag;
			m_mediumUnit = 5 * m_millimeter * m_flag;
		}
		else if (shrinkScale >= 1.2*m_flag && shrinkScale < 2 * m_flag) {
			m_unit = 2*m_millimeter * m_flag;
			m_longUnit = 10 * m_millimeter * m_flag;
			m_mediumUnit = 0;
		}
		else if (shrinkScale >= 2 * m_flag && shrinkScale < 2.2*m_flag) {
			textCoef = 2 * m_flag;
			m_unit = 2*m_millimeter * m_flag;
			m_longUnit = 20 * m_millimeter * m_flag;
			m_mediumUnit = 10 * m_millimeter * m_flag;
		}
		else if (shrinkScale >= 2.2*m_flag && shrinkScale < 5 * m_flag) {
			textCoef = 2 * m_flag;
			m_unit = 5 * m_millimeter * m_flag;
			m_longUnit = 20 * m_millimeter * m_flag;
			m_mediumUnit = 10 * m_millimeter * m_flag;
		}
		else if (shrinkScale >= 5 * m_flag && shrinkScale < 5.5 * m_flag) {
			textCoef = 5 * m_flag;
			m_unit = 5 * m_millimeter * m_flag;
			m_longUnit = 50 * m_millimeter * m_flag;
			m_mediumUnit = 10 * m_millimeter * m_flag;
		}
		else if (shrinkScale >= 5.5 * m_flag && shrinkScale < 10 * m_flag) {
			textCoef = 5 * m_flag;
			m_unit = 10 * m_millimeter * m_flag;
			m_longUnit = 50 * m_millimeter * m_flag;
			m_mediumUnit = 0;
		}
	}
	if (m_longUnit < 20) {
		font.setPixelSize(8);
		painter.setFont(font);
	}
	QRectF rect = this->rect();
	QRectF documentRect = m_viewer->scene()->backgroundItem()->rect();
	QRectF viewerRect = m_viewer->rect();
	qreal dimension = 0;
	qLogD << "Drawing ruler: " << rect << ", documentRect.height: " << documentRect;
	if (m_isVertical) {
		dimension = rect.height();
		if (rect.height() > documentRect.height()) {
			dimension = rect.bottom() - m_original.y();
			qLogD << "original: " << m_original << ", dimension: " << dimension;
			drawRuler(m_original.y() - rect.top(), textCoef, painter, false);
		}
		drawRuler(dimension, textCoef, painter);
	}
	else {
		dimension = rect.width();
		if (rect.width() >documentRect.width()) {
			dimension = rect.right() - m_original.x();
			qLogD << "original: " << m_original << ", dimension: " << dimension;
			drawRuler(m_original.x() - rect.left(), textCoef, painter, false);
		}
		drawRuler(dimension, textCoef, painter);
	}
	
	
}

void RulerWidget::drawRuler(qreal dimension, int textCoef, QPainter& painter,bool isPositive)
{
	QRectF rect = this->rect();
	int longSize = dimension / m_longUnit;
	for (int i = 0; i <= longSize; i++) {
		qreal originalStart = m_original.x();
		if (m_isVertical) {
			originalStart = m_original.y();
		}
		float longStart = originalStart + i * m_longUnit;
		if (!isPositive) {
			longStart = originalStart - i * m_longUnit;
		}
		qreal edge = rect.top();
		if (m_isVertical) {
			edge = rect.left();
			painter.drawLine(QPointF( edge + 7, longStart), QPointF(edge + m_minWidthSize, longStart));
			painter.drawText(QPointF(edge, longStart+10), QString::number(i * textCoef));
		}
		else {
			painter.drawLine(QPointF(longStart, edge + 7), QPointF(longStart, edge + m_minHeightSize));
			painter.drawText(QPointF(longStart+2, edge + 8), QString::number(i * textCoef));
		}
		
		if (m_mediumUnit > 5) {
			int mediumSize = m_longUnit / m_mediumUnit;
			for (int mi = 1; mi < mediumSize; mi++) {
				float mStart = longStart + mi * m_mediumUnit;
				if (!isPositive) {
					mStart = longStart - mi * m_mediumUnit;
				}
				if (m_isVertical) {
					painter.drawLine(QPointF(edge + 12, mStart), QPointF(edge + m_minWidthSize, mStart));
				}
				else {
					painter.drawLine(QPointF(mStart, edge + 10), QPointF(mStart, edge + m_minHeightSize));
				}
				drawSmallUnit(m_unit, m_mediumUnit, mStart, edge, painter);
			}
			drawSmallUnit(m_unit, m_mediumUnit, longStart, edge, painter);
		}
		else {
			drawSmallUnit(m_unit, m_longUnit, longStart, edge, painter);
		}
	}
}

void RulerWidget::drawSmallUnit(qreal unit,qreal _length, qreal _original, qreal _edge, QPainter& _painter, bool isToRight) {
	//if (unit > 3) {
		int smallSize = _length / unit;
		
		for (int si = 1; si < smallSize; si++) {
			float sStart = _original + si * unit;
			if (!isToRight) {
				sStart = _original - si * unit;
			}
			if (m_isVertical) {
				_painter.drawLine(QPointF(_edge + 14, sStart), QPointF(_edge + m_minWidthSize, sStart));
			}
			else {
				_painter.drawLine(QPointF(sStart, _edge + 12), QPointF(sStart, _edge + m_minHeightSize));
			}
			
		}
	//}
}
