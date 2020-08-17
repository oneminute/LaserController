#include "MachiningTask.h"
#include "laser/LaserDriver.h"

MachiningTask::MachiningTask(LaserDriver* driver, const QString& filename, bool zeroPointStyle, QObject* parent)
    : DriverTask(driver, parent)
    , m_zeroPointStyle(zeroPointStyle)
    , m_filename(filename)
{
    connect(driver, &LaserDriver::machiningStarted, this, &MachiningTask::onStarted);
    //connect(driver, &LaserDriver::machiningStopped, this, &MachiningTask::onStopped);
    //connect(driver, &LaserDriver::idle, this, &MachiningTask::onStopped);
    connect(driver, &LaserDriver::downloading, this, &MachiningTask::onDownloading);
    connect(driver, &LaserDriver::downloaded, this, &MachiningTask::onDownloaded);
    //connect(driver, &LaserDriver::machiningStopped, this, &MachiningTask::onCompleted);
}

MachiningTask::~MachiningTask()
{

}

void MachiningTask::start()
{
    //driver()->stopMachining();
    Task::start();
    
    setProgress(0.05f);
    onStopped();
}

void MachiningTask::onStarted()
{
    qDebug() << "start machining.";
}

void MachiningTask::onPaused()
{
}

void MachiningTask::onStopped()
{
    //disconnect(driver(), &LaserDriver::machiningStopped, this, &MachiningTask::onStopped);
    //disconnect(driver(), &LaserDriver::idle, this, &MachiningTask::onStopped);
    setProgress(0.1f);
    driver()->loadDataFromFile(m_filename);
    driver()->startMachining(m_zeroPointStyle);
}

void MachiningTask::onCompleted(bool isSuccess, const QString& errorMsg)
{
    if (isSuccess)
    {
        stop();
    }
    else
    {
        setError(false, errorMsg);
        stop();
    }
}

void MachiningTask::onDownloading(int current, int total, float progress)
{
    setProgress(progress * 0.2f + 0.1f);
}

void MachiningTask::onDownloaded()
{
    //driver()->startMachining(m_zeroPointStyle);
}