#ifndef TASK_H
#define TASK_H

#include <QObject>

#include "common/common.h"
#include "util/Utils.h"

class LaserDriver;

class Task;

class DriverTask;
class ConnectionTask;
class MahciningTask;

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

signals:
    void started();
    void stopped();
    void error();
    void progressUpdated(float progress);

private:
    bool m_isRunning;
    bool m_isError;
    QString m_errorMsg;
    float m_progress;
};

#endif // TASK_H