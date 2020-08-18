#include "MachiningTask.h"
#include "laser/LaserDriver.h"

MachiningTask::MachiningTask(LaserDriver* driver, const QString& filename, bool zeroPointStyle, QObject* parent)
    : DriverTask(driver, parent)
    , m_zeroPointStyle(zeroPointStyle)
    , m_filename(filename)
{
    connect(driver, &LaserDriver::machiningStarted, this, &MachiningTask::onStarted);
    connect(driver, &LaserDriver::machiningPaused, this, &MachiningTask::onPaused);
    connect(driver, &LaserDriver::downloading, this, &MachiningTask::onDownloading);
    connect(driver, &LaserDriver::downloaded, this, &MachiningTask::onDownloaded);
    connect(driver, &LaserDriver::machiningStopped, this, &MachiningTask::onStopped);
    connect(driver, &LaserDriver::machiningCompleted, this, &MachiningTask::onCompleted);
}

MachiningTask::~MachiningTask()
{

}

void MachiningTask::start()
{
    driver()->loadDataFromFile(m_filename);
    Task::start();
}

void MachiningTask::onStarted()
{
    qDebug() << "start machining.";
}

void MachiningTask::onPaused()
{
    qDebug() << "machining paused.";
}

void MachiningTask::onStopped()
{
    setProgress(0.1f);
    stop();
}

void MachiningTask::onCompleted()
{
    stop();
}

void MachiningTask::onDownloading(int current, int total, float progress)
{
    setProgress(progress * 0.2f + 0.1f);
}

void MachiningTask::onDownloaded()
{
    setProgress(0.3f);
}

void MachiningTask::onWorkingCanceled()
{
    setError(true, "working has been canceled.");
}

void MachiningTask::onunknownError()
{
    setError(true, "unknown error.");
}
