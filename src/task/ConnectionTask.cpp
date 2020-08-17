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
    connect(driver, &LaserDriver::sysParamFromCardArrived, this, &ConnectionTask::onSysParamFromCardArrived);
    connect(driver, &LaserDriver::sysParamFromCardError, this, &ConnectionTask::onSysParamFromCardError);
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
    m_registerList.clear();
    m_registerList
        << LaserDriver::REG_03
        << LaserDriver::REG_05
        << LaserDriver::REG_06
        << LaserDriver::REG_07
        << LaserDriver::REG_08
        << LaserDriver::REG_09
        << LaserDriver::REG_10
        << LaserDriver::REG_11
        << LaserDriver::REG_12
        << LaserDriver::REG_13
        << LaserDriver::REG_14
        << LaserDriver::REG_15
        << LaserDriver::REG_16
        << LaserDriver::REG_17
        << LaserDriver::REG_18
        << LaserDriver::REG_19
        << LaserDriver::REG_20
        << LaserDriver::REG_21
        << LaserDriver::REG_22
        << LaserDriver::REG_23
        << LaserDriver::REG_24
        << LaserDriver::REG_25
        << LaserDriver::REG_26
        << LaserDriver::REG_27
        << LaserDriver::REG_31
        << LaserDriver::REG_32
        << LaserDriver::REG_33
        << LaserDriver::REG_34
        << LaserDriver::REG_35
        << LaserDriver::REG_36
        << LaserDriver::REG_38
        << LaserDriver::REG_39
        << LaserDriver::REG_40;
    driver()->readSysParamFromCard(m_registerList);
}

void ConnectionTask::onError(const QString& errorMsg)
{
    qWarning() << "connect error:" << errorMsg;
    setError(true, errorMsg);
    stop();
}

void ConnectionTask::onSysParamFromCardArrived(const QString & eventData)
{
    qDebug() << "read register params.";
    QString rawData = eventData;
    if (eventData.endsWith(";"))
        rawData = eventData.left(eventData.size() - 1);
    QStringList values = rawData.split(";");
    if (values.size() == m_registerList.size())
    {
        for (int i = 0; i < m_registerList.size(); i++)
        {
            LaserDriver::RegisterType rt = static_cast<LaserDriver::RegisterType>(m_registerList[i]);
            QString rawValue = values[i];
            QStringList pair = rawValue.split(",");
            if (pair.size() != 2)
            {
                qWarning() << "parse register value of " << rt << "error";
                continue;
            }
            int rawRt = pair[0].toInt();
            LaserDriver::RegisterType rtResponsed = static_cast<LaserDriver::RegisterType>(rawRt);
            if (rt != rtResponsed)
            {
                qWarning() << "register data from card does not match requested register.";
                continue;
            }
            driver()->setRegister(rt, pair[1]);
            qDebug() << "got register value:" << rt << "=" << pair[1];
        }
    }
    else
    {
        setError(true, "Fetched registers' values don't match registers requested.");
    }
    stop();
}

void ConnectionTask::onSysParamFromCardError()
{
    setError(true, "Fetch registers' values error.");
}
