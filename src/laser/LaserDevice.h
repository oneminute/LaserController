#ifndef LASERDEVICE_H
#define LASERDEVICE_H

#include "common/common.h"
#include "LaserRegister.h"
#include <QObject>
#include <QMutex>
#include <QTransform>
#include <QWaitCondition>

class ConfigItem;
class LaserDriver;
class LaserDevicePrivate;
class LaserDevice : public QObject
{
    Q_OBJECT
public:
    explicit LaserDevice(LaserDriver* driver, QObject* parent = nullptr);
    ~LaserDevice();

    bool isInit() const;
    bool isConnected() const;

    QString name() const;
    void setName(const QString& name);
    QString portName() const;

    qreal layoutWidth() const;
    qreal layoutHeight() const;
    //void setLayoutRect(const QRectF& rect, bool toCard = true);

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

    bool readUserRegister(int address);
    bool writeUserReigister(int address, const QVariant& value);
    bool readSystemRegister(int address);
    bool writeSystemReigister(int address, const QVariant& value);

    void showLibraryVersion();

    void checkVersionUpdate(bool hardware, const QString& flag, int currentVersion, const QString& versionNoteToJsonFile);

    void moveTo(const QVector3D& pos, QUADRANT quad);
    void moveBy(const QVector3D& pos);

    bool isAvailable() const;

    void showAboutWindow(int interval = 0, bool modal = true);
    void closeAboutWindow();

    LaserRegister* userRegister(int addr) const;
    LaserRegister* systemRegister(int addr) const;

    QPointF origin() const;
    QPointF deviceOrigin() const;
    QTransform transform() const;
    QTransform deviceTransform() const;

    void batchParse(const QString& raw, bool isSystem, ModifiedBy modifiedBy);

    LaserRegister::RegistersMap userRegisterValues(bool onlyModified = false) const;
    LaserRegister::RegistersMap systemRegisterValues(bool onlyModified = false) const;

public slots:
    bool load();
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