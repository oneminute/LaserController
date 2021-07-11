#include "LaserDriver.h"

#include <QDebug>
#include <QErrorMessage>
#include <QMessageBox>
#include <QFile>
#include <QDataStream>

#include "LaserDevice.h"
#include "LaserRegister.h"
#include "exception/LaserException.h"
#include "state/StateController.h"
#include "util/Utils.h"
#include "util/TypeUtils.h"

QMap<int, QString> LaserDriver::g_registerComments;
LaserDriver* LaserDriver::g_driver(nullptr);

LaserDriver::LaserDriver(QObject* parent)
    : QObject(parent)
    , m_isLoaded(false)
    , m_isConnected(false)
    , m_isMachining(false)
    , m_isPaused(false)
    , m_portName("")
    , m_parentWidget(nullptr)
    , m_isDownloading(false)
    , m_packagesCount(0)
    , m_device(nullptr)
    , m_isClosed(false)
{
    if (g_driver)
    {
        throw LaserFatalException(tr("Device driver initialize failure."));
    }
    g_driver = this;

    ADD_TRANSITION(deviceIdleState, deviceMachiningState, this, &LaserDriver::machiningStarted);
    ADD_TRANSITION(deviceMachiningState, devicePausedState, this, &LaserDriver::machiningPaused);
    ADD_TRANSITION(devicePausedState, deviceMachiningState, this, &LaserDriver::continueWorking);
    ADD_TRANSITION(devicePausedState, deviceIdleState, this, &LaserDriver::machiningStopped);
    ADD_TRANSITION(deviceMachiningState, deviceIdleState, this, &LaserDriver::machiningStopped);
    ADD_TRANSITION(deviceMachiningState, deviceIdleState, this, &LaserDriver::machiningCompleted);

    /*QFile deviceCache("dev_cache.bin");
    if (!deviceCache.open(QIODevice::ReadOnly))
    {
        qWarning() << "Can not read cache file!";
        return;
    }

    qDebug() << "available bytes:" << deviceCache.bytesAvailable();
    QDataStream stream(&deviceCache);
    QMap<int, QVariant> registers;
    stream >> registers;
    qDebug() << "load cached registers:" << registers;
    m_registers.clear();
    for (QMap<int, QVariant>::ConstIterator i = registers.constBegin(); i != registers.constEnd(); i++)
    {
        m_registers.insert(static_cast<RegisterType>(i.key()), i.value());
    }

    deviceCache.close();*/
}

LaserDriver::~LaserDriver()
{
    //unload();
    g_driver = nullptr;
    qLogD << "driver destroyed";
}

LaserDriver& LaserDriver::instance()
{
    return *g_driver;
}

void LaserDriver::ProgressCallBackHandler(void* ptr, int position, int totalCount)
{
    if (instance().m_isClosed)
        return;

    float progress = position * 1.0f / totalCount;
    qDebug() << "Progress callback handler: position = " << position << ", totalCount = " << totalCount << ", progress = " << QString("%1%").arg(static_cast<double>(progress * 100), 3, 'g', 4)
        ;
    if (instance().m_isDownloading)
    {
        emit instance().downloading(position, totalCount, progress);
    }
}

void LaserDriver::SysMessageCallBackHandler(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData)
{
    if (instance().m_isClosed)
        return;

    QString eventData = QString::fromWCharArray(sysEventData);
    qLogD << "System message callback handler: index = " << sysMsgIndex << ", code = " << sysMsgCode << ", event data = " << eventData;
    if (sysMsgCode >= E_Base && sysMsgCode < M_Base)
    {
        emit instance().raiseError(sysMsgCode, eventData);
    }
    else if (sysMsgCode >= M_Base)
    {
        emit instance().sendMessage(sysMsgCode, eventData);
    }
}

void LaserDriver::parseAndRefreshRegisters(QString& eventData, LaserRegister::RegistersMap& registers)
{
    //instance().m_registers.clear();
    for (QString i : eventData.split(";"))
    {
        QString str = i.trimmed();
        if (str.isEmpty() || str.isNull())
            continue;

        QStringList tokens = str.split(",");
        if (tokens.length() != 2)
            continue;

        int addr = 0;
        bool ok = false;
        addr = tokens[0].toInt(&ok);
        if (!ok)
            continue;

        QVariant value = tokens[1];

        if (registers.contains(addr))
        {
            registers[addr] = value;
        }
        else
        {
            registers.insert(addr, value);
        }
    }

    QFile deviceCache("dev_cache.bin");
    if (!deviceCache.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << "Can not create cache file!";
        return;
    }

    QDataStream stream(&deviceCache);
    stream << instance().m_registers;

    deviceCache.close();
}

void LaserDriver::ProcDataProgressCallBackHandler(void* ptr, int position, int totalCount)
{
    float progress = position * 1.0f / totalCount;
    qDebug() << "Proc progress callback handler:" << position << totalCount << QString("%1%").arg(static_cast<double>(progress * 100), 3, 'g', 4);

}

bool LaserDriver::load()
{
    /*m_registerComments.insert(REG_03, tr("Reset calib speed."));
    m_registerComments.insert(REG_04, tr("Engraving Launching Speed."));
    m_registerComments.insert(REG_05, tr("Move fast speed."));
    m_registerComments.insert(REG_06, tr("Cutting speed."));
    m_registerComments.insert(REG_07, tr("Move to origin speed."));
    m_registerComments.insert(REG_08, tr("Working quadrant."));
    m_registerComments.insert(REG_09, tr("X Axis pulse length."));
    m_registerComments.insert(REG_10, tr("Y Axis pulse length."));
    m_registerComments.insert(REG_11, tr("X Axis backlash."));
    m_registerComments.insert(REG_12, tr("Y Axis backlash."));
    m_registerComments.insert(REG_13, tr("Engraving column step."));
    m_registerComments.insert(REG_14, tr("Engraving row step."));
    m_registerComments.insert(REG_15, tr("Engraving laser power."));
    m_registerComments.insert(REG_16, tr("Max engraving gray value."));
    m_registerComments.insert(REG_17, tr("Min engraving gray value."));
    m_registerComments.insert(REG_18, tr("Cutting laser power."));
    m_registerComments.insert(REG_19, tr("Cutting running speed ratio."));
    m_registerComments.insert(REG_20, tr("Cutting launching speed ratio."));
    m_registerComments.insert(REG_21, tr("Motor phase."));
    m_registerComments.insert(REG_22, tr("Limit phase."));
    m_registerComments.insert(REG_23, tr("Total working duration."));
    m_registerComments.insert(REG_24, tr("Total laser duration."));
    m_registerComments.insert(REG_25, tr("Cutting laser frequency."));
    m_registerComments.insert(REG_26, tr("Registion."));
    m_registerComments.insert(REG_27, tr("Engraving laser frequency."));
    m_registerComments.insert(REG_31, tr("Custom 1 X."));
    m_registerComments.insert(REG_32, tr("Custom 1 Y."));
    m_registerComments.insert(REG_33, tr("Custom 2 X."));
    m_registerComments.insert(REG_34, tr("Custom 2 Y."));
    m_registerComments.insert(REG_35, tr("Custom 3 X."));
    m_registerComments.insert(REG_36, tr("Custom 3 Y."));
    m_registerComments.insert(REG_38, tr("Layout size."));
    m_registerComments.insert(REG_39, tr("Painting unit."));
    m_registerComments.insert(REG_40, tr("Move fast launching speed."));*/

    if (m_isLoaded)
        return true;

    qRegisterMetaType<LaserState>("LaserState");

    m_library.setFileName("LaserLib32.dll");
    if (!m_library.load())
    {
        qDebug() << "load LaserLib failure:" << m_library.errorString();
        emit libraryLoaded(false);
        return false;
    }

    m_fnGetAPILibVersion = (FN_WCHART_VOID)m_library.resolve("GetAPILibVersion");
    m_fnGetAPILibCompileInfo = (FN_WCHART_VOID)m_library.resolve("GetAPILibCompileInfo");
    m_fnInitLib = (FN_VOID_INT)m_library.resolve("InitLib");
    m_fnUnInitLib = (FN_VOID_VOID)m_library.resolve("UnInitLib");
    m_fnProgressCallBack = (FNProgressCallBack)m_library.resolve("ProgressCallBack");
    m_fnSysMessageCallBack = (FNSysMessageCallBack)m_library.resolve("SysMessageCallBack");
    //m_fnProcDataProgressCallBack = (FNProcDataProgressCallBack)m_library.resolve("ProcDataProgressCallBack");
    m_fnGetComPortList = (FN_WCHART_VOID)m_library.resolve("GetComPortList");

    //m_fnProcDataProgressCallBack(LaserDriver::ProcDataProgressCallBackHandler);

    m_fnInitComPort = (FN_INT_INT)m_library.resolve("InitComPort");
    m_fnUnInitComPort = (FN_INT_VOID)m_library.resolve("UnInitComPort");

    m_fnSetTransTimeOutInterval = (FN_VOID_INT)m_library.resolve("SetTransTimeOutInterval");
    m_fnSetSoftwareInitialization = (FNSetSoftwareInitialization)m_library.resolve("SetSoftwareInitialization");
    m_fnSetRotateDeviceParam = (FNSetRotateDeviceParam)m_library.resolve("SetRotateDeviceParam");
    m_fnSetHardwareInitialization = (FNSetHardwareInitialization)m_library.resolve("SetHardwareInitialization");

    m_fnWriteSysParamToCard = (FN_INT_WCHART_WCHART)m_library.resolve("WriteSysParamToCard");
    m_fnReadSysParamFromCard = (FN_INT_WCHART)m_library.resolve("ReadSysParamFromCard");
    m_fnWriteUserParamToCard = (FN_INT_WCHART_WCHART)m_library.resolve("WriteUserParamToCard");
    m_fnReadUserParamFromCard = (FN_INT_WCHART)m_library.resolve("ReadUserParamFromCard");

    m_fnShowAboutWindow = (FN_VOID_VOID)m_library.resolve("ShowAboutWindow");
    m_fnCheckFactoryPassword = (FN_INT_WCHART)m_library.resolve("CheckFactoryPassWord");
    m_fnWriteFactoryPassword = (FN_INT_WCHART_WCHART)m_library.resolve("WriteFactoryPassWord");

    m_fnLPenMoveToOriginalPoint = (FN_VOID_DOUBLE)m_library.resolve("LPenMoveToOriginalPoint");
    m_fnLPenQuickMoveTo = (FNLPenQuickMoveTo)m_library.resolve("LPenQuickMoveTo");
    m_fnControlHDAction = (FN_VOID_INT)m_library.resolve("ControlHDAction");

    m_fnGetMainCardID = (FN_WCHART_VOID)m_library.resolve("GetMainCardID");
    m_fnActiveMainCard = (FNActivationMainCard)m_library.resolve("ActivationMainCard");
    m_fnGetDeviceId = (FN_WCHART_BOOL)m_library.resolve("GetDeviceID");
    m_fnGetHardwareKeyID = (FN_WCHART_VOID)m_library.resolve("GetHardwareKeyID");
    m_fnGetMainCardRegState = (FN_VOID_VOID)m_library.resolve("GetMainCardRegState");
    m_fnGetMainCardInfo = (FN_WCHART_VOID)m_library.resolve("GetMainCardInfo");
    m_fnCreateLicenseFile = (FN_BOOL_WCHART)m_library.resolve("CreateLicenseFile");

    m_fnGetCurrentLaserPos = (FN_WCHART_VOID)m_library.resolve("GetCurrentLaserPos");
    m_fnSmallScaleMovement = (FNSmallScaleMovement)m_library.resolve("SmallScaleMovement");
    m_fnStartMachining = (FN_VOID_BOOL)m_library.resolve("StartMachining");
    m_fnPauseContinueMachining = (FN_INT_BOOL)m_library.resolve("PauseContinueMachining");
    m_fnStopMachining = (FN_VOID_VOID)m_library.resolve("StopMachining");
    m_fnControlMotor = (FN_INT_BOOL)m_library.resolve("ControlMotor");
    m_fnTestLaserLight = (FN_INT_BOOL)m_library.resolve("TestLaserLight");

    m_fnLoadDataFromFile = (FN_INT_WCHART)m_library.resolve("LoadDataFromFile");
    m_fnGetDeviceWorkState = (FN_VOID_VOID)m_library.resolve("GetDeviceWorkState");

    m_fnMillimeter2MicroStep = (FN_INT_DOUBLE_BOOL)m_library.resolve("Millimeter2MicroStep");

    Q_ASSERT(m_fnLoadDataFromFile);

    m_isLoaded = true;
    emit libraryLoaded(true);
    return true;
}

void LaserDriver::unload()
{
    if (m_isLoaded)
        m_fnUnInitLib();
    m_isLoaded = false;
    m_isClosed = true;
    emit libraryUnloaded();
}

void LaserDriver::setDevice(LaserDevice* device)
{
    m_device = device;
}

LaserDevice* LaserDriver::device() const
{
    return m_device;
}

QString LaserDriver::getVersion()
{
    wchar_t* raw = m_fnGetAPILibVersion();
    QString version = QString::fromWCharArray(raw);
    return version;
}

QString LaserDriver::getCompileInfo()
{
    wchar_t* raw = m_fnGetAPILibCompileInfo();
    QString info = QString::fromWCharArray(raw);
    return info;
}

void LaserDriver::init(int winId)
{
    m_fnInitLib(winId);
    m_fnProgressCallBack(LaserDriver::ProgressCallBackHandler);
    m_fnSysMessageCallBack(LaserDriver::SysMessageCallBackHandler);
    emit libraryInitialized();
}

void LaserDriver::unInit()
{
    m_fnUnInitLib();
    m_isLoaded = false;
    emit libraryUninitialized();
}

QStringList LaserDriver::getPortList()
{
    QString portList = QString::fromWCharArray(m_fnGetComPortList());
    QStringList portNames = portList.split(";");

    return portNames;
}

bool LaserDriver::initComPort(const QString& name)
{
    int port = utils::parsePortName(name);
    int result = m_fnInitComPort(port);
    if (result == 0)
    {
        m_isConnected = true;
        m_portName = name;
    }
    return result == 0;
}

bool LaserDriver::uninitComPort()
{
    int result = m_fnUnInitComPort();
    return result == 0;
}

void LaserDriver::setTransTimeOutInterval(int interval)
{
    m_fnSetTransTimeOutInterval(interval);
}

void LaserDriver::setSoftwareInitialization(int printerDrawUnit, double pageZeroX, double pageZeroY, double pageWidth, double pageHeight)
{
    m_fnSetSoftwareInitialization(printerDrawUnit, pageZeroX, pageZeroY, pageWidth, pageHeight);
}

void LaserDriver::setRotateDeviceParam(int type, int perimeterPulse, int materialPerimeter, int deviceDPI, bool autoScaleDimensions)
{
    m_fnSetRotateDeviceParam(type, perimeterPulse, materialPerimeter, deviceDPI, autoScaleDimensions);
}

void LaserDriver::setHardwareInitialization(double curveToSpeedRatio, int logicalResolution, int maxSpeed, char zeroCoordinates)
{
    m_fnSetHardwareInitialization(curveToSpeedRatio, logicalResolution, maxSpeed, zeroCoordinates);
}

bool LaserDriver::writeSysParamToCard(const LaserRegister::RegistersMap& values)
{
    if (values.count() == 0)
        return false;

    QString addrBuf, valuesBuf;
    QStringList addrList;
    QStringList valuesList;
    for (LaserRegister::RegistersMap::ConstIterator i = values.constBegin(); i != values.constEnd(); i++)
    {
        addrList.append(QString("%1").arg(i.key()));
        valuesList.append(i.value().toString());
    }
    addrBuf = addrList.join(",");
    valuesBuf = valuesList.join(",");

    wchar_t* wcAddrs = typeUtils::qStringToWCharPtr(addrBuf);
    wchar_t* wcValues = typeUtils::qStringToWCharPtr(valuesBuf);

    qDebug() << "address list: " << addrBuf;
    qDebug() << "values list: " << valuesBuf;

    bool success = m_fnWriteSysParamToCard(wcAddrs, wcValues) != -1;
    delete[] wcAddrs;
    delete[] wcValues;
    return success;
}

bool LaserDriver::readSysParamFromCard(QList<int> addresses)
{
    if (addresses.length() == 0)
        return false;

    QStringList addrList;
    for (int i = 0; i < addresses.length(); i++)
    {
        addrList.append(QString("%1").arg(addresses[i]));
    }
    QString addrStr = addrList.join(",");
    wchar_t* addrBuf = typeUtils::qStringToWCharPtr(addrStr);

    bool success = m_fnReadSysParamFromCard(addrBuf) != -1;
    delete[] addrBuf;
    return success;
}

bool LaserDriver::readAllSysParamFromCard()
{
    QList<int> params;
    for (int i = 0; i < 52; i++)
    {
        params << i;
    }
    return readSysParamFromCard(params);
}

bool LaserDriver::writeUserParamToCard(const LaserRegister::RegistersMap& values)
{
    if (values.count() == 0)
        return false;

    QString addrBuf, valuesBuf;
    QStringList addrList;
    QStringList valuesList;
    for (LaserRegister::RegistersMap::ConstIterator i = values.constBegin(); i != values.constEnd(); i++)
    {
        addrList.append(QString("%1").arg(i.key()));
        valuesList.append(i.value().toString());
    }
    addrBuf = addrList.join(",");
    valuesBuf = valuesList.join(",");

    wchar_t* wcAddrs = typeUtils::qStringToWCharPtr(addrBuf);
    wchar_t* wcValues = typeUtils::qStringToWCharPtr(valuesBuf);

    qDebug() << "address list: " << addrBuf;
    qDebug() << "values list: " << valuesBuf;

    bool success = m_fnWriteUserParamToCard(wcAddrs, wcValues) != -1;
    delete[] wcAddrs;
    delete[] wcValues;
    return success;
}

bool LaserDriver::readUserParamFromCard(QList<int> addresses)
{
    if (addresses.length() == 0)
        return false;

    QStringList addrList;
    for (int i = 0; i < addresses.length(); i++)
    {
        addrList.append(QString("%1").arg(addresses[i]));
    }
    QString addrStr = addrList.join(",");
    wchar_t* addrBuf = typeUtils::qStringToWCharPtr(addrStr);

    bool success = m_fnReadUserParamFromCard(addrBuf) != -1;
    delete[] addrBuf;
    return success;
}

bool LaserDriver::readAllUserParamFromCard()
{
    QList<int> params;
    for (int i = 0; i < 23; i++)
    {
        params << i;
    }
    return readSysParamFromCard(params);

}

void LaserDriver::showAboutWindow()
{
    m_fnShowAboutWindow();
}

bool LaserDriver::checkFactoryPassword(const QString& password)
{
    wchar_t* wcPassword = typeUtils::qStringToWCharPtr(password);
    bool success = m_fnCheckFactoryPassword(wcPassword) != -1;
    delete[] wcPassword;
    return success;
}

bool LaserDriver::changeFactoryPassword(const QString& oldPassword, const QString& newPassword)
{
    wchar_t* wcOldPassword = typeUtils::qStringToWCharPtr(oldPassword);
    wchar_t* wcNewPassword = typeUtils::qStringToWCharPtr(newPassword);
    bool success = m_fnWriteFactoryPassword(wcOldPassword, wcNewPassword) != -1;
    delete[] wcOldPassword;
    delete[] wcNewPassword;
    return success;
}

void LaserDriver::lPenMoveToOriginalPoint(double speed)
{
    m_fnLPenMoveToOriginalPoint(speed);
}

void LaserDriver::lPenQuickMoveTo(char xyzStyle, bool zeroPointStyle, double x, double y, double z, int startSpeed, int workSpeed)
{
    m_fnLPenQuickMoveTo(xyzStyle, zeroPointStyle, x, y, z, startSpeed, workSpeed);
}

void LaserDriver::controlHDAction(int action)
{
    m_fnControlHDAction(action);
}

QString LaserDriver::getMainCardID()
{
    QString id = QString::fromWCharArray(m_fnGetMainCardID());
    return id;
}

QString LaserDriver::activateMainCard(const QString& name, const QString& address,
    const QString& phone, const QString& qq, const QString& wx, const QString& email,
    const QString& country, const QString& distributor, const QString& trademark,
    const QString& model, const QString& cardId)
{
    wchar_t* nameBuf = typeUtils::qStringToWCharPtr(name);
    wchar_t* addressBuf = typeUtils::qStringToWCharPtr(address);
    wchar_t* phoneBuf = typeUtils::qStringToWCharPtr(phone);
    wchar_t* qqBuf = typeUtils::qStringToWCharPtr(qq);
    wchar_t* wxBuf = typeUtils::qStringToWCharPtr(wx);
    wchar_t* emailBuf = typeUtils::qStringToWCharPtr(email);
    wchar_t* countryBuf = typeUtils::qStringToWCharPtr(country);
    wchar_t* distributorBuf = typeUtils::qStringToWCharPtr(distributor);
    wchar_t* trademarkBuf = typeUtils::qStringToWCharPtr(trademark);
    wchar_t* modelBuf = typeUtils::qStringToWCharPtr(model);
    wchar_t* cardIdBuf = typeUtils::qStringToWCharPtr(cardId);

    wchar_t* resultBuf = m_fnActiveMainCard(nameBuf, addressBuf, phoneBuf, qqBuf, wxBuf,
        emailBuf, countryBuf, distributorBuf, trademarkBuf, modelBuf, cardIdBuf);
    QString result = QString::fromWCharArray(resultBuf);
    delete[] nameBuf;
    delete[] addressBuf;
    delete[] phoneBuf;
    delete[] qqBuf;
    delete[] wxBuf;
    delete[] emailBuf;
    delete[] countryBuf;
    delete[] distributorBuf;
    delete[] trademarkBuf;
    delete[] modelBuf;
    delete[] cardIdBuf;
    return result;
}

QString LaserDriver::getDeviceId(bool reload)
{
    wchar_t* result = m_fnGetDeviceId(reload);
    return QString::fromWCharArray(result);
}

QString LaserDriver::getDongleId()
{
    return QString::fromWCharArray(m_fnGetHardwareKeyID());
}

void LaserDriver::getMainCardRegisterState()
{
    m_fnGetMainCardRegState();
}

QString LaserDriver::getMainCardInfo()
{
    return QString::fromWCharArray(m_fnGetMainCardInfo());
}

bool LaserDriver::createLicenseFile(const QString& licenseCode)
{
    wchar_t* licBuf = typeUtils::qStringToWCharPtr(licenseCode);
    bool result = m_fnCreateLicenseFile(licBuf);
    delete[] licBuf;
    return result;
}

QVector3D LaserDriver::GetCurrentLaserPos()
{
    wchar_t* raw = m_fnGetCurrentLaserPos();
    QString posStr = QString::fromWCharArray(raw);
    QVector3D pos;
    QStringList posSegs = posStr.split(';', QString::SplitBehavior::SkipEmptyParts);
    Q_ASSERT(posSegs.length() == 3);
    pos.setX(posSegs[0].toFloat());
    pos.setY(posSegs[1].toFloat());
    pos.setZ(posSegs[2].toFloat());
    qDebug() << "Raw pos value:" << posStr;
    qDebug() << "Pos value:" << pos;
    return pos;
}

void LaserDriver::smallScaleMovement(bool fromZeroPoint, bool laserOn, char motorAxis, int deviation, int laserPower, int moveSpeed)
{
    m_fnSmallScaleMovement(fromZeroPoint, laserOn, motorAxis, deviation, laserPower, moveSpeed);
}

void LaserDriver::startMachining(bool zeroPointStyle)
{
    m_fnStartMachining(zeroPointStyle);
}

int LaserDriver::pauseContinueMachining(bool pause)
{
    return m_fnPauseContinueMachining(pause);
}

void LaserDriver::stopMachining()
{
    m_fnStopMachining();
}

int LaserDriver::controlMotor(bool open)
{
    return m_fnControlMotor(open);
}

int LaserDriver::testLaserLight(bool open)
{
    return m_fnTestLaserLight(open);
}

int LaserDriver::loadDataFromFile(const QString& filename, bool withMachining)
{
    int ret = 0;
    m_isWithMachining = withMachining;
    wchar_t* filenameBuf = typeUtils::qStringToWCharPtr(filename);
    m_isDownloading = true;
    m_packagesCount = m_fnLoadDataFromFile(filenameBuf);
    qDebug() << "packages of transformed data:" << m_packagesCount;
    delete[] filenameBuf;
    m_isDownloading = false;
    //emit machiningStarted();
    return ret;
}

void LaserDriver::getDeviceWorkState()
{
    m_fnGetDeviceWorkState();
}

void LaserDriver::setSystemRegister(LaserDriver::SystemRegisterType rt, QVariant value)
{
    m_registers[rt] = value;
}

bool LaserDriver::getSystemRegister(LaserDriver::SystemRegisterType rt, QVariant& value)
{
    if (m_registers.contains(rt))
    {
        value = m_registers[rt];
        return true;
    }
    return false;
}

//QString LaserDriver::registerComment(RegisterType rt)
//{
//    if (m_registerComments.contains(rt))
//        return m_registerComments[rt];
//    return QString("");
//}

bool LaserDriver::getLayout(float& width, float& height)
{
    if (!m_registers.contains(LaserDriver::RT_X_MAX_LENGTH) ||
        !m_registers.contains(LaserDriver::RT_Y_MAX_LENGTH))
        return false;

    width = m_registers[RT_X_MAX_LENGTH].toInt();
    height = m_registers[RT_Y_MAX_LENGTH].toInt();

    return true;
}

