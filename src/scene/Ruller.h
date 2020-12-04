#ifndef RULLER_H
#define RULLER_H

#include <QMatrix>
#include <QGraphicsView>
class Ruller
{

public:
    Ruller(QGraphicsView* _view);
    qreal lineFactor() const;
    qreal lineWidth(qreal width = 1.0f) const;
    QMatrix fromSceneMatrix() const;
    QMatrix toSceneMatrix() const;
    void drawAxes();
    void drawTick();
    void draw();
private:
    QPointF  m_origin;
    qreal m_factor;
    QGraphicsView* m_view;
};

#endif // RULLER_H
