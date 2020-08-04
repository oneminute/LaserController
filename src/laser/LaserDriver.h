#ifndef LASERCONTROLLER_H
#define LASERCONTROLLER_H

#include <QObject>
#include <QLibrary>

struct LaserState
{
public:
    LaserState()
        : workingMode(0)
        , operation(0)
        , x(0)
        , y(0)
        , z(0)
        , power(0)
        , gray(0)
    {}
    int workingMode;
    int operation;
    double x;
    double y;
    double z;
    double power;
    double gray;
};

class LaserDriver : public QObject
{
    Q_OBJECT
private:
    typedef wchar_t* (*FN_WCHART_VOID)();
    typedef void(__stdcall *FN_VOID_INT)(int value);
    typedef void(*FN_VOID_VOID)();

    typedef void(__cdecl *FNProgressCallBackHandler)(int position, int totalCount);
    typedef void(*FNProgressCallBack)(FNProgressCallBackHandler callback);

    typedef void(__cdecl *FNSysMessageCallBackHandler)(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData);
    typedef void(*FNSysMessageCallBack)(FNSysMessageCallBackHandler callback);

    typedef void(__cdecl *FNProcDataProgressCallBackHandler)(int position, int totalCount);
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
    enum LaserWorkMode
    {
        Working,
        Pause,
        Stop
    };

    static LaserDriver& instance();
    static void ProgressCallBackHandler(int position, int totalCount);
    static void SysMessageCallBackHandler(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData);
    static void ProcDataProgressCallBackHandler(int position, int totalCount);

    bool load();
    void unload();

    QString getVersion();
    QString getCompileInfo();
    void init(int handle);
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
    int loadDataFromFile(const QString& filename);

    bool isLoaded() const { return m_isLoaded; }
    bool isConnected() const { return m_isConnected; }
    QString portName() const { return m_portName; }

private:
    bool m_isLoaded;
    bool m_isConnected;
    QString m_portName;

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
