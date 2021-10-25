#ifndef LASERCONTROLLER_H
#define LASERCONTROLLER_H

#include <QObject>
#include <QLibrary>
#include <QMap>
#include <QVector3D>
#include <QWidget>

#include "LaserDefines.h"
#include "laser/LaserRegister.h"

class LaserDriver : public QObject
{
    Q_OBJECT

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

    bool writeSysParamToCard(const LaserRegister::RegistersMap& values);
    bool readSysParamFromCard(QList<int> addresses);
    bool readAllSysParamFromCard();
    bool writeUserParamToCard(const LaserRegister::RegistersMap& values);
    bool readUserParamFromCard(QList<int> addresses);
    bool readAllUserParamFromCard();
    bool writeHostParamToCard(const LaserRegister::RegistersMap& values);
    bool readHostParamFromCard(QList<int> addresses);
    bool readAllHostParamFromCard();

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
    MainCardActivateResult autoActiveMainCard();
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

    QVector3D getCurrentLaserPos();
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

protected slots:

signals:
    void libraryLoaded(bool success = true);
    void libraryUnloaded();
    void libraryInitialized();
    void libraryUninitialized();

    void comPortError(const QString& errorMsg);
    void comPortsFetchError();
    void machiningStarted();
    void machiningPaused();
    void continueWorking();
    void machiningStopped();
    void machiningCompleted();
    void downloading(int current, int total, float progress);
    void downloaded();
    void idle();
    void sysParamFromCardArrived(const QString& data);
    void registersFectched(const LaserRegister::RegistersMap& data);
    void sysParamFromCardError();
    void unknownError();
    void workingCanceled();
    void rightManufacturerPassword();
    void wrongManufacturerPassword();
    void changeManufacturerPasswordOk();
    void changeManufacturerPasswordFailure();
    void raiseError(int code, const QString& message);
    void sendMessage(int code, const QString& message);

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

#endif // LASERCONTROLLER_H
