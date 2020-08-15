#ifndef TASK_H
#define TASK_H

#include <QObject>

#include "common/common.h"
#include "util/Utils.h"

class LaserDriver;

class Task : public QObject
{
    Q_OBJECT
public:
    explicit Task(QObject* parent = nullptr) 
        : QObject(parent)
        , m_isRunning(false)
        , m_isError(false)
        , m_errorMsg("")
        , m_progress(0.f)
    {
        setObjectName(utils::createUUID("task_"));
    }
    virtual ~Task() {}

    bool isRunning() const { return m_isRunning; }
    bool isError() const { return m_isError; }
    QString errorMessage() const { return m_errorMsg; }
    float progress() const { return m_progress; }

    virtual void start()
    {
        qDebug().noquote() << "start a task:" << objectName();
        m_isRunning = true;
        m_isError = false;
        m_errorMsg = "";
        m_progress = 0;
        emit started();
    }

    virtual void stop()
    {
        qDebug().noquote() << "stop a task:" << objectName();
        /*m_isRunning = false;
        m_isError = false;
        m_errorMsg = "";*/
        emit stopped();
        this->deleteLater();
    }

protected:
    void setProgress(float progress) 
    {
        //qDebug() << "progress:" << progress;
        m_progress = progress; 
        emit progressUpdated(progress);
    }
    void setRunning(bool running) { m_isRunning = running; }
    void setError(bool error, const QString& errorMsg = "")
    {
        m_isError = error;
        m_errorMsg = errorMsg;
    }

public slots:
    /*void onError(const QString& errorMsg = "")
    {
        qDebug().noquote() << "an error occured:" << objectName() << ", " << errorMsg;
        m_isRunning = false;
        m_isError = true;
        m_errorMsg = errorMsg;
        emit stopped();
        this->deleteLater();
    }*/

signals:
    void started();
    void stopped();
    void progressUpdated(float progress);

private:
    bool m_isRunning;
    bool m_isError;
    QString m_errorMsg;
    float m_progress;
};

class DriverTask : public Task
{
    Q_OBJECT
public:
    explicit DriverTask(LaserDriver* driver, QObject* parent = nullptr)
        : Task(parent)
        , m_driver(driver)
    {}

    virtual ~DriverTask() {}

    LaserDriver* driver() const { return m_driver; }

private:
    LaserDriver* m_driver;
};

class MachiningTask: public DriverTask
{
    Q_OBJECT
public:
    explicit MachiningTask(LaserDriver* driver, const QString& filename, bool zeroPointStyle = false, QObject* parent = nullptr);
    virtual ~MachiningTask();
    
    virtual void start();

    bool zeroPointStyle() const { return m_zeroPointStyle; }
    void setZeroPointStyle(bool zeroPointStyle) { m_zeroPointStyle = zeroPointStyle; }

    QString filename() const { return m_filename; }
    void setFilename(const QString& filename) { m_filename = filename; }

signals:
    void initialized();
    void downloaded();
    void started();
    void paused();
    void stopped();

public slots:
    void onStarted();
    void onPaused();
    void onStopped();
    void onCompleted(bool isSuccess = true, const QString& errorMsg = "");
    void onDownloading(int current, int total, float progress);
    void onDownloaded();

private:
    bool m_zeroPointStyle;
    QString m_filename;
};

//class ComPortTask : public DriverTask
//{
//    Q_OBJECT
//public:
//    explicit ComPortTask(LaserDriver* driver, QObject* parent = nullptr);
//    virtual ~ComPortTask();
//
//    virtual void start();
//    virtual void stop();
//
//signals:
//
//};

//class StartMachiningTask : public DriverTask
//{
//    Q_OBJECT
//public:
//    explicit StartMachiningTask(LaserDriver* driver, QObject* parent = nullptr);
//    virtual ~StartMachiningTask();
//
//    virtual void start();
//    virtual void stop();
//};


//class SignalTask : public Task
//{
//    Q_OBJECT
//public:
//    explicit SignalTask(QObject* parent = nullptr)
//        : Task(parent)
//    {}
//    virtual ~SignalTask() {}
//
//    template<typename Func1>
//    QMetaObject::Connection bindSignal(QObject* sender, Func1 func1)
//    {
//        connect(sender, func1, this, onStopped);
//    }
//
//    template<typename Func1>
//    QMetaObject::Connection bindErrorSignal(QObject* sender, Func1 func1)
//    {
//        connect(sender, func1, this, onError);
//    }
//
//private:
//};

#endif // TASK_H