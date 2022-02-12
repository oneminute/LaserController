#include "LaserFontComboBox.h"
#include <QDebug>
LaserFontComboBox::LaserFontComboBox(QWidget * parent)
    :QFontComboBox(parent), m_isShowPopup(false)
{
   
}

LaserFontComboBox::~LaserFontComboBox()
{
}

void LaserFontComboBox::showPopup()
{
    QFontComboBox::showPopup();
    m_isShowPopup = true;
}



void LaserFontComboBox::hidePopup()
{
    QFontComboBox::hidePopup();
    if (m_isShowPopup) {
        emit hidePopupSignal();
    } 
    m_isShowPopup = false;
}

/*
void LaserFontComboBox::focusOutEvent(QFocusEvent * e)
{
    QFontComboBox::focusOutEvent(e);
    //this->
    //if()
    //emit outFocus();
}

void LaserFontComboBox::leaveEvent(QEvent * event)
{
    qDebug() << "";
}

void LaserFontComboBox::mouseReleaseEvent(QMouseEvent * e)
{
    qDebug() << "";
}

void LaserFontComboBox::setIsChangedItem(bool bl)
{
    m_isChangedItem = bl;
}

bool LaserFontComboBox::isChangedItem()
{
    return m_isChangedItem;
}*/
