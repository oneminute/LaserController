#include "LayerButton.h"

#include <QMouseEvent>
#include <QColorDialog>
#include <QPainter>

LayerButton::LayerButton(QWidget* parent)
    : QLabel(parent)
    , m_color(Qt::red)
    , m_pressed(false)
    , m_layer(nullptr)
{
}

LayerButton::~LayerButton()
{
}

QColor LayerButton::color() const 
{
    return m_color; 
}

void LayerButton::setColor(const QColor & color)
{
    m_color = color; 
    update();
}

void LayerButton::contextMenuEvent(QContextMenuEvent * event)
{
    qDebug() << "context menu event";
    QColorDialog dialog;
    if (dialog.exec() == QDialog::Accepted)
    {
        m_color = dialog.currentColor();
        qDebug() << m_color;
        emit colorUpdated();
    }
}

void LayerButton::mouseDoubleClickEvent(QMouseEvent * event)
{
    qDebug() << "double click";
}

void LayerButton::mousePressEvent(QMouseEvent * event)
{
    m_pressed = true;
}

void LayerButton::mouseReleaseEvent(QMouseEvent * event)
{
    if (m_pressed)
    {
        emit clicked();
        m_pressed = false;
    }
}

void LayerButton::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QTransform t;
    t.translate(width() / 2, 0);
    painter.setTransform(t);

    painter.setBrush(QBrush(m_color));
    painter.setPen(QPen(m_color));
    painter.drawRect(-10, 0, 20, 20);

    t = QTransform::fromTranslate(0, 0);
    painter.setTransform(t);
    painter.setPen(Qt::black);
    painter.drawText(QRect(0, 20, width(), height() - 20), Qt::AlignCenter, text());
}

