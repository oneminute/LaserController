#ifndef DISCONNECTIONTASK_H
#define DISCONNECTIONTASK_H

#include "Task.h"
#include "DriverTask.h"

class DisconnectionTask : public DriverTask
{
    Q_OBJECT
public:
    explicit DisconnectionTask(LaserDriver* driver, QWidget* parentWidget = nullptr, QObject* parent = nullptr);
    virtual ~DisconnectionTask();

    virtual void start() override;

protected slots:
    void onDisconnected();

private:
    QWidget* m_parentWidget;
};

#endif // DISCONNECTIONTASK_H