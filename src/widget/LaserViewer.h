#ifndef LASERVIEWER_H
#define LASERVIEWER_H

#include <QObject>
#include <QGraphicsView>
#include "scene/Ruller.h"

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

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    void zoomBy(qreal factor);
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QScopedPointer<LaserScene> m_scene;
    //LaserScene* m_scene;
    bool m_rubberBandActive;
    QPoint m_rubberBandOrigin;
    bool m_mousePressed;
    QPoint m_lastDragPos;
    QPointF m_selectionStartPoint;
    QPointF m_selectionEndPoint;
	Ruller m_ruller;
};

#endif // LASERVIEWER_H