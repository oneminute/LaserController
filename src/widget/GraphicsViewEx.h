#ifndef GRAPHICSVIEWEX_H
#define GRAPHICSVIEWEX_H

#include <QGraphicsView>

class GraphicsViewEx : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsViewEx(QWidget* parent = nullptr);
    ~GraphicsViewEx();

    void fitZoom();
    void zoomToActual();

protected:
    virtual void wheelEvent(QWheelEvent* e) override;
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;

public slots:

protected slots:

signals:
    void clicked(const QPointF& pos);

private:
    bool m_zoomEnabled;
    bool m_mousePressed;
    QPoint m_lastMousePos;
    QPoint m_mousePressedPos;
};

#endif // GRAPHICSVIEWEX_H