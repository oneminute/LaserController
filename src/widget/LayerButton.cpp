#include "LayerButton.h"

#include <QMouseEvent>
#include <QColorDialog>
#include <QPainter>
#include <QVBoxLayout> 

LayerButton::LayerButton(LaserViewer* viewer, QWidget* parent)
    : QLabel(parent)
    , m_color(Qt::red)
    , m_pressed(false)
	, m_moved(false)
	//, m_clicked(false)
	, m_checked(false)
    , m_layer(nullptr)
{
	m_viewer = viewer;
	installEventFilter(this);
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

bool LayerButton::checked()
{
	return m_checked;
}

void LayerButton::setChecked(bool checked)
{
	m_checked = checked;
}

void LayerButton::setLayerIndex(int index)
{
	m_layerIndex = index;
}

void LayerButton::setCheckedTrue()
{
	m_checked = true;
	m_viewer->setCurLayerIndex(m_layerIndex);
	this->repaint();
	m_viewer->viewport()->repaint();
}

void LayerButton::contextMenuEvent(QContextMenuEvent * event)
{
    qDebug() << "context menu event";
	if (!this->isEnabled()) {
		return;
	}
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
	if (!this->isEnabled()) {
		return;
	}
}

void LayerButton::mousePressEvent(QMouseEvent * event)
{
	if (!this->isEnabled()) {
		return;
	}
    m_pressed = true;
	this->repaint();
}

void LayerButton::mouseReleaseEvent(QMouseEvent * event)
{
	if (!this->isEnabled()) {
		return;
	}
    if (m_pressed)
    {
        emit clicked();
        m_pressed = false;
		//m_clicked = true;
		
		for each(QObject* object in this->parent()->children()) {
			LayerButton* widget = qobject_cast<LayerButton*>(object);
			if (widget) {
				widget->setChecked(false);
				widget->repaint();
			}
		}
		setCheckedTrue();
	}

	
}

bool LayerButton::eventFilter(QObject *watched, QEvent *event)
{
	if (!this->isEnabled()) {
		true;
	}
	if (watched == this && event->type() == QEvent::Enter) {
		m_moved = true;
		this->repaint();
	}
	if (watched == this && event->type() == QEvent::Move) {
		//qDebug() << event->type();
	}
	if (watched == this && event->type() == QEvent::Leave) {
		m_moved = false;
		this->repaint();
	}
	return false;
}

void LayerButton::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
	if (m_checked) {
		painter.setBrush(QBrush(QColor(198, 198, 198, 150)));
		painter.setPen(QPen(Qt::black, 2));
		painter.drawRoundedRect(1, 1, width() - 2, height() - 2, 5, 5);
	}
	else {
		if (m_moved) {
			//QTransform t1;
			//t1.translate(-10, -10);
			//painter.setTransform(t1);
			painter.setBrush(QBrush(QColor(198, 198, 198, 150)));
			painter.setPen(QPen(QColor(213, 213, 213, 255), 2));
			painter.drawRoundedRect(1, 1, width() - 2, height() - 2, 5, 5);
			//t1 = QTransform::fromTranslate(0, 0);
			//painter.setTransform(t1);
		}
		
	}
	
    QTransform t;
    //t.translate(width() / 2, height() / 2);
	t.translate(5, 5);
    painter.setTransform(t);

	QColor color = m_color;
	if (!this->isEnabled()) {
		color = QColor(198, 198, 198, 150);
	}

    painter.setBrush(QBrush(color));
    painter.setPen(QPen(color));
    //painter.drawRect(0, 0, 20, 20);
	painter.drawRect(0, 0, width()-10, height()-10);

    t = QTransform::fromTranslate(0, 0);
    painter.setTransform(t);
    painter.setPen(Qt::black);
	
    //painter.drawText(QRect(0, 20, width(), height() - 20), Qt::AlignCenter, text());
}

