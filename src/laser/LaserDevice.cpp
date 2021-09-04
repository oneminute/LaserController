#include "LaserDevice.h"

#include <QDate>
#include <QMessageBox>

#include "LaserApplication.h"
#include "LaserDriver.h"
#include "LaserRegister.h"
#include "common/common.h"
#include "common/Config.h"
#include "exception/LaserException.h"
#include "state/StateController.h"
#include "ui/LaserControllerWindow.h"

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
        , deviceTransform()
        , deviceOrigin(0, 0)
        , transform()
        , origin(0, 0)
    {}

    void updateDeviceOriginAndTransform();

    LaserDevice* q_ptr;
    LaserDriver* driver;
    bool isInit;
    bool connected;

    QString name;
    QString portName;
    int printerDrawUnit;    // 绘图仪单位，这里值的意思是一英寸分为多少个单位

    QTransform deviceTransform;
    QPointF deviceOrigin;
    QTransform transform;
    QPointF origin;

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

    QMap<int, LaserRegister*> userRegisters;
    QMap<int, LaserRegister*> systemRegisters;
};

void LaserDevicePrivate::updateDeviceOriginAndTransform()
{
    qreal dx = 0;
    qreal dy = 0;
    switch (Config::SystemRegister::deviceOrigin())
    {
    case 0:
        dx = 0;
        dy = 0;
        break;
    case 1:
        dx = Config::SystemRegister::xMaxLength() / 1000.0;
        dy = 0;
        break;
    case 2:
        dx = Config::SystemRegister::xMaxLength() / 1000.0;
        dy = Config::SystemRegister::yMaxLength() / 1000.0;
        break;
    case 3:
        dx = 0;
        dy = Config::SystemRegister::yMaxLength() / 1000.0;
        break;
    }

    deviceTransform = QTransform::fromTranslate(-dx, -dy);
    deviceOrigin = QPointF(dx, dy);
}

LaserDevice::LaserDevice(LaserDriver* driver, QObject* parent)
    : QObject(parent)
    , m_ptr(new LaserDevicePrivate(this))
{
    Q_D(LaserDevice);
    d->driver = driver;

    ADD_TRANSITION(deviceUnconnectedState, deviceConnectedState, this, &LaserDevice::connected);
    ADD_TRANSITION(deviceConnectedState, deviceUnconnectedState, this, &LaserDevice::disconnected);

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
    d->userRegisters.insert(15, new LaserRegister(15, Config::UserRegister::scanRowIntervalItem(), false));
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

    d->systemRegisters.insert(0, new LaserRegister(0, Config::SystemRegister::headItem(), true));
    d->systemRegisters.insert(1, new LaserRegister(1, Config::SystemRegister::passwordItem(), true, false, true));
    d->systemRegisters.insert(2, new LaserRegister(2, Config::SystemRegister::storedPasswordItem(), true, false, true));
    d->systemRegisters.insert(3, new LaserRegister(3, Config::SystemRegister::hardwareID1Item(), true, true));
    d->systemRegisters.insert(4, new LaserRegister(4, Config::SystemRegister::hardwareID2Item(), true, true));
    d->systemRegisters.insert(5, new LaserRegister(5, Config::SystemRegister::hardwareID3Item(), true, true));
    d->systemRegisters.insert(6, new LaserRegister(6, Config::SystemRegister::cdKey1Item(), true));
    d->systemRegisters.insert(7, new LaserRegister(7, Config::SystemRegister::cdKey2Item(), true));
    d->systemRegisters.insert(8, new LaserRegister(8, Config::SystemRegister::cdKey3Item(), true));
    d->systemRegisters.insert(9, new LaserRegister(9, Config::SystemRegister::sysRunTimeItem(), true));
    d->systemRegisters.insert(10, new LaserRegister(10, Config::SystemRegister::laserRunTimeItem(), true));
    d->systemRegisters.insert(11, new LaserRegister(11, Config::SystemRegister::sysRunNumItem(), true));
    d->systemRegisters.insert(12, new LaserRegister(12, Config::SystemRegister::xMaxLengthItem(), true));
    d->systemRegisters.insert(13, new LaserRegister(13, Config::SystemRegister::xDirPhaseItem(), true));
    d->systemRegisters.insert(14, new LaserRegister(14, Config::SystemRegister::xLimitPhaseItem(), true));
    d->systemRegisters.insert(15, new LaserRegister(15, Config::SystemRegister::xZeroDevItem(), true));
    d->systemRegisters.insert(16, new LaserRegister(16, Config::SystemRegister::xStepLengthItem(), true));
    d->systemRegisters.insert(17, new LaserRegister(17, Config::SystemRegister::xLimitNumItem(), true));
    d->systemRegisters.insert(18, new LaserRegister(18, Config::SystemRegister::xResetEnabledItem(), true));
    d->systemRegisters.insert(19, new LaserRegister(19, Config::SystemRegister::xMotorNumItem(), true));
    d->systemRegisters.insert(20, new LaserRegister(20, Config::SystemRegister::xMotorCurrentItem(), true));
    d->systemRegisters.insert(21, new LaserRegister(21, Config::SystemRegister::xStartSpeedItem(), true));
    d->systemRegisters.insert(22, new LaserRegister(22, Config::SystemRegister::xMaxSpeedItem(), true));
    d->systemRegisters.insert(23, new LaserRegister(23, Config::SystemRegister::xMaxAccelerationItem(), true));
    d->systemRegisters.insert(24, new LaserRegister(24, Config::SystemRegister::xUrgentAccelerationItem(), true));
    d->systemRegisters.insert(25, new LaserRegister(25, Config::SystemRegister::yMaxLengthItem(), true));
    d->systemRegisters.insert(26, new LaserRegister(26, Config::SystemRegister::yDirPhaseItem(), true));
    d->systemRegisters.insert(27, new LaserRegister(27, Config::SystemRegister::yLimitPhaseItem(), true));
    d->systemRegisters.insert(28, new LaserRegister(28, Config::SystemRegister::yZeroDevItem(), true));
    d->systemRegisters.insert(29, new LaserRegister(29, Config::SystemRegister::yStepLengthItem(), true));
    d->systemRegisters.insert(30, new LaserRegister(30, Config::SystemRegister::yLimitNumItem(), true));
    d->systemRegisters.insert(31, new LaserRegister(31, Config::SystemRegister::yResetEnabledItem(), true));
    d->systemRegisters.insert(32, new LaserRegister(32, Config::SystemRegister::yMotorNumItem(), true));
    d->systemRegisters.insert(33, new LaserRegister(33, Config::SystemRegister::yMotorCurrentItem(), true));
    d->systemRegisters.insert(34, new LaserRegister(34, Config::SystemRegister::yStartSpeedItem(), true));
    d->systemRegisters.insert(35, new LaserRegister(35, Config::SystemRegister::yMaxSpeedItem(), true));
    d->systemRegisters.insert(36, new LaserRegister(36, Config::SystemRegister::yMaxAccelerationItem(), true));
    d->systemRegisters.insert(37, new LaserRegister(37, Config::SystemRegister::yUrgentAccelerationItem(), true));
    d->systemRegisters.insert(38, new LaserRegister(38, Config::SystemRegister::zMaxLengthItem(), true));
    d->systemRegisters.insert(39, new LaserRegister(39, Config::SystemRegister::zDirPhaseItem(), true));
    d->systemRegisters.insert(40, new LaserRegister(40, Config::SystemRegister::zLimitPhaseItem(), true));
    d->systemRegisters.insert(41, new LaserRegister(41, Config::SystemRegister::zZeroDevItem(), true));
    d->systemRegisters.insert(42, new LaserRegister(42, Config::SystemRegister::zStepLengthItem(), true));
    d->systemRegisters.insert(43, new LaserRegister(43, Config::SystemRegister::zLimitNumItem(), true));
    d->systemRegisters.insert(44, new LaserRegister(44, Config::SystemRegister::zResetEnabledItem(), true));
    d->systemRegisters.insert(45, new LaserRegister(45, Config::SystemRegister::zMotorNumItem(), true));
    d->systemRegisters.insert(46, new LaserRegister(46, Config::SystemRegister::zMotorCurrentItem(), true));
    d->systemRegisters.insert(47, new LaserRegister(47, Config::SystemRegister::zStartSpeedItem(), true));
    d->systemRegisters.insert(48, new LaserRegister(48, Config::SystemRegister::zMaxSpeedItem(), true));
    d->systemRegisters.insert(49, new LaserRegister(49, Config::SystemRegister::zMaxAccelerationItem(), true));
    d->systemRegisters.insert(50, new LaserRegister(50, Config::SystemRegister::zUrgentAccelerationItem(), true));
    d->systemRegisters.insert(51, new LaserRegister(51, Config::SystemRegister::laserMaxPowerItem(), true));
    d->systemRegisters.insert(52, new LaserRegister(52, Config::SystemRegister::laserMinPowerItem(), true));
    d->systemRegisters.insert(53, new LaserRegister(53, Config::SystemRegister::laserPowerFreqItem(), true));
    d->systemRegisters.insert(54, new LaserRegister(54, Config::SystemRegister::xPhaseEnabledItem(), true));
    d->systemRegisters.insert(55, new LaserRegister(55, Config::SystemRegister::yPhaseEnabledItem(), true));
    d->systemRegisters.insert(56, new LaserRegister(56, Config::SystemRegister::zPhaseEnabledItem(), true));
    d->systemRegisters.insert(57, new LaserRegister(57, Config::SystemRegister::deviceOriginItem(), true));

    connect(this, &LaserDevice::comPortsFetched, this, &LaserDevice::onComPortsFetched);
    connect(this, &LaserDevice::connected, this, &LaserDevice::onConnected);
    connect(this, &LaserDevice::mainCardRegistered, this, &LaserDevice::onMainCardRegistered);
    connect(this, &LaserDevice::mainCardActivated, this, &LaserDevice::onMainCardActivated);
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

bool LaserDevice::load()
{
    Q_D(LaserDevice);

    connect(d->driver, &LaserDriver::raiseError, this, &LaserDevice::handleError, Qt::ConnectionType::QueuedConnection);
    connect(d->driver, &LaserDriver::sendMessage, this, &LaserDevice::handleMessage, Qt::ConnectionType::QueuedConnection);
    connect(d->driver, &LaserDriver::libraryLoaded, this, &LaserDevice::onLibraryLoaded);
    connect(d->driver, &LaserDriver::libraryInitialized, this, &LaserDevice::onLibraryInitialized);
    if (d->driver->load())
    {
        return true;
    }

    return false;
}

qreal LaserDevice::layoutWidth() const
{
    Q_D(const LaserDevice);
    return Config::SystemRegister::xMaxLength() / 1000.0;
}

qreal LaserDevice::layoutHeight() const
{
    Q_D(const LaserDevice);
    return Config::SystemRegister::yMaxLength() / 1000.0;
}

//void LaserDevice::setLayoutRect(const QRectF& rect, bool toCard)
//{
//    Q_D(LaserDevice);
//    d->layoutRect = rect;
//    if (d->driver && toCard && d->layoutRect.isValid())
//    {
//        d->driver->setSoftwareInitialization(
//            d->printerDrawUnit,
//            d->layoutRect.left(),
//            d->layoutRect.right(),
//            d->layoutRect.width(),
//            d->layoutRect.height());
//    }
//}

int LaserDevice::printerDrawUnit() const
{
    Q_D(const LaserDevice);
    return d->printerDrawUnit;
}

void LaserDevice::setPrinterDrawUnit(int unit, bool toCard)
{
    Q_D(LaserDevice);
    d->printerDrawUnit = unit;
    //setLayoutRect(d->layoutRect, toCard);
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
        return d->driver->getMainCardID();
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
    return d->dongle;
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

bool LaserDevice::verifyManufacturePassword(const QString& password)
{
    Q_D(LaserDevice);
    //return d->driver->checkFactoryPassword(password);
    return true;
}

bool LaserDevice::writeUserRegisters()
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->writeUserParamToCard(userRegisterValues(true));
}

bool LaserDevice::writeSystemRegisters()
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->writeSysParamToCard(systemRegisterValues(true));
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
    //d->driver->showAboutWindow();
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

void LaserDevice::moveTo(const QVector3D& pos, QUADRANT quad)
{
    // Get layout size
    float layoutWidth = LaserApplication::device->layoutWidth();
    float layoutHeight = LaserApplication::device->layoutHeight();

    QVector3D dest = utils::limitToLayout(pos, quad, layoutWidth, layoutHeight);
    char xyzStyle = 0;
    int xPos = qRound(dest.x() * 1000);
    int yPos = qRound(dest.y() * 1000);
    int zPos = qRound(dest.z() * 1000);
    bool xEnabled = xPos != 0;
    bool yEnabled = yPos != 0;
    bool zEnabled = zPos != 0;
    LaserDriver::instance().lPenQuickMoveTo(
        xEnabled, true, xPos,
        yEnabled, true, yPos,
        zEnabled, true, zPos
    );
}

void LaserDevice::moveBy(const QVector3D& pos)
{
    // Get layout size
    float layoutWidth = LaserApplication::device->layoutWidth();
    float layoutHeight = LaserApplication::device->layoutHeight();

    //QVector3D dest = utils::limitToLayout(pos, quad, layoutWidth, layoutHeight);
    int xPos = qRound(pos.x() * 1000);
    int yPos = qRound(pos.y() * 1000);
    int zPos = qRound(pos.z() * 1000);
    bool xEnabled = xPos != 0;
    bool yEnabled = yPos != 0;
    bool zEnabled = zPos != 0;
    LaserDriver::instance().lPenQuickMoveTo(
        xEnabled, false, xPos,
        yEnabled, false, yPos,
        zEnabled, false, zPos
    );
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

QPointF LaserDevice::origin() const
{
    return QPointF();
}

QPointF LaserDevice::deviceOrigin() const
{
    Q_D(const LaserDevice);
    return d->deviceOrigin;
}

QTransform LaserDevice::transform() const
{
    Q_D(const LaserDevice);
    return d->transform;
}

QTransform LaserDevice::deviceTransform() const
{
    Q_D(const LaserDevice);
    return d->deviceTransform;
}

void LaserDevice::batchParse(const QString& raw, bool isSystem, ModifiedBy modifiedBy)
{
    Q_D(LaserDevice);
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
        if (isSystem)
        {
            if (d->systemRegisters.contains(addr))
            {
                d->systemRegisters[addr]->parse(value, modifiedBy);
            }
        }
        else
        {
            if (d->userRegisters.contains(addr))
            {
                d->userRegisters[addr]->parse(value, modifiedBy);
            }
        }
    }
}

LaserRegister::RegistersMap LaserDevice::userRegisterValues(bool onlyModified) const
{
    Q_D(const LaserDevice);
    LaserRegister::RegistersMap map;
    for (LaserRegister* item : d->userRegisters.values())
    {
        if (onlyModified && !item->configItem()->isModified())
            continue;

        LaserRegister::RegisterPair pair(item->address(), item->value());
        if (!pair.second.isValid())
            continue;
        map.insert(pair.first, pair.second);
    }
    return map;
}

LaserRegister::RegistersMap LaserDevice::systemRegisterValues(bool onlyModified) const
{
    Q_D(const LaserDevice);
    LaserRegister::RegistersMap map;
    for (LaserRegister* item : d->systemRegisters.values())
    {
        if (onlyModified && !item->configItem()->isModified())
            continue;

        LaserRegister::RegisterPair pair(item->address(), item->value());
        if (!pair.second.isValid())
            continue;
        map.insert(pair.first, pair.second);
    }
    return map;
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

QString LaserDevice::activateMainCard(const QString& name, const QString& address, const QString& phone, const QString& qq, const QString& wx, const QString& email, const QString& country, const QString& distributor, const QString& trademark, const QString& model)
{
    Q_D(LaserDevice);
    QString cardId = requestMainCardId();
    qLogD << "cardId: " << cardId;
    QString result = d->driver->activateMainCard(name, address, phone, qq, wx, email, country, distributor, trademark, model, cardId);
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

void LaserDevice::moveToOrigin(qreal speed)
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->lPenMoveToOriginalPoint(speed);
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
        throw new LaserDeviceSecurityException(code, tr("Dongle does not exist"));
        break;
    case E_DongleActiveDisabled:
        throw new LaserDeviceSecurityException(code, tr("Dongle activation is disabled"));
        break;
    case E_MainCardRegisterError:
        throw new LaserDeviceSecurityException(code, tr("Failed to register main card"));
        break;
    case E_MainCardInactivated:
        throw new LaserDeviceSecurityException(code, tr("Main card inactivated"));
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
        throw new LaserDeviceSecurityException(code, tr("Failed to change factory password"));
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
        throw new LaserDeviceIOException(code, tr("Retransfer data after timeout"));
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
        throw new LaserDeviceMachiningException(code, tr("This operation is not supported during machining"));
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
    case M_MainCardRegisterOK:
    {
        emit mainCardRegistered();
        break;
    }
    case M_MainCardIsGenuine:
    {
        emit mainCardActivated(false);
        break;
    }
    case M_MainCardIsGenuineEx:
    {
        emit mainCardActivated(true);
        break;
    }
    case M_MainCardMachineMoreInfo:
    {
        if (message.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Main card info incomplete."));
        }
        QStringList items = message.split(";");
        if (items.length() != 13)
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Main card info incomplete."));
        }
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
        if (message.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Registers data incomplete."));
        }
        batchParse(message, true, MB_Register);
        break;
    }
    case M_WriteSysParamToCardOK:
    {
        if (message.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Registers data incomplete."));
        }
        batchParse(message, true, MB_RegisterConfirmed);
        break;
    }
    case M_ReadUserParamFromCardOK:
    {
        if (message.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Registers data incomplete."));
        }
        batchParse(message, false, MB_Register);
        break;
    }
    case M_WriteUserParamToCardOK:
    {
        if (message.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Registers data incomplete."));
        }
        batchParse(message, false, MB_RegisterConfirmed);
        break;
    }
    case M_ReadComputerParamFromCardOK:
    {
        break;
    }
    case M_WriteComputerParamToCardOK:
    {
        break;
    }
    case M_FactoryPasswordValid:
    {
        emit manufacturePasswordVerified(true);
        break;
    }
    case M_ChangeFactoryPasswordOK:
    {
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
        break;
    }
    case M_PauseWorking:
    {
        break;
    }
    case M_ContinueWorking:
    {
        break;
    }
    case M_StopWorking:
    {
        break;
    }
    case M_MachineWorking:
    {
        break;
    }
    case M_Idle:
    //case M_WorkFinished:
    {
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

void LaserDevice::onLibraryLoaded(bool success)
{
    Q_D(LaserDevice);
    qLogD << "LaserDevice::onLibraryLoaded: success = " << success;
    try {
        d->isInit = false;
        d->driver->init(LaserApplication::mainWindow->winId());
    }
    catch (...) {
        d->isInit = false;
    }
}

void LaserDevice::onLibraryInitialized()
{
    Q_D(LaserDevice);
    qLogD << "LaserDevice::onLibraryInitialized";
    showAboutWindow(5);
    d->driver->setupCallbacks();
    d->isInit = true;
    d->driver->setLanguage(Config::General::language() == QLocale::Chinese ? 1 : 0);
    QString systemDate(__DATE__);
    qLogD << "system date: " << systemDate;
    QDate compileDate = QLocale("en_US").toDate(systemDate.simplified(), "MMM d yyyy");
    int year = compileDate.year() % 100;
    int month = compileDate.month();
    int day = compileDate.day();
    int version = year * 10000 + month * 100 + day;
    //int winId = d->driver->getUpdatePanelHandle(version, LaserApplication::mainWindow->winId());
    //LaserApplication::mainWindow->createUpdateDockPanel(winId);
    d->driver->getPortList();
}

void LaserDevice::onComPortsFetched(const QStringList& portNames)
{
    Q_D(LaserDevice);
    if (portNames.length() == 1)
    {
        connectDevice(portNames[0]);
    }
    else if (portNames.length() > 1)
    {
        if (Config::Device::autoConnectFirst())
        {
            connectDevice(portNames[0]);
        }
    }
}

void LaserDevice::onConnected()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->setFactoryType("LaserController");

        //d->driver->getMainCardRegisterState();
        //QString compileInfo = d->driver->getCompileInfo();
        //qLogD << "compile info: " << compileInfo;
        //QString laserLibraryInfo = d->driver->getLaserLibraryInfo();
        //qLogD << "laser library info: " << laserLibraryInfo;
        //QString mainCardId = d->driver->getMainCardID();
        //qLogD << "main card id: " << mainCardId;
    }
}

void LaserDevice::onMainCardRegistered()
{
    Q_D(LaserDevice);
    d->mainCard = requestMainCardId();
    qLogD << "Hardware id: " << d->mainCard;
    qLogD << "Main card id: " << requestMainCardId();
    qLogD << "Dongle id: " << requestDongleId();
    requestMainCardInfo();
}

void LaserDevice::onMainCardActivated(bool temp)
{
    Q_D(LaserDevice);
    qLogD << "main card activated. temp? " << temp;
}

void LaserDevice::onConfigStartFromChanged(const QVariant& value, ModifiedBy modifiedBy)
{
    Q_D(LaserDevice);
    d->updateDeviceOriginAndTransform();
}

void LaserDevice::onConfigJobOriginChanged(const QVariant& value, ModifiedBy modifiedBy)
{
    Q_D(LaserDevice);
    d->updateDeviceOriginAndTransform();
}

