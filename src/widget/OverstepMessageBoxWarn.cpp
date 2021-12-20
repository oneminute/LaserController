#include "OverstepMessageBoxWarn.h"
#include <QKeyEvent>
#include <QDebug>
#include <QDialog>
OverstepMessageBoxWarn::OverstepMessageBoxWarn(QWidget *parent)
{
    QMessageBox::warning(parent, "", "");
    //setIcon(QMessageBox::Warning);
    //addButton(QMessageBox::StandardButton::Ok);
}
OverstepMessageBoxWarn::~OverstepMessageBoxWarn()
{
}

void OverstepMessageBoxWarn::keyPressEvent(QKeyEvent * event)
{
    QDialog::keyPressEvent(event);
    switch (event->key())
    {
    case Qt::Key_Enter: {
        qDebug() << "enter";
        event->accept();
        break;
    }
    }
}

void OverstepMessageBoxWarn::keyReleaseEvent(QKeyEvent * event)
{
    QDialog::keyReleaseEvent(event);
    
    
}

