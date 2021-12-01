#include "LaserMeuu.h"
#include "LaserApplication.h"
#include "ui/LaserControllerWindow.h"
#include <QKeyEvent>
#include<QDebug>
#include "state/StateController.h"
LaserMenu::LaserMenu(QWidget *parent)
    :QMenu(parent)
{
}
LaserMenu::~LaserMenu()
{
}

void LaserMenu::keyPressEvent(QKeyEvent * e)
{
    switch (e->key())
    {
    case Qt::Key_Tab: {
        if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
            LaserApplication::mainWindow->tabAlignTarget();
        }
    }
    }
}
