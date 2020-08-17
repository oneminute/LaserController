#include "DisconnectionTask.h"
#include "laser/LaserDriver.h"

#include <QMessageBox>
#include <QWidget>

DisconnectionTask::DisconnectionTask(LaserDriver* driver, QWidget* parentWidget, QObject* parent)
    : DriverTask(driver, parent)
    , m_parentWidget(parentWidget)
{
    connect(driver, &LaserDriver::comPortDisconnected, this, &DisconnectionTask::onDisconnected);
}

DisconnectionTask::~DisconnectionTask()
{

}

void DisconnectionTask::start()
{
    if (QMessageBox::Apply == QMessageBox::question(m_parentWidget, tr("Disconnect"), tr("Do you want to disconnect from laser machine?"), QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Discard))
    {
        driver()->uninitComPort();
        DriverTask::start();
    }
    else
    {
        stop();
    }
}

void DisconnectionTask::onDisconnected()
{
    stop();
}