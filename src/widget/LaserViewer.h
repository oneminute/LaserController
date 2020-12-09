#ifndef LASERVIEWER_H
#define LASERVIEWER_H

#include <QObject>
#include <QGraphicsView>
#include "scene/Ruller.h"
#include "widget/SplineNode.h"

class LaserScene;

class LaserViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit LaserViewer(QWidget* parent = nullptr);
    //explicit LaserViewer(LaserScene* scene, QWidget* parent = nullptr);
    ~LaserViewer();

    qreal zoomFactor() const;
	

private:
    void init();
	void DetectMouseRange(QRectF _rect, QPointF _pos);

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void zoomChanged();
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
	//void creatingSplineStartReady();
	void creatingSpline();
	void readySpline();
protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    void zoomBy(qreal factor);
	//mouse
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
	//key
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;


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

	QPointF m_creatingLineStartPoint;
	QPointF m_creatingLineEndPoint;

	QPointF m_creatingPolygonStartPoint;
	QPointF m_creatingPolygonEndPoint;
	QVector<QPointF> m_creatingPolygonPoints;
	//qreal m_polygonStartRectWidth;
	QRectF m_polygonStartRect;
	bool m_isMouseInStartRect;

	QVector<SplineNode*> m_creatingSplineNodes;
	QPointF m_creatingSplinePoint;

	bool m_isKeyShiftPressed;

	Ruller m_ruller;
};

#endif // LASERVIEWER_H