#include "LaserDevice.h"

#include <QDate>
#include <QMessageBox>
#include <QInputDialog>

#include "LaserApplication.h"
#include "LaserDriver.h"
#include "LaserRegister.h"
#include "scene/LaserDocument.h"
#include "common/common.h"
#include "common/Config.h"
#include "exception/LaserException.h"
#include "state/StateController.h"
#include "task/ProgressItem.h"
#include "ui/LaserControllerWindow.h"
#include "util/Utils.h"

class LaserDevicePrivate
{
    Q_DECLARE_PUBLIC(LaserDevice)
public:
    LaserDevicePrivate(LaserDevice* ptr)
        : q_ptr(ptr)
        , driver(nullptr)
        , isInit(false)
        , connected(false)
        , name("unknown")
        , portName("")
        , printerDrawUnit(1016)
        , absoluteOrigin(0, 0)
        , layoutRect()
        , transformToDevice()
        , mainCardActivated(false)
        , mainCardRegistered(false)
        , progress(nullptr)
    {}

    ~LaserDevicePrivate()
    {
        delete progress;
    }

    void updateDeviceOriginAndTransform();

    LaserDevice* q_ptr;
    LaserDriver* driver;
    bool isInit;
    bool connected;

    QString name;
    QString portName;
    int printerDrawUnit;    // 绘图仪单位，这里值的意思是一英寸分为多少个单位

    QPoint absoluteOrigin;
    QRect layoutRect;
    QTransform transformToDevice;

    QString mainCard;
    QString mainCardRegisteredDate;
    QString mainCardActivatedDate;
    QString boundDongle;
    QString boundDongleRegisteredDate;
    QString boundDongleActivatedDate;
    QString boundDongleBindingTimes;
    QString dongle;
    QString dongleRegisteredDate;
    QString dongleActivatedDate;
    QString dongleBindingTimes;
    QString hardwareRegisteredDate;
    QString hardwareActivatedDate;
    QString hardwareMaintainingTimes;

    QMap<int, LaserRegister*> externalRegisters;
    QMap<int, LaserRegister*> userRegisters;
    QMap<int, LaserRegister*> systemRegisters;

    bool mainCardActivated;
    bool mainCardRegistered;
    DeviceState lastState;
    QString password;
    ProgressItem* progress;
};

void LaserDevicePrivate::updateDeviceOriginAndTransform()
{
    int dx = 0;
    int dy = 0;
    int maxX = Config::SystemRegister::xMaxLength();
    int maxY = Config::SystemRegister::yMaxLength();
    switch (Config::SystemRegister::deviceOrigin())
    {
    case 0:
        dx = 0;
        dy = 0;
        layoutRect = QRect(QPoint(0, 0), QSize(maxX, maxY));
        break;
    case 3:
        dx = maxX;
        dy = 0;
        layoutRect = QRect(QPoint(-maxX, 0), QSize(maxX, maxY));
        break;
    case 2:
        dx = maxX;
        dy = maxY;
        layoutRect = QRect(QPoint(-maxX, -maxY), QSize(maxX, maxY));
        break;
    case 1:
        dx = 0;
        dy = maxY;
        layoutRect = QRect(QPoint(0, -maxY), QSize(maxX, maxY));
        break;
    }

    absoluteOrigin = QPoint(dx, dy);
    transformToDevice = QTransform::fromTranslate(-absoluteOrigin.x(), -absoluteOrigin.y());
}

LaserDevice::LaserDevice(LaserDriver* driver, QObject* parent)
    : QObject(parent)
    , m_ptr(new LaserDevicePrivate(this))
{
    Q_D(LaserDevice);
    d->driver = driver;

    ADD_TRANSITION(deviceUnconnectedState, deviceConnectedState, this, &LaserDevice::connected);
    ADD_TRANSITION(deviceConnectedState, deviceUnconnectedState, this, &LaserDevice::disconnected);
    ADD_TRANSITION(deviceIdleState, deviceMachiningState, this, &LaserDevice::machiningStarted);
    ADD_TRANSITION(deviceIdleState, deviceDownloadingState, this, &LaserDevice::downloadingToBoard);
    ADD_TRANSITION(deviceMachiningState, devicePausedState, this, &LaserDevice::machiningPaused);
    ADD_TRANSITION(devicePausedState, deviceMachiningState, this, &LaserDevice::continueWorking);
    ADD_TRANSITION(devicePausedState, deviceIdleState, this, &LaserDevice::machiningStopped);
    ADD_TRANSITION(deviceMachiningState, deviceIdleState, this, &LaserDevice::machiningStopped);
    ADD_TRANSITION(deviceMachiningState, deviceIdleState, this, &LaserDevice::machiningFinished);
    ADD_TRANSITION(deviceDownloadingState, deviceIdleState, this, &LaserDevice::downloadFinished);

    d->externalRegisters.insert(11, new LaserRegister(11, Config::ExternalRegister::x1Item(), false));
    d->externalRegisters.insert(12, new LaserRegister(12, Config::ExternalRegister::y1Item(), false));
    d->externalRegisters.insert(13, new LaserRegister(13, Config::ExternalRegister::z1Item(), false));
    d->externalRegisters.insert(14, new LaserRegister(14, Config::ExternalRegister::u1Item(), false));
    d->externalRegisters.insert(15, new LaserRegister(15, Config::ExternalRegister::x2Item(), false));
    d->externalRegisters.insert(16, new LaserRegister(16, Config::ExternalRegister::y2Item(), false));
    d->externalRegisters.insert(17, new LaserRegister(17, Config::ExternalRegister::z2Item(), false));
    d->externalRegisters.insert(18, new LaserRegister(18, Config::ExternalRegister::u2Item(), false));
    d->externalRegisters.insert(19, new LaserRegister(19, Config::ExternalRegister::x3Item(), false));
    d->externalRegisters.insert(20, new LaserRegister(20, Config::ExternalRegister::y3Item(), false));
    d->externalRegisters.insert(21, new LaserRegister(21, Config::ExternalRegister::z3Item(), false));
    d->externalRegisters.insert(22, new LaserRegister(22, Config::ExternalRegister::u3Item(), false));

    d->userRegisters.insert(0, new LaserRegister(0, Config::UserRegister::headItem(), false));
    d->userRegisters.insert(1, new LaserRegister(1, Config::UserRegister::accModeItem(), false));
    d->userRegisters.insert(2, new LaserRegister(2, Config::UserRegister::cuttingMoveSpeedItem(), false));
    d->userRegisters.insert(3, new LaserRegister(3, Config::UserRegister::cuttingMoveAccItem(), false));
    d->userRegisters.insert(4, new LaserRegister(4, Config::UserRegister::cuttingTurnSpeedItem(), false));
    d->userRegisters.insert(5, new LaserRegister(5, Config::UserRegister::cuttingTurnAccItem(), false));
    d->userRegisters.insert(6, new LaserRegister(6, Config::UserRegister::cuttingWorkAccItem(), false));
    d->userRegisters.insert(7, new LaserRegister(7, Config::UserRegister::cuttingMoveSpeedFactorItem(), false));
    d->userRegisters.insert(8, new LaserRegister(8, Config::UserRegister::cuttingWorkSpeedFactorItem(), false));
    d->userRegisters.insert(9, new LaserRegister(9, Config::UserRegister::cuttingSpotSizeItem(), false));
    d->userRegisters.insert(10, new LaserRegister(10, Config::UserRegister::scanXStartSpeedItem(), false));
    d->userRegisters.insert(11, new LaserRegister(11, Config::UserRegister::scanYStartSpeedItem(), false));
    d->userRegisters.insert(12, new LaserRegister(12, Config::UserRegister::scanXAccItem(), false));
    d->userRegisters.insert(13, new LaserRegister(13, Config::UserRegister::scanYAccItem(), false));
    d->userRegisters.insert(14, new LaserRegister(14, Config::UserRegister::scanRowSpeedItem(), false));
    d->userRegisters.insert(15, new LaserRegister(15, Config::UserRegister::scanLaserFrequencyItem(), false));
    d->userRegisters.insert(16, new LaserRegister(16, Config::UserRegister::scanReturnErrorItem(), false));
    d->userRegisters.insert(17, new LaserRegister(17, Config::UserRegister::scanLaserPowerItem(), false));
    d->userRegisters.insert(18, new LaserRegister(18, Config::UserRegister::scanXResetEnabledItem(), false));
    d->userRegisters.insert(19, new LaserRegister(19, Config::UserRegister::scanYResetEnabledItem(), false));
    d->userRegisters.insert(20, new LaserRegister(20, Config::UserRegister::scanZResetEnabledItem(), false));
    d->userRegisters.insert(21, new LaserRegister(21, Config::UserRegister::resetSpeedItem(), false));
    d->userRegisters.insert(22, new LaserRegister(22, Config::UserRegister::scanReturnPosItem(), false));
    d->userRegisters.insert(23, new LaserRegister(23, Config::UserRegister::backlashXIntervalItem(), false));
    d->userRegisters.insert(24, new LaserRegister(24, Config::UserRegister::backlashYIntervalItem(), false));
    d->userRegisters.insert(25, new LaserRegister(25, Config::UserRegister::backlashZIntervalItem(), false));
    d->userRegisters.insert(26, new LaserRegister(26, Config::UserRegister::defaultRunSpeedItem(), false));
    d->userRegisters.insert(27, new LaserRegister(27, Config::UserRegister::defaultMaxCuttingPowerItem(), false));
    d->userRegisters.insert(28, new LaserRegister(28, Config::UserRegister::defaultMinCuttingPowerItem(), false));
    d->userRegisters.insert(29, new LaserRegister(29, Config::UserRegister::defaultScanSpeedItem(), false));
    d->userRegisters.insert(30, new LaserRegister(30, Config::UserRegister::maxScanGrayRatioItem(), false));
    d->userRegisters.insert(31, new LaserRegister(31, Config::UserRegister::minScanGrayRatioItem(), false));
    d->userRegisters.insert(32, new LaserRegister(32, Config::UserRegister::cuttingTurnOnDelayItem(), false));
    d->userRegisters.insert(33, new LaserRegister(33, Config::UserRegister::cuttingTurnOffDelayItem(), false));
    d->userRegisters.insert(34, new LaserRegister(34, Config::UserRegister::spotShotPowerItem(), false));
    d->userRegisters.insert(35, new LaserRegister(35, Config::UserRegister::fillingSpeedItem(), false));
    d->userRegisters.insert(36, new LaserRegister(36, Config::UserRegister::fillingStartSpeedItem(), false));
    d->userRegisters.insert(37, new LaserRegister(37, Config::UserRegister::fillingAccelerationItem(), false));
    d->userRegisters.insert(38, new LaserRegister(38, Config::UserRegister::maxFillingPowerItem(), false));
    d->userRegisters.insert(39, new LaserRegister(39, Config::UserRegister::minFillingPowerItem(), false));
    d->userRegisters.insert(40, new LaserRegister(40, Config::UserRegister::fillingAccRatioItem(), false));
    d->userRegisters.insert(41, new LaserRegister(41, Config::UserRegister::zSpeedItem(), false));
    d->userRegisters.insert(42, new LaserRegister(42, Config::UserRegister::materialThicknessItem(), false));
    d->userRegisters.insert(43, new LaserRegister(43, Config::UserRegister::movementStepLengthItem(), false));
    d->userRegisters.insert(44, new LaserRegister(44, Config::UserRegister::focalLengthItem(), false));

    d->systemRegisters.insert(0, new LaserRegister(0, Config::SystemRegister::headItem(), true));
    d->systemRegisters.insert(1, new LaserRegister(1, Config::SystemRegister::passwordItem(), true, false, true));
    d->systemRegisters.insert(2, new LaserRegister(2, Config::SystemRegister::storedPasswordItem(), true, false, true));
    d->systemRegisters.insert(3, new LaserRegister(3, Config::SystemRegister::hardwareID1Item(), true, true));
    d->systemRegisters.insert(4, new LaserRegister(4, Config::SystemRegister::hardwareID2Item(), true, true));
    d->systemRegisters.insert(5, new LaserRegister(5, Config::SystemRegister::hardwareID3Item(), true, true));
    d->systemRegisters.insert(6, new LaserRegister(6, Config::SystemRegister::cdKey1Item(), true));
    d->systemRegisters.insert(7, new LaserRegister(7, Config::SystemRegister::cdKey2Item(), true));
    d->systemRegisters.insert(8, new LaserRegister(8, Config::SystemRegister::cdKey3Item(), true));
    d->systemRegisters.insert(12, new LaserRegister(12, Config::SystemRegister::sysRunTimeItem(), true));
    d->systemRegisters.insert(13, new LaserRegister(13, Config::SystemRegister::laserRunTimeItem(), true));
    d->systemRegisters.insert(14, new LaserRegister(14, Config::SystemRegister::sysRunNumItem(), true));
    d->systemRegisters.insert(15, new LaserRegister(15, Config::SystemRegister::xMaxLengthItem(), true));
    d->systemRegisters.insert(16, new LaserRegister(16, Config::SystemRegister::xDirPhaseItem(), true));
    d->systemRegisters.insert(17, new LaserRegister(17, Config::SystemRegister::xLimitPhaseItem(), true));
    d->systemRegisters.insert(18, new LaserRegister(18, Config::SystemRegister::xZeroDevItem(), true));
    d->systemRegisters.insert(19, new LaserRegister(19, Config::SystemRegister::xStepLengthItem(), true));
    d->systemRegisters.insert(20, new LaserRegister(20, Config::SystemRegister::xLimitNumItem(), true));
    d->systemRegisters.insert(21, new LaserRegister(21, Config::SystemRegister::xResetEnabledItem(), true));
    d->systemRegisters.insert(22, new LaserRegister(22, Config::SystemRegister::xMotorNumItem(), true));
    d->systemRegisters.insert(23, new LaserRegister(23, Config::SystemRegister::xMotorCurrentItem(), true));
    d->systemRegisters.insert(24, new LaserRegister(24, Config::SystemRegister::xStartSpeedItem(), true));
    d->systemRegisters.insert(25, new LaserRegister(25, Config::SystemRegister::xMaxSpeedItem(), true));
    d->systemRegisters.insert(26, new LaserRegister(26, Config::SystemRegister::xMaxAccelerationItem(), true));
    d->systemRegisters.insert(27, new LaserRegister(27, Config::SystemRegister::xUrgentAccelerationItem(), true));
    d->systemRegisters.insert(28, new LaserRegister(28, Config::SystemRegister::yMaxLengthItem(), true));
    d->systemRegisters.insert(29, new LaserRegister(29, Config::SystemRegister::yDirPhaseItem(), true));
    d->systemRegisters.insert(30, new LaserRegister(30, Config::SystemRegister::yLimitPhaseItem(), true));
    d->systemRegisters.insert(31, new LaserRegister(31, Config::SystemRegister::yZeroDevItem(), true));
    d->systemRegisters.insert(32, new LaserRegister(32, Config::SystemRegister::yStepLengthItem(), true));
    d->systemRegisters.insert(33, new LaserRegister(33, Config::SystemRegister::yLimitNumItem(), true));
    d->systemRegisters.insert(34, new LaserRegister(34, Config::SystemRegister::yResetEnabledItem(), true));
    d->systemRegisters.insert(35, new LaserRegister(35, Config::SystemRegister::yMotorNumItem(), true));
    d->systemRegisters.insert(36, new LaserRegister(36, Config::SystemRegister::yMotorCurrentItem(), true));
    d->systemRegisters.insert(37, new LaserRegister(37, Config::SystemRegister::yStartSpeedItem(), true));
    d->systemRegisters.insert(38, new LaserRegister(38, Config::SystemRegister::yMaxSpeedItem(), true));
    d->systemRegisters.insert(39, new LaserRegister(39, Config::SystemRegister::yMaxAccelerationItem(), true));
    d->systemRegisters.insert(40, new LaserRegister(40, Config::SystemRegister::yUrgentAccelerationItem(), true));
    d->systemRegisters.insert(41, new LaserRegister(41, Config::SystemRegister::zMaxLengthItem(), true));
    d->systemRegisters.insert(42, new LaserRegister(42, Config::SystemRegister::zDirPhaseItem(), true));
    d->systemRegisters.insert(43, new LaserRegister(43, Config::SystemRegister::zLimitPhaseItem(), true));
    d->systemRegisters.insert(44, new LaserRegister(44, Config::SystemRegister::zZeroDevItem(), true));
    d->systemRegisters.insert(45, new LaserRegister(45, Config::SystemRegister::zStepLengthItem(), true));
    d->systemRegisters.insert(46, new LaserRegister(46, Config::SystemRegister::zLimitNumItem(), true));
    d->systemRegisters.insert(47, new LaserRegister(47, Config::SystemRegister::zResetEnabledItem(), true));
    d->systemRegisters.insert(48, new LaserRegister(48, Config::SystemRegister::zMotorNumItem(), true));
    d->systemRegisters.insert(49, new LaserRegister(49, Config::SystemRegister::zMotorCurrentItem(), true));
    d->systemRegisters.insert(50, new LaserRegister(50, Config::SystemRegister::zStartSpeedItem(), true));
    d->systemRegisters.insert(51, new LaserRegister(51, Config::SystemRegister::zMaxSpeedItem(), true));
    d->systemRegisters.insert(52, new LaserRegister(52, Config::SystemRegister::zMaxAccelerationItem(), true));
    d->systemRegisters.insert(53, new LaserRegister(53, Config::SystemRegister::zUrgentAccelerationItem(), true));
    d->systemRegisters.insert(54, new LaserRegister(54, Config::SystemRegister::laserMaxPowerItem(), true));
    d->systemRegisters.insert(55, new LaserRegister(55, Config::SystemRegister::laserMinPowerItem(), true));
    d->systemRegisters.insert(56, new LaserRegister(56, Config::SystemRegister::laserPowerFreqItem(), true));
    d->systemRegisters.insert(57, new LaserRegister(57, Config::SystemRegister::xPhaseEnabledItem(), true));
    d->systemRegisters.insert(58, new LaserRegister(58, Config::SystemRegister::yPhaseEnabledItem(), true));
    d->systemRegisters.insert(59, new LaserRegister(59, Config::SystemRegister::zPhaseEnabledItem(), true));
    d->systemRegisters.insert(60, new LaserRegister(60, Config::SystemRegister::deviceOriginItem(), true));
    d->systemRegisters.insert(61, new LaserRegister(61, Config::SystemRegister::zResetSpeedItem(), true));
    d->systemRegisters.insert(63, new LaserRegister(63, Config::SystemRegister::uDirPhaseItem(), true));
    d->systemRegisters.insert(66, new LaserRegister(66, Config::SystemRegister::uStepLengthItem(), true));
    d->systemRegisters.insert(69, new LaserRegister(69, Config::SystemRegister::uMotorNumItem(), true));
    d->systemRegisters.insert(70, new LaserRegister(70, Config::SystemRegister::uMotorCurrentItem(), true));
    d->systemRegisters.insert(71, new LaserRegister(71, Config::SystemRegister::uStartSpeedItem(), true));
    d->systemRegisters.insert(72, new LaserRegister(72, Config::SystemRegister::uMaxSpeedItem(), true));
    d->systemRegisters.insert(73, new LaserRegister(73, Config::SystemRegister::uMaxAccelerationItem(), true));
    d->systemRegisters.insert(74, new LaserRegister(74, Config::SystemRegister::uUrgentAccelerationItem(), true));
    d->systemRegisters.insert(75, new LaserRegister(75, Config::SystemRegister::uPhaseEnabledItem(), true));

    connect(Config::SystemRegister::xMaxLengthItem(), &ConfigItem::valueChanged, this, &LaserDevice::onLayerWidthChanged);
    connect(Config::SystemRegister::yMaxLengthItem(), &ConfigItem::valueChanged, this, &LaserDevice::onLayerHeightChanged);
    connect(this, &LaserDevice::connected, this, &LaserDevice::onConnected);
    connect(this, &LaserDevice::mainCardRegistrationChanged, this, &LaserDevice::onMainCardRegistrationChanged);
    connect(this, &LaserDevice::mainCardActivationChanged, this, &LaserDevice::onMainCardActivationChanged);
    connect(Config::SystemRegister::deviceOriginItem(),
        &ConfigItem::valueChanged, this,
        &LaserDevice::onConfigStartFromChanged);

    d->updateDeviceOriginAndTransform();
}

LaserDevice::~LaserDevice()
{
    qDebug() << "device destroyed";
}

bool LaserDevice::isInit() const
{
    Q_D(const LaserDevice);
    return d->isInit;
}

bool LaserDevice::isConnected() const
{
    Q_D(const LaserDevice);
    return d->connected;
}

QString LaserDevice::name() const
{
    Q_D(const LaserDevice);
    return d->name;
}

void LaserDevice::setName(const QString& name)
{
    Q_D(LaserDevice);
    d->name = name;
}

QString LaserDevice::portName() const
{
    Q_D(const LaserDevice);
    return d->portName;
}

void LaserDevice::load()
{
    Q_D(LaserDevice);

    connect(d->driver, &LaserDriver::progress, this, &LaserDevice::handleProgress/*, Qt::ConnectionType::QueuedConnection*/);
    connect(d->driver, &LaserDriver::raiseError, this, &LaserDevice::handleError/*, Qt::ConnectionType::QueuedConnection*/);
    connect(d->driver, &LaserDriver::sendMessage, this, &LaserDevice::handleMessage/*, Qt::ConnectionType::QueuedConnection*/);
    if (d->driver->load())
    {
        d->isInit = false;
        if (d->driver->init(LaserApplication::mainWindow->winId()))
        {
            qLogD << "LaserDevice::onLibraryInitialized";
            d->driver->setupCallbacks();
            d->isInit = true;
            d->driver->getPortList();
            updateDriverLanguage();
            QString dongleId = d->driver->getDongleId();
            if (!dongleId.isEmpty() && !dongleId.isNull())
                emit dongleConnected();
        }
    }
}

int LaserDevice::layoutWidth() const
{
    Q_D(const LaserDevice);
    return Config::SystemRegister::xMaxLength();
}

int LaserDevice::layoutHeight() const
{
    Q_D(const LaserDevice);
    return Config::SystemRegister::yMaxLength();
}

int LaserDevice::printerDrawUnit() const
{
    Q_D(const LaserDevice);
    return d->printerDrawUnit;
}

void LaserDevice::setPrinterDrawUnit(int unit, bool toCard)
{
    Q_D(LaserDevice);
    d->printerDrawUnit = unit;
}

QString LaserDevice::password() const
{
    Q_D(const LaserDevice);
    return d->password;
}

void LaserDevice::setPassword(const QString& value)
{
    Q_D(LaserDevice);
    d->password = value;
}

ProgressItem* LaserDevice::progress()
{
    Q_D(LaserDevice);
    return d->progress;
}

void LaserDevice::clearProgress()
{
    Q_D(LaserDevice);
    d->progress = nullptr;
}

void LaserDevice::resetProgress(ProgressItem* parent)
{
    Q_D(LaserDevice);
    d->progress = new ProgressItem(tr("Device"), ProgressItem::PT_Simple, parent);
}

QString LaserDevice::requestHardwareId() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        d->driver->getDeviceId(true);
        QString id = d->driver->getDeviceId(false);
        int count = 0;
        while (id.isEmpty())
        {
            QThread::sleep(0);
            id = d->driver->getDeviceId(false);
            count++;
        }
        qLogD << "get device id count: " << count;
        return id;
    }
    return "";
}

QString LaserDevice::requestMainCardId() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        QStringList segs = d->driver->getMainCardID().split(";");
        return segs[0];
    }
    return "";
}

QString LaserDevice::requestRegisteId() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        QStringList segs = d->driver->getMainCardID().split(";");
        if (segs.length() > 1)
            return segs[1];
        return "";
    }
    return "";
}

QString LaserDevice::requestDongleId() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        return d->driver->getDongleId();
    }
    return "";
}

void LaserDevice::updateWorkState()
{
    Q_D(LaserDevice);
    if (d->driver)
        d->driver->getDeviceWorkState();
}

void LaserDevice::changeState(const DeviceState& state)
{
    Q_D(LaserDevice);
    d->lastState = state;
    /*switch (state.workingMode)
    {
    case LWM_WORKING:
        break;
    case LWM_PAUSE:
        break;
    case LWM_STOP:
        break;
    }*/
    emit workStateUpdated(state);
}

void LaserDevice::requestMainCardInfo()
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        QString infoString = d->driver->getMainCardInfo();
        /*if (infoString.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Main card info incomplete."));
        }
        QStringList items = infoString.split(";");
        if (items.length() != 13)
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Main card info incomplete."));
        }
        QMap<QString, QString> info;
        info.insert("mainCard", items[0]);
        info.insert("mainCardRegisteredDate", items[1]);
        info.insert("mainCardActivatedDate", items[2]);
        info.insert("boundDongle", items[3]);
        info.insert("boundDongleRegisteredDate", items[4]);
        info.insert("boundDongleActivatedDate", items[5]);
        info.insert("boundDongleBindingTimes", items[6]);
        info.insert("dongle", items[7]);
        info.insert("dongleRegisteredDate", items[8]);
        info.insert("dongleActivatedDate", items[9]);
        info.insert("dongleBindingTimes", items[10]);
        info.insert("hardwareRegisteredDate", items[11]);
        info.insert("hardwareActivatedDate", items[12]);
        info.insert("hardwareMaintainingTimes", items[13]);
        emit mainCardInfoFetched(info);*/

    }
}

void LaserDevice::requestMainCardRegInfo()
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        d->driver->getMainCardRegisterState();
    }
}

QString LaserDevice::getMainCardModal()
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        return d->driver->getMainHardModal();
    }
    return "";
}

QString LaserDevice::firmwareVersion() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        return d->driver->firmwareVersion();
    }
    return "";
}

QString LaserDevice::hardwareIdentID() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        return d->driver->getHardwareIdentID();
    }
    return "";
}

QString LaserDevice::mainCardId() const
{
    Q_D(const LaserDevice);
    return d->mainCard;
}

QString LaserDevice::mainCardRegisteredDate() const
{
    Q_D(const LaserDevice);
    return d->mainCardRegisteredDate;
}

QString LaserDevice::mainCardActivatedDate() const
{
    Q_D(const LaserDevice);
    return d->mainCardActivatedDate;
}

QString LaserDevice::boundDongleId() const
{
    Q_D(const LaserDevice);
    return d->boundDongle;
}

QString LaserDevice::boundDongleRegisteredDate() const
{
    Q_D(const LaserDevice);
    return d->boundDongleRegisteredDate;
}

QString LaserDevice::boundDongleActivatedDate() const
{
    Q_D(const LaserDevice);
    return d->boundDongleActivatedDate;
}

QString LaserDevice::boundDongleBindingTimes() const
{
    Q_D(const LaserDevice);
    return d->boundDongleBindingTimes;
}

QString LaserDevice::dongleId() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        return d->driver->getDongleId();
    }
    return "";
}

QString LaserDevice::dongleRegisteredDate() const
{
    Q_D(const LaserDevice);
    return d->dongleRegisteredDate;
}

QString LaserDevice::dongleActivatedDate() const
{
    Q_D(const LaserDevice);
    return d->dongleActivatedDate;
}

QString LaserDevice::dongleBindingTimes() const
{
    Q_D(const LaserDevice);
    return d->dongleBindingTimes;
}

QString LaserDevice::hardwareRegisteredDate() const
{
    Q_D(const LaserDevice);
    return d->hardwareRegisteredDate;
}

QString LaserDevice::hardwareActivatedDate() const
{
    Q_D(const LaserDevice);
    return d->hardwareActivatedDate;
}

QString LaserDevice::hardwareMaintainingTimes() const
{
    Q_D(const LaserDevice);
    return d->hardwareMaintainingTimes;
}

bool LaserDevice::isMainCardActivated() const
{
    Q_D(const LaserDevice);
    return d->mainCardActivated;
}

bool LaserDevice::isMainCardRegistered() const
{
    Q_D(const LaserDevice);
    return d->mainCardRegistered;
}

bool LaserDevice::isDongleConnected()
{
    Q_D(LaserDevice);
    QString dongleId = d->driver->getDongleId();
    if (!dongleId.isEmpty() && !dongleId.isNull())
        return true;
    return false;
}

bool LaserDevice::availableForMachining()
{
    if (!isDongleConnected())
    {
        QMessageBox::warning(LaserApplication::mainWindow, tr("Alert"), tr("No available dongle connected."));
        return false;
    }
    int dongleType = getHardwareKeyType();
    if (dongleType < 4)
    {
        return true;
    }
    else if (dongleType == 4)
    {
        if (!isMainCardActivated())
        {
            QMessageBox::warning(LaserApplication::mainWindow, tr("Alert"), tr("Main card is not activated."));
            return false;
        }
        if (!isMainCardRegistered())
        {
            QMessageBox::warning(LaserApplication::mainWindow, tr("Alert"), tr("Main card is not registered."));
            return false;
        }
        return true;
    }
    QMessageBox::warning(LaserApplication::mainWindow, tr("Alert"), tr("No available dongle connected."));
    return false;
}

int LaserDevice::getHardwareKeyType(qint16 type)
{
    Q_D(LaserDevice);
    if (d->driver)
        return d->driver->getHardwareKeyType(type);
    return -1;
}

int LaserDevice::importData(const QByteArray& data)
{
    Q_D(LaserDevice);
    if (d->driver)
        return d->driver->importData(data.data(), data.length());
}

int LaserDevice::drawBounding(const QByteArray& data)
{
    Q_D(LaserDevice);
    if (d->driver)
        return d->driver->drawBounding(data.data(), data.length());
}

QString LaserDevice::apiLibVersion() const
{
    Q_D(const LaserDevice);
    if (d->driver)
        return d->driver->getVersion();
    return "";
}

QString LaserDevice::apiLibCompileInfo() const
{
    Q_D(const LaserDevice);
    if (d->driver)
        return d->driver->getCompileInfo();
    return "";
}

bool LaserDevice::verifyManufacturePassword(const QString& password, int errorCount)
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        return d->driver->checkFactoryPassword(password, errorCount);
    }
    errorCount = 0;
    return false;
}

bool LaserDevice::changeManufacturePassword(const QString& password, const QString& newPassword)
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        return d->driver->changeFactoryPassword(password, newPassword);
    }
    return false;
}

QString LaserDevice::showManufacturePasswordDialog(QWidget* parentWnd)
{
    Q_D(LaserDevice);
    parentWnd = parentWnd ? parentWnd : LaserApplication::mainWindow;
    QString password = QInputDialog::getText(parentWnd, tr("Manufacture Password"), tr("Password"));
    return password;
}

MainCardActivateResult LaserDevice::autoActivateMainCard()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        return d->driver->autoActiveMainCard();
    }
    return MAR_Other;
}

bool LaserDevice::sendAuthenticationEmail(const QString& email)
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        return d->driver->sendAuthenticationEmail(email);
    }
    return false;
}

bool LaserDevice::registerMainCard(const QString& registeCode, QWidget* parentWidget)
{
    Q_D(LaserDevice);
    QInputDialog dlg(parentWidget);
    dlg.setWindowTitle(tr("Password"));
    dlg.setLabelText(tr("Please input vendor password."));
    dlg.setOkButtonText(tr("Ok"));
    dlg.setCancelButtonText(tr("Cancel"));
    if (dlg.exec() == QDialog::Rejected)
        return false;
    QString password = dlg.textValue();
    QString retCode = d->driver->registerMainCard(registeCode, password);
    if (retCode == registeCode)
    {
        QMessageBox::information(parentWidget, tr("Registration success"), tr("Registration success!"), QMessageBox::StandardButton::Ok);
        return true;
    }
    QMessageBox::warning(parentWidget, tr("Registration failure"), tr("Registration failure!"), QMessageBox::StandardButton::Ok);
    return false;
}

bool LaserDevice::writeExternalRegisters(bool onlyModified)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->writeExternalParamToCard(externalRegisterValues(onlyModified));
}

bool LaserDevice::writeUserRegisters(bool onlyModified)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->writeUserParamToCard(userRegisterValues(onlyModified));
}

bool LaserDevice::writeSystemRegisters(const QString& password, bool onlyModified)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    LaserRegister::RegistersMap registers = systemRegisterValues(onlyModified);
    registers.insert(1, password);
    return d->driver->writeSysParamToCard(registers);
}

bool LaserDevice::readUserRegisters()
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->readAllUserParamFromCard();
}

bool LaserDevice::readSystemRegisters()
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->readAllSysParamFromCard();
}

bool LaserDevice::readHostRegisters()
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->readAllExternalParamFromCard();
}

bool LaserDevice::readUserRegister(int address)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    QList<int> addresses;
    addresses << address;
    return d->driver->readUserParamFromCard(addresses);
}

bool LaserDevice::writeUserReigister(int address, const QVariant& value)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    LaserRegister::RegistersMap pair;
    pair.insert(address, value);
    return d->driver->writeUserParamToCard(pair);
}

bool LaserDevice::readSystemRegister(int address)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    QList<int> addresses;
    addresses << address;
    return d->driver->readSysParamFromCard(addresses);
}

bool LaserDevice::writeSystemReigister(int address, const QVariant& value)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    LaserRegister::RegistersMap pair;
    pair.insert(address, value);
    return d->driver->writeSysParamToCard(pair);
}

void LaserDevice::showLibraryVersion()
{
    Q_D(LaserDevice);
    //QString compileInfo = d->driver->getCompileInfo();
    //qLogD << "compile info: " << compileInfo;
    QString laserLibraryInfo = d->driver->getLaserLibraryInfo();
    qLogD << "laser library info: " << laserLibraryInfo;
    QString mainCardId = d->driver->getMainCardID();
    qLogD << "main card id: " << mainCardId;
    d->driver->showAboutWindow();
}

void LaserDevice::checkVersionUpdate(bool hardware, const QString& flag, int currentVersion, const QString& versionNoteToJsonFile)
{
    Q_D(LaserDevice);
    QString systemDate(__DATE__);
    qLogD << "system date: " << systemDate;
    QDate compileDate = QLocale("en_US").toDate(systemDate.simplified(), "MMM d yyyy");
    int year = compileDate.year() % 100;
    int month = compileDate.month();
    int day = compileDate.day();
    int version = year * 10000 + month * 100 + day;
    d->driver->checkVersionUpdate(hardware, flag, 0, versionNoteToJsonFile);
}

void LaserDevice::moveTo(const QVector4D& pos, bool xEnabled, bool yEnabled, bool zEnabled, bool uEnabled)
{
    if (!checkLayoutForMoving(pos.toPoint()))
        return;

    //QVector3D dest = utils::limitToLayout(pos, Config::SystemRegister::deviceOrigin(), layoutWidth, layoutHeight);
    QVector4D dest = pos;
    int xPos = qRound(dest.x());
    int yPos = qRound(dest.y());
    int zPos = qRound(dest.z());
    int uPos = qRound(dest.w());
    LaserApplication::driver->lPenQuickMoveTo(
        xEnabled, true, xPos,
        yEnabled, true, yPos,
        zEnabled, true, zPos,
        uEnabled, true, uPos
    );
}

void LaserDevice::moveBy(const QVector4D& pos, bool xEnabled, bool yEnabled, bool zEnabled, bool uEnabled)
{
    //QVector3D dest = utils::limitToLayout(pos, quad, layoutWidth, layoutHeight);
    int xPos = qRound(pos.x());
    int yPos = qRound(pos.y());
    int zPos = qRound(pos.z());
    int uPos = qRound(pos.w());
    // 相对移动要根据传入的pos判断哪个轴enabled
    xEnabled = xEnabled && !qFuzzyIsNull(pos.x());
    yEnabled = yEnabled && !qFuzzyIsNull(pos.y());
    zEnabled = zEnabled && !qFuzzyIsNull(pos.z());
    uEnabled = uEnabled && !qFuzzyIsNull(pos.w());
    LaserApplication::driver->checkMoveLaserMotors(
        Config::Ui::autoRepeatDelay(),
        xEnabled, false, xPos,
        yEnabled, false, yPos,
        zEnabled, false, zPos,
        uEnabled, false, uPos
    );
}

void LaserDevice::moveToZOrigin()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        int target = Config::UserRegister::focalLength() - Config::Device::calibrationBlockThickness();
        target = qBound(0, target, Config::UserRegister::focalLength());
        d->driver->lPenMoveToOriginalPointZ(target);
    }
}

void LaserDevice::moveToUOrigin()
{
    Q_D(LaserDevice);
    if (!d->driver)
        return;

    LaserApplication::device->laserPosition();
}

void LaserDevice::moveToXYOrigin()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->lPenMoveToOriginalPoint(0);
    }
}

bool LaserDevice::isAvailable() const
{
    Q_D(const LaserDevice);
    return d->driver && d->isInit;
}

void LaserDevice::showAboutWindow(int interval, bool modal)
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->showAboutWindow(interval, modal);
    }
}

void LaserDevice::closeAboutWindow()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->closeAboutWindow();
    }
}

int LaserDevice::showUpdateDialog()
{
    Q_D(LaserDevice);
    QString systemDate(__DATE__);
    qLogD << "system date: " << systemDate;
    QDate compileDate = QLocale("en_US").toDate(systemDate.simplified(), "MMM d yyyy");
    int year = compileDate.year() % 100;
    int month = compileDate.month();
    int day = compileDate.day();
    int version = year * 10000 + month * 100 + day;
    //return d->driver->getUpdatePanelHandle(version, LaserApplication::mainWindow->winId());
    //LaserApplication::mainWindow->createUpdateDockPanel(winId);
    return 0;
}

void LaserDevice::showSoftwareUpdateWizard()
{
    Q_D(LaserDevice);
    if (d->driver)
        d->driver->startSoftUpdateWizard();
}

void LaserDevice::showFirmwareUpdateWizard()
{
    Q_D(LaserDevice);
    if (d->driver)
        d->driver->startFirmwareUpdateWizard();
}

void LaserDevice::updateDriverLanguage()
{
    Q_D(LaserDevice);
    if (d->driver)
        d->driver->setLanguage(Config::General::language() == QLocale::Chinese ? 1 : 0);
}

bool LaserDevice::checkLayoutForMoving(const QPoint& dest)
{
    QRectF bounding = LaserApplication::device->layoutRect();
    if (bounding.contains(dest))
        return true;
    else
    {
        QMessageBox::warning(LaserApplication::mainWindow, tr("Exceed layout"), 
            tr("The target point exceeds the device layout! Please check your target point."));
        return false;
    }
}

bool LaserDevice::checkLayoutForMachining(const QRect& docBounding, const QRect& engravingBounding)
{
    QRect layoutRect = this->layoutRect();
    QString info1 = "";
    QString info2 = "";

    bool ok1 = true;
    bool ok2 = true;
    bool needCheckAcc = docBounding != engravingBounding;
    bool exceedLeft = false;
    bool exceedRight = false;

    int docLeft = docBounding.left();
    int docRight = docLeft + docBounding.width();
    int docTop = docBounding.top();
    int docBottom = docTop + docBounding.height();

    int layoutLeft = layoutRect.left();
    int layoutRight = layoutLeft + layoutRect.width();
    int layoutTop = layoutRect.top();
    int layoutBottom = layoutTop + layoutRect.height();

    if (docLeft < layoutLeft)
    {
        ok1 = false;
        info1.append(tr("Left edge of current document bounding exceeds device layout: %1mm\n")
            .arg((layoutLeft - docLeft) * 0.001, 0, 'f', 3));
        exceedLeft = true;
    }
    if (docTop < layoutTop && !Config::Device::switchToU())
    {
        ok1 = false;
        info1.append(tr("Top edge of current document bounding exceeds device layout: %1mm\n")
            .arg((layoutTop - docTop) * 0.001, 0, 'f', 3));
    }
    if (docRight > layoutRight)
    {
        ok1 = false;
        info1.append(tr("Right edge of current document bounding exceeds device layout: %1mm\n")
            .arg((docRight - layoutRight) * 0.001, 0, 'f', 3));
        exceedRight = true;
    }
    if (docBottom > layoutBottom && !Config::Device::switchToU())
    {
        ok1 = false;
        info1.append(tr("Bottom edge of current document bounding exceeds device layout: %1mm\n")
            .arg((docBottom - layoutBottom) * 0.001, 0, 'f', 3));
    }

    if (ok1)
        info1 = tr("Document bounding is OK.\n");

    if (needCheckAcc)
    {
        QRect boundingAcc = docBounding.united(engravingBounding);
        int accLeft = boundingAcc.left();
        int accRight = accLeft + boundingAcc.width();

        if (accLeft < layoutLeft)
        {
            ok2 = false;
            info2.append(tr("Left edge of current document bounding with acc interval exceeds device layout: %1mm\n")
                .arg((layoutLeft - accLeft) * 0.001, 0, 'f', 3));
        }
        if (accRight > layoutRight)
        {
            ok2 = false;
            info2.append(tr("Right edge of current document bounding with acc interval exceeds device layout: %1mm\n")
                .arg((accRight - layoutRight) * 0.001, 0, 'f', 3));
        }

        if (ok2)
            info2 = tr("Document bounding with acc interval is OK.\n");
    }

    QString info;
    if (ok1 && !ok2)
    {
        info = "";
        info.append(info1);
        info.append(info2);
    }
    else
    {
        if (!ok1)
        {
            info.append(info1);
        }
        if (!ok2)
        {
            info.append(info2);
        }
    }

    info = info.trimmed();
    if (!ok1 || !ok2)
        QMessageBox::warning(LaserApplication::mainWindow, tr("Exceed layout"), info);

    return ok1 && ok2;
}

LaserRegister* LaserDevice::userRegister(int addr) const
{
    Q_D(const LaserDevice);
    if (d->userRegisters.contains(addr))
    {
        return d->userRegisters[addr];
    }
    else
        return nullptr;
}

LaserRegister* LaserDevice::systemRegister(int addr) const
{
    Q_D(const LaserDevice);
    if (d->systemRegisters.contains(addr))
    {
        return d->systemRegisters[addr];
    }
    else
        return nullptr;
}

QPoint LaserDevice::originOffset() const
{
    Q_D(const LaserDevice);
    return d->absoluteOrigin;
}

QPoint LaserDevice::laserPosition() const
{
    Q_D(const LaserDevice);
    return d->lastState.pos.toPoint();
}

const DeviceState& LaserDevice::deviceState() const
{
    Q_D(const LaserDevice);
    return d->lastState;
}

QVector4D LaserDevice::userOrigin() const
{
    QVector4D origin;
    switch (Config::Device::userOriginSelected())
    {
    case 0:
        origin = QVector4D(
            Config::ExternalRegister::x1(),
            Config::ExternalRegister::y1(),
            Config::ExternalRegister::z1(),
            Config::ExternalRegister::u1()
        );
        break;
    case 1:
        origin = QVector4D(
            Config::ExternalRegister::x2(),
            Config::ExternalRegister::y2(),
            Config::ExternalRegister::z2(),
            Config::ExternalRegister::u2()
        );
        break;
    case 2:
        origin = QVector4D(
            Config::ExternalRegister::x3(),
            Config::ExternalRegister::y3(),
            Config::ExternalRegister::z3(),
            Config::ExternalRegister::u3()
        );
        break;
    }
    return origin;
}

QRect LaserDevice::layoutRect() const
{
    Q_D(const LaserDevice);
    return d->layoutRect;
}

QSize LaserDevice::layoutSize() const
{
    return QSize(Config::SystemRegister::xMaxLength(), Config::SystemRegister::yMaxLength());
}

QPoint LaserDevice::currentOrigin() const
{
    QPoint origin;
    switch (Config::Device::startFrom())
    {
    case SFT_AbsoluteCoords:
        origin = this->origin();
        break;
    case SFT_UserOrigin:
        origin = userOrigin().toPoint();
        break;
    case SFT_CurrentPosition:
        origin = laserPosition();
        break;
    }
    return origin;
}

int LaserDevice::currentZ() const
{
    Q_D(const LaserDevice);
    return d->lastState.pos.z();
}

bool LaserDevice::isAbsolute() const
{
    if (Config::Device::startFrom() == SFT_AbsoluteCoords ||
        StateControllerInst.isInState(StateControllerInst.documentPrintAndCutAligningState()))
        return true;
    return false;
}

LaserRegister::RegistersMap LaserDevice::externalRegisterValues(bool onlyModified) const
{
    Q_D(const LaserDevice);
    return registerValues(d->externalRegisters, onlyModified);
}

LaserRegister::RegistersMap LaserDevice::userRegisterValues(bool onlyModified) const
{
    Q_D(const LaserDevice);
    return registerValues(d->userRegisters, onlyModified);
}

LaserRegister::RegistersMap LaserDevice::systemRegisterValues(bool onlyModified) const
{
    Q_D(const LaserDevice);
    return registerValues(d->systemRegisters, onlyModified);
}

QMap<int, LaserRegister*> LaserDevice::externalRegisters(bool onlyModified) const
{
    Q_D(const LaserDevice);
    return getRegisters(d->externalRegisters, onlyModified);
}

QMap<int, LaserRegister*> LaserDevice::userRegisters(bool onlyModified) const
{
    Q_D(const LaserDevice);
    return getRegisters(d->userRegisters, onlyModified);
}

QMap<int, LaserRegister*> LaserDevice::systemRegisters(bool onlyModified) const
{
    Q_D(const LaserDevice);
    return getRegisters(d->systemRegisters, onlyModified);
}

int LaserDevice::engravingAccLength(int engravingRunSpeed) const
{
    qreal minSpeed = Config::UserRegister::scanXStartSpeed();
    qreal acc = Config::UserRegister::scanXAcc();
    qreal maxSpeed = qMax(static_cast<qreal>(engravingRunSpeed), minSpeed);
    return qAbs(qRound((maxSpeed * maxSpeed - minSpeed * minSpeed) / (acc * 2.0)));
}

void LaserDevice::debugPrintUserRegisters() const
{
#ifdef _DEBUG
    Q_D(const LaserDevice);
    for (QMap<int, LaserRegister*>::ConstIterator i = d->userRegisters.constBegin();
        i != d->userRegisters.constEnd(); i++)
    {
        qLogD << "user register [" << i.key() << "](" << i.value()->configItem()->title() << ") = "
            << i.value()->configItem()->value();
    }
#endif
}

void LaserDevice::debugPrintSystemRegisters() const
{
#ifdef _DEBUG
    Q_D(const LaserDevice);
    for (QMap<int, LaserRegister*>::ConstIterator i = d->systemRegisters.constBegin();
        i != d->systemRegisters.constEnd(); i++)
    {
        qLogD << "system register [" << i.key() << "](" << i.value()->configItem()->title() << ") = "
            << i.value()->configItem()->value();
    }
#endif
}

void LaserDevice::debugPrintRegisters() const
{
#ifdef _DEBUG
    debugPrintUserRegisters();
    debugPrintSystemRegisters();
#endif
}

QPoint LaserDevice::mapFromQuadToCurrent(const QPoint& pt, const QPoint& topLeftFrom)
{
    QRect layout = this->layoutRect();
    QPoint topLeftCurr = layout.topLeft();
    QPoint trans = topLeftCurr - topLeftFrom;
    return pt + trans;
}

QPoint LaserDevice::mapFromCurrentToQuad(const QPoint& pt, const QPoint& topLeftTo)
{
    QRect layout = this->layoutRect();
    QPoint topLeftCurr = layout.topLeft();
    QPoint trans = topLeftTo - topLeftCurr;
    return pt + trans;
}

QTransform LaserDevice::to1stQuad()
{
    QPoint offset = originOffset();
    QTransform t = QTransform::fromTranslate(offset.x(), offset.y());
    return t;
}

void LaserDevice::unload()
{
    Q_D(LaserDevice);
}

void LaserDevice::connectDevice(const QString& portName)
{
    Q_D(LaserDevice);
    qLogD << "connecting to device";
    d->portName = portName;
    d->driver->initComPort(portName);
}

void LaserDevice::disconnectDevice()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->uninitComPort();
    }
}

QString LaserDevice::activateMainCard(
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
)
{
    Q_D(LaserDevice);
    QString cardId = requestMainCardId();
    qLogD << "cardId: " << cardId;
    QString result = d->driver->activateMainCard(
        email, code, name, phone, address, qq, wx, country, distributor, brand, model, mainCardId()
    );
    qLogD << "activation result: " << result;
    return result;
}

bool LaserDevice::requestTemporaryLicense()
{
    return createLicenseFile("{FFFD38EB-DC3A-45A8-A06D-B10671CF18B3}");
}

bool LaserDevice::createLicenseFile(const QString& licenseCode)
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        return d->driver->createLicenseFile(licenseCode);
    }
    return false;
}

void LaserDevice::updateDeviceOriginAndTransform()
{
    Q_D(LaserDevice);
    d->updateDeviceOriginAndTransform();
}

void LaserDevice::startMachining()
{
    Q_D(LaserDevice);
    if (d->driver)
        d->driver->startMachining();
}

void LaserDevice::downloadToBoard()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        emit downloadingToBoard();
        d->driver->download();
    }
}

LaserRegister::RegistersMap LaserDevice::registerValues(const QMap<int, LaserRegister*>& registers, bool onlyModified) const
{
    Q_D(const LaserDevice);
    LaserRegister::RegistersMap map;
    for (LaserRegister* item : registers.values())
    {
        if (onlyModified && !item->configItem()->isModified() && !item->configItem()->isDirty())
            continue;

        LaserRegister::RegisterPair pair(item->address(), item->configItem()->value());
        if (!pair.second.isValid())
            continue;
        map.insert(pair.first, pair.second);
    }
    return map;
}

QMap<int, LaserRegister*> LaserDevice::getRegisters(const QMap<int, LaserRegister*>& registers, bool onlyModified) const
{
    Q_D(const LaserDevice);
    if (onlyModified)
    {
        QMap<int, LaserRegister*> map;
        for (LaserRegister* item : registers.values())
        {
            if (onlyModified && !item->configItem()->isModified() && !item->configItem()->isDirty())
                continue;

            map.insert(item->address(), item);
        }
        return map;
    }
    else
    {
        return registers;
    }
}

void LaserDevice::parseSystemRegisters(const QString& raw)
{
    Q_D(LaserDevice);
    batchParse(raw, d->systemRegisters);
}

void LaserDevice::parseUserRegisters(const QString& raw)
{
    Q_D(LaserDevice);
    batchParse(raw, d->userRegisters);
}

void LaserDevice::parseExternalRegisters(const QString& raw)
{
    Q_D(LaserDevice);
    batchParse(raw, d->externalRegisters);
}

void LaserDevice::batchParse(const QString& raw, const QMap<int, LaserRegister*>& registers)
{
    if (raw.isEmpty())
    {
        throw new LaserDeviceDataException(E_TransferDataError, tr("Registers data incomplete."));
    }
    QStringList segments = raw.split(";");
    for (QString seg : segments)
    {
        QStringList segItem = seg.split(",");
        if (segItem.length() != 2)
        {
            continue;
        }
        bool ok;
        int addr = segItem[0].toInt(&ok);
        if (!ok)
        {
            continue;
        }
        QString value = segItem[1];
        if (registers.contains(addr))
        {
            registers[addr]->parse(value);
            registers[addr]->configItem()->loadValue(value);
        }
    }
}

void LaserDevice::handleProgress(int position, int total, float progress)
{
    Q_D(LaserDevice);
    if (d->progress)
    {
        d->progress->setMaximum(total);
        d->progress->setProgress(position);
        if (progress >= 1)
            d->progress->finish();
    }
}

void LaserDevice::handleError(int code, const QString& message)
{
    Q_D(LaserDevice);
    LaserException* exception = nullptr;
    switch (code)
    {
    case E_SystemFatalError:
        throw new LaserDeviceFatalException(code, tr("Laser device fatal error"));
        break;
    case E_UnknownError:
        throw new LaserDeviceUnknownException(code);
        break;
    case E_InitializeError:
        d->isInit = false;
        throw new LaserDeviceConnectionException(code, tr("Failed to initialize laser device"));
        break;
    case E_UninitializeError:
        throw new LaserDeviceConnectionException(code, tr("Failed to uninitialize laser device normally"));
        break;
    case E_ComPortNotAvailable:
        throw new LaserDeviceConnectionException(code, tr("Com port not available"));
        break;
    case E_GetComPortListError:
        throw new LaserDeviceConnectionException(code, tr("Failed to get COM port list"));
        break;
    case E_DongleNotExists:
        emit dongleRemoved();
        break;
    case E_DongleActiveDisabled:
        throw new LaserDeviceSecurityException(code, tr("Dongle activation is disabled"));
        break;
    case E_MainCardRegisterError:
        //throw new LaserDeviceSecurityException(code, tr("Failed to register main card"));
        emit mainCardRegistrationChanged(false);
        break;
    case E_MainCardInactivated:
        //throw new LaserDeviceSecurityException(code, tr("Main card inactivated"));
        emit mainCardActivationChanged(false);
        break;
    case E_InvalidMainCardId:
        throw new LaserDeviceSecurityException(code, tr("Invalid main card ID"));
        break;
    case E_InvalidDongleId:
        throw new LaserDeviceSecurityException(code, tr("Invalid dongle ID"));
        break;
    case E_CardBindDongleError:
        throw new LaserDeviceSecurityException(code, tr("Failed to bind card with dongle"));
        break;
    case E_CardBindDongleRepeatedly:
        throw new LaserDeviceSecurityException(code, tr("The card is repeatedly bound to the dongle"));
        break;
    case E_CardDongleBoundExceedsTimes:
        throw new LaserDeviceSecurityException(code, tr("The number of times the card is bound to the dongle exceeds the allowable range"));
        break;
    case E_CardDongleBoundIllegal:
        throw new LaserDeviceSecurityException(code, tr("The card is illegally bound to the dongle"));
        break;
    case E_ClearLaserTubeDurationError:
        throw new LaserDeviceException(code, tr("Failed to clear duration of laser tube"));
        break;
    case E_FactoryPasswordIncorrect:
        throw new LaserDeviceSecurityException(code, tr("Incorrect factory password"));
        //emit manufacturePasswordVerified(false);
        QMessageBox::warning(LaserApplication::mainWindow, tr("Invalid password"), tr("Invalid Manufacture password"));
        break;
    case E_FactoryPasswordLengthError:
        throw new LaserDeviceSecurityException(code, tr("Invalid length of factory password"));
        break;
    case E_FactoryPasswordExpired:
        throw new LaserDeviceSecurityException(code, tr("Factory password expired"));
        break;
    case E_PasswordIncorrectTooManyTimes:
        throw new LaserDeviceSecurityException(code, tr("Input incorrect factory password too many times"));
        break;
    case E_ChangeFactoryPasswordError:
        //throw new LaserDeviceSecurityException(code, tr("Failed to change factory password"));
        emit manufacturePasswordChangeFailed();
        break;
    case E_ReadSysParamFromCardError:
        throw new LaserDeviceIOException(code, tr("Failed to read parameters from device"));
        break;
    case E_WriteSysParamToCardError:
        throw new LaserDeviceIOException(code, tr("Failed to write parameters to device"));
        break;
    case E_ReadUserParamFromCardError:
        throw new LaserDeviceIOException(code, tr("Failed to read parameters from device"));
        break;
    case E_WriteUserParamToCardError:
        throw new LaserDeviceIOException(code, tr("Failed to write parameters to device"));
        break;
    case E_SaveParamsToServerError:
        throw new LaserNetworkException(code, tr("Failed to save parameters to server"));
        break;
    case E_LoadParamsFromServerError:
        throw new LaserNetworkException(code, tr("Failed to load parameters from server"));
        break;
    case E_FileNotExistsError:
        throw new LaserDeviceDataException(code, tr("File does not exist"));
        break;
    case E_InvalidDataFormat:
        throw new LaserDeviceDataException(code, tr("Invalid data format"));
        break;
    case E_DecryptCommandError:
        throw new LaserDeviceDataException(code, tr("Failed to decrypt data"));
        break;
    case E_InvalidImageData:
        throw new LaserDeviceDataException(code, tr("Invalid image data"));
        break;
    case E_ImageMinSizeTooSmall:
        throw new LaserDeviceDataException(code, tr("Min size of image is too small"));
        break;
    case E_ImageMaxSizeTooLarge:
        throw new LaserDeviceDataException(code, tr("Max size of image is too large"));
        break;
    case E_NoDataError:
        throw new LaserDeviceIOException(code, tr("No data transfered"));
        break;
    case E_TransferDataTimeout:
        throw new LaserDeviceIOException(code, tr("Transfering data timeout"));
        break;
    case E_RetransferAfterTimeout:
        //throw new LaserDeviceIOException(code, tr("Retransfer data after timeout"));
        break;
    case E_RetransferTooManyTimes:
        throw new LaserDeviceIOException(code, tr("Retransfer data too many times"));
        break;
    case E_TransferDataError:
        throw new LaserDeviceIOException(code, tr("Failed to transfer data"));
        break;
    case E_ReceiveInvalidDataTooManyTimes:
        throw new LaserDeviceIOException(code, tr("Receive invalid data too many times"));
        break;
    case E_BreakpointDataError:
        throw new LaserDeviceIOException(code, tr("Failed to transfer data with breakpoint"));
        break;
    case E_CanNotDoOnWorking:
        //throw new LaserDeviceMachiningException(code, tr("This operation is not supported during machining"));
        break;
    case E_PingServerFail:
        throw new LaserNetworkException(code, tr("Failed to connect to server"));
        break;
    case E_ConnectServerError:
        throw new LaserNetworkException(code, tr("Failed to log in to server"));
        break;
    case E_ConnectFrequently:
        throw new LaserNetworkException(code, tr("Login too frequently"));
        break;
    case E_SubmitToServerError:
        throw new LaserNetworkException(code, tr("Failed to submit data"));
        break;
    case E_UpdateInfoFileNotExists:
        throw new LaserNetworkException(code, tr("Updating info file does not exist"));
        break;
    case E_UpdateFileNotExists:
        throw new LaserNetworkException(code, tr("Updating file dose not exist"));
        break;
    case E_UpdateFailed:
        throw new LaserNetworkException(code, tr("Failed to update"));
        break;
    case E_DownloadFirmwareDataError:
        throw new LaserNetworkException(code, tr("Failed to download firmware"));
        break;
    case E_UpdateFirmwareTimeout:
        throw new LaserNetworkException(code, tr("Update firmware timeout"));
        break;
    case E_InadequatePermissions:
        break;
    case E_SendEmailFailed:
    case E_MailboxInvalid:
    case E_MailboxAccountError:
    case E_ActiveCodeInvalid:
    case E_ValidateCodeInvalid:
    case E_MailboxNameInvalid:
        emit activeFailed(code);
        break;
    case E_DeviceOriginDisaccord:
        break;
    case E_DataOutofCacheSize:
        utils::warning(tr("Warning"), tr("Data cache is out of size."));
        break;
    }
    /*if (exception)
    {
        throw exception;
    }*/
}

void LaserDevice::handleMessage(int code, const QString& message)
{
    Q_D(LaserDevice);
    switch (code)
    {
    case M_GetComPortListOK:
    {
        QStringList portNames = message.split(";");
        emit comPortsFetched(portNames);
        break;
    }
    case M_ComPortOpened:
    {
        d->connected = true;
        QString id = d->driver->getHardwareIdentID();
        if (!id.isEmpty() && !id.isNull())
        {
            emit dongleConnected();
        }
        emit comPortConnected(portName());
        emit connected();
        break;
    }
    case M_ComPortClosed:
    {
        d->connected = false;
        emit disconnected();
        break;
    }
    case M_USBArrival:
        break;
    case M_USBRemove:
        break;
    case M_DongleConnected:
        emit dongleConnected();
        break;
    case M_DongleDisconnected:
        emit dongleDisconnected();
        break;
    case M_MainCardRegisterOK:
    {
        emit mainCardRegistrationChanged(true);
        break;
    }
    case M_MainCardIsGenuine:
    {
        emit mainCardActivationChanged(true);
        break;
    }
    case M_MainCardIsGenuineEx:
    {
        emit mainCardActivationChanged(true);
        break;
    }
    case M_MainCardMachineMoreInfo:
    {
        if (message.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Main card info incomplete."));
        }
        QStringList items = message.split(";");
        d->mainCard = items[0];
        d->mainCardRegisteredDate = items[1];
        d->mainCardActivatedDate = items[2];
        d->boundDongle = items[3];
        d->boundDongleRegisteredDate = items[4];
        d->boundDongleActivatedDate = items[5];
        d->boundDongleBindingTimes = items[6];
        d->dongle = items[7];
        d->dongleRegisteredDate = items[8];
        d->dongleActivatedDate = items[9];
        d->dongleBindingTimes = items[10];
        d->hardwareRegisteredDate = items[11];
        d->hardwareActivatedDate = items[12];
        d->hardwareMaintainingTimes = items[13];
        emit mainCardInfoFetched();
        emit dongleConnected();
        break;
    }
    case M_CardDongleBindOK:
    {
        break;
    }
    case M_LaserTubeZeroClearingOK:
    {
        break;
    }
    case M_ReadSysParamFromCardOK:
    {
        parseSystemRegisters(message);
        //debugPrintSystemRegisters();
        break;
    }
    case M_WriteSysParamToCardOK:
    {
        //debugPrintSystemRegisters();
        parseSystemRegisters(message);
        emit systemRegistersConfirmed();
        break;
    }
    case M_ReadUserParamFromCardOK:
    {
        parseUserRegisters(message);
        //debugPrintUserRegisters();
        break;
    }
    case M_WriteUserParamToCardOK:
    {
        parseUserRegisters(message);
        emit userRegistersConfirmed();
        break;
    }
    case M_ReadComputerParamFromCardOK:
    {
        parseExternalRegisters(message);
        break;
    }
    case M_WriteComputerParamToCardOK:
    {
        parseExternalRegisters(message);
        break;
    }
    case M_FactoryPasswordValid:
    {
        emit manufacturePasswordVerified(true);
        break;
    }
    case M_ChangeFactoryPasswordOK:
    {
        emit manufacturePasswordChangeOk();
        break;
    }
    case M_ReturnTextMsgFromCallback:
    {
        break;
    }
    case M_ImportFromFile:
    {
        break;
    }
    case M_CancelCurrentWork:
    {
        break;
    }
    case M_TimeConsuming:
    {
        break;
    }
    case M_EstimatedWorkTime:
    {
        break;
    }
    case M_StartProcData:
    {
        break;
    }
    case M_DataTransCompleted:
    {
        emit downloadFinished();
        break;
    }
    case M_RequestAndContinue:
    {
        break;
    }
    case M_MotorLock:
    {
        break;
    }
    case M_MotorUnlock:
    {
        break;
    }
    case M_LaserLightOn:
    {
        break;
    }
    case M_LaserLightOff:
    {
        break;
    }
    case M_StartWorking:
    {
        emit machiningStarted();
        break;
    }
    case M_PauseWorking:
    {
        emit machiningPaused();
        break;
    }
    case M_ContinueWorking:
    {
        emit continueWorking();
        break;
    }
    case M_StopWorking:
    {
        emit machiningStopped();
        break;
    }
    case M_MachineWorking:
    {
        DeviceState state;
        if (state.parse(message))
        {
            changeState(state);
        }
        break;
    }
    case M_WorkFinished:
    {
        DeviceState state;
        if (state.parse(message))
        {
            changeState(state);
        }
        emit machiningFinished();
        break;
    }
    case M_DeviceIdInfo:
    {
        break;
    }
    case M_ClientAddressInfo:
    {
        break;
    }
    case M_ConnectedServer:
    {
        break;
    }
    case M_DisconnectServer:
    {
        break;
    }
    case M_ConnectServerOK:
    {
        break;
    }
    case M_SubmitToServerOK:
    {
        break;
    }
    case M_DownloadBegin:
    {
        break;
    }
    case M_DownloadEnd:
    {
        break;
    }
    case M_NewVersionChecking:
    {
        break;
    }
    case M_NewVersionCheckFinished:
    {
        break;
    }
    case M_IsLatestVersion:
    {
        break;
    }
    case M_ReadyToUpdateFile:
    {
        break;
    }
    case M_DownloadUpdateInfoFile:
    {
        break;
    }
    case M_FoundSoftNewVersion:
    {
        break;
    }
    case M_DownloadFileCounts:
    {
        break;
    }
    case M_DownloadFileIndex:
    {
        break;
    }
    case M_DownloadSoftDataStart:
    {
        break;
    }
    case M_StartSoftUpdate:
    {
        break;
    }
    case M_CancelSoftUpdate:
    {
        break;
    }
    case M_SoftUpdateFinished:
    {
        break;
    }
    case M_FoundFirmwareNewVersion:
    {
        break;
    }
    case M_DownloadFirmwareDataStart:
    {
        break;
    }
    case M_DownloadFirmwareDataEnd:
    {
        break;
    }
    case M_SendFirmwareDataStart:
    {
        break;
    }
    case M_SendFirmwareDataEnd:
    {
        break;
    }
    case M_UpdateFirmwareStart:
    {
        break;
    }
    case M_UpdateFirmwareEnd:
    {
        break;
    }
    case M_UpdateFirmwareAbort:
    {
        break;
    }
    case M_SaveParamsToServerOK:
    {
        break;
    }
    case M_ReadParamsFromServerOK:
    {
        break;
    }
    case M_UpdateComplete:
    {
        LaserApplication::mainWindow->close();
        break;
    }
    }

}

void LaserDevice::onConnected()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->setFactoryType("EFSLaserController");
        d->driver->getDeviceWorkState();
    }
}

void LaserDevice::onMainCardRegistrationChanged(bool registered)
{
    Q_D(LaserDevice);
    d->mainCardRegistered = registered;
    qLogD << "main card registration changed: " << registered;
}

void LaserDevice::onMainCardActivationChanged(bool activated)
{
    Q_D(LaserDevice);
    d->mainCardActivated = activated;
    qLogD << "main card activation changed: " << activated;
}

void LaserDevice::onConfigStartFromChanged(const QVariant& value, void* senderPtr)
{
    Q_D(LaserDevice);
    d->updateDeviceOriginAndTransform();
    emit layoutChanged(layoutSize());
}

void LaserDevice::onConfigJobOriginChanged(const QVariant& value, void* senderPtr)
{
    Q_D(LaserDevice);
    d->updateDeviceOriginAndTransform();
}

void LaserDevice::onDeviceOriginChanged(const QVariant& value, void* senderPtr)
{
    updateDeviceOriginAndTransform();
    emit layoutChanged(layoutSize());
}

void LaserDevice::onLayerWidthChanged(const QVariant& value, void* senderPtr)
{
    updateDeviceOriginAndTransform();
    emit layoutChanged(layoutSize());
}

void LaserDevice::onLayerHeightChanged(const QVariant& value, void* senderPtr)
{
    updateDeviceOriginAndTransform();
    emit layoutChanged(layoutSize());
}

