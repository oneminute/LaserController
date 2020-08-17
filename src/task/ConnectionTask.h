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
    void onSysParamFromCardArrived(const QString& eventData);
    void onSysParamFromCardError();

private:
    QWidget* m_parentWidget;
    QList<int> m_registerList;
};

#endif // CONNECTIONTASK_H