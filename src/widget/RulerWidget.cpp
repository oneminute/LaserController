#include "RulerWidget.h"
#include "LaserViewer.h"
#include "scene/LaserScene.h"
#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "common/Config.h"
#include <QObject>
#include <QPainter>
#include <QDebug>
#include <QGraphicsRectItem>
#include <state/StateController.h>
#include "qmath.h"
RulerWidget::RulerWidget(QWidget * parent, bool _isVertical)
	:QWidget(parent), m_isVertical(_isVertical), m_mousePoint(0, 0)
{
	//m_baseMillimeter = Global::convertFromUm(1000);
    m_baseMillimeter = 1.0;
	if (m_isVertical) {
		//m_baseMillimeter = Global::convertFromUm(1000);
        m_baseMillimeter = 1.0;
	}
	m_millimeter = m_baseMillimeter;
	m_unit = m_millimeter;
	//refresh();
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

void RulerWidget::viewZoomChanged(const QPointF& topleft) {
	//m_original = topleft;
    //m_original = LaserApplication::device->originInScene();
	repaint();
}

void RulerWidget::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);
	if (!StateControllerInst.isInState(StateControllerInst.documentWorkingState())) {
		return;
	}
	QPainter painter(this);
	//mouse position
	if (m_isMarkMouse) {
		painter.setPen(QPen(QColor(242, 149, 149), 1.2, Qt::DotLine));
		if (m_isVertical) {
			painter.drawLine(QPoint(0, m_mousePoint.y()), QPoint(m_minWidthSize, m_mousePoint.y()));
		}
		else {
			painter.drawLine(QPoint(m_mousePoint.x(), 0), QPoint(m_mousePoint.x(), m_minHeightSize));
		}
	}
    //ruler scale is 1,when scene size adapter View size
    qreal delta = m_viewer->adapterViewScale();
	m_scale = m_viewer->zoomValue() / delta;
    //qDebug() << m_scale;
	//Ruller
	QRectF rect = this->rect();
	painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::SolidLine));
	
	//Axes
	m_millimeter = (m_baseMillimeter * 1000) * m_viewer->zoomValue();
	qreal minification = 1 / m_scale;
	int pow_10 = qLn(minification) / qLn(10);
	m_flag = qRound(qPow(10, pow_10));//1,10,100,1000...��С����
	LaserBackgroundItem* backgroundItem = m_viewer->scene()->backgroundItem();
	if (!backgroundItem) {
		return;
	}
	//m_original = m_viewer->mapFromScene(backgroundItem->pos());
    m_original = m_viewer->mapFromScene(LaserApplication::device->origin());
	int textCoef = 1 * m_flag;
	if (m_scale >= 1.0 && m_scale < 22) {
		m_unit = m_millimeter;
		m_longUnit = 10 * m_millimeter;
		m_mediumUnit = 5 * m_millimeter;
	}
    else if(m_scale >= 22){
        m_unit = m_millimeter * 0.5;
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
		else if (shrinkScale >= 1.2*m_flag && shrinkScale < 2.0 * m_flag) {
			m_unit = 2*m_millimeter * m_flag;
			m_longUnit = 10 * m_millimeter * m_flag;
			m_mediumUnit = 0;
		}
		else if (shrinkScale >= 2.0 * m_flag && shrinkScale < 3.0*m_flag) {
			textCoef = 2 * m_flag;
			m_unit = 2*m_millimeter * m_flag;
			m_longUnit = 20 * m_millimeter * m_flag;
			m_mediumUnit = 10 * m_millimeter * m_flag;
		}
		else if (shrinkScale >= 3.0*m_flag && shrinkScale < 5.0 * m_flag) {
			textCoef = 2 * m_flag;
			m_unit = 5 * m_millimeter * m_flag;
			m_longUnit = 20 * m_millimeter * m_flag;
			m_mediumUnit = 10 * m_millimeter * m_flag;
		}
		else if (shrinkScale >= 5.0 * m_flag && shrinkScale < 7.0 * m_flag) {
			textCoef = 5 * m_flag;
			m_unit = 5 * m_millimeter * m_flag;
			m_longUnit = 50 * m_millimeter * m_flag;
			m_mediumUnit = 10 * m_millimeter * m_flag;
		}
		else if (shrinkScale >= 7.0 * m_flag && shrinkScale < 10 * m_flag) {
			textCoef = 5 * m_flag;
			m_unit = 10 * m_millimeter * m_flag;
			m_longUnit = 50 * m_millimeter * m_flag;
			m_mediumUnit = 0;
		}
		
	}
	qreal dimension = 0;
	if (m_isVertical) {
		
		if (m_original.y() > 0) {
			dimension = rect.bottom() - m_original.y();
            drawRuler(m_original.y() - rect.top(), textCoef, painter, false, true);
            drawRuler(dimension, textCoef, painter, true, false);
		}
		else {
			dimension = rect.height() + (-m_original.y());
		}
		drawRuler(dimension, textCoef, painter);
	}
	else {
		
		if (m_original.x() > 0) {
			dimension = rect.right() - m_original.x();
            drawRuler(m_original.x() - rect.left(), textCoef, painter,false, true);
            drawRuler(dimension, textCoef, painter, true, false);
		}
		else {
			dimension = rect.width() + (-m_original.x());
            drawRuler(dimension, textCoef, painter);
		}

		
	}
}
//isNegative: m_original在viewer这个控件中的位置x,y是否为负值
void RulerWidget::drawRuler(qreal dimension, int textCoef, QPainter& painter, bool isNegative, bool showMinusSign)
{
	//text
	QFont font = painter.font();
	font.setPixelSize(m_textMaxSize);
	painter.setFont(font);
	painter.setPen(QPen(QColor(200, 200, 200), 1));
    	
	//size
	QRectF rect = this->rect();
	int longSize = qRound(dimension / m_longUnit);
    qreal originalStart = m_original.x();
    qreal edge = rect.top();
    if (m_isVertical) {
        originalStart = m_original.y();
        edge = rect.left();
    }
    else {
        //水平方向显示符号
        if (showMinusSign) {
            textCoef *= -1;
        }
    }
    
	for (int i = 0; i <= longSize; i++) {		
		float longStart = originalStart + i * m_longUnit;
		if (!isNegative) {
			longStart = originalStart - i * m_longUnit;
		}
		if (m_isVertical) {
			painter.drawLine(QPointF(edge + 7, longStart), QPointF(edge + m_minWidthSize, longStart));
			//text
			if (m_longUnit > 38 || i % 2 == 0) {
				QString number_str = QString::number(i * textCoef * 10);
				if (textCoef == 0) {
					number_str = QString::number(i * 10);
				}
				if (i % 2 == 0) {
					painter.setPen(QPen(QColor(63, 63, 63), 1));
				}
				else {
					painter.setPen(QPen(QColor(8, 137, 246), 1));
				}

				int length = number_str.length();
				if (m_scale <= 8) {
                    //垂直方向的负值与水平方向的相反
                    if (i > 0 && showMinusSign) {
                        painter.drawText(edge - 2, longStart + 10, QString("-"));

                    }
					for (int j = 0; j < length; j++) {
                        painter.drawText(QPointF(edge + 3, longStart + 10 + j * font.pixelSize()), QString(number_str[j]));
                    }
                }
				painter.setPen(QPen(QColor(200, 200, 200), 1));
			}
		}
		else {
			painter.drawLine(QPointF(longStart, edge + 7), QPointF(longStart, edge + m_minHeightSize));
			//text
			if (m_longUnit > 38 || i % 2 == 0) {
				//QString number_str = QString::number(LaserApplication::device->deviceTranslateXMm(i * textCoef * 10));
				QString number_str = QString::number(i * textCoef * 10);
				if (textCoef == 0) {
					//number_str = QString::number(LaserApplication::device->deviceTranslateXMm(i * 10));
					number_str = QString::number(i * 10);
				}
				if (i % 2 == 0) {
					painter.setPen(QPen(QColor(63, 63, 63), 1));
				}
				else {
					painter.setPen(QPen(QColor(8, 137, 246), 1));
				}
                if (m_scale <= 8) {
                    painter.drawText(QPointF(longStart + 2, edge + 8), number_str);
                }
				painter.setPen(QPen(QColor(200, 200, 200), 1));
			}
			
		}
		
		if (m_mediumUnit > 0) {
			int mediumSize = qRound(m_longUnit / m_mediumUnit);
			for (int mi = 1; mi < mediumSize; mi++) {
				float mStart = longStart + mi * m_mediumUnit;
				if (!isNegative) {
					mStart = longStart - mi * m_mediumUnit;
				}
				if (m_isVertical) {
					painter.drawLine(QPointF(edge + 10, mStart), QPointF(edge + m_minWidthSize, mStart));
				}
				else {
					painter.drawLine(QPointF(mStart, edge + 10), QPointF(mStart, edge + m_minHeightSize));
				}
				drawSmallUnit(m_mediumUnit, mStart, edge, painter, isNegative);
			}
			drawSmallUnit(m_mediumUnit, longStart, edge, painter, isNegative);
		}
		else {
			drawSmallUnit(m_longUnit, longStart, edge, painter, isNegative);
		}
	}
    //放大到一定比例时，unit显示刻度
    if (m_scale > 8) {
        int smallSize = qRound(dimension / m_unit);
        float unitStart = originalStart;
        if (m_isVertical) {
            if (showMinusSign) {
                //painter.drawText(edge - 2, edge + 8, QString("-"));
            }
        }
        else {
            if (showMinusSign) {
                
            }
        }
        for (int sj = 0; sj < smallSize; sj++) {
            QString number_str = QString::number(sj);
            if (m_scale > 22) {
                number_str = QString::number(sj * 0.5, 'f', 1);
            }
            painter.setPen(QPen(QColor(63, 63, 63), 1));
            unitStart = originalStart + sj * m_unit;
            if (!isNegative) {
                unitStart = originalStart - sj * m_unit;
            }
            if (m_isVertical) {
                //垂直方向的负值与水平方向的相反，垂直方向显示符号
                if (sj > 0 && showMinusSign) {
                    painter.drawText(QPointF(edge - 3, unitStart + 10), QString("-"));
                }
                for (int j = 0; j < number_str.size(); j++) {
                    painter.drawText(QPointF(edge + 3, unitStart + 10 + j * font.pixelSize()), QString(number_str[j]));
                }
            } 
            else {
                if (sj > 0 && showMinusSign) {
                    painter.drawText(QPointF(unitStart-2, edge + 8), QString("-"));
                }
                painter.drawText(QPointF(unitStart + 2, edge + 8), number_str);
            }
            
            
        }
    }
}

void RulerWidget::drawMediumUnit(qreal _length, qreal _left, qreal _top, QPainter & _painter, bool isToRight)
{

}

void RulerWidget::drawSmallUnit(qreal _length, qreal _original, qreal _edge, QPainter& _painter, bool isToRight) {
	if (m_unit > 4) {
		int smallSize = qRound(_length / m_unit);
		
		for (int si = 1; si < smallSize; si++) {
			float sStart = _original + si * m_unit;
			if (!isToRight) {
				sStart = _original - si * m_unit;
			}
			if (m_isVertical) {
				_painter.drawLine(QPointF(_edge + 12, sStart), QPointF(_edge + m_minWidthSize, sStart));
			}
			else {
				_painter.drawLine(QPointF(sStart, _edge + 12), QPointF(sStart, _edge + m_minHeightSize));
			}
			
		}
	}
}
