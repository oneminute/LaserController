#ifndef LASERDEVICE_H
#define LASERDEVICE_H

#include "common/common.h"
#include "LaserRegister.h"
#include "laser/LaserDefines.h"
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
    qreal layoutWidthMachining() const;
    qreal layoutHeightMachining() const;

    int printerDrawUnit() const;
    void setPrinterDrawUnit(int unit, bool toCard = true);

    QString requestHardwareId() const;
    QString requestMainCardId() const;
    QString requestRegisteId() const;
    QString requestDongleId() const;
    void updateWorkState();

    void requestMainCardInfo();
    void requestMainCardRegInfo();
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

    bool verifyManufacturePassword(const QString& password);

    MainCardActivateResult autoActivateMainCard();
    bool sendAuthenticationEmail(const QString& email);
    bool registeMainCard(const QString& registeCode, QWidget* parentWidget = nullptr);

    bool writeUserRegisters();
    bool writeSystemRegisters(const QString& password);
    bool readUserRegisters();
    bool readSystemRegisters();
    bool readHostRegisters();

    bool readUserRegister(int address);
    bool writeUserReigister(int address, const QVariant& value);
    bool readSystemRegister(int address);
    bool writeSystemReigister(int address, const QVariant& value);

    void showLibraryVersion();

    void checkVersionUpdate(bool hardware, const QString& flag, int currentVersion, const QString& versionNoteToJsonFile);

    void moveToMachining(const QVector3D& pos, bool xEnabled = true, bool yEnabled = true, bool zEnabled = true);
    void moveBy(const QVector3D& pos, bool xEnabled = true, bool yEnabled = true, bool zEnabled = true);

    bool isAvailable() const;

    void showAboutWindow(int interval = 0, bool modal = true);
    void closeAboutWindow();
    int showUpdateDialog();
    void showSoftwareUpdateWizard();
    void showFirmwareUpdateWizard();
    void updateDriverLanguage();

    bool checkLayoutForMoving(const QPointF& dest);
    bool checkLayoutForMachining(const QRectF& docBounding, const QRectF& docBoundingAcc);

    LaserRegister* userRegister(int addr) const;
    LaserRegister* systemRegister(int addr) const;

    /// <summary>
    /// 在机械坐标系下原点的绝对坐标值。左上角恒为(0, 0)
    /// </summary>
    /// <returns></returns>
    QPointF absoluteOriginInMech() const;

    /// <summary>
    /// 在画布场景坐标系下的原点坐标值。一般以像素为单位。
    /// </summary>
    /// <returns></returns>
    QPointF originInScene() const;

    /// <summary>
    /// 在设备坐标系下原点的坐标值。该值恒为(0, 0)。
    /// </summary>
    /// <returns></returns>
    QPointF originInDevice() const { return QPointF(0, 0); }

    /// <summary>
    /// 在机械坐标系下，向设备坐标系变换的平移变换矩阵。
    /// </summary>
    /// <returns></returns>
    QTransform transformToDevice() const;

    QTransform transformToMech() const;

    QTransform transformDeviceToScene() const;

    QTransform transformSceneToDevice() const;

    /// <summary>
    /// 当前激光点在机械坐标系下的绝对坐标值。
    /// </summary>
    /// <returns></returns>
    QPointF laserPositionInMech() const;

    /// <summary>
    /// 当前激光点在设备坐标系下的坐标值。
    /// </summary>
    /// <returns></returns>
    QPointF laserPositionInDevice() const;

    /// <summary>
    /// 当前激光点在画面场中的坐标值。
    /// </summary>
    /// <returns></returns>
    QPointF laserPositionInScene() const;

    /// <summary>
    /// 用户原点在机械坐标系下的绝对坐标值。
    /// </summary>
    /// <returns></returns>
    QPointF userOriginInMech() const;

    /// <summary>
    /// 用户原点在设备坐标系下的坐标值。注意，直接从机器读取的激光
    /// 坐标即该值。
    /// </summary>
    /// <returns></returns>
    QPointF userOriginInDevice() const;

    /// <summary>
    /// 用户原点在画布坐标系下的坐标值。
    /// </summary>
    /// <returns></returns>
    QPointF userOriginInScene() const;

    /// <summary>
    /// 加工幅面矩形，以机械坐标系下的绝对坐标值表示。
    /// </summary>
    /// <returns></returns>
    QRectF layoutRectInMech() const;

    /// <summary>
    /// 加工幅面矩形，以设备坐标系下的坐标值表示。
    /// </summary>
    /// <returns></returns>
    QRectF layoutRectInDevice() const;

    /// <summary>
    /// 加工幅面矩形，以画布坐标系下的坐标值表示。
    /// </summary>
    /// <returns></returns>
    QRectF layoutRectInScene() const;

    /// <summary>
    /// 在绝对坐标下返回加工幅面四角原点。
    /// 在用户原点下返回用户原点。在当前位置下反回激
    /// 光点的当前位置。
    /// </summary>
    /// <returns></returns>
    QPointF currentOriginInScene() const;
    QPointF currentOriginInMech() const;
    QPointF currentOriginInDevice() const;

    void batchParse(const QString& raw, bool isSystem, bool isConfirmed);

    LaserRegister::RegistersMap userRegisterValues(bool onlyModified = false) const;
    LaserRegister::RegistersMap systemRegisterValues(bool onlyModified = false) const;
    QMap<int, LaserRegister*> userRegisters(bool onlyModified = false) const;
    QMap<int, LaserRegister*> systemRegisters(bool onlyModified = false) const;

    qreal engravingAccLength(qreal engravingRunSpeed) const;

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
    void moveToOrigin(qreal speed = 15);
    void updateDeviceOriginAndTransform();

protected:

protected slots:
    void handleError(int code, const QString& message);
    void handleMessage(int code, const QString& message);

    void onLibraryLoaded(bool success);
    void onLibraryInitialized();
    void onComPortsFetched(const QStringList& portNames);
    void onConnected();
    void onMainCardRegistrationChanged(bool registered);
    void onMainCardActivationChanged(bool activated);

    void onConfigStartFromChanged(const QVariant& value, void* senderPtr);
    void onConfigJobOriginChanged(const QVariant& value, void* senderPtr);

    void onLayerWidthChanged(const QVariant& value);
    void onLayerHeightChanged(const QVariant& value);

signals:
    void comPortsFetched(const QStringList& ports);
    void comPortConnected(const QString& portName);
    void connected();
    void disconnected();
    void mainCardRegistrationChanged(bool registered);
    void mainCardActivationChanged(bool activated);
    void mainCardInfoFetched();
    void manufacturePasswordVerified(bool pass);
    void workStateUpdated(DeviceState state);
    void layoutChanged(const QSizeF& size);
    void systemRegistersConfirmed();
    void userRegistersConfirmed();

private:
    QScopedPointer<LaserDevicePrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, LaserDevice)
    Q_DISABLE_COPY(LaserDevice)

    friend class LaserDriver;
};

#endif // LASERDEVICE_H