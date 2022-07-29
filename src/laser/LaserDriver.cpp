#include "LaserDriver.h"

#include <QDataStream>
#include <QDebug>
#include <QErrorMessage>
#include <QFile>
#include <QMessageBox>
#include <QTextCodec>

#include "LaserDevice.h"
#include "LaserRegister.h"
#include "exception/LaserException.h"
#include "state/StateController.h"
#include "util/Utils.h"
#include "util/TypeUtils.h"
#include "LaserApplication.h"

#define CHECK_STR(fn) #fn
#define CHECK_FN(fn) \
    if (!fn) \
    { \
        qLogW << CHECK_STR(fn) << " is nullptr."; \
    }

LaserDriver::LaserDriver(QObject* parent)
    : QObject(parent)
    , m_isLoaded(false)
    , m_isConnected(false)
    , m_isMachining(false)
    , m_isPaused(false)
    , m_portName("")
    , m_parentWidget(nullptr)
    , m_packagesCount(0)
    , m_isClosed(false)
{
}

LaserDriver::~LaserDriver()
{
    qLogD << "driver destroyed";
}

void LaserDriver::ProgressCallBackHandler(int position, int totalCount)
{
    if (LaserApplication::driver->m_isClosed)
        return;

    float progress = position * 1.0f / totalCount;
    qDebug() << "Progress callback handler: position = " << position << ", totalCount = " << totalCount << ", progress = " << QString("%1%").arg(static_cast<double>(progress * 100), 3, 'g', 4);
    emit LaserApplication::driver->progress(position, totalCount, progress);
}

void LaserDriver::SysMessageCallBackHandler(int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData)
{
    static QTextCodec* codecGb = QTextCodec::codecForName("GBK");
    static QTextCodec* codecUtf16 = QTextCodec::codecForName("UTF-16");
    static QTextCodec* codecUtf8 = QTextCodec::codecForName("UTF-8");
    if (!LaserApplication::driver)
        return;

    if (LaserApplication::driver->m_isClosed)
        return;

    QString eventData;
    //QTextCodec* codec = QTextCodec::codecForLocale();
    //QTextCodec::setCodecForLocale(codecGb);
    eventData = QString::fromWCharArray(sysEventData);
    qLogD << "System message callback handler: index = " << sysMsgIndex << ", code = " << sysMsgCode << ", event data = " << eventData;
    //QTextCodec::setCodecForLocale(codec);
    if (sysMsgCode >= E_Base && sysMsgCode < M_Base)
    {
        emit LaserApplication::driver->raiseError(sysMsgCode, eventData);
        //LaserApplication::device->handleError(sysMsgCode, eventData);
    }
    else if (sysMsgCode >= M_Base)
    {
        emit LaserApplication::driver->sendMessage(sysMsgCode, eventData);
        //LaserApplication::device->handleMessage(sysMsgCode, eventData);
    }
}

void LaserDriver::ProcDataProgressCallBackHandler(int position, int totalCount)
{
    float progress = position * 1.0f / totalCount;
    qDebug() << "Proc progress callback handler:" << position << totalCount << QString("%1%").arg(static_cast<double>(progress * 100), 3, 'g', 4);

}

bool LaserDriver::load()
{
    if (m_isLoaded)
        return true;

    qRegisterMetaType<DeviceState>("DeviceState");
    QString libName = "LaserLib32.dll";
#ifdef ARCH_x64
    libName = "LaserLib64.dll";
#endif
    m_library.setFileName(libName);
    if (!m_library.load())
    {
        qDebug() << "load LaserLib failure:" << m_library.errorString();
        return false;
    }

    m_fnGetAPILibVersion = (FN_WCHART_VOID)m_library.resolve("GetAPILibVersion");
    CHECK_FN(m_fnGetAPILibVersion)

    m_fnGetAPILibCompileInfo = (FN_WCHART_VOID)m_library.resolve("GetAPILibCompileInfo");
    CHECK_FN(m_fnGetAPILibCompileInfo)

    m_fnGetLanguage = (FN_INT_VOID)m_library.resolve("GetLanguage");
    CHECK_FN(m_fnGetLanguage)

    m_fnSetLanguage = (FN_INT_INT)m_library.resolve("SetLanguage");
    CHECK_FN(m_fnSetLanguage)

    m_fnInitLib = (FN_BOOL_INT)m_library.resolve("InitLib");
    CHECK_FN(m_fnInitLib)

    m_fnUnInitLib = (FN_VOID_VOID)m_library.resolve("UnInitLib");
    CHECK_FN(m_fnUnInitLib)

    m_fnProgressCallBack = (FNProgressCallBack)m_library.resolve("ProgressCallBack");
    CHECK_FN(m_fnProgressCallBack)

    m_fnSysMessageCallBack = (FNSysMessageCallBack)m_library.resolve("SysMessageCallBack");
    CHECK_FN(m_fnSysMessageCallBack)

    //m_fnGetComPortList = (FN_WCHART_VOID)m_library.resolve("GetComPortListEx");
    m_fnGetComPortList = (FN_WCHART_VOID)m_library.resolve("GetComPortList");
    CHECK_FN(m_fnGetComPortList)

    m_fnInitComPort = (FN_INT_INT)m_library.resolve("InitComPort");
    CHECK_FN(m_fnInitComPort)

    m_fnUnInitComPort = (FN_INT_VOID)m_library.resolve("UnInitComPort");
    CHECK_FN(m_fnUnInitComPort)

    m_fnWriteSysParamToCard = (FN_INT_WCHART_WCHART)m_library.resolve("WriteSysParamToCard");
    CHECK_FN(m_fnWriteSysParamToCard)

    m_fnReadSysParamFromCard = (FN_INT_WCHART)m_library.resolve("ReadSysParamFromCard");
    CHECK_FN(m_fnReadSysParamFromCard)

    m_fnSaveMainBoardParamsToServer = (FN_VOID_VOID)m_library.resolve("SaveMainBoardParamsToServer");
    CHECK_FN(m_fnSaveMainBoardParamsToServer)

    m_fnReadMainBoardParamsFromServer = (FN_VOID_VOID)m_library.resolve("ReadMainBoardParamsFromServer");
    CHECK_FN(m_fnReadMainBoardParamsFromServer)

    m_fnWriteUserParamToCard = (FN_INT_WCHART_WCHART)m_library.resolve("WriteUserParamToCard");
    CHECK_FN(m_fnWriteUserParamToCard)

    m_fnReadUserParamFromCard = (FN_INT_WCHART)m_library.resolve("ReadUserParamFromCard");
    CHECK_FN(m_fnReadUserParamFromCard)

    m_fnWriteComputerParamToCard = (FN_INT_WCHART_WCHART)m_library.resolve("WriteComputerParamToCard");
    CHECK_FN(m_fnWriteComputerParamToCard)

    m_fnReadComputerParamFromCard = (FN_INT_WCHART)m_library.resolve("ReadComputerParamFromCard");
    CHECK_FN(m_fnReadComputerParamFromCard)

    m_fnShowAboutWindow = (FN_INT_INT_BOOL)m_library.resolve("ShowAboutWindow");
    CHECK_FN(m_fnShowAboutWindow)

    m_fnCloseAboutWindow = (FN_VOID_VOID)m_library.resolve("CloseAboutWindow");
    CHECK_FN(m_fnCloseAboutWindow)

    m_fnGetLaserLibInfo = (FN_VOID_VOID)m_library.resolve("GetLaserLibInfo");
    CHECK_FN(m_fnGetLaserLibInfo)

    m_fnShowLoaddingInfo = (FN_VOID_WCHART)m_library.resolve("ShowLoaddingInfo");
    CHECK_FN(m_fnShowLoaddingInfo)

    m_fnSetFactoryType = (FN_VOID_WCHART)m_library.resolve("SetFactoryType");
    CHECK_FN(m_fnSetFactoryType)

    m_fnSetTransTimeOutInterval = (FN_VOID_INT)m_library.resolve("SetTransTimeOutInterval");
    CHECK_FN(m_fnSetTransTimeOutInterval)

    m_fnOpenDetailedLog = (FN_VOID_BOOL)m_library.resolve("OpenDetailedLog");
    CHECK_FN(m_fnOpenDetailedLog)

    m_fnDebugLogger = (FN_BOOL_BOOL)m_library.resolve("DebugLogger");
    CHECK_FN(m_fnDebugLogger)

    m_fnCheckFactoryPassword = (FN_BOOL_WCHART_INTREF)m_library.resolve("CheckFactoryPassword");
    CHECK_FN(m_fnCheckFactoryPassword)

    m_fnChangeFactoryPassword = (FN_INT_WCHART_WCHART)m_library.resolve("ChangeFactoryPassword");
    CHECK_FN(m_fnChangeFactoryPassword)

    m_fnSaveUStepLength = (FN_VOID_INT)m_library.resolve("SaveUstepLength");
    CHECK_FN(m_fnSaveUStepLength)

    m_fnGetClientAddr = (FN_WCHART_BOOL)m_library.resolve("GetClientAddr");
    CHECK_FN(m_fnGetClientAddr)

    m_fnLPenMoveToOriginalPoint = (FN_VOID_DOUBLE)m_library.resolve("LPenMoveToOriginalPoint");
    CHECK_FN(m_fnLPenMoveToOriginalPoint)

    m_fnLPenQuickMoveTo = (FNLPenQuickMoveTo)m_library.resolve("LPenQuickMoveTo");
    CHECK_FN(m_fnLPenQuickMoveTo)

    m_fnCheckMoveLaserMotors = (FNCheckMoveLaserMotors)m_library.resolve("CheckMoveLaserMotors");
    CHECK_FN(m_fnCheckMoveLaserMotors)

    m_fnStartMoveLaserMotors = (FN_VOID_VOID)m_library.resolve("StartMoveLaserMotors");
    CHECK_FN(m_fnStartMoveLaserMotors)

    m_fnGetMainHardVersion = (FN_WCHART_VOID)m_library.resolve("GetMainHardVersion");
    CHECK_FN(m_fnGetMainHardVersion);

    m_fnGetHardwareIdentID = (FN_WCHART_VOID)m_library.resolve("GetHardwareIdentID");
    CHECK_FN(m_fnGetHardwareIdentID);

    m_fnGetMainCardID = (FN_WCHART_VOID)m_library.resolve("GetMainCardID");
    CHECK_FN(m_fnGetMainCardID)

    m_fnActiveMainCard = (FNActivationMainCard)m_library.resolve("ActivationMainCard");
    CHECK_FN(m_fnActiveMainCard)

    m_fnGetDeviceId = (FN_WCHART_BOOL)m_library.resolve("GetDeviceID");
    CHECK_FN(m_fnGetDeviceId)

    m_fnGetHardwareKeyInfo = (FN_WCHART_VOID)m_library.resolve("GetHardwareKeyInfo");
    CHECK_FN(m_fnGetHardwareKeyInfo)

    m_fnGetHardwareKeyID = (FN_WCHART_VOID)m_library.resolve("GetHardwareKeyID");
    CHECK_FN(m_fnGetHardwareKeyID)

    m_fnGetMainCardRegState = (FN_VOID_VOID)m_library.resolve("GetMainCardRegState");
    CHECK_FN(m_fnGetMainCardRegState)

    m_fnGetMainCardInfo = (FN_WCHART_VOID)m_library.resolve("GetMainCardInfo");
    CHECK_FN(m_fnGetMainCardInfo)

    m_fnGetMainHardModal = (FN_WCHART_VOID)m_library.resolve("GetMainHardModal");
    CHECK_FN(m_fnGetMainHardModal)

    m_fnGetHardWareKeyType = (FN_INT_INT16)m_library.resolve("GetHardWareKeyType");
    CHECK_FN(m_fnGetHardWareKeyType)

    m_fnCreateLicenseFile = (FN_BOOL_WCHART)m_library.resolve("CreateLicenseFile");
    CHECK_FN(m_fnCreateLicenseFile)

    m_fnGetCurrentLaserPos = (FN_WCHART_VOID)m_library.resolve("GetCurrentLaserPos");
    CHECK_FN(m_fnGetCurrentLaserPos)

    m_fnStartMachining = (FN_VOID_INT)m_library.resolve("StartMachining");
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

    m_fnStartDownLoadToCache = (FN_VOID_LONG)m_library.resolve("StartDownLoadToCache");
    CHECK_FN(m_fnStartDownLoadToCache)

    m_fnGetDeviceWorkState = (FN_VOID_VOID)m_library.resolve("GetDeviceWorkState");
    CHECK_FN(m_fnGetDeviceWorkState)

    m_fnChangeYorUaxis = (FN_VOID_BOOL)m_library.resolve("ChangeYorUaxis");
    CHECK_FN(m_fnChangeYorUaxis)

    m_fnCheckVersionUpdate = (FN_BOOL_WCHART_INT_WCHART)m_library.resolve("CheckVersionUpdate");
    CHECK_FN(m_fnCheckVersionUpdate)

    m_fnStartVersionUpdate = (FNStartVersionUpdate)m_library.resolve("StartVersionUpdate");
    CHECK_FN(m_fnStartVersionUpdate)

    m_fnAbortVersionUpdate = (FN_INT_VOID)m_library.resolve("AbortVersionUpdate");
    CHECK_FN(m_fnAbortVersionUpdate)

    m_fnStartSoftUpdateWizard = (FN_VOID_VOID)m_library.resolve("StartSoftUpdateWizard");
    CHECK_FN(m_fnStartSoftUpdateWizard)

    m_fnStartFirmwareUpdateWizard = (FN_VOID_VOID)m_library.resolve("StartFirmwareUpdateWizard");
    CHECK_FN(m_fnStartFirmwareUpdateWizard)

    m_fnActivationMainCardEx = (FN_INT_WCHART)m_library.resolve("ActivationMainCardEx");
    CHECK_FN(m_fnActivationMainCardEx);

    m_fnRegisterMainCard = (FN_WCHART_WCHART_WCHART)m_library.resolve("RegisterMainCard");
    CHECK_FN(m_fnRegisterMainCard)

    m_fnSendAuthenticationEmail = (FN_INT_WCHART_WCHART_INT)m_library.resolve("SendAuthenticationEmail");
    CHECK_FN(m_fnSendAuthenticationEmail)

    m_fnImportData = (FN_INT_BYTEPTR_INT)m_library.resolve("ImportData");
    CHECK_FN(m_fnImportData)

    m_fnDrawRectangularBorder = (FN_INT_BYTEPTR_INT)m_library.resolve("DrawRectangularBorderA");
    CHECK_FN(m_fnDrawRectangularBorder)

    m_fnLPenMoveToOriginalPointZ = (FN_VOID_INT)m_library.resolve("LPenMoveToOriginalPointZ");
    CHECK_FN(m_fnLPenMoveToOriginalPointZ)

    Q_ASSERT(m_fnLoadDataFromFile);

    m_isLoaded = true;
    return true;
}

void LaserDriver::unload()
{
    //if (m_isLoaded)
        //m_fnUnInitLib();
    m_isLoaded = false;
    m_isClosed = true;
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
}

QStringList LaserDriver::getPortList()
{
    wchar_t* str = m_fnGetComPortList();
    QString portList = QString::fromWCharArray(str);
    QStringList portNames = portList.split(";");
    qLogD << "portNames: " << portNames;

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
        if (i.key() == 24)
            qLogD << "debugging reg: " << i.key() << ", " << i.value();
        QString value = i.value().toString();
        switch (i.value().type())
        {
        case QVariant::Bool:
            value = i.value().toBool() ? "1" : "0";
            break;
        case QVariant::Int:
            value = QString("%1").arg(i.value().toInt());
            break;
        case QVariant::UInt:
            value = QString("%1").arg(i.value().toUInt());
            break;
        case QVariant::LongLong:
            value = QString("%1").arg(i.value().toLongLong());
            break;
        case QVariant::ULongLong:
            value = QString("%1").arg(i.value().toULongLong());
            break;
        case QVariant::Double:
            value = QString("%1").arg(i.value().toDouble(), 0, 'f');
            break;
        }
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
    for (int i = 0; i <= 79; i++)
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
    for (int i = 0; i <= 44; i++)
    {
        params << i;
    }
    return readUserParamFromCard(params);

}

bool LaserDriver::writeExternalParamToCard(const LaserRegister::RegistersMap& values)
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

    bool success = m_fnWriteComputerParamToCard(wcAddrs, wcValues) != -1;
    delete[] wcAddrs;
    delete[] wcValues;
    return success;
}

bool LaserDriver::readExternalParamFromCard(QList<int> addresses)
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

    bool success = m_fnReadComputerParamFromCard(addrBuf) != -1;
    delete[] addrBuf;
    return success;
}

bool LaserDriver::readAllExternalParamFromCard()
{
    QList<int> params;
    for (int i = 0; i <= 49; i++)
    {
        params << i;
    }
    return readExternalParamFromCard(params);

}

int LaserDriver::getHardwareKeyType(qint16 type)
{
    return m_fnGetHardWareKeyType(type);
}

int LaserDriver::showAboutWindow(int interval, bool modal)
{
    return m_fnShowAboutWindow(interval, modal);
    //return 0;
}

void LaserDriver::closeAboutWindow()
{
    if (m_fnCloseAboutWindow)
        m_fnCloseAboutWindow();
}

QString LaserDriver::getLaserLibraryInfo()
{
    wchar_t* strInfo = m_fnGetMainCardID();
    //QString info = QString::fromWCharArray(strInfo);
    QString info = QString::fromWCharArray(strInfo);
    return info;
}

void LaserDriver::setFactoryType(const QString& factory)
{
    wchar_t* strFactory = typeUtils::qStringToWCharPtr(factory);
    m_fnSetFactoryType(strFactory);
    delete[] strFactory;
}

bool LaserDriver::checkFactoryPassword(const QString& password, int& errorCount)
{
    wchar_t* wcPassword = typeUtils::qStringToWCharPtr(password);
    bool success = m_fnCheckFactoryPassword(wcPassword, &errorCount);
    delete[] wcPassword;
    return success;
}

bool LaserDriver::changeFactoryPassword(const QString& oldPassword, const QString& newPassword)
{
    wchar_t* wcOldPassword = typeUtils::qStringToWCharPtr(oldPassword);
    wchar_t* wcNewPassword = typeUtils::qStringToWCharPtr(newPassword);
    bool success = m_fnChangeFactoryPassword(wcOldPassword, wcNewPassword) == 0;
    delete[] wcOldPassword;
    delete[] wcNewPassword;
    return success;
}

void LaserDriver::lPenMoveToOriginalPoint(double speed)
{
    m_fnLPenMoveToOriginalPoint(speed);
}

void LaserDriver::lPenQuickMoveTo(
        bool xMoveEnable,
        bool xMoveStyle,
        int xPos,
        bool yMoveEnable,
        bool yMoveStyle,
        int yPos,
        bool zMoveEnable,
        bool zMoveStyle,
        int zPos,
        bool uMoveEnable,
        bool uMoveStyle,
        int uPos)
{
    m_fnLPenQuickMoveTo(
        xMoveEnable, xMoveStyle, xPos,
        yMoveEnable, yMoveStyle, yPos,
        zMoveEnable, zMoveStyle, zPos,
        uMoveEnable, uMoveStyle, uPos
        );
}

void LaserDriver::checkMoveLaserMotors(quint16 delay,
    bool xMoveEnable, bool xMoveStyle, int xPos, 
    bool yMoveEnable, bool yMoveStyle, int yPos, 
    bool zMoveEnable, bool zMoveStyle, int zPos,
    bool uMoveEnable, bool uMoveStyle, int uPos)
{
    qLogD << "move " << xPos << ", " << yPos << ", " << zPos;
    qLogD << "enabled " << xMoveEnable << ", " << yMoveEnable << ", " << zMoveEnable;
    m_fnCheckMoveLaserMotors(
        delay,
        xMoveEnable, xMoveStyle, xPos,
        yMoveEnable, yMoveStyle, yPos,
        zMoveEnable, zMoveStyle, zPos,
        uMoveEnable, uMoveStyle, uPos
        );
}

void LaserDriver::startMoveLaserMotors()
{
    m_fnStartMoveLaserMotors();
}

QString LaserDriver::firmwareVersion()
{
    wchar_t* strId = m_fnGetMainHardVersion();
    //QString id = QString::fromWCharArray(strId);
    QString id = QString::fromWCharArray(strId);
    return id;
}

QString LaserDriver::getMainCardID()
{
    wchar_t* strId = m_fnGetMainCardID();
    //QString id = QString::fromWCharArray(strId);
    QString id = QString::fromWCharArray(strId);
    return id;
}

QString LaserDriver::getHardwareIdentID()
{
    wchar_t* strId = m_fnGetHardwareIdentID();
    QString id = QString::fromWCharArray(strId);
    return id;
}

MainCardActivateResult LaserDriver::autoActiveMainCard()
{
    int result = m_fnActivationMainCardEx(L"{3567D29E-394B-4814-80C4-510331CD39CD}");
    if (result == 0)
        return MAR_Activated;
    else if (result < 0)
    {
        qLogW << "auto activae main card error: " << result;
        return MAR_Error;
    }
    else if (result == 1)
    {
        qLogW << "auto activae main card failure: inactivated";
        return MAR_Inactivated;
    }
    return MAR_Other;
}

bool LaserDriver::sendAuthenticationEmail(const QString& email)
{
    wchar_t* emailBuf = typeUtils::qStringToWCharPtr(email);
    return m_fnSendAuthenticationEmail(
        L"{464D4EDA-2645-4427-AA92-80D53231089F}",
        emailBuf, -1);
}

QString LaserDriver::activateMainCard(
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
    const QString& model,
    const QString& machineId
)
{
    wchar_t* emailBuf = typeUtils::qStringToWCharPtr(email);
    wchar_t* codeBuf = typeUtils::qStringToWCharPtr(code);
    wchar_t* nameBuf = typeUtils::qStringToWCharPtr(name);
    wchar_t* phoneBuf = typeUtils::qStringToWCharPtr(phone);
    wchar_t* addressBuf = typeUtils::qStringToWCharPtr(address);
    wchar_t* qqBuf = typeUtils::qStringToWCharPtr(qq);
    wchar_t* wxBuf = typeUtils::qStringToWCharPtr(wx);
    wchar_t* countryBuf = typeUtils::qStringToWCharPtr(country);
    wchar_t* distributorBuf = typeUtils::qStringToWCharPtr(distributor);
    wchar_t* trademarkBuf = typeUtils::qStringToWCharPtr(brand);
    wchar_t* modelBuf = typeUtils::qStringToWCharPtr(model);
    wchar_t* cardIdBuf = typeUtils::qStringToWCharPtr(machineId);

    wchar_t* resultBuf = m_fnActiveMainCard(
        emailBuf, codeBuf, nameBuf, phoneBuf, addressBuf, qqBuf, wxBuf,
        countryBuf, distributorBuf, trademarkBuf, modelBuf, cardIdBuf);
    QString result = QString::fromWCharArray(resultBuf);
    delete[] emailBuf;
    delete[] codeBuf;
    delete[] nameBuf;
    delete[] phoneBuf;
    delete[] addressBuf;
    delete[] qqBuf;
    delete[] wxBuf;
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
    wchar_t* str = m_fnGetHardwareKeyID();
    return QString::fromWCharArray(str);
}

void LaserDriver::getMainCardRegisterState()
{
    m_fnGetMainCardRegState();
}

QString LaserDriver::getMainCardInfo()
{
    wchar_t* str = m_fnGetMainCardInfo();
    return QString::fromWCharArray(str);
}

QString LaserDriver::getMainHardModal()
{
    wchar_t* str = m_fnGetMainHardModal();
    return QString::fromWCharArray(str);
}

bool LaserDriver::createLicenseFile(const QString& licenseCode)
{
    wchar_t* licBuf = typeUtils::qStringToWCharPtr(licenseCode);
    bool result = m_fnCreateLicenseFile(licBuf);
    delete[] licBuf;
    return result;
}

QVector3D LaserDriver::getCurrentLaserPos()
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

void LaserDriver::startMachining(int packIndex)
{
    m_fnStartMachining(packIndex);
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
    m_packagesCount = m_fnLoadDataFromFile(filenameBuf);
    qDebug() << "packages of transformed data:" << m_packagesCount;
    delete[] filenameBuf;
    return ret;
}

void LaserDriver::download(unsigned long index)
{
    m_fnStartDownLoadToCache(index);
}

int LaserDriver::importData(const char* data, int length)
{
    return m_fnImportData(const_cast<char*>(data), length);
}

int LaserDriver::drawBounding(const char* data, int length)
{
    return m_fnDrawRectangularBorder(const_cast<char*>(data), length);
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

void LaserDriver::startSoftUpdateWizard()
{
    m_fnStartSoftUpdateWizard();
}

void LaserDriver::startFirmwareUpdateWizard()
{
    m_fnStartFirmwareUpdateWizard();
}

void LaserDriver::lPenMoveToOriginalPointZ(int moveTo)
{
    m_fnLPenMoveToOriginalPointZ(moveTo);
}

QString LaserDriver::registerMainCard(const QString& registeCode, const QString& password)
{
    wchar_t* registeCodeBuf = typeUtils::qStringToWCharPtr(registeCode);
    wchar_t* passwordBuf = typeUtils::qStringToWCharPtr(password);
    wchar_t* returnBuf = m_fnRegisterMainCard(registeCodeBuf, passwordBuf);
    QString returnRegisteCode = QString::fromWCharArray(returnBuf);
    delete[] registeCodeBuf;
    delete[] passwordBuf;
    return returnRegisteCode;
}

void LaserDriver::openDetailedLog(bool isOut)
{
    m_fnOpenDetailedLog(isOut);
}

void LaserDriver::debugLogger(bool enabled)
{
    m_fnDebugLogger(enabled);
}

