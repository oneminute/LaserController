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

    bool isConnected() const;

    QString portName() const;

    qreal layoutWidth() const;
    qreal layoutHeight() const;
    void setLayoutRect(const QRectF& rect, bool toCard = true);

    int printerDrawUnit() const;
    void setPrinterDrawUnit(int unit, bool toCard = true);

    QString requestHardwareId() const;
    QString requestMainCardId() const;
    QString requestDongleId() const;

    void requestMainCardInfo();
    QString mainCardId() const;
    QString mainCardRegisteredDate() const;
    QString mainCardActivatedDate() const;
    QString boundDongleId() const;
    QString boundDongleRegisteredDate() const;
    QString boundDongleActivatedDate() const;
    QString boundDongleBindingTimes() const;
    QString dongleId() const;
    QString dongleRegisteredDate() const;
    QString dongleActivatedDate() const;
    QString dongleBindingTimes() const;
    QString hardwareRegisteredDate() const;
    QString hardwareActivatedDate() const;
    QString hardwareMaintainingTimes() const;

    bool verifyManufacturePassword(const QString& password);

    bool writeUserRegisters();
    bool writeSystemRegisters();
    bool readUserRegisters();
    bool readSystemRegisters();

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
    bool requestTemporaryLicense();
    bool createLicenseFile(const QString& licenseCode);
    void moveToOrigin(qreal speed = 15);

protected:
    void unbindDriver();

protected slots:
    void handleError(int code, const QString& message);
    void handleMessage(int code, const QString& message);

    void onLibraryLoaded(bool success);
    void onLibraryInitialized();
    void onComPortsFetched(const QStringList& portNames);
    void onConnected();
    void onMainCardRegistered();
    void onMainCardActivated(bool temp);

signals:
    void comPortsFetched(const QStringList& ports);
    void comPortConnected(const QString& portName);
    void connected();
    void disconnected();
    void mainCardRegistered();
    void mainCardActivated(bool temp);
    void mainCardInfoFetched();
    void manufacturePasswordVerified(bool pass);

private:
    QScopedPointer<LaserDevicePrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, LaserDevice)
    Q_DISABLE_COPY(LaserDevice)
};

#endif // LASERDEVICE_H