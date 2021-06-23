#ifndef LASERDEVICE_H
#define LASERDEVICE_H

#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class LaserDriver;
class LaserDevicePrivate;
class LaserDevice : public QObject
{
    Q_OBJECT
public:
    explicit LaserDevice(QObject* parent = nullptr);
    ~LaserDevice();

    void resetDriver(LaserDriver* driver);

    QString portName() const;

    qreal layoutWidth() const;
    void setLayoutWidth(qreal width);
    qreal layoutHeight() const;
    void setLayoutHeight(qreal height);
    
public slots:
    void load();
    void unload();
    void connectDevice(const QString& portName);
    void disconnectDevice();
    QString activateMainCard(const QString& name,
        const QString& address,
        const QString& phone,
        const QString& qq,
        const QString& wx,
        const QString& email,
        const QString& country,
        const QString& distributor,
        const QString& trademark,
        const QString& model
    );

protected:
    void unbindDriver();

protected slots:
    void handleError(int code, const QString& message);
    void handleMessage(int code, const QString& message);

    void onLibraryLoaded(bool success);
    void onLibraryInitialized();
    void onComPortsFetched(const QStringList& portNames);

signals:
    void comPortsFetched(const QStringList& ports);
    void comPortConnected(const QString& portName);
    void connected();
    void disconnected();
    void mainCardRegistered();
    void mainCardActivated();

private:
    QScopedPointer<LaserDevicePrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, LaserDevice)
    Q_DISABLE_COPY(LaserDevice)
};

#endif // LASERDEVICE_H