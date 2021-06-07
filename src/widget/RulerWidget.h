#ifndef RULERWIDGET_H
#define RULERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPoint>
#include "LaserViewer.h"

class LaserScene;

class RulerWidget :public QWidget
{
	Q_OBJECT
public:
	explicit RulerWidget(QWidget* parent = 0, bool _isVertical = false);
	~RulerWidget();
	void setIsVertical(bool _bl);
	void setViewer(LaserViewer* _v);
	void setFactor(qreal _factor);
	void setScale(qreal _scale);
	void setIsMarkMouse(bool _bl);
	void setMousePoint(const QPoint& _point);
	void refresh();
	
protected:
	void paintEvent(QPaintEvent *event);
private:
	bool m_isVertical;
	bool m_isMarkMouse = false;//是否标记鼠标
	int m_minHeightSize = 15;
	int m_minWidthSize = 18;
	qreal m_factor = 1.0;
	qreal m_scale = 1.0;
	qreal m_millimeter = 5;//变化的
	QPointF m_original = QPointF(0, 0);
	LaserViewer* m_viewer = nullptr;
	qreal m_baseMillimeter = 5;//假设一毫米等于5个像素
	int m_flag = 1;
	qreal m_unit = 0;
	qreal m_longUnit = 0;
	qreal m_mediumUnit = 0;
	QPoint m_mousePoint;
private :
	void drawRuler(qreal dimension, int textCoef, QPainter& painter, bool isPositive = true);
	void drawSmallUnit(qreal unit, qreal _length, qreal _left, qreal _top, QPainter& _painter, bool isToRight = true);
public slots:
	void viewZoomChanged(qreal _factor, const QPointF& topleft);
};

#endif // RULERWIDGET_H