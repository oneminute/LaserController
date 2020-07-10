#ifndef LASERVIEWER_H
#define LASERVIEWER_H

#include <QObject>
#include <QGraphicsView>

class LaserViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit LaserViewer(QWidget* parent = nullptr);
    explicit LaserViewer(QGraphicsScene* scene, QWidget* parent = nullptr);
    ~LaserViewer();
};

#endif // LASERVIEWER_H