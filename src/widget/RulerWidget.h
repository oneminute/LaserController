#ifndef RULERWIDGET_H
#define RULERWIDGET_H

#include <QObject>
#include <QWidget>
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
	void refresh();
	
protected:
	void paintEvent(QPaintEvent *event);
private:
	bool isVertical;
	bool isVisible = false;
	int minHeightSize = 15;
	int minWidthSize = 18;
	qreal factor = 1.0;
	qreal scale = 1.0;
	qreal millimeter = 5;//变化的
	QPointF original = QPointF(0, 0);
	LaserViewer* viewer = nullptr;
	qreal baseMillimeter = 5;//假设一毫米等于5个像素
	int flag = 1;
	qreal unit = 0;
	qreal longUnit = 0;
	qreal mediumUnit = 0;
private :
	void drawRuler(qreal dimension, int textCoef, QPainter& painter, bool isPositive = true);
	void drawSmallUnit(qreal unit, qreal _length, qreal _left, qreal _top, QPainter& _painter, bool isToRight = true);
public slots:
	void viewZoomChanged(qreal _factor, const QPointF& topleft);
};

#endif // RULERWIDGET_H