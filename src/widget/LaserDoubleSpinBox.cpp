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
	
	
}

void LaserDoubleSpinBox::keyReleaseEvent(QKeyEvent * event)
{
    QDoubleSpinBox::keyReleaseEvent(event);
    switch (event->key()) {
    case Qt::Key_Return://Enter¼ü
    case Qt::Key_Enter://Êý×Ö¼üÅÌµÄEnter¼ü
    {
        event->accept();
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


