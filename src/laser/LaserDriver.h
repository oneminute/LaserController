#ifndef LASERCONTROLLER_H
#define LASERCONTROLLER_H

#include <QObject>
#include <QLibrary>
#include <QWidget>

enum LaserWorkMode
{
    LWM_WORKING,
    LWM_PAUSE,
    LWM_STOP
};

struct LaserState
{
public:
    LaserState()
        : workingMode(LaserWorkMode::LWM_STOP)
        , operation(0)
        , x(0)
        , y(0)
        , z(0)
        , power(0)
        , gray(0)
    {}
    LaserWorkMode workingMode;
    int operation;
    double x;
    double y;
    double z;
    double power;
    double gray;

    bool parse(const QString& data)
    {
        QStringList values = data.split(";");
        if (values.size() != 7)
            return false;

        for (int i = 0; i < values.size(); i++)
        {
            QString value = values[i];
            bool ok = false;
            int iValue = value.toInt(&ok);
            if (!ok)
                return false;

            switch (i)
            {
            case 0:
                workingMode = static_cast<LaserWorkMode>(iValue);
                break;
            case 1:
                operation = iValue;
                break;
            case 2:
                x = iValue;
                break;
            case 3:
                y = iValue;
                break;
            case 4:
                z = iValue;
                break;
            case 5:
                power = iValue;
                break;
            case 6:
                gray = iValue;
                break;
            }
        }
        return true;
    }
};

Q_DECLARE_METATYPE(LaserState)

class LaserDriver : public QObject
{
    Q_OBJECT

    enum SysMsgCode
    {
        InitComPortError = 1,
        UnInitComPortError = 2,
        SendDataStrError = 3,
        SendDataBufError = 4,
        InitializeError = 5,
        UninitializeError = 6,
        ComPortExceptionError = 7,
        GraphicMaxSizeError = 8,
        SendStreamError = 9,
        SendCommandError = 10,
        DecryptCommandError = 11,
        RetransTimeOutError = 12,
        TransCompleteOK = 13,
        ComPortNotOpened = 14,
        GraphicMinSizeError = 15,
        NoneDataTransError = 16,
        ComPortOpened = 17,
        ComPortClosed = 18,
        TransTimeOutError = 19,
        DataFormatError = 20,
        FileNotExistsError = 21,
        FunctionRestrict = 22,
        SendDataError = 23,
        ReceiveWrongDataOutTimes = 24,
        StartWorking = 25,
        ImageDataError = 26,
        USBArrival = 27,
        USBRemove = 28,
        ReadSysParamFromCardError = 30,
        WriteSysParamToCardError = 31,
        ReadSysParamFromCardOK = 32,
        WriteSysParamToCardOK = 33,
        ReturnTextMsgFromCallBack = 34,
        UnknownError = 35,
        PauseWorking = 36,
        ContinueWorking = 37,
        StopWorking = 38,
        MotorLock = 39,
        MotorUnlock = 40,
        LaserLightOn = 41,
        LaserLightOff = 42,
        TimeOutResend = 43,
        DisabledOnWorking = 44,
        GetComPortListError = 45,
        GetComPortListOK = 46,
        MachineOffLine = 47,
        ComPortIndexErr = 48,
        SystemFatalError = 49,
        ComPortNotAvailable = 50,
        CancelCurrentWork = 51,
        ReturnWorkState = 52,
        CanNotDoOnWorking = 53,
        RequestAndContinue = 54,
        BreakpointResume = 55,
        NotWorking = 56,
        BreakpointDataError = 57,
        WorkFinished = 58,
        EnlockerNotExists = 59,
        FactoryPasswordValid = 60,
        FactoryPasswordInvalid = 61,
        ChangeFactoryPasswordOK = 62,
        ChangeFactoryPasswordError = 63,
        FactoryPasswordLengthError = 64,
        FactoryPasswordExpired = 65
    };

private:
    typedef wchar_t* (*FN_WCHART_VOID)();
    typedef void(__stdcall *FN_VOID_INT)(int value);
    typedef void(*FN_VOID_VOID)();

    typedef void(__cdecl *FNProgressCallBackHandler)(void* ptr, int position, int totalCount);
    typedef void(*FNProgressCallBack)(FNProgressCallBackHandler callback);

    typedef void(__cdecl *FNSysMessageCallBackHandler)(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData);
    typedef void(*FNSysMessageCallBack)(FNSysMessageCallBackHandler callback);

    typedef void(__cdecl *FNProcDataProgressCallBackHandler)(void* ptr, int position, int totalCount);
    typedef void(*FNProcDataProgressCallBack)(FNProcDataProgressCallBackHandler callback);

    //typedef wchar_t*(*FNGetComPortList)();

    typedef int(__stdcall *FN_INT_INT)(int comPort);
    typedef int(*FN_INT_VOID)();

    typedef void(__stdcall *FNSetSoftwareInitialization)(int printerDrawUnit, double pageZeroX, double pageZeroY, double pageWidth, double pageHeight);
    typedef void(__stdcall *FNSetRotateDeviceParam)(int type, int perimeterPulse, int materialPerimeter, int deviceDPI, bool autoScaleDimensions);
    typedef void(__stdcall *FNSetHardwareInitialization)(double curveToSpeedRatio, int logicalResolution, int maxSpeed, char zeroCoordinates);

    typedef int(__stdcall *FN_INT_WCHART_WCHART)(wchar_t* address, wchar_t* data);
    typedef int(__stdcall *FN_INT_WCHART)(wchar_t* address);

    typedef void(__stdcall *FN_VOID_DOUBLE)(double speed);
    typedef void(__stdcall *FNLPenQuickMoveTo)(char xyzStyle, bool zeroPointStyle, double x, double y, double z, double startSpeed, double workSpeed);
    typedef void(__stdcall *FNSmallScaleMovement)(bool fromZeroPoint, bool laserOn, char motorAxis, int deviation, int laserPower, int moveSpeed);

    typedef void(__stdcall *FN_VOID_BOOL)(bool zeroPointStyle);
    typedef int(__stdcall *FN_INT_BOOL)(bool pause);

    explicit LaserDriver(QObject* parent = nullptr);
    ~LaserDriver();

public:
    

    static LaserDriver& instance();
    static void ProgressCallBackHandler(void* ptr, int position, int totalCount);
    static void SysMessageCallBackHandler(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData);
    static void ProcDataProgressCallBackHandler(void* ptr, int position, int totalCount);

    bool load();
    void unload();

    QString getVersion();
    QString getCompileInfo();
    void init(QWidget* parentWidget);
    void unInit();
    QStringList getPortList();
    bool initComPort(const QString& name);
    bool unInitComPort();
    void setTransTimeOutInterval(int interval);
    void setSoftwareInitialization(int printerDrawUnit, double pageZeroX, double pageZeroY, double pageWidth, double pageHeight);
    void setRotateDeviceParam(int type, int perimeterPulse, int materialPerimeter, int deviceDPI, bool autoScaleDimensions);
    void setHardwareInitialization(double curveToSpeedRatio, int logicalResolution, int maxSpeed, char zeroCoordinates);
    bool writeSysParamToCard(QList<int> addresses, QList<double> values);
    bool readSysParamFromCard(QList<int> addresses);
    bool readAllSysParamFromCard();
    void showAboutWindow();
    void lPenMoveToOriginalPoint(double speed);
    void lPenQuickMoveTo(char xyzStyle, bool zeroPointStyle, double x, double y, double z, double startSpeed, double workSpeed);
    void controlHDAction(int action);
    QString getMainCardID();
    int GetCurrentLaserPos();
    void smallScaleMovement(bool fromZeroPoint, bool laserOn, char motorAxis, int deviation, int laserPower, int moveSpeed);
    void startMachining(bool zeroPointStyle);
    int pauseContinueMachining(bool pause);
    void stopMachining();
    int controlMotor(bool open);
    int testLaserLight(bool open);
    int loadDataFromFile(const QString& filename, bool withMachining = true);

    bool isLoaded() const { return m_isLoaded; }
    bool isConnected() const { return m_isConnected; }
    bool isMachining() const { return m_isMachining; }
    QString portName() const { return m_portName; }

signals:
    void libraryLoaded(bool success = true);
    void libraryUnloaded();
    void libraryInitialized();
    void libraryUninitialized();
    void comPortConnected();
    void comPortDisconnected(bool isError = false, const QString& errorMsg = "");
    void machiningStarted();
    void machiningPaused();
    void machiningStopped(bool isSuccess = true, const QString& errorMsg = "");
    void downloading(int current, int total, float progress);
    void downloaded();
    void workStateUpdated(LaserState state);
    void idle();

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
    LaserWorkMode m_workMode;

    QLibrary m_library;

    FN_WCHART_VOID m_fnGetAPILibVersion;
    FN_WCHART_VOID m_fnGetAPILibCompileInfo;
    FN_VOID_INT m_fnInitLib;
    FN_VOID_VOID m_fnUnInitLib;

    FNProgressCallBack m_fnProgressCallBack;
    FNSysMessageCallBack m_fnSysMessageCallBack;
    FNProcDataProgressCallBack m_fnProcDataProgressCallBack;

    FN_WCHART_VOID m_fnGetComPortList;
    FN_INT_INT m_fnInitComPort;
    FN_INT_VOID m_fnUnInitComPort;

    FN_VOID_INT m_fnSetTRansTimeOutInterval;
    FNSetSoftwareInitialization m_fnSetSoftwareInitialization;
    FNSetRotateDeviceParam m_fnSetRotateDeviceParam;
    FNSetHardwareInitialization m_fnSetHardwareInitialization;

    FN_INT_WCHART_WCHART m_fnWriteSysParamToCard;
    FN_INT_WCHART m_fnReadSysParamFromCard;

    FN_VOID_VOID m_fnShowAboutWindow;

    FN_VOID_DOUBLE m_fnLPenMoveToOriginalPoint;
    FNLPenQuickMoveTo m_fnLPenQuickMoveTo;
    FN_VOID_INT m_fnControlHDAction;
    FN_WCHART_VOID m_fnGetMainCardID;
    FN_INT_VOID m_fnGetCurrentLaserPos;
    FNSmallScaleMovement m_fnSmallScaleMovement;
    FN_VOID_BOOL m_fnStartMachining;
    FN_INT_BOOL m_fnPauseContinueMachining;
    FN_VOID_VOID m_fnStopMachining;
    FN_INT_BOOL m_fnControlMotor;
    FN_INT_BOOL m_fnTestLaserLight;

    FN_INT_WCHART m_fnLoadDataFromFile;

    wchar_t m_wcharBuffer[2048];
};

#endif // LASERCONTROLLER_H
