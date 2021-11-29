#include "LaserDoubleSpinBox.h"
#include<QDebug>

LaserDoubleSpinBox::LaserDoubleSpinBox(QWidget * parent)
	:QDoubleSpinBox(parent), m_isPressEnterKey(false), m_isValueChanged(false)
{
    /*connect(this, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double _val) {
        if (_val != m_lastValue) {
            m_isValueChanged = true;
        }
        else {
            m_isValueChanged = false;
        }
        
    });*/
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
        //qDebug() << "keyReleaseEvent";
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


