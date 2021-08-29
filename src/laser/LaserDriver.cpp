#include "LaserDriver.h"

#include <QDataStream>
#include <QDebug>
#include <QErrorMessage>
#include <QFile>
#include <QMessageBox>

#include "LaserDevice.h"
#include "LaserRegister.h"
#include "exception/LaserException.h"
#include "state/StateController.h"
#include "util/Utils.h"
#include "util/TypeUtils.h"

#define CHECK_STR(fn) #fn
#define CHECK_FN(fn) \
    if (!fn) \
    { \
        qLogW << CHECK_STR(fn) << " is nullptr."; \
        return false; \
    }

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
    CHECK_FN(m_fnGetAPILibVersion)

    m_fnGetAPILibCompileInfo = (FN_WCHART_VOID)m_library.resolve("GetAPILibCompileInfo");
    CHECK_FN(m_fnGetAPILibCompileInfo)

    // TODO: 修改为SetLanguage
    m_fnSetLanguage = (FN_INT_INT)m_library.resolve("SetLanguge");
    CHECK_FN(m_fnSetLanguage)

    m_fnInitLib = (FN_BOOL_INT)m_library.resolve("InitLib");
    CHECK_FN(m_fnInitLib)

    m_fnUnInitLib = (FN_VOID_VOID)m_library.resolve("UnInitLib");
    CHECK_FN(m_fnUnInitLib)

    m_fnProgressCallBack = (FNProgressCallBack)m_library.resolve("ProgressCallBack");
    CHECK_FN(m_fnProgressCallBack)

    m_fnSysMessageCallBack = (FNSysMessageCallBack)m_library.resolve("SysMessageCallBack");
    CHECK_FN(m_fnSysMessageCallBack)

    //m_fnProcDataProgressCallBack = (FNProcDataProgressCallBack)m_library.resolve("ProcDataProgressCallBack");
    m_fnGetComPortList = (FN_WCHART_VOID)m_library.resolve("GetComPortList");
    CHECK_FN(m_fnGetComPortList)

    m_fnInitComPort = (FN_INT_INT)m_library.resolve("InitComPort");
    CHECK_FN(m_fnInitComPort)

    m_fnUnInitComPort = (FN_INT_VOID)m_library.resolve("UnInitComPort");
    CHECK_FN(m_fnUnInitComPort)

    m_fnSetTransTimeOutInterval = (FN_VOID_INT)m_library.resolve("SetTransTimeOutInterval");
    CHECK_FN(m_fnSetTransTimeOutInterval)

    m_fnSetSoftwareInitialization = (FNSetSoftwareInitialization)m_library.resolve("SetSoftwareInitialization");
    CHECK_FN(m_fnSetSoftwareInitialization)

    //m_fnSetRotateDeviceParam = (FNSetRotateDeviceParam)m_library.resolve("SetRotateDeviceParam");
    //CHECK_FN(m_fnSetRotateDeviceParam)

    m_fnSetHardwareInitialization = (FNSetHardwareInitialization)m_library.resolve("SetHardwareInitialization");
    CHECK_FN(m_fnSetHardwareInitialization)

    m_fnWriteSysParamToCard = (FN_INT_WCHART_WCHART)m_library.resolve("WriteSysParamToCard");
    CHECK_FN(m_fnWriteSysParamToCard)

    m_fnReadSysParamFromCard = (FN_INT_WCHART)m_library.resolve("ReadSysParamFromCard");
    CHECK_FN(m_fnReadSysParamFromCard)

    m_fnWriteUserParamToCard = (FN_INT_WCHART_WCHART)m_library.resolve("WriteUserParamToCard");
    CHECK_FN(m_fnWriteUserParamToCard)

    m_fnReadUserParamFromCard = (FN_INT_WCHART)m_library.resolve("ReadUserParamFromCard");
    CHECK_FN(m_fnReadUserParamFromCard)

    m_fnShowAboutWindow = (FN_INT_INT_BOOL)m_library.resolve("ShowAboutWindow");
    CHECK_FN(m_fnShowAboutWindow)

    m_fnCloseAboutWindow = (FN_VOID_VOID)m_library.resolve("CloseAboutWindow");
    //CHECK_FN(m_fnCloseAboutWindow)

    m_fnGetLaserLibInfo = (FN_VOID_VOID)m_library.resolve("GetLaserLibInfo");
    CHECK_FN(m_fnGetLaserLibInfo)

    m_fnSetFactoryType = (FN_VOID_WCHART)m_library.resolve("SetFactoryType");
    CHECK_FN(m_fnSetFactoryType)

    //m_fnCheckFactoryPassword = (FN_INT_WCHART)m_library.resolve("CheckFactroyPassWord");
    //CHECK_FN(m_fnCheckFactoryPassword)

    m_fnWriteFactoryPassword = (FN_INT_WCHART_WCHART)m_library.resolve("WriteFactoryPassWord");
    CHECK_FN(m_fnWriteFactoryPassword)

    m_fnLPenMoveToOriginalPoint = (FN_VOID_DOUBLE)m_library.resolve("LPenMoveToOriginalPoint");
    CHECK_FN(m_fnLPenMoveToOriginalPoint)

    m_fnLPenQuickMoveTo = (FNLPenQuickMoveTo)m_library.resolve("LPenQuickMoveTo");
    CHECK_FN(m_fnLPenQuickMoveTo)

    m_fnControlHDAction = (FN_VOID_INT)m_library.resolve("ControlHDAction");
    CHECK_FN(m_fnControlHDAction)

    m_fnGetMainCardID = (FN_WCHART_VOID)m_library.resolve("GetMainCardID");
    CHECK_FN(m_fnGetMainCardID)

    m_fnActiveMainCard = (FNActivationMainCard)m_library.resolve("ActivationMainCard");
    CHECK_FN(m_fnActiveMainCard)

    m_fnGetDeviceId = (FN_WCHART_BOOL)m_library.resolve("GetDeviceID");
    CHECK_FN(m_fnGetDeviceId)

    m_fnGetHardwareKeyID = (FN_WCHART_VOID)m_library.resolve("GetHardwareKeyID");
    CHECK_FN(m_fnGetHardwareKeyID)

    m_fnGetMainCardRegState = (FN_VOID_VOID)m_library.resolve("GetMainCardRegState");
    CHECK_FN(m_fnGetMainCardRegState)

    m_fnGetMainCardInfo = (FN_WCHART_VOID)m_library.resolve("GetMainCardInfo");
    CHECK_FN(m_fnGetMainCardInfo)

    m_fnCreateLicenseFile = (FN_BOOL_WCHART)m_library.resolve("CreateLicenseFile");
    CHECK_FN(m_fnCreateLicenseFile)

    m_fnGetCurrentLaserPos = (FN_WCHART_VOID)m_library.resolve("GetCurrentLaserPos");
    CHECK_FN(m_fnGetCurrentLaserPos)

    m_fnSmallScaleMovement = (FNSmallScaleMovement)m_library.resolve("SmallScaleMovement");
    CHECK_FN(m_fnSmallScaleMovement)

    m_fnStartMachining = (FN_VOID_BOOL)m_library.resolve("StartMachining");
    CHECK_FN(m_fnStartMachining)

    m_fnPauseContinueMachining = (FN_INT_BOOL)m_library.resolve("PauseContinueMachining");
    CHECK_FN(m_fnPauseContinueMachining)

    m_fnStopMachining = (FN_VOID_VOID)m_library.resolve("StopMachining");
    CHECK_FN(m_fnStopMachining)

    m_fnControlMotor = (FN_INT_BOOL)m_library.resolve("ControlMotor");
    CHECK_FN(m_fnControlMotor)

    m_fnTestLaserLight = (FN_INT_BOOL)m_library.resolve("TestLaserLight");
    CHECK_FN(m_fnTestLaserLight)

    m_fnLoadDataFromFile = (FN_INT_WCHART)m_library.resolve("LoadDataFromFile");
    CHECK_FN(m_fnLoadDataFromFile)

    m_fnGetDeviceWorkState = (FN_VOID_VOID)m_library.resolve("GetDeviceWorkState");
    CHECK_FN(m_fnGetDeviceWorkState)

    m_fnMillimeter2MicroStep = (FN_INT_DOUBLE_BOOL)m_library.resolve("Millimeter2MicroStep");
    CHECK_FN(m_fnMillimeter2MicroStep)

    m_fnCheckVersionUpdate = (FN_BOOL_WCHART_INT_WCHART)m_library.resolve("CheckVersionUpdate");
    CHECK_FN(m_fnCheckVersionUpdate)

    m_fnGetUpdatePanelHandle = (FN_INT_INT_INT)m_library.resolve("GetUpdatePanelHandle");
    CHECK_FN(m_fnGetUpdatePanelHandle)

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

int LaserDriver::setLanguage(int lang)
{
    return m_fnSetLanguage(lang);
}

bool LaserDriver::init(int winId)
{
    try 
    {
        m_fnInitLib(winId);
        emit libraryInitialized();
    }
    catch (...)
    {
        return false;
    }
    return true;
}

void LaserDriver::setupCallbacks()
{
    m_fnProgressCallBack(LaserDriver::ProgressCallBackHandler);
    m_fnSysMessageCallBack(LaserDriver::SysMessageCallBackHandler);
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

//void LaserDriver::setRotateDeviceParam(int type, int perimeterPulse, int materialPerimeter, int deviceDPI, bool autoScaleDimensions)
//{
//    m_fnSetRotateDeviceParam(type, perimeterPulse, materialPerimeter, deviceDPI, autoScaleDimensions);
//}

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
        QString value = i.value().toString();
        if (i.value().type() == QVariant::Bool)
            value = i.value().toBool() ? "1" : "0";
        valuesList.append(value);
    }
    addrBuf = addrList.join(",");
    valuesBuf = valuesList.join(",");

    wchar_t* wcAddrs = typeUtils::qStringToWCharPtr(addrBuf);
    wchar_t* wcValues = typeUtils::qStringToWCharPtr(valuesBuf);

    qDebug() << "writing address list: " << addrBuf;
    qDebug() << "writing values list: " << valuesBuf;

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
    for (int i = 0; i <= 57; i++)
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
        QString value = i.value().toString();
        if (i.value().type() == QVariant::Bool)
            value = i.value().toBool() ? "1" : "0";
        valuesList.append(value);
    }
    addrBuf = addrList.join(",");
    valuesBuf = valuesList.join(",");

    wchar_t* wcAddrs = typeUtils::qStringToWCharPtr(addrBuf);
    wchar_t* wcValues = typeUtils::qStringToWCharPtr(valuesBuf);

    qDebug() << "writing address list: " << addrBuf;
    qDebug() << "writing values list: " << valuesBuf;

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
    for (int i = 0; i <= 31; i++)
    {
        params << i;
    }
    return readUserParamFromCard(params);

}

int LaserDriver::showAboutWindow(int interval, bool modal)
{
    //return m_fnShowAboutWindow(interval, modal);
    return 0;
}

void LaserDriver::closeAboutWindow()
{
    //if (m_fnCloseAboutWindow)
        //m_fnCloseAboutWindow();
}

QString LaserDriver::getLaserLibraryInfo()
{
    wchar_t* strInfo = m_fnGetMainCardID();
    QString info = QString::fromWCharArray(strInfo);
    return info;
}

void LaserDriver::setFactoryType(const QString& factory)
{
    wchar_t* strFactory = typeUtils::qStringToWCharPtr(factory);
    m_fnSetFactoryType(strFactory);
    delete[] strFactory;
}

//bool LaserDriver::checkFactoryPassword(const QString& password)
//{
//    wchar_t* wcPassword = typeUtils::qStringToWCharPtr(password);
//    bool success = m_fnCheckFactoryPassword(wcPassword) != -1;
//    delete[] wcPassword;
//    return success;
//}

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
    wchar_t* strId = m_fnGetMainCardID();
    QString id = QString::fromWCharArray(strId);
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

void LaserDriver::checkVersionUpdate(bool hardware, const QString& flag, int currentVersion, const QString& versionNoteToJsonFile)
{
    wchar_t* flagBuf = typeUtils::qStringToWCharPtr(flag);
    wchar_t* vntjfBuf = typeUtils::qStringToWCharPtr(versionNoteToJsonFile);
    m_fnCheckVersionUpdate(hardware, flagBuf, currentVersion, vntjfBuf);
    delete[] flagBuf;
    delete[] vntjfBuf;
}

int LaserDriver::getUpdatePanelHandle(int version, int wndId)
{
    return m_fnGetUpdatePanelHandle(version, wndId);
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

