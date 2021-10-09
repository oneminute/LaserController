#ifndef PREVIEWVIEWER_H
#define PREVIEWVIEWER_H

#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <scene/LaserScene.h>

class PreviewPathItemPrivate;
class PreviewPathItem : public QGraphicsPathItem
{

};

class PreviewViewerPrivate;
class PreviewViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit PreviewViewer(QWidget* parent = nullptr);
    ~PreviewViewer();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    qreal zoomValue() const;
    void setZoomValue(qreal zoomScale);
    bool zoomBy(qreal factor, QPointF zoomAnchor = QPointF(0, 0), bool zoomAnchorCenter = false);
    void reset();
private:
    QPointF m_anchorPoint;
    QGraphicsScene* m_scene;
    QPointF m_lastViewDragPoint;
protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
};

#endif // PREVIEWVIEWER_H
