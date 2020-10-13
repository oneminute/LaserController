#include "LaserDriver.h"

#include <QDebug>
#include <QErrorMessage>
#include <QMessageBox>

#include "state/StateController.h"
#include "task/ConnectionTask.h"
#include "task/DisconnectionTask.h"
#include "task/MachiningTask.h"
#include "util/Utils.h"
#include "util/TypeUtils.h"

QMap<int, QString> LaserDriver::m_registerComments;

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
{
    ADD_TRANSITION(deviceUnconnectedState, deviceConnectedState, this, &LaserDriver::comPortConnected);
    ADD_TRANSITION(deviceConnectedState, deviceUnconnectedState, this, &LaserDriver::comPortDisconnected);
    ADD_TRANSITION(deviceIdleState, deviceMachiningState, this, &LaserDriver::machiningStarted);
    ADD_TRANSITION(deviceMachiningState, devicePausedState, this, &LaserDriver::machiningPaused);
    ADD_TRANSITION(devicePausedState, deviceMachiningState, this, &LaserDriver::continueWorking);
    ADD_TRANSITION(deviceMachiningState, deviceIdleState, this, &LaserDriver::machiningStopped);
    ADD_TRANSITION(deviceMachiningState, deviceIdleState, this, &LaserDriver::machiningCompleted);
}

LaserDriver::~LaserDriver()
{
    unload();
}

LaserDriver & LaserDriver::instance()
{
    static LaserDriver driver;
    return driver;
}

void LaserDriver::ProgressCallBackHandler(void* ptr, int position, int totalCount)
{
    float progress = position * 1.0f / totalCount;
    qDebug() << "Progress callback handler:" << position << totalCount << QString("%1%").arg(static_cast<double>(progress * 100), 3, 'g', 4)
;
    if (instance().m_isDownloading)
    {
        emit instance().downloading(position, totalCount, progress);
    }
}

void LaserDriver::SysMessageCallBackHandler(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t * sysEventData)
{
    QString eventData = QString::fromWCharArray(sysEventData);
    qDebug() << "System message callback handler:" << sysMsgIndex << sysMsgCode << eventData;
    switch (sysMsgCode)
    {
    case InitComPortError:      // 初始化串口错误
    {
        emit instance().comPortError(tr("Initialize com port error."));
    }
    break;
    case ComPortExceptionError: // 串口异常
    {
        emit instance().comPortError(tr("Com port exception."));
    }
    break;
    case ComPortNotOpened:      // 串口未打开
    {
        emit instance().comPortError(tr("Can not open com port."));
    }
    break;
    case ComPortOpened:    // 串口打开
    {
        instance().m_isConnected = true;
        emit instance().comPortConnected();
    }
    break;
    case ComPortClosed:    // 串口关闭
    {
        instance().m_isConnected = false;
        emit instance().comPortDisconnected();
    }
    break;
    case StartWorking:    // 开始加工
    {
        instance().m_isMachining = true;
        emit instance().machiningStarted();
    }
    break;
    case USBArrival:    // USB设备已连接
    {

    }
    break;
    case USBRemove:    // USB设备已断开
    {
    }
    break;
    case DataTransformed:   // 数据传输完成
    {
        //emit instance().downloaded();
    }
    break;
    case ReadSysParamFromCardError:
    {
        emit instance().sysParamFromCardError();
    }
    break;
    case ReadSysParamFromCardOK:    // 读取系统参数后返回的数据
    {
        instance().m_registers.clear();
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
            instance().m_registers.insert((RegisterType)addr, value);
        }
        emit instance().registersFectched(instance().m_registers);
        emit instance().sysParamFromCardArrived(eventData);
    }
    break;
    case UnknownError:
    {
        emit instance().unknownError();
    }
    break;
    case PauseWorking:    // 暂停加工
    {
        instance().m_isPaused = true;
        emit instance().machiningPaused();
    }
    break;
    case StopWorking:    // 停止加工。
    {
        emit instance().machiningStopped();
    }
    break;
    case GetComPortListError:
    {
        emit instance().comPortsFetchError();
    }
    break;
    case GetComPortListOK:
    {
        QStringList portNames = eventData.split(";");
        emit instance().comPortsFetched(portNames);
    }
    break;
    case CancelCurrentWork:
    {
        emit instance().workingCanceled();
    }
    break;
    case ReturnWorkState:
    {
        LaserState state;
        if (state.parse(eventData))
        {
            emit instance().workStateUpdated(state);
        }
    }
    break;
    case NotWorking:
    {
        emit instance().idle();
    }
    break;
    case WorkFinished:    // 加工完成
    {
        instance().m_isMachining = false;
        emit instance().machiningStopped();
    }
    break;
    }
}

void LaserDriver::ProcDataProgressCallBackHandler(void* ptr, int position, int totalCount)
{
    float progress = position * 1.0f / totalCount;
    qDebug() << "Proc progress callback handler:" << position << totalCount << QString("%1%").arg(static_cast<double>(progress * 100), 3, 'g', 4);
    
}

bool LaserDriver::load()
{
    m_registerComments.insert(REG_03, tr("Reset calib speed."));
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
    m_registerComments.insert(REG_21, tr("Machine phase."));
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
    m_registerComments.insert(REG_40, tr("Move fast launching speed."));

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
    emit libraryUnloaded();
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

void LaserDriver::init(QWidget* parentWidget)
{
    m_fnInitLib(parentWidget->winId());
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

void LaserDriver::getPortListAsyn()
{
    m_fnGetComPortList();
}

bool LaserDriver::initComPort(const QString & name)
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

bool LaserDriver::writeSysParamToCard(const QMap<RegisterType, QVariant>& values)
{
    if (values.count() == 0)
        return false;

    QString addrBuf, valuesBuf;
    QStringList addrList;
    QStringList valuesList;
    for (QMap<RegisterType, QVariant>::ConstIterator i = values.constBegin(); i != values.constEnd(); i++)
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
    params << 3 << 5 << 6 << 7 << 8 << 9 << 10 << 11 << 12 << 13 << 14 << 15
        << 16 << 17 << 18 << 19 << 20 << 21 << 22 << 23 << 27 << 28 << 29 
        << 30 << 31 << 32 << 34 << 35 << 36 << 38 << 39 << 40;
    return readSysParamFromCard(params);
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

int LaserDriver::loadDataFromFile(const QString & filename, bool withMachining)
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

int LaserDriver::mm2MicroStep(double mm, bool xaxis)
{
    return m_fnMillimeter2MicroStep(mm, xaxis);
}

void LaserDriver::setRegister(RegisterType rt, QVariant value)
{
    m_registers[rt] = value;
}

bool LaserDriver::getRegister(RegisterType rt, QVariant & value)
{
    if (m_registers.contains(rt))
    {
        value = m_registers[rt];
        return true;
    }
    return false;
}

QString LaserDriver::registerComment(RegisterType rt)
{
    if (m_registerComments.contains(rt))
        return m_registerComments[rt];
    return QString("");
}

ConnectionTask * LaserDriver::createConnectionTask(QWidget* parentWidget)
{
    return new ConnectionTask(&instance(), parentWidget);
}

DisconnectionTask * LaserDriver::createDisconnectionTask(QWidget * parentWidget)
{
    return new DisconnectionTask(&instance(), parentWidget);
}

MachiningTask * LaserDriver::createMachiningTask(const QString & filename, bool zeroPointStyle)
{
    return new MachiningTask(&instance(), filename, zeroPointStyle);
}
