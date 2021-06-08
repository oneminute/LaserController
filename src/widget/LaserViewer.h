#ifndef LASERVIEWER_H
#define LASERVIEWER_H

#include <QObject>
#include <QGraphicsView>
#include <QTime>
#include <QTextEdit>
class RulerWidget;
class LaserScene;
//Spline Node Struct
struct SplineNodeStruct {
	QPointF node;
	QPointF handler1;
	QPointF handler2;
	SplineNodeStruct(QPointF _node) {
		node = _node;
		handler1 = node;
		handler2 = node;
	}
};
//Spline Struct
struct SplineStruct{
	QString objectName;//id,LaserPrimitive's objectName is UUid
	QList<SplineNodeStruct> nodeList;
};
class LaserViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit LaserViewer(QWidget* parent = nullptr);
    ~LaserViewer();

    qreal zoomScale() const;
	void setZoomScale(qreal zoomScale);
	void createSpline();
	LaserScene* scene();
	void setHorizontalRuler(RulerWidget* _r);
	void setVerticalRuler(RulerWidget * _r);

private:
    void init();
	void initSpline();
	void creatTextEdit();
	void releaseTextEdit();

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
	void textAreaChanged();

signals:
    void zoomChanged(qreal factor, const QPointF& topleft);
	void scaleChanged(qreal scale);
    void beginSelecting();
    void endSelecting();
    void cancelSelecting();
    void mouseMoved(const QPointF& pos);
	void creatingRectangle();
	void readyRectangle();
	void creatingEllipse();
	void readyEllipse();
	void creatingLine();
	void readyLine();
	void creatingPolygonStartRect();
	void creatingPolygon();
	void readyPolygon();
	void creatingSpline();
	void readySpline();
	void creatingText();
	void readyText();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    void zoomBy(qreal factor);
	//mouse
	virtual void leaveEvent(QEvent *event) override;
	virtual void enterEvent(QEvent *event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
	//key
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	//scroll

	virtual void scrollContentsBy(int dx, int dy) override;


private:
    QScopedPointer<LaserScene> m_scene;
    //LaserScene* m_scene;
    bool m_rubberBandActive;
    QPoint m_rubberBandOrigin;
    bool m_mousePressed;
    QPoint m_lastDragPos;

    QPointF m_selectionStartPoint;
    QPointF m_selectionEndPoint;

	QPointF m_creatingRectStartPoint;
	QPointF m_creatingRectEndPoint;

	QPointF m_creatingEllipseStartPoint;
	QPointF m_creatingEllipseStartInitPoint;
	QPointF m_creatingEllipseEndPoint;
	QPointF m_EllipseEndPoint;

	QPointF m_creatingLineStartPoint;
	QPointF m_creatingLineEndPoint;
	//Polygon
	QPointF m_creatingPolygonStartPoint;
	QPointF m_creatingPolygonEndPoint;
	QVector<QPointF> m_creatingPolygonPoints;
	QRectF m_polygonStartRect;
	bool m_isMouseInStartRect;
	//Spline
	SplineStruct m_handlingSpline;//creating and editing
	QList<SplineStruct> m_splineList;
	QPointF m_creatingSplineMousePos;
	QPointF m_handlingSplinePoint;
	QRectF m_mouseHoverRect;
	qreal m_splineNodeDrawWidth;
	qreal m_splineNodeEditWidth;
	qreal m_splineHandlerWidth;
	//Text
	QPoint m_textInputPoint;
	QTime m_time;
	int m_curTime;
	int m_lastTime;
	QTextEdit *m_textEdit;

	bool m_isKeyShiftPressed;
	bool m_isKeyDelPress;
	//Ruller
	RulerWidget* m_horizontalRuler;
	RulerWidget* m_verticalRuler;
};

#endif // LASERVIEWER_H