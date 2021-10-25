#include <QtCore>
#include <QApplication>
#include <QDebug>
#include <QMainWindow>

wchar_t * qStringToWCharPtr(const QString & str)
{
    wchar_t* buf = new wchar_t[str.length() + 1];
    str.toWCharArray(buf);
    buf[str.length()] = 0;
    return buf;
}

char* qStringToCharPtr(const QString& str)
{
    char* buf = new char[str.length() + 1];
    const char* cStr = str.toStdString().c_str();
    memcpy(buf, cStr, str.length());
    buf[str.length()] = 0;
    return buf;
}

class LaserDriver : public QObject
{
public:
private:
    typedef wchar_t* (*FN_WCHART_VOID)();
    typedef void(__stdcall *FN_VOID_INT)(int value);
    typedef bool(__stdcall *FN_BOOL_INT)(int value);
    typedef void(*FN_VOID_VOID)();
    typedef void(__stdcall* FN_VOID_WCHART)(wchar_t*);

    typedef void(__cdecl *FNProgressCallBackHandler)(void* ptr, int position, int totalCount);
    typedef void(*FNProgressCallBack)(FNProgressCallBackHandler callback);

    typedef void(__cdecl *FNSysMessageCallBackHandler)(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData);
    typedef void(*FNSysMessageCallBack)(FNSysMessageCallBackHandler callback);

    typedef void(__cdecl *FNProcDataProgressCallBackHandler)(void* ptr, int position, int totalCount);
    typedef void(*FNProcDataProgressCallBack)(FNProcDataProgressCallBackHandler callback);

    typedef int(__stdcall *FN_INT_INT)(int);
    typedef int(__stdcall *FN_INT_INT_INT)(int, int);
    typedef int(*FN_INT_VOID)();

    typedef void(__stdcall *FNSetSoftwareInitialization)(int printerDrawUnit, double pageZeroX, double pageZeroY, double pageWidth, double pageHeight);
    //typedef void(__stdcall *FNSetRotateDeviceParam)(int type, int perimeterPulse, int materialPerimeter, int deviceDPI, bool autoScaleDimensions);
    typedef void(__stdcall *FNSetHardwareInitialization)(double curveToSpeedRatio, int logicalResolution, int maxSpeed, char zeroCoordinates);

    typedef int(__stdcall *FN_INT_WCHART_WCHART)(wchar_t* address, wchar_t* data);
    typedef int(__stdcall *FN_INT_WCHART)(wchar_t* address);

    typedef void(__stdcall *FN_VOID_DOUBLE)(double speed);
    typedef void(__stdcall *FNLPenQuickMoveTo)(
        bool xMoveEnable,
        bool xMoveStyle,
        int xPos,
        bool yMoveEnable,
        bool yMoveStyle,
        int yPos,
        bool zMoveEnable,
        bool zMoveStyle,
        int zPos);
    typedef void(__stdcall *FNCheckMoveLaserMotors)(
        quint16 delay,
        bool xMoveEnable,
        bool xMoveStyle,
        int xPos,
        bool yMoveEnable,
        bool yMoveStyle,
        int yPos,
        bool zMoveEnable,
        bool zMoveStyle,
        int zPos);
    typedef void(__stdcall *FNSmallScaleMovement)(bool fromZeroPoint, bool laserOn, char motorAxis, int deviation, int laserPower, int moveSpeed);

    typedef void(__stdcall *FN_VOID_BOOL)(bool zeroPointStyle);
    typedef int(__stdcall *FN_INT_BOOL)(bool pause);

    typedef int(__stdcall *FN_INT_DOUBLE_BOOL)(double millimeter, bool xaxis);

    typedef wchar_t* (__stdcall *FNActivationMainCard)(
        wchar_t*, wchar_t*, wchar_t*, wchar_t*,
        wchar_t*, wchar_t*, wchar_t*, wchar_t*, 
        wchar_t*, wchar_t*, wchar_t*, wchar_t*);

    typedef wchar_t* (__stdcall* FN_WCHART_BOOL)(bool reload);

    typedef bool(__stdcall* FN_BOOL_WCHART)(wchar_t* licenseCode);

    typedef void(__stdcall* FN_BOOL_WCHART_INT_WCHART)(bool, wchar_t*, int, wchar_t*);
    typedef int(__stdcall* FN_INT_INT_BOOL)(int, bool);

    typedef wchar_t* (__stdcall* FN_WCHART_WCHART)(wchar_t*);
    typedef int(__stdcall* FN_INT_WCHART_WCHART_INT)(wchar_t*, wchar_t*, int);

public:
    explicit LaserDriver(QObject* parent = nullptr);
    ~LaserDriver();

    static void ProgressCallBackHandler(void* ptr, int position, int totalCount);
    static void SysMessageCallBackHandler(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData);
    static void ProcDataProgressCallBackHandler(void* ptr, int position, int totalCount);

    bool load();
    void unload();

    QString getVersion();
    QString getCompileInfo();
    int setLanguage(int lang);
    bool init(int winId);
    void setupCallbacks();
    void unInit();
    QStringList getPortList();
    bool initComPort(const QString& name);
    bool uninitComPort();
    void setTransTimeOutInterval(int interval);
    void setSoftwareInitialization(int printerDrawUnit, double pageZeroX, double pageZeroY, double pageWidth, double pageHeight);
    //void setRotateDeviceParam(int type, int perimeterPulse, int materialPerimeter, int deviceDPI, bool autoScaleDimensions);
    void setHardwareInitialization(double curveToSpeedRatio, int logicalResolution, int maxSpeed, char zeroCoordinates);

    int showAboutWindow(int interval = 0, bool modal = true);
    void closeAboutWindow();
    QString getLaserLibraryInfo();
    void setFactoryType(const QString& factory);
    //bool checkFactoryPassword(const QString& password);
    bool changeFactoryPassword(const QString& oldPassword, const QString& newPassword);
    void lPenMoveToOriginalPoint(double speed);
    void lPenQuickMoveTo(
        bool xMoveEnable,
        bool xMoveStyle,
        int xPos,
        bool yMoveEnable,
        bool yMoveStyle,
        int yPos,
        bool zMoveEnable,
        bool zMoveStyle,
        int zPos);
    void checkMoveLaserMotors(
        quint16 delay,
        bool xMoveEnable,
        bool xMoveStyle,
        int xPos,
        bool yMoveEnable,
        bool yMoveStyle,
        int yPos,
        bool zMoveEnable,
        bool zMoveStyle,
        int zPos);
    void startMoveLaserMotors();
    void controlHDAction(int action);

    QString firmwareVersion();
    QString getMainCardID();
    bool sendAuthenticationEmail(const QString& email);
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
        const QString& model,
        const QString& machineId
    );
    QString getDeviceId(bool reload = true);
    QString getDongleId();
    void getMainCardRegisterState();
    QString getMainCardInfo();
    bool createLicenseFile(const QString& licenseCode);

    void smallScaleMovement(bool fromZeroPoint, bool laserOn, char motorAxis, int deviation, int laserPower, int moveSpeed);
    void startMachining(int packIndex);
    int pauseContinueMachining(bool pause);
    void stopMachining();
    int controlMotor(bool open);
    int testLaserLight(bool open);
    int loadDataFromFile(const QString& filename, bool withMachining = true);
    void getDeviceWorkState();
    void checkVersionUpdate(bool hardware, const QString& flag, int currentVersion, const QString& versionNoteToJsonFile);
    int getUpdatePanelHandle(int version, int wndId);

    QString registeMainCard(const QString& registeCode);

    bool isLoaded() const { return m_isLoaded; }
    bool isConnected() const { return m_isConnected; }
    bool isMachining() const { return m_isMachining; }
    QString portName() const { return m_portName; }

private:
    bool m_isLoaded;
    bool m_isConnected;
    bool m_isMachining;
    bool m_isPaused;
    bool m_isWithMachining;
    bool m_isDownloading;
    int m_packagesCount;
    QString m_portName;
    QWidget* m_parentWidget;

    QLibrary m_library;

    FN_WCHART_VOID m_fnGetAPILibVersion;
    FN_WCHART_VOID m_fnGetAPILibCompileInfo;
    FN_INT_INT m_fnSetLanguage;
    FN_BOOL_INT m_fnInitLib;
    FN_VOID_VOID m_fnUnInitLib;

    FNProgressCallBack m_fnProgressCallBack;
    FNSysMessageCallBack m_fnSysMessageCallBack;
    FNProcDataProgressCallBack m_fnProcDataProgressCallBack;

    FN_WCHART_VOID m_fnGetComPortList;
    FN_INT_INT m_fnInitComPort;
    FN_INT_VOID m_fnUnInitComPort;

    FN_VOID_INT m_fnSetTransTimeOutInterval;
    FNSetSoftwareInitialization m_fnSetSoftwareInitialization;
    //FNSetRotateDeviceParam m_fnSetRotateDeviceParam;
    FNSetHardwareInitialization m_fnSetHardwareInitialization;

    FN_INT_WCHART_WCHART m_fnWriteSysParamToCard;
    FN_INT_WCHART m_fnReadSysParamFromCard;
    FN_INT_WCHART_WCHART m_fnWriteUserParamToCard;
    FN_INT_WCHART m_fnReadUserParamFromCard;
    FN_INT_WCHART_WCHART m_fnWriteComputerParamToCard;
    FN_INT_WCHART m_fnReadComputerParamFromCard;

    FN_INT_INT_BOOL m_fnShowAboutWindow;
    FN_VOID_VOID m_fnCloseAboutWindow;
    FN_VOID_VOID m_fnGetLaserLibInfo;
    FN_VOID_WCHART m_fnSetFactoryType;
    //FN_INT_WCHART m_fnCheckFactoryPassword;
    FN_INT_WCHART_WCHART m_fnWriteFactoryPassword;

    FN_VOID_DOUBLE m_fnLPenMoveToOriginalPoint;
    FNLPenQuickMoveTo m_fnLPenQuickMoveTo;
    FNCheckMoveLaserMotors m_fnCheckMoveLaserMotors;
    FN_VOID_VOID m_fnStartMoveLaserMotors;
    FN_VOID_INT m_fnControlHDAction;

    FN_WCHART_VOID m_fnGetMainHardVersion;
    FN_WCHART_VOID m_fnGetMainCardID;
    FNActivationMainCard m_fnActiveMainCard;
    FN_WCHART_BOOL m_fnGetDeviceId;
    FN_WCHART_VOID m_fnGetHardwareKeyID;
    FN_VOID_VOID m_fnGetMainCardRegState;
    FN_WCHART_VOID m_fnGetMainCardInfo;
    FN_BOOL_WCHART m_fnCreateLicenseFile;

    FN_WCHART_VOID m_fnGetCurrentLaserPos;
    FNSmallScaleMovement m_fnSmallScaleMovement;
    FN_VOID_INT m_fnStartMachining;
    FN_INT_BOOL m_fnPauseContinueMachining;
    FN_VOID_VOID m_fnStopMachining;
    FN_INT_BOOL m_fnControlMotor;
    FN_INT_BOOL m_fnTestLaserLight;

    FN_INT_WCHART m_fnLoadDataFromFile;

    FN_VOID_VOID m_fnGetDeviceWorkState;

    FN_INT_DOUBLE_BOOL m_fnMillimeter2MicroStep;

    FN_BOOL_WCHART_INT_WCHART m_fnCheckVersionUpdate;
    FN_INT_INT_INT m_fnGetUpdatePanelHandle;

    FN_INT_WCHART m_fnActivationMainCardEx;
    FN_WCHART_WCHART m_fnRegisteMainCard;
    FN_INT_WCHART_WCHART_INT m_fnSendAuthenticationEmail;

    wchar_t m_wcharBuffer[2048];

    bool m_isClosed;
};

#define CHECK_STR(fn) #fn
#define CHECK_FN(fn) \
    if (!fn) \
    { \
        qWarning() << CHECK_STR(fn) << " is nullptr."; \
    }

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
    , m_isClosed(false)
{
    
}

LaserDriver::~LaserDriver()
{
    qDebug() << "driver destroyed";
}

void LaserDriver::ProgressCallBackHandler(void* ptr, int position, int totalCount)
{
    float progress = position * 1.0f / totalCount;
    qDebug() << "Progress callback handler: position = " << position << ", totalCount = " << totalCount << ", progress = " << QString("%1%").arg(static_cast<double>(progress * 100), 3, 'g', 4);
}

void LaserDriver::SysMessageCallBackHandler(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData)
{
    QString eventData = QString::fromWCharArray(sysEventData);
    qDebug() << "System message callback handler: index = " << sysMsgIndex << ", code = " << sysMsgCode << ", event data = " << eventData;
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

    m_library.setFileName("LaserLib32.dll");
    if (!m_library.load())
    {
        qDebug() << "load LaserLib failure:" << m_library.errorString();
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

    m_fnCheckMoveLaserMotors = (FNCheckMoveLaserMotors)m_library.resolve("CheckMoveLaserMotors");
    CHECK_FN(m_fnCheckMoveLaserMotors)

    m_fnStartMoveLaserMotors = (FN_VOID_VOID)m_library.resolve("StartMoveLaserMotors");
    CHECK_FN(m_fnStartMoveLaserMotors)

    m_fnControlHDAction = (FN_VOID_INT)m_library.resolve("ControlHDAction");
    CHECK_FN(m_fnControlHDAction)

    m_fnGetMainHardVersion = (FN_WCHART_VOID)m_library.resolve("GetMainHardVersion");
    CHECK_FN(m_fnGetMainHardVersion);

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

    m_fnGetDeviceWorkState = (FN_VOID_VOID)m_library.resolve("GetDeviceWorkState");
    CHECK_FN(m_fnGetDeviceWorkState)

    m_fnMillimeter2MicroStep = (FN_INT_DOUBLE_BOOL)m_library.resolve("Millimeter2MicroStep");
    CHECK_FN(m_fnMillimeter2MicroStep)

    m_fnCheckVersionUpdate = (FN_BOOL_WCHART_INT_WCHART)m_library.resolve("CheckVersionUpdate");
    CHECK_FN(m_fnCheckVersionUpdate)

    m_fnGetUpdatePanelHandle = (FN_INT_INT_INT)m_library.resolve("GetUpdatePanelHandle");
    CHECK_FN(m_fnGetUpdatePanelHandle)

    m_fnActivationMainCardEx = (FN_INT_WCHART)m_library.resolve("ActivationMainCardEx");
    CHECK_FN(m_fnActivationMainCardEx);

    m_fnRegisteMainCard = (FN_WCHART_WCHART)m_library.resolve("RegisterMainCard");
    CHECK_FN(m_fnRegisteMainCard)

    m_fnSendAuthenticationEmail = (FN_INT_WCHART_WCHART_INT)m_library.resolve("SendAuthenticationEmail");
    CHECK_FN(m_fnSendAuthenticationEmail)

    Q_ASSERT(m_fnLoadDataFromFile);

    m_isLoaded = true;
    return true;
}

void LaserDriver::unload()
{
    if (m_isLoaded)
        m_fnUnInitLib();
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
    //m_fnSysMessageCallBack(
    //    [=](void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData) {
    //        if (LaserApplication::driver->m_isClosed)
    //            return;

    //        QString eventData = QString::fromWCharArray(sysEventData);
    //        qDebug() << "System message callback handler: index = " << sysMsgIndex << ", code = " << sysMsgCode << ", event data = " << eventData;
    //        if (sysMsgCode >= E_Base && sysMsgCode < M_Base)
    //        {
    //            //emit LaserApplication::driver->raiseError(sysMsgCode, eventData);
    //            LaserApplication::device->handleError(sysMsgCode, eventData);
    //        }
    //        else if (sysMsgCode >= M_Base)
    //        {
    //            //emit LaserApplication::driver->sendMessage(sysMsgCode, eventData);
    //            LaserApplication::device->handleMessage(sysMsgCode, eventData);
    //        }
    //    }
    //);
}

void LaserDriver::unInit()
{
    m_fnUnInitLib();
    m_isLoaded = false;
}

QStringList LaserDriver::getPortList()
{
    QString portList = QString::fromWCharArray(m_fnGetComPortList());
    QStringList portNames = portList.split(";");

    return portNames;
}

bool LaserDriver::initComPort(const QString& name)
{
    QRegExp re(".*COM(\\d+)");
    re.indexIn(name);
    QString portName = re.cap(1);
    bool ok = false;
    int port = portName.toInt(&ok);
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

void LaserDriver::setHardwareInitialization(double curveToSpeedRatio, int logicalResolution, int maxSpeed, char zeroCoordinates)
{
    m_fnSetHardwareInitialization(curveToSpeedRatio, logicalResolution, maxSpeed, zeroCoordinates);
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
    QString info = QString::fromWCharArray(strInfo);
    return info;
}

void LaserDriver::setFactoryType(const QString& factory)
{
    wchar_t* strFactory = qStringToWCharPtr(factory);
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
    wchar_t* wcOldPassword = qStringToWCharPtr(oldPassword);
    wchar_t* wcNewPassword = qStringToWCharPtr(newPassword);
    bool success = m_fnWriteFactoryPassword(wcOldPassword, wcNewPassword) != -1;
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
        int zPos)
{
    m_fnLPenQuickMoveTo(
        xMoveEnable, xMoveStyle, xPos,
        yMoveEnable, yMoveStyle, yPos,
        zMoveEnable, zMoveStyle, zPos
        );
}

void LaserDriver::checkMoveLaserMotors(quint16 delay, bool xMoveEnable, bool xMoveStyle, int xPos, bool yMoveEnable, bool yMoveStyle, int yPos, bool zMoveEnable, bool zMoveStyle, int zPos)
{
    qDebug() << "move " << xPos << ", " << yPos << ", " << zPos;
    qDebug() << "enabled " << xMoveEnable << ", " << yMoveEnable << ", " << zMoveEnable;
    m_fnCheckMoveLaserMotors(
        delay,
        xMoveEnable, xMoveStyle, xPos,
        yMoveEnable, yMoveStyle, yPos,
        zMoveEnable, zMoveStyle, zPos
        );
}

void LaserDriver::startMoveLaserMotors()
{
    m_fnStartMoveLaserMotors();
}

void LaserDriver::controlHDAction(int action)
{
    m_fnControlHDAction(action);
}

QString LaserDriver::firmwareVersion()
{
    wchar_t* strId = m_fnGetMainHardVersion();
    QString id = QString::fromWCharArray(strId);
    return id;
}

QString LaserDriver::getMainCardID()
{
    wchar_t* strId = m_fnGetMainCardID();
    QString id = QString::fromWCharArray(strId);
    return id;
}

bool LaserDriver::sendAuthenticationEmail(const QString& email)
{
    wchar_t* emailBuf = qStringToWCharPtr(email);
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
    wchar_t* emailBuf = qStringToWCharPtr(email);
    wchar_t* codeBuf = qStringToWCharPtr(code);
    wchar_t* nameBuf = qStringToWCharPtr(name);
    wchar_t* phoneBuf = qStringToWCharPtr(phone);
    wchar_t* addressBuf = qStringToWCharPtr(address);
    wchar_t* qqBuf = qStringToWCharPtr(qq);
    wchar_t* wxBuf = qStringToWCharPtr(wx);
    wchar_t* countryBuf = qStringToWCharPtr(country);
    wchar_t* distributorBuf = qStringToWCharPtr(distributor);
    wchar_t* trademarkBuf = qStringToWCharPtr(brand);
    wchar_t* modelBuf = qStringToWCharPtr(model);
    wchar_t* cardIdBuf = qStringToWCharPtr(machineId);

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
    wchar_t* licBuf = qStringToWCharPtr(licenseCode);
    bool result = m_fnCreateLicenseFile(licBuf);
    delete[] licBuf;
    return result;
}

void LaserDriver::smallScaleMovement(bool fromZeroPoint, bool laserOn, char motorAxis, int deviation, int laserPower, int moveSpeed)
{
    m_fnSmallScaleMovement(fromZeroPoint, laserOn, motorAxis, deviation, laserPower, moveSpeed);
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
    wchar_t* filenameBuf = qStringToWCharPtr(filename);
    //wchar_t* filenameBuf = L"D:\\LaserController\\LaserController\\export.json";
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
    wchar_t* flagBuf = qStringToWCharPtr(flag);
    wchar_t* vntjfBuf = qStringToWCharPtr(versionNoteToJsonFile);
    m_fnCheckVersionUpdate(hardware, flagBuf, currentVersion, vntjfBuf);
    delete[] flagBuf;
    delete[] vntjfBuf;
}

int LaserDriver::getUpdatePanelHandle(int version, int wndId)
{
    return m_fnGetUpdatePanelHandle(version, wndId);
}

QString LaserDriver::registeMainCard(const QString& registeCode)
{
    wchar_t* registeCodeBuf = qStringToWCharPtr(registeCode);
    wchar_t* returnBuf = m_fnRegisteMainCard(registeCodeBuf);
    delete[] registeCodeBuf;
    QString returnRegisteCode = QString::fromWCharArray(returnBuf);
    return returnRegisteCode;
}


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.show();

    LaserDriver* driver = new LaserDriver;
    driver->load();
    driver->init(window.winId());
    driver->setupCallbacks();
    driver->setLanguage(0);
    QStringList portNames = driver->getPortList();
    driver->initComPort(portNames[0]);

    return app.exec();
}