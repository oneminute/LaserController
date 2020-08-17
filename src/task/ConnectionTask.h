#ifndef CONNECTIONTASK_H
#define CONNECTIONTASK_H

#include "DriverTask.h"

class ConnectionTask : public DriverTask
{
    Q_OBJECT
public:
    explicit ConnectionTask(LaserDriver* driver, QWidget* parentWidget = nullptr, QObject* parent = nullptr);
    virtual ~ConnectionTask();

    virtual void start() override;

protected slots:
    void comPortsFetched(const QStringList& ports);
    void comPortsFetchError();
    void onConnected();
    void onError(const QString& errorMsg);

private:
    QWidget* m_parentWidget;
};

#endif // CONNECTIONTASK_H