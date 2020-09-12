#include "LayerButton.h"

#include <QMouseEvent>
#include <QColorDialog>

LayerButton::LayerButton(QWidget* parent)
    : QLabel(parent)
    , m_color(Qt::red)
    , m_pressed(false)
    , m_layer(nullptr)
{
    updateColor();
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
    updateColor();
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

void LayerButton::updateColor()
{
    QVariant v(m_color);
    QString styleSheet = QString("QLabel {background-color: %1}").arg(v.toString());
    setStyleSheet(styleSheet);
}
