#ifndef LASERVIEWER_H
#define LASERVIEWER_H

#include <QObject>
#include <QGraphicsView>
#include <QTime>
#include <QTextEdit>
#include <QState>
#include <QUndoStack>
class RulerWidget;
class LaserScene;
class LaserPrimitiveGroup;
class LaserPrimitive;
class LaserBitmap;
class LaserLayer;
class LaserText;
struct LaserTextRowPath;
class LaserControllerWindow;

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
    //zoom
    //输入的点zoomAnchor是view的widget为坐标系
    bool zoomBy(qreal factor, QPointF zoomAnchor = QPointF(0, 0), bool zoomAnchorCenter = false);
    qreal zoomValue() const;
    qreal zoomValueNormal();
	void setZoomValue(qreal zoomScale);
    qreal adapterViewScale();
    LaserPrimitive* mirrorLine();
    void setMirrorLine(LaserPrimitive* l);

	void createSpline();
	LaserScene* scene();
	void setHorizontalRuler(RulerWidget* _r);
	void setVerticalRuler(RulerWidget * _r);
    RulerWidget* horizontalRuler();
    RulerWidget* verticalRuler();
	LaserPrimitiveGroup* group();
    LaserPrimitive* cursorInLaserPrimitive(QPointF mousePosInScene);
	QRect selectedItemsSceneBoundingRect();
    QRect AllItemsSceneBoundingRect();
	void resetSelectedItemsGroupRect(QRectF _sceneRect, qreal _xscale, qreal _yscale,qreal rotate,
        int _state, int _transformType, int _pp, bool _unitIsMM);//change selection property by tool bar
	void setAnchorPoint(QPointF point);
	bool detectIntersectionByMouse(QPointF& result, QPointF mousePoint, bool& isSpecialPoint);//draw
	QLineF detectItemEdge(LaserPrimitive*& result, QPointF mousePoint, float scop, bool ignoreFillSolid = false);//selection
	bool detectItemByMouse(LaserPrimitive*& result, QPointF mousePoint);
	bool detectFillSolidByMouse(LaserPrimitive*& result, QPointF mousePoint);//selection
    bool detectTextInsertPosition(QPointF insertPoint, LaserText*& text);//被找到的text
	
	QMap<QGraphicsItem*, QTransform> clearGroupSelection();

	QState* currentState();

	QUndoStack* undoStack();

	QMap<LaserPrimitive*, QTransform>& copyedList();
	QMap<QString, QList<LaserPrimitive*>>& groupedMap();
	int curLayerIndex();

	void setCurLayerIndex(int index);
    //text
    QLineF modifyTextCursor();
    QFont* textFont();
    LaserText* editingText();
    void setEditingText(LaserText * text);
    int textAlignH();
    int textAlignV();
    void setTextAlignH(int align);
    void setTextAlignV(int align);
    //tree
    void updateGroupTreeNode();
    //mirror
    void AcrossLineMirror();
    //detect bounds is in max region
    bool detectBoundsInMaxRegion(QRectF bounds);

	bool showLaserPos() const;
	void setShowLaserPos(bool laserPos);

	bool addPrimitiveAndExamRegionByBounds(LaserPrimitive* primitive);
	bool addPrimitiveAndExamRegionByBounds(QList<LaserPrimitive*>& primitives, LaserPrimitive* parent = nullptr);
	
private:
    void init();
	void initSpline();
	//void creatTextEdit();
	void addText(QString str);
    void removeBackText();
    void removeFrontText();
    void addTextByKeyInput(QString str);
	void selectedHandleScale();
	void selectedHandleRotate();
	
	//void getSelctedItemsRect(qreal& left, qreal&right, qreal& top, qreal& bottom);
	void detectRect(LaserPrimitive& item, int i, int& left, int& right, int& top, int& bottom);
	//bool detectPoint(QVector<QPointF> points, QList<QLineF> lines, QPointF& point);
	//bool detectLine(QList<QLineF> lines, QPointF startPoint, QPointF point);
	bool isRepeatPoint();
	bool isStartPoint();
	qreal leftScaleMirror(qreal rate, qreal x);
	qreal rightScaleMirror(qreal rate, qreal x);
	qreal topScaleMirror(qreal rate, qreal y);
	qreal bottomScaleMirror(qreal rate, qreal y);
    
	void setGroupNull();
	void pointSelectWhenSelectedState(int handleIndex, LaserPrimitive* primitive);
	void selectingReleaseInBlank();//释放鼠标时，点选和框选的区域为空白（没有图元）
    
	//undo
	void selectionUndoStackPushBefore();
	void selectionUndoStackPush();
	//void reshapeUndoStackPushBefore();
	//ReshapeUndoCommand* reshapeUndoStackPush();
	void transformUndoStackPushBefore(LaserPrimitive* item = nullptr);
	void transformUndoStackPush(LaserPrimitive* item = nullptr);
    
    
public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
	void zoomToSelection();
	//void textAreaChanged();
	void onEndSelecting();
	void onEndSelectionFillGroup();
	void onDocumentIdle();
	QMap<QGraphicsItem*, QTransform> onCancelSelected();
	bool resetGroup();
	bool onSelectedFillGroup();
    void createGroup();
	QMap<QGraphicsItem*, QTransform> onReplaceGroup(LaserPrimitive* item);
	void onReplaceGroup(QList<LaserPrimitive*> primitives);
    //text
    void onEndText();
signals:
    void zoomChanged(const QPointF& topleft);
	void scaleChanged(qreal scale);
    void beginSelecting();
    void endSelecting();
	void idleToSelected();
    void selectionToIdle();
	void cancelSelected();
	void beginSelectedEditing();
	void beginIdelEditing();
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
	void selectedChangedFromMouse();
    void selectedChangedFromToolBar();
	void beginViewDraging();
	void endViewDraging();
	void selectedSizeChanged();//发生框选，点选时

protected:
    virtual void paintEvent(QPaintEvent* event) override;
	void paintSelectedState(QPainter& painter);
	int setSelectionArea(const QPointF& _startPoint, const QPointF& _endPoint);
    virtual void wheelEvent(QWheelEvent *event) override;
    
	void resizeEvent(QResizeEvent *event) override;
	
	//mouse
	virtual void leaveEvent(QEvent *event) override;
	virtual void enterEvent(QEvent *event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;

	virtual void dragEnterEvent(QDragEnterEvent *event) override;

	virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
	virtual void dragMoveEvent(QDragMoveEvent *event) override;
	virtual void dropEvent(QDropEvent *event) override;


	//key
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;

    virtual void inputMethodEvent(QInputMethodEvent *event) override;

	//scroll

	virtual void scrollContentsBy(int dx, int dy) override;
	bool isOnControllHandlers(const QPoint& point, int& handlerIndex, QRectF& handlerRect = QRectF());
	
	

private:
	//QPointF testPoint;
    QScopedPointer<LaserScene> m_scene;
    //LaserScene* m_scene;
    bool m_rubberBandActive;
    QPoint m_rubberBandOrigin;
    //bool m_mousePressed;
    QPoint m_lastDragPos;
	QState* m_mousePressState;
	bool m_mousePressed;

	bool m_isGridNode;//是否为网格交点node
	QPoint m_gridNode;
	bool m_isPrimitiveInteractPoint;//绘制是否获取附近图元的最近点
	QPointF m_primitiveInteractPoint;//scene
	//QPointF m_isPrimitiveInteractPoint;

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
	LaserPrimitive* m_lastPolygon;
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
    bool m_isTextMessageBoxShow;

	bool m_isKeyShiftPressed;
	bool m_isKeyCtrlPress;
    bool m_isCapsLock;
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
	bool m_isItemScaleChangeX = true;
	bool m_isItemScaleChangeY = true;

	qreal m_selectedEditCount = 0;
	LaserPrimitiveGroup* m_group;
    QGraphicsRectItem* m_group2;
	bool m_isLeftSelecting = true;//����߿�ʼѡ�����ұ߿�ʼѡ��

	bool m_isFirstPaint = true;
	//QRectF m_fitInRect;

	QPointF m_lastViewDragPoint;
	QPointF m_anchorPoint;

	bool m_isItemEdge;
	bool m_isItemEdgeCenter;
	QLine m_detectedEdge;
	LaserPrimitive* m_detectedPrimitive;
	//QPolygonF testRect;
	//QPolygonF testBoundinRect;
	LaserPrimitive* m_detectedFillSolid;
	//undo stack
	QUndoStack* m_undoStack;
	QMap<QGraphicsItem*, QTransform> m_undoSelectionList;
	QMap<QGraphicsItem*, QTransform> m_undoTransformList;
    QTransform m_groupLastTransform;
    QTransform m_singleLastTransform;
	//arrange
	QMap<LaserPrimitive*, QTransform> m_copyedList;
	QMap<QString, QList<LaserPrimitive*>> m_groupedMap;
	//layer
	int m_curLayerIndex;
    //text
    //QPointF m_textInsertPos;
    LaserText* m_editingText;
    QPointF m_textMousePressPos;
    QFont m_textFont;
    QLineF m_textCursorLine;//scene
    //QLineF m_startTextCursorLine;//scene
    int m_insertIndex;//-1时说明editingText为null
    int m_textAlighH;
    int m_textAlighV;
    //mirror
    LaserPrimitive* m_mirrorLine;

	bool m_showLaserPos;

	friend class LaserScene;
    
};

#endif // LASERVIEWER_H