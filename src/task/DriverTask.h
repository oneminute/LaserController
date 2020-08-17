#ifndef DRIVERTASK_H
#define DRIVERTASK_H

#include "Task.h"

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

#endif // DRIVERTASK_H