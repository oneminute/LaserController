#ifndef LASERCONTROLLER_H
#define LASERCONTROLLER_H

#include <QObject>
#include <QLibrary>
#include <QMap>
#include <QVector3D>
#include <QWidget>

#include "task/Task.h"

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

public:
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
        DataTransformed = 29,
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
        FactoryPasswordExpired = 65,
        MainCardRegisterOK = 66,
        MainCardRegisterError = 67,
        MachineWorking = 68
    };

    enum RegisterType
    {
        REG_03 = 3,                     
        RT_RESET_CALIB_SPEED = 3,
        REG_04 = 4,
        RT_ENGRAVING_LAUNCING_SPEED = 4,
        REG_05 = 5,                     
        RT_MOVE_FAST_SPEED = 5,
        REG_06 = 6,                     
        RT_CUTTING_SPEED = 6,
        REG_07 = 7,                     
        RT_MOVE_TO_ORI_SPEED = 7,
        REG_08 = 8,                     
        RT_WORKING_QUADRANT = 8,
        REG_09 = 9,                     
        RT_X_AXIS_PULSE_LENGTH = 9,
        REG_10 = 10,                    
        RT_Y_AXIS_PULSE_LENGTH = 10,
        REG_11 = 11,                    
        RT_X_AXIS_BACKLASH = 11,
        REG_12 = 12,                    
        RT_Y_AXIS_BACKLASH = 12,
        REG_13 = 13,                    
        RT_ENGRAVING_COLUMN_STEP = 13,
        REG_14 = 14,                    
        RT_ENGRAVING_ROW_STEP = 14,
        REG_15 = 15,                    
        RT_LIGHT_ON_DELAY = 15,
        REG_16 = 16,                    
        RT_LIGHT_OFF_DELAY = 16,
        REG_17 = 17,                    
        RT_MIN_ENGRAVING_GRAY_VALUE = 17,
        REG_18 = 18,                    
        RT_CUTTING_LASER_POWER = 18,
        REG_19 = 19,                    
        RT_MIN_LASER_ENERGY = 19,
        REG_20 = 20,                    
        RT_MAX_LASER_ENERGY = 20,
        REG_21 = 21,                    
        RT_MACHINE_PHASE = 21,
        REG_22 = 22,                    
        RT_LIMIT_PHASE = 22,
        REG_23 = 23,                    
        RT_TOTAL_WORKING_DURATION = 23,
        REG_24 = 24,                    
        RT_TOTAL_LASER_DURATION = 24,
        REG_25 = 25,                    
        RT_CUTTING_LASER_FREQ = 25,
        REG_26 = 26,                    
        RT_REGISTION = 26,
        REG_27 = 27,                    
        RT_ENGRAVING_LASER_FREQ = 27,
        REG_31 = 31,                    
        RT_CUSTOM_1_X = 31,
        REG_32 = 32,                    
        RT_CUSTOM_1_Y = 32,
        REG_33 = 33,                    
        RT_CUSTOM_2_X = 33,
        REG_34 = 34,                    
        RT_CUSTOM_2_Y = 34,
        REG_35 = 35,                    
        RT_CUSTOM_3_X = 35,
        REG_36 = 36,                    
        RT_CUSTOM_3_Y = 36,
        REG_38 = 38,                    
        RT_LAYOUT_SIZE = 38,
        REG_39 = 39,                    
        RT_PAINTING_UNIT = 39,
        REG_40 = 40,                    
        RT_MOVE_FAST_LAUNCHING_SPEED = 40
    };

    typedef QMap<RegisterType, QVariant> RegistersMap;

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

    typedef int(__stdcall *FN_INT_INT)(int comPort);
    typedef int(*FN_INT_VOID)();

    typedef void(__stdcall *FNSetSoftwareInitialization)(int printerDrawUnit, double pageZeroX, double pageZeroY, double pageWidth, double pageHeight);
    typedef void(__stdcall *FNSetRotateDeviceParam)(int type, int perimeterPulse, int materialPerimeter, int deviceDPI, bool autoScaleDimensions);
    typedef void(__stdcall *FNSetHardwareInitialization)(double curveToSpeedRatio, int logicalResolution, int maxSpeed, char zeroCoordinates);

    typedef int(__stdcall *FN_INT_WCHART_WCHART)(wchar_t* address, wchar_t* data);
    typedef int(__stdcall *FN_INT_WCHART)(wchar_t* address);

    typedef void(__stdcall *FN_VOID_DOUBLE)(double speed);
    typedef void(__stdcall *FNLPenQuickMoveTo)(char xyzStyle, bool zeroPointStyle, double x, double y, double z, int startSpeed, int workSpeed);
    typedef void(__stdcall *FNSmallScaleMovement)(bool fromZeroPoint, bool laserOn, char motorAxis, int deviation, int laserPower, int moveSpeed);

    typedef void(__stdcall *FN_VOID_BOOL)(bool zeroPointStyle);
    typedef int(__stdcall *FN_INT_BOOL)(bool pause);

    typedef int(__stdcall *FN_INT_DOUBLE_BOOL)(double millimeter, bool xaxis);

    explicit LaserDriver(QObject* parent = nullptr);
    ~LaserDriver();

public:
    static LaserDriver& instance();
    static void ProgressCallBackHandler(void* ptr, int position, int totalCount);
    static void SysMessageCallBackHandler(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData);
    static void parseAndRefreshRegisters(QString &eventData);
    static void ProcDataProgressCallBackHandler(void* ptr, int position, int totalCount);

    bool load();
    void unload();

    QString getVersion();
    QString getCompileInfo();
    void init(QWidget* parentWidget);
    void unInit();
    QStringList getPortList();
    void getPortListAsyn();
    bool initComPort(const QString& name);
    bool uninitComPort();
    void setTransTimeOutInterval(int interval);
    void setSoftwareInitialization(int printerDrawUnit, double pageZeroX, double pageZeroY, double pageWidth, double pageHeight);
    void setRotateDeviceParam(int type, int perimeterPulse, int materialPerimeter, int deviceDPI, bool autoScaleDimensions);
    void setHardwareInitialization(double curveToSpeedRatio, int logicalResolution, int maxSpeed, char zeroCoordinates);
    bool writeSysParamToCard(const RegistersMap& values);
    bool readSysParamFromCard(QList<int> addresses);
    bool readAllSysParamFromCard();
    void showAboutWindow();
    bool checkFactoryPassword(const QString& password);
    bool changeFactoryPassword(const QString& oldPassword, const QString& newPassword);
    void lPenMoveToOriginalPoint(double speed);
    void lPenQuickMoveTo(char xyzStyle, bool zeroPointStyle, double x, double y, double z, int startSpeed, int workSpeed);
    void controlHDAction(int action);
    QString getMainCardID();
    QVector3D GetCurrentLaserPos();
    void smallScaleMovement(bool fromZeroPoint, bool laserOn, char motorAxis, int deviation, int laserPower, int moveSpeed);
    void startMachining(bool zeroPointStyle);
    int pauseContinueMachining(bool pause);
    void stopMachining();
    int controlMotor(bool open);
    int testLaserLight(bool open);
    int loadDataFromFile(const QString& filename, bool withMachining = true);
    void getDeviceWorkState();

    bool isLoaded() const { return m_isLoaded; }
    bool isConnected() const { return m_isConnected; }
    bool isMachining() const { return m_isMachining; }
    QString portName() const { return m_portName; }

    void setRegister(RegisterType rt, QVariant value);
    bool getRegister(RegisterType rt, QVariant& value);
    QString registerComment(RegisterType rt);
    bool getLayout(float& width, float& height);

    static ConnectionTask* createConnectionTask(QWidget* parentWidget);
    static DisconnectionTask* createDisconnectionTask(QWidget* parentWidget);
    static MachiningTask* createMachiningTask(const QString& filename, bool zeroPointStyle = false);

signals:
    void libraryLoaded(bool success = true);
    void libraryUnloaded();
    void libraryInitialized();
    void libraryUninitialized();
    void comPortConnected();
    void comPortDisconnected(bool isError = false, const QString& errorMsg = "");
    void comPortError(const QString& errorMsg);
    void comPortsFetched(const QStringList& ports);
    void comPortsFetchError();
    void machiningStarted();
    void machiningPaused();
    void continueWorking();
    void machiningStopped();
    void machiningCompleted();
    void downloading(int current, int total, float progress);
    void downloaded();
    void workStateUpdated(LaserState state);
    void idle();
    void sysParamFromCardArrived(const QString& data);
    void registersFectched(const RegistersMap& data);
    void sysParamFromCardError();
    void unknownError();
    void workingCanceled();
    void rightManufacturerPassword();
    void wrongManufacturerPassword();
    void changeManufacturerPasswordOk();
    void changeManufacturerPasswordFailure();

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
    RegistersMap m_registers;

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

    FN_VOID_INT m_fnSetTransTimeOutInterval;
    FNSetSoftwareInitialization m_fnSetSoftwareInitialization;
    FNSetRotateDeviceParam m_fnSetRotateDeviceParam;
    FNSetHardwareInitialization m_fnSetHardwareInitialization;

    FN_INT_WCHART_WCHART m_fnWriteSysParamToCard;
    FN_INT_WCHART m_fnReadSysParamFromCard;

    FN_VOID_VOID m_fnShowAboutWindow;
    FN_INT_WCHART m_fnCheckFactoryPassword;
    FN_INT_WCHART_WCHART m_fnWriteFactoryPassword;

    FN_VOID_DOUBLE m_fnLPenMoveToOriginalPoint;
    FNLPenQuickMoveTo m_fnLPenQuickMoveTo;
    FN_VOID_INT m_fnControlHDAction;
    FN_WCHART_VOID m_fnGetMainCardID;
    FN_WCHART_VOID m_fnGetCurrentLaserPos;
    FNSmallScaleMovement m_fnSmallScaleMovement;
    FN_VOID_BOOL m_fnStartMachining;
    FN_INT_BOOL m_fnPauseContinueMachining;
    FN_VOID_VOID m_fnStopMachining;
    FN_INT_BOOL m_fnControlMotor;
    FN_INT_BOOL m_fnTestLaserLight;

    FN_INT_WCHART m_fnLoadDataFromFile;

    FN_VOID_VOID m_fnGetDeviceWorkState;

    FN_INT_DOUBLE_BOOL m_fnMillimeter2MicroStep;

    wchar_t m_wcharBuffer[2048];

    static QMap<int, QString> m_registerComments;
};

#endif // LASERCONTROLLER_H
