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
	:QWidget(parent), isVertical(_isVertical)
{
	baseMillimeter = Global::mm2PixelsX(1.0);
	millimeter = baseMillimeter;
	unit = millimeter;
	refresh();
}

RulerWidget::~RulerWidget()
{

}

void RulerWidget::setIsVertical(bool _bl)
{
	isVertical = _bl;
	
}

void RulerWidget::setViewer(LaserViewer* _v)
{
	viewer = _v;
}

void RulerWidget::setFactor(qreal _factor)
{
	factor = _factor;
}

void RulerWidget::setScale(qreal _scale)
{
	scale = _scale;
}

void RulerWidget::refresh()
{
	
	if (isVertical) {
		this->setMinimumWidth(minWidthSize);
	}
	else {
		this->setMinimumHeight(minHeightSize);
	}
	
}



void RulerWidget::viewZoomChanged(qreal _factor, const QPointF& topleft) {
	factor = _factor;
	original = topleft;
	scale *= factor;
	repaint();
}

void RulerWidget::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);
	if (!StateControllerInst.onState(StateControllerInst.documentWorkingState())) {
		return;
	}
	QPainter painter(this);
	painter.setPen(QPen(Qt::black, 0.5, Qt::SolidLine));
	QFont font = painter.font();
	font.setPixelSize(10);
	painter.setFont(font);
	//Ruller
	//Axes
	millimeter = baseMillimeter * scale;
	qreal minification = 1 / scale;
	int pow_10 = qLn(minification) / qLn(10);
	flag = qRound(qPow(10, pow_10));//1,10,100,1000...ËõÐ¡±¶Êý
	original = viewer->mapFromScene(viewer->scene()->backgroundItem()->pos());

	
	
	int textCoef = 1 * flag;
	if (scale >= 1.0) {
		unit = millimeter;
		longUnit = 10 * millimeter;
		mediumUnit = 5 * millimeter;
	}
	else {
		qreal shrinkScale = 1 / scale;
		if (shrinkScale > 1*flag && shrinkScale < 1.2*flag) {
			unit = millimeter * flag;
			longUnit = 10 * millimeter * flag;
			mediumUnit = 5 * millimeter * flag;
		}
		else if (shrinkScale >= 1.2*flag && shrinkScale < 2 * flag) {
			unit = 2*millimeter * flag;
			longUnit = 10 * millimeter * flag;
			mediumUnit = 0;
		}
		else if (shrinkScale >= 2 * flag && shrinkScale < 2.2*flag) {
			textCoef = 2 * flag;
			unit = 2*millimeter * flag;
			longUnit = 20 * millimeter * flag;
			mediumUnit = 10 * millimeter * flag;
		}
		else if (shrinkScale >= 2.2*flag && shrinkScale < 5 * flag) {
			textCoef = 2 * flag;
			unit = 5 * millimeter * flag;
			longUnit = 20 * millimeter * flag;
			mediumUnit = 10 * millimeter * flag;
		}
		else if (shrinkScale >= 5 * flag && shrinkScale < 5.5 * flag) {
			textCoef = 5 * flag;
			unit = 5 * millimeter * flag;
			longUnit = 50 * millimeter * flag;
			mediumUnit = 10 * millimeter * flag;
		}
		else if (shrinkScale >= 5.5 * flag && shrinkScale < 10 * flag) {
			textCoef = 5 * flag;
			unit = 10 * millimeter * flag;
			longUnit = 50 * millimeter * flag;
			mediumUnit = 0;
		}
	}
	if (longUnit < 20) {
		font.setPixelSize(8);
		painter.setFont(font);
	}
	QRectF rect = this->rect();
	QRectF documentRect = viewer->scene()->backgroundItem()->rect();
	QRectF viewerRect = viewer->rect();
	qreal dimension = 0;
	if (isVertical) {
		if (rect.height() >documentRect.height()) {
			dimension = rect.bottom() - original.y();
			drawRuler(original.y() - rect.top(), textCoef, painter, false);
		}
		drawRuler(dimension, textCoef, painter);
	}
	else {
		if (rect.width() >documentRect.width()) {
			dimension = rect.right() - original.x();
			drawRuler(original.x() - rect.left(), textCoef, painter, false);
		}
		drawRuler(dimension, textCoef, painter);
	}
	
	
}

void RulerWidget::drawRuler(qreal dimension, int textCoef, QPainter& painter,bool isPositive)
{
	QRectF rect = this->rect();
	int longSize = dimension / longUnit;
	for (int i = 0; i <= longSize; i++) {
		qreal originalStart = original.x();
		if (isVertical) {
			originalStart = original.y();
		}
		float longStart = originalStart + i * longUnit;
		if (!isPositive) {
			longStart = originalStart - i * longUnit;
		}
		qreal edge = rect.top();
		if (isVertical) {
			edge = rect.left();
			painter.drawLine(QPointF( edge + 7, longStart), QPointF(edge + minWidthSize, longStart));
			painter.drawText(QPointF(edge, longStart+10), QString::number(i * textCoef));
		}
		else {
			painter.drawLine(QPointF(longStart, edge + 7), QPointF(longStart, edge + minHeightSize));
			painter.drawText(QPointF(longStart+2, edge + 8), QString::number(i * textCoef));
		}
		
		if (mediumUnit > 5) {
			int mediumSize = longUnit / mediumUnit;
			for (int mi = 1; mi < mediumSize; mi++) {
				float mStart = longStart + mi * mediumUnit;
				if (!isPositive) {
					mStart = longStart - mi * mediumUnit;
				}
				if (isVertical) {
					painter.drawLine(QPointF(edge + 12, mStart), QPointF(edge + minWidthSize, mStart));
				}
				else {
					painter.drawLine(QPointF(mStart, edge + 10), QPointF(mStart, edge + minHeightSize));
				}
				drawSmallUnit(unit, mediumUnit, mStart, edge, painter);
			}
			drawSmallUnit(unit, mediumUnit, longStart, edge, painter);
		}
		else {
			drawSmallUnit(unit, longUnit, longStart, edge, painter);
		}
	}
}

void RulerWidget::drawSmallUnit(qreal unit,qreal _length, qreal _original, qreal _edge, QPainter& _painter, bool isToRight) {
	if (unit > 3) {
		int smallSize = _length / unit;
		
		for (int si = 1; si < smallSize; si++) {
			float sStart = _original + si * unit;
			if (!isToRight) {
				sStart = _original - si * unit;
			}
			if (isVertical) {
				_painter.drawLine(QPointF(_edge + 14, sStart), QPointF(_edge + minWidthSize, sStart));
			}
			else {
				_painter.drawLine(QPointF(sStart, _edge + 12), QPointF(sStart, _edge + minHeightSize));
			}
			
		}
	}
}
