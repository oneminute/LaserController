#include "LaserDriver.h"

#include <QDebug>

LaserDriver::LaserDriver(QObject* parent)
    : QObject(parent)
{

}

LaserDriver::~LaserDriver()
{
}

LaserDriver & LaserDriver::instance()
{
    static LaserDriver driver;
    return driver;
}

void LaserDriver::ProgressCallBackHandler(int position, int totalCount)
{
    qDebug() << "Progress callback handler:" << position << totalCount;
}

void LaserDriver::SysMessageCallBackHandler(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t * sysEventData)
{
    QString eventData = QString::fromWCharArray(sysEventData);
    qDebug() << "System message callback handler:" << sysMsgIndex << sysMsgCode << eventData;
}

void LaserDriver::ProcDataProgressCallBackHandler(int position, int totalCount)
{
    qDebug() << "Proc progress callback handler:" << position << totalCount;
}

bool LaserDriver::load()
{
    if (m_isLoaded)
        return true;

    m_library.setFileName("LaserLib32.dll");
    if (!m_library.load())
    {
        qDebug() << "load LaserLib failure:" << m_library.errorString();
        return false;
    }

    m_fnGetAPILibVersion = (FN_WCHART_VOID)m_library.resolve("GetAPILibVersion");
    m_fnGetAPILibCompileInfo = (FN_WCHART_VOID)m_library.resolve("GetAPILibCompileInfo");
    m_fnInitLib = (FN_VOID_INT)m_library.resolve("InitLib");
    m_fnUnInitLib = (FN_VOID_VOID)m_library.resolve("UnInitLib");
    m_fnProgressCallBack = (FNProgressCallBack)m_library.resolve("ProgressCallBack");
    m_fnSysMessageCallBack = (FNSysMessageCallBack)m_library.resolve("SysMessageCallBack");
    m_fnProcDataProgressCallBack = (FNProcDataProgressCallBack)m_library.resolve("ProcDataProgressCallBack");
    m_fnGetComPortList = (FN_WCHART_VOID)m_library.resolve("GetComPortList");

    m_fnProgressCallBack(LaserDriver::ProgressCallBackHandler);
    m_fnSysMessageCallBack(LaserDriver::SysMessageCallBackHandler);
    m_fnProcDataProgressCallBack(LaserDriver::ProcDataProgressCallBackHandler);

    m_fnInitComPort = (FN_INT_INT)m_library.resolve("InitComPort");
    m_fnUnInitComPort = (FN_INT_VOID)m_library.resolve("UnInitComPort");

    m_fnSetTRansTimeOutInterval = (FN_VOID_INT)m_library.resolve("SetTransTimeOutInterval");
    m_fnSetSoftwareInitialization = (FNSetSoftwareInitialization)m_library.resolve("SetSoftwareInitialization");
    m_fnSetRotateDeviceParam = (FNSetRotateDeviceParam)m_library.resolve("SetRotateDeviceParam");
    m_fnSetHardwareInitialization = (FNSetHardwareInitialization)m_library.resolve("SetHardwareInitialization");

    m_fnWriteSysParamToCard = (FN_INT_WCHART_WCHART)m_library.resolve("WriteSysParamToCard");
    m_fnReadSysParamFromCard = (FN_INT_WCHART)m_library.resolve("ReadSysParamFromCard");
    m_fnShowAboutWindow = (FN_VOID_VOID)m_library.resolve("ShowAboutWindow");

    m_fnLPenMoveToOriginalPoint = (FN_VOID_DOUBLE)m_library.resolve("LPenMoveToOriginalPoint");
    m_fnLPenQuickMoveTo = (FNLPenQuickMoveTo)m_library.resolve("LPenQuickMoveTo");
    m_fnControlHDAction = (FN_VOID_INT)m_library.resolve("ControlHDAction");
    m_fnGetMainCardID = (FN_WCHART_VOID)m_library.resolve("GetMainCardID");
    m_fnGetCurrentLaserPos = (FN_INT_VOID)m_library.resolve("GetCurrentLaserPos");
    m_fnSmallScaleMovement = (FNSmallScaleMovement)m_library.resolve("SmallScaleMovement");
    m_fnStartMachining = (FN_VOID_BOOL)m_library.resolve("StartMachining");
    m_fnPauseContinueMachining = (FN_INT_BOOL)m_library.resolve("PauseContinueMachining");
    m_fnStopMachining = (FN_VOID_VOID)m_library.resolve("StopMachining");
    m_fnControlMotor = (FN_INT_BOOL)m_library.resolve("ControlMotor");
    m_fnTestLaserLight = (FN_INT_BOOL)m_library.resolve("TestLaserLight");

    qDebug() << m_fnGetAPILibVersion;
    qDebug() << m_fnGetAPILibCompileInfo;
    qDebug() << m_fnInitLib;
    qDebug() << m_fnUnInitLib;

    m_isLoaded = true;
    return true;
}

void LaserDriver::unload()
{
    if (m_isLoaded)
        m_fnUnInitLib();
    m_isLoaded = false;
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

void LaserDriver::init(int handle)
{
    m_fnInitLib(handle);
}

void LaserDriver::unInit()
{
    m_fnUnInitLib();
    m_isLoaded = false;
}

QList<int> LaserDriver::getPortList()
{
    QString portList = QString::fromWCharArray(m_fnGetComPortList());
    QStringList portNames = portList.split(";");
    QList<int> ports;
    QRegExp re("COM(\\d+)");
    for (int i = 0; i < portNames.length(); i++)
    {
        QString portItem = portNames[i];
        re.indexIn(portItem);
        QString portName = re.cap(1);
        qDebug() << portItem << portName;
        bool ok = false;
        int port = portName.toInt(&ok);
        if (ok)
        {
            ports.append(port);
        }
    }
    return ports;
}

bool LaserDriver::initComPort(int index)
{
    int result = m_fnInitComPort(index);
    return result == 0;
}

bool LaserDriver::unInitComPort()
{
    int result = m_fnUnInitComPort();
    return result == 0;
}

void LaserDriver::setTransTimeOutInterval(int interval)
{
    m_fnSetTRansTimeOutInterval(interval);
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

bool LaserDriver::writeSysParamToCard(QList<int> addresses, QList<double> values)
{
    if (addresses.length() == 0 || values.length() == 0)
        return false;
    if (addresses.length() != values.length())
        return false;

    QString addrBuf, valuesBuf;
    QStringList addrList;
    QStringList valuesList;
    for (int i = 0; i < addresses.length(); i++)
    {
        addrList.append(QString("%1").arg(addresses[i]));
        valuesList.append(QString("%1").arg(values[i]));
    }
    addrBuf = addrList.join(",");
    valuesBuf = valuesList.join(",");

    wchar_t* wcAddrs = new wchar_t[addrBuf.length() + 1];
    wchar_t* wcValues = new wchar_t[valuesBuf.length() + 1];
    addrBuf.toWCharArray(wcAddrs);
    wcAddrs[addrBuf.length()] = 0;
    valuesBuf.toWCharArray(wcValues);
    wcValues[valuesBuf.length()] = 0;
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
    wchar_t* addrBuf = new wchar_t[addrStr.length() + 1];
    addrStr.toWCharArray(addrBuf);
    addrBuf[addrStr.length() + 1] = 0;
    bool success = m_fnReadSysParamFromCard(addrBuf) != -1;
    return success;
}

void LaserDriver::showAboutWindow()
{
    m_fnShowAboutWindow();
}

void LaserDriver::lPenMoveToOriginalPoint(double speed)
{
    m_fnLPenMoveToOriginalPoint(speed);
}

void LaserDriver::lPenQuickMoveTo(char xyzStyle, bool zeroPointStyle, double x, double y, double z, double startSpeed, double workSpeed)
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

int LaserDriver::GetCurrentLaserPos()
{
    return m_fnGetCurrentLaserPos();
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


