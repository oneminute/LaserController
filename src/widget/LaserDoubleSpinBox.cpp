#include "LaserDoubleSpinBox.h"
#include<QDebug>

LaserDoubleSpinBox::LaserDoubleSpinBox(QWidget * parent)
	:QDoubleSpinBox(parent)
{
}

LaserDoubleSpinBox::~LaserDoubleSpinBox()
{
}

void LaserDoubleSpinBox::keyPressEvent(QKeyEvent * event)
{
	QDoubleSpinBox::keyPressEvent(event);
	switch (event->key()) {
		case Qt::Key_Return://Enter��
		case Qt::Key_Enter://���ּ��̵�Enter��
		{
			emit enterOrLostFocus();
			break;
		}
	}
	
}

void LaserDoubleSpinBox::focusOutEvent(QFocusEvent * event)
{
	QDoubleSpinBox::focusOutEvent(event);
	emit enterOrLostFocus();
	//qDebug() << "lost focus";
}


