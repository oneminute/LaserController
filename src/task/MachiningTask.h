#ifndef MACHININGTASK_H
#define MACHININGTASK_H

#include "Task.h"
#include "DriverTask.h"

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
    void onCompleted();
    void onDownloading(int current, int total, float progress);
    void onDownloaded();
    void onWorkingCanceled();
    void onunknownError();

private:
    bool m_zeroPointStyle;
    QString m_filename;
};

#endif // MACHININGTASK_H