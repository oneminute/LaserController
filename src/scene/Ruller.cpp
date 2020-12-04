#include "Ruller.h"
#include <QMatrix>
#include <QDebug>

Ruller::Ruller(QGraphicsView* _view):
    m_view(_view),
    m_origin(0, 0),
    m_factor(50)
{

}
qreal Ruller::lineFactor() const
{
    return 1.0f / m_factor;
}
qreal Ruller::lineWidth(qreal width) const
{
    return width * lineFactor();
}
QMatrix Ruller::fromSceneMatrix() const
{
    //QPointF origin = m_view->mapFrom(m_origin);
	QPointF origin = m_origin;
    QTransform t(m_view->transform());
    QMatrix m(m_factor, 0, 0, -m_factor, origin.x(), origin.y());
    m = t.toAffine() * m;
    return m;
}

QMatrix Ruller::toSceneMatrix() const
{
    QMatrix matrix;
    matrix.scale(1.0 / m_factor, -1.0 / m_factor);
    matrix.translate(-m_origin.x(), -m_origin.y());
    return matrix;
}
void Ruller::drawAxes()
{
    QPainter painter(m_view->viewport());
    painter.setMatrix(fromSceneMatrix());
    painter.setPen(QPen(Qt::lightGray, lineWidth(2), Qt::SolidLine, Qt::PenCapStyle::RoundCap));

    QRectF rect = toSceneMatrix().mapRect(m_view->sceneRect());
	//QRectF rect = m_view->rect();
	qDebug() << m_view->rect();
    //painter.drawRect(rect);
    painter.drawLine(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
    painter.drawLine(QPointF(0, rect.top()), QPointF(0, rect.bottom()));

}

void Ruller::drawTick()
{
    QPainter painter(m_view->viewport());
    //QPainter textPainter(viewport());

    //textPainter.setFont(QFont("Microsoft YaHei", 15));
    painter.setMatrix(fromSceneMatrix());
    painter.setPen(QPen(Qt::lightGray, lineWidth(), Qt::SolidLine, Qt::RoundCap));

    QRectF rect = toSceneMatrix().mapRect(m_view->sceneRect());
	//QRectF rect = m_view->rect();
    qDebug()<<"left: "<<rect.left();
    qDebug()<<"right: "<<rect.right();
    qDebug()<<"top: "<<rect.top();
    qDebug()<<"bottom: "<<rect.bottom();
    qreal unit = 1.0 / m_factor;
    qreal size = m_factor / 10;
    //横轴
    for (int i = std::round(rect.left()); i <= std::round(rect.right()); i++)
    {
        if(i <=  std::round(rect.right()) - 1)
        for(int j = 0; j < size; j ++){
            painter.drawLine(QPointF(i + j* unit*10, rect.bottom()), QPointF(i + j*unit*10, rect.bottom()-unit * 3));
        }
        painter.drawLine(QPointF(i, rect.bottom()), QPointF(i, rect.bottom()-unit * 8));
    }
    //纵轴
    for (int i = std::round(rect.top()); i <= std::round(rect.bottom()); i++)
    {
        if(i <=  std::round(rect.bottom()) - 1)
        for(int j = 0; j < size; j ++){
            painter.drawLine(QPointF(rect.left(), i + j* unit*10), QPointF(rect.left()+unit * 3, i + j*unit*10));
        }
        painter.drawLine(QPointF(rect.left(), i), QPointF(rect.left()+unit * 8, i));
        //painter.drawText((QPoint(rect.left(), i), QPoint(rect.left()-unit * 28, i-unit * 28)), "5");

        //textPainter.drawText(rect.left(), i, "5");
    }

}

void Ruller::draw()
{
    drawAxes();
    drawTick();
}
