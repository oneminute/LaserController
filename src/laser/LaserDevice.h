#ifndef LASERDEVICE_H
#define LASERDEVICE_H

#include "common/common.h"
#include "LaserRegister.h"
#include "laser/LaserDefines.h"
#include <QObject>
#include <QMutex>
#include <QTransform>
#include <QWaitCondition>
#include <QVector4D>
#include <QVector3D>

class ConfigItem;
class LaserDriver;
class LaserDevicePrivate;
class LaserDocument;
class ProgressItem;

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

    int layoutWidth() const;
    int layoutHeight() const;

    int printerDrawUnit() const;
    void setPrinterDrawUnit(int unit, bool toCard = true);

    QString password() const;
    void setPassword(const QString& value);

    ProgressItem* progress();
    void clearProgress();
    void resetProgress(ProgressItem* parent);

    QString requestHardwareId() const;
    QString requestMainCardId() const;
    QString requestRegisteId() const;
    QString requestDongleId() const;
    void updateWorkState();
    void changeState(const DeviceState& state);

    void requestMainCardInfo();
    void requestMainCardRegInfo();
    QString getMainCardModal();
    QString firmwareVersion() const;
    QString hardwareIdentID() const;
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
    bool isMainCardActivated() const;
    bool isMainCardRegistered() const;

    QString apiLibVersion() const;
    QString apiLibCompileInfo() const;

    bool verifyManufacturePassword(const QString& password, int errorCount);
    bool changeManufacturePassword(const QString& password, const QString& newPassword);
    QString showManufacturePasswordDialog(QWidget* parentWnd = nullptr);

    MainCardActivateResult autoActivateMainCard();
    bool sendAuthenticationEmail(const QString& email);
    bool registerMainCard(const QString& registeCode, QWidget* parentWidget = nullptr);

    bool writeExternalRegisters(bool onlyModified = true);
    bool writeUserRegisters(bool onlyModified = true);
    bool writeSystemRegisters(const QString& password, bool onlyModified = true);
    bool readUserRegisters();
    bool readSystemRegisters();
    bool readHostRegisters();

    bool readUserRegister(int address);
    bool writeUserReigister(int address, const QVariant& value);
    bool readSystemRegister(int address);
    bool writeSystemReigister(int address, const QVariant& value);

    void showLibraryVersion();

    void checkVersionUpdate(bool hardware, const QString& flag, int currentVersion, const QString& versionNoteToJsonFile);

    void moveTo(const QVector4D& pos, bool xEnabled = true, 
        bool yEnabled = true, bool zEnabled = true, bool uEnabled = true);
    void moveBy(const QVector4D& pos, bool xEnabled = true, 
        bool yEnabled = true, bool zEnabled = true, bool uEnabled = true);
    void moveToZOrigin();
    void moveToUOrigin();
    void moveToXYOrigin();
    void drawRectangularBorder(LaserDocument* doc);

    bool isAvailable() const;

    void showAboutWindow(int interval = 0, bool modal = true);
    void closeAboutWindow();
    int showUpdateDialog();
    void showSoftwareUpdateWizard();
    void showFirmwareUpdateWizard();
    void updateDriverLanguage();

    bool checkLayoutForMoving(const QPoint& dest);
    bool checkLayoutForMachining(const QRect& docBounding, const QRect& engravingBounding);

    LaserRegister* userRegister(int addr) const;
    LaserRegister* systemRegister(int addr) const;

    /// <summary>
    /// 在设备坐标系下原点的坐标值。该值恒为(0, 0)。
    /// </summary>
    /// <returns></returns>
    QPoint origin() const { return QPoint(0, 0); }

    /// <summary>
    /// 当前坐标原点相对于绝对左上角0,0点偏移的量。
    /// </summary>
    /// <returns></returns>
    QPoint originOffset() const;

    /// <summary>
    /// 当前激光点在设备坐标系下的坐标值。
    /// </summary>
    /// <returns></returns>
    QPoint laserPosition() const;

    const DeviceState& deviceState() const;

    /// <summary>
    /// 用户原点在设备坐标系下的坐标值。注意，直接从机器读取的激光
    /// 坐标即该值。
    /// </summary>
    /// <returns></returns>
    QVector4D userOrigin() const;

    /// <summary>
    /// 加工幅面矩形，以设备坐标系下的坐标值表示。
    /// </summary>
    /// <returns></returns>
    QRect layoutRect() const;

    QSize layoutSize() const;

    /// <summary>
    /// 在绝对坐标下返回加工幅面四角原点。
    /// 在用户原点下返回用户原点。在当前位置下反回激
    /// 光点的当前位置。
    /// </summary>
    /// <returns></returns>
    QPoint currentOrigin() const;

    int currentZ() const;

    bool isAbsolute() const;

    LaserRegister::RegistersMap externalRegisterValues(bool onlyModified = false) const;
    LaserRegister::RegistersMap userRegisterValues(bool onlyModified = false) const;
    LaserRegister::RegistersMap systemRegisterValues(bool onlyModified = false) const;
    QMap<int, LaserRegister*> externalRegisters(bool onlyModified = false) const;
    QMap<int, LaserRegister*> userRegisters(bool onlyModified = false) const;
    QMap<int, LaserRegister*> systemRegisters(bool onlyModified = false) const;

    int engravingAccLength(int engravingRunSpeed) const;

    void debugPrintUserRegisters() const;
    void debugPrintSystemRegisters() const;
    void debugPrintRegisters() const;

    QPoint mapFromQuadToCurrent(const QPoint& pt, const QPoint& topLeftFrom = QPoint(0, 0));
    QPoint mapFromCurrentToQuad(const QPoint& pt, const QPoint& topLeftTo = QPoint(0, 0));
    QTransform to1stQuad();

public slots:
    void load();
    void unload();
    void connectDevice(const QString& portName);
    void disconnectDevice();
    QString activateMainCard(
        const QString& email,
        const QString& code,
        const QString& name,
        const QString& phone,
        const QString& address,
        const QString& qq,
        const QString& wx,
        const QString& country,
        const QString& distributor,
        const QString& brand,
        const QString& model
    );
    bool requestTemporaryLicense();
    bool createLicenseFile(const QString& licenseCode);
    void updateDeviceOriginAndTransform();

protected:
    LaserRegister::RegistersMap registerValues(const QMap<int, LaserRegister*>& registers, bool onlyModified = false) const;
    QMap<int, LaserRegister*> getRegisters(const QMap<int, LaserRegister*>& registers, bool onlyModified = false) const;
    void parseSystemRegisters(const QString& raw);
    void parseUserRegisters(const QString& raw);
    void parseExternalRegisters(const QString& raw);
    void batchParse(const QString& raw, const QMap<int, LaserRegister*>& registers);

protected slots:
    void handleProgress(int position, int total, float progress);
    void handleError(int code, const QString& message);
    void handleMessage(int code, const QString& message);

    void onConnected();
    void onMainCardRegistrationChanged(bool registered);
    void onMainCardActivationChanged(bool activated);

    void onConfigStartFromChanged(const QVariant& value, void* senderPtr);
    void onConfigJobOriginChanged(const QVariant& value, void* senderPtr);
    void onDeviceOriginChanged(const QVariant& value, void* senderPtr);

    void onLayerWidthChanged(const QVariant& value, void* senderPtr);
    void onLayerHeightChanged(const QVariant& value, void* senderPtr);

signals:
    void comPortsFetched(const QStringList& ports);
    void comPortConnected(const QString& portName);
    void connected();
    void disconnected();
    void machiningStarted();
    void machiningPaused();
    void continueWorking();
    void machiningStopped();
    void machiningFinished();
    void idle();
    void downloadFinished();
    void mainCardRegistrationChanged(bool registered);
    void mainCardActivationChanged(bool activated);
    void mainCardInfoFetched();
    void manufacturePasswordVerified(bool pass);
    void manufacturePasswordChangeFailed();
    void manufacturePasswordChangeOk();
    void workStateUpdated(DeviceState state);
    void layoutChanged(const QSize& size);
    void systemRegistersConfirmed();
    void userRegistersConfirmed();
    void activeFailed(int reason);
    void dongleConnected();
    void dongleDisconnected();
    void dongleRemoved();

private:
    QScopedPointer<LaserDevicePrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, LaserDevice)
    Q_DISABLE_COPY(LaserDevice)

    friend class LaserDriver;
};

#endif // LASERDEVICE_H