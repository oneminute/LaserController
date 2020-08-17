#include "ConnectionTask.h"
#include "laser/LaserDriver.h"
#include "ui/ConnectionDialog.h"

ConnectionTask::ConnectionTask(LaserDriver* driver, QWidget* parentWidget, QObject* parent)
    : DriverTask(driver, parent)
    , m_parentWidget(parentWidget)
{
    connect(driver, &LaserDriver::comPortsFetched, this, &ConnectionTask::comPortsFetched);
    connect(driver, &LaserDriver::comPortsFetchError, this, &ConnectionTask::comPortsFetchError);
    connect(driver, &LaserDriver::comPortConnected, this, &ConnectionTask::onConnected);
    connect(driver, &LaserDriver::comPortError, this, &ConnectionTask::onError);
}

ConnectionTask::~ConnectionTask()
{

}

void ConnectionTask::start()
{
    // show Com port Selection dialog
    driver()->getPortListAsyn();
    DriverTask::start();
    
}

void ConnectionTask::comPortsFetched(const QStringList& ports)
{
    ConnectionDialog dialog(ports, m_parentWidget);
    if (dialog.exec() == QDialog::Accepted)
    {
        driver()->initComPort(dialog.portName());
    }
    else
    {
        stop();
    }
}

void ConnectionTask::comPortsFetchError()
{
    qWarning() << "get com ports error.";
    setError(true, tr("Get com ports error."));
    stop();
}

void ConnectionTask::onConnected()
{
    qDebug() << "connect successful.";
    stop();
}

void ConnectionTask::onError(const QString& errorMsg)
{
    qWarning() << "connect error:" << errorMsg;
    setError(true, errorMsg);
    stop();
}