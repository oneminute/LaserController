#ifndef LASERVIEWER_H
#define LASERVIEWER_H

#include <QObject>
#include <QGraphicsView>
#include <QTime>
#include <QTextEdit>
class RulerWidget;
class LaserScene;
class LaserPrimitiveGroup;
class LaserPrimitive;

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

    qreal zoomValue() const;
	void setZoomValue(qreal zoomScale);
	void createSpline();
	LaserScene* scene();
	void setHorizontalRuler(RulerWidget* _r);
	void setVerticalRuler(RulerWidget * _r);
	LaserPrimitiveGroup* group();
	QRectF selectedItemsSceneBoundingRect();
	void resetSelectedItemsGroupRect(QRectF _sceneRect, qreal _xscale, qreal _yscale,qreal rotate, int _state, int _transformType);//change selection property by tool bar

private:
    void init();
	void initSpline();
	void creatTextEdit();
	void releaseTextEdit();
	void selectedHandleScale();
	void selectedHandleRotate();
	
	//void getSelctedItemsRect(qreal& left, qreal&right, qreal& top, qreal& bottom);
	void detectRect(LaserPrimitive& item, int i, qreal& left, qreal& right, qreal& top, qreal& bottom);
	bool checkTwoPointEqueal(const QPointF& point1, const QPointF& point2);
	bool detectPoint(QVector<QPointF> points, QList<QLineF> lines, QPointF& point);
	bool detectLine(QList<QLineF> lines, QPointF startPoint, QPointF point);
	bool isAllPolygonStartPoint();
	void drawGrids(QPainter& painter);
public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
	void textAreaChanged();
	void onEndSelecting();
	void onDocumentIdle();
	void onCancelSelected();
	void onSelectedFillGroup();
	void onReplaceGroup(LaserPrimitive* item);
signals:
    void zoomChanged(const QPointF& topleft);
	void scaleChanged(qreal scale);
    void beginSelecting();
    void endSelecting();
    void cancelSelecting();
	void cancelSelected();
	void beginSelectedEditing();
	void endSelectedEditing();
    void mouseMoved(const QPointF& pos);
	void creatingRectangle();
	void readyRectangle();
	void creatingEllipse();
	void readyEllipse();
	void creatingLine();
	void readyLine();
	//void creatingPolygonStartRect();
	void creatingPolygon();
	void readyPolygon();
	void creatingSpline();
	void readySpline();
	void creatingText();
	void readyText();
	void selectedEding();
	void selectedChange();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
	void paintSelectedState(QPainter& painter);
	int setSelectionArea(const QPointF& _startPoint, const QPointF& _endPoint);
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
	bool isOnControllHandlers(const QPoint& point, int& handlerIndex, QRectF& handlerRect = QRectF());
	
	void clearGroup();

private:
    QScopedPointer<LaserScene> m_scene;
    //LaserScene* m_scene;
    bool m_rubberBandActive;
    QPoint m_rubberBandOrigin;
    //bool m_mousePressed;
    QPoint m_lastDragPos;

    QPointF m_selectionStartPoint;
    QPointF m_selectionEndPoint;

	QPointF m_creatingRectStartPoint;
	QPointF m_creatingRectEndPoint;
	QPointF m_creatingRectBeforeShiftPoint;

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
	QList<QLineF> m_creatingPolygonLines;
	/*QRectF m_polygonStartRect;
	bool m_isMouseInStartRect;*/
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
	//select
	int m_handleRectPixel = 10;
	QList<QRectF> m_selectedHandleList;
	QPoint m_mousePoint;
	int m_curSelectedHandleIndex = -1;
	QRectF m_selectedRect;
	//QMap<LaserPrimitive*, QPointF> m_selectedItemsDxDy;

	QPointF m_origin;
	QPointF m_originUntrans;
	QPointF m_newOrigin;
	//QPointF m_groupOldTranslate;
	QPoint m_lastPos;
	qreal m_rate;
	QTransform m_oldTransform;
	qreal m_radians;

	qreal m_selectedEditCount = 0;
	LaserPrimitiveGroup* m_group;
	bool m_isLeftSelecting = true;//����߿�ʼѡ�����ұ߿�ʼѡ��
	
	friend class LaserScene;
};

#endif // LASERVIEWER_H