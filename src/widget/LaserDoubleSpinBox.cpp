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
		case Qt::Key_Return://Enter¼ü
		case Qt::Key_Enter://Êı×Ö¼üÅÌµÄEnter¼ü
		{
			emit enterOrLostFocus();
			break;
		}
	}
	
}
