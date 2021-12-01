#include "LaserToolButton.h"
#include<QDebug>
#include<QMenu>
#include <LaserApplication.h>
#include "ui/LaserControllerWindow.h"
LaserToolButton::LaserToolButton(QWidget *parent) 
    :QToolButton(parent)
{
    qDebug() << "";
}
LaserToolButton::~LaserToolButton()
{
}

void LaserToolButton::showMenuWidget()
{
    this->menu()->move(this->parentWidget()->mapToGlobal(this->geometry().bottomLeft()));
    this->menu()->show();
}

void LaserToolButton::mousePressEvent(QMouseEvent * event)
{
    emit showMenu();
    this->menu()->hide();
}

void LaserToolButton::mouseReleaseEvent(QMouseEvent * event)
{
   
    showMenuWidget();
    //LaserApplication::mainWindow->setFocus();
    //QToolButton::mouseReleaseEvent(event);
}
