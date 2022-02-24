#include "LaserDoubleSpinBox.h"
#include<QDebug>
#include "LaserApplication.h"
#include "ui/LaserControllerWindow.h"

LaserDoubleSpinBox::LaserDoubleSpinBox(QWidget * parent)
	:QDoubleSpinBox(parent), m_isPressEnterKey(false), m_isValueChanged(false)
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
    case Qt::Key_Return://Enter��
    case Qt::Key_Enter://���ּ��̵�Enter��
    {
        event->accept();
        emit enterOrLostFocus();
        m_isPressEnterKey = true;
        LaserApplication::mainWindow->labelPercentage()->setFocus();
        break;
    }
    }
}

void LaserDoubleSpinBox::focusOutEvent(QFocusEvent * event)
{
	QDoubleSpinBox::focusOutEvent(event);
    if (!m_isPressEnterKey) {
        emit enterOrLostFocus();
    }	
    m_isPressEnterKey = false;
    
	//qDebug() << "lost focus";
}


