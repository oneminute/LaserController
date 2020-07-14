#ifndef LASERVIEWER_H
#define LASERVIEWER_H

#include <QObject>
#include <QGraphicsView>

class LaserScene;

class LaserViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit LaserViewer(QWidget* parent = nullptr);
    explicit LaserViewer(LaserScene* scene, QWidget* parent = nullptr);
    ~LaserViewer();

    qreal zoomFactor() const;

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void zoomChanged();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    void zoomBy(qreal factor);

private:
    LaserScene* m_scene;
};

#endif // LASERVIEWER_H