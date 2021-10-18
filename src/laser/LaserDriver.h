#ifndef LASERCONTROLLER_H
#define LASERCONTROLLER_H

#include <QObject>
#include <QLibrary>
#include <QMap>
#include <QVector3D>
#include <QWidget>

#include "LaserDefines.h"
#include "laser/LaserRegister.h"
#include "task/Task.h"

class LaserDevice;

class LaserDriver : public QObject
{
    Q_OBJECT

public:
    enum SystemRegisterType
    {
        REG_SYS_09 = 9,                     
        RT_SYS_RUN_TIME = 9,
        REG_SYS_10 = 10,
        RT_LASER_RUN_TIME = 10,
        REG_SYS_11 = 11,                     
        RT_SYS_RUN_NUM = 11,
        REG_SYS_12 = 12,                     
        RT_X_MAX_LENGTH = 12,
        REG_SYS_13 = 13,                     
        RT_X_DIR_PHASE = 13,
        REG_SYS_14 = 14,                     
        RT_X_LIMIT_PHASE = 14,
        REG_SYS_15 = 15,                     
        RT_X_ZERO_DEV = 15,
        REG_SYS_16 = 16,                    
        RT_X_STEP_LENGTH = 16,
        REG_SYS_17 = 17,                    
        RT_X_LIMIT_NUM = 17,
        REG_SYS_18 = 18,                    
        RT_SYS_X_RESET_ENABLED = 18,
        REG_SYS_19 = 19,                    
        RT_X_MOTOR_NUM = 19,
        REG_SYS_20 = 20,                    
        RT_X_MOTOR_I = 20,
        REG_SYS_21 = 21,                    
        RT_X_START_SPEED = 21,
        REG_SYS_22 = 22,                    
        RT_X_MAX_SPEED = 22,
        REG_SYS_23 = 23,                    
        RT_X_MAX_A = 23,
        REG_SYS_24 = 24,                    
        RT_X_URGENT_A = 24,
        REG_SYS_25 = 25,                    
        RT_Y_MAX_LENGTH = 25,
        REG_SYS_26 = 26,                    
        RT_Y_DIR_PHASE = 26,
        REG_SYS_27 = 27,                    
        RT_Y_LIMIT_PHASE = 27,
        REG_SYS_28 = 28,                    
        RT_Y_ZERO_DEV = 28,
        REG_SYS_29 = 29,                    
        RT_Y_STEP_LENGTH = 29,
        REG_SYS_30 = 30,                    
        RT_Y_LIMIT_NUM = 30,
        REG_SYS_31 = 31,                    
        RT_SYS_Y_RESET_ENABLED = 31,
        REG_SYS_32 = 32,
        RT_Y_MOTOR_NUM = 32,
        REG_SYS_33 = 33,
        RT_Y_MOTOR_I = 33,
        REG_SYS_34 = 34,
        RT_Y_START_SPEED = 34,
        REG_SYS_35 = 35,
        RT_Y_MAX_SPEED = 35,
        REG_SYS_36 = 36,
        RT_Y_MAX_A = 36,
        REG_SYS_37 = 37,
        RT_Y_URGENT_A = 37,
        REG_SYS_38 = 38,
        RT_Z_MAX_LENGTH = 38,
        REG_SYS_39 = 39,
        RT_Z_LIMIT_PHASE = 39,
        REG_SYS_40 = 40,
        RT_Z_ZERO_DEV = 40,
        REG_SYS_41 = 41,
        RT_Z_STEP_LENGTH = 41,
        REG_SYS_42 = 42,
        RT_Z_LIMIT_NUM = 42,
        REG_SYS_43 = 43,
        RT_SYS_Z_RESET_ENABLED = 43,
        REG_SYS_44 = 44,
        RT_Z_MOTOR_NUM = 44,
        REG_SYS_45 = 45,
        RT_Z_MOTOR_I = 45,
        REG_SYS_46 = 46,
        RT_Z_START_SPEED = 46,
        REG_SYS_47 = 47,
        RT_Z_MAX_SPEED = 47,
        REG_SYS_48 = 48,
        RT_Z_MAX_A = 48,
        REG_SYS_49 = 49,
        RT_Z_URGENT_A = 49,
        REG_SYS_50 = 50,
        RT_LASER_MAX_POWER = 50,
        REG_SYS_51 = 51,
        RT_LASER_MIN_POWER = 51,
        REG_SYS_52 = 52,
        RT_LASER_POWER_FREQ = 52
    };

    enum UserRegisterType
    {
        REG_USER_00 = 0,
        RT_HEAD = 0,
        REG_USER_01 = 1,
        RT_ACC_MODE = 1,
        REG_USER_02 = 2,
        RT_CUT_MOVE_SPEED = 2,
        REG_USER_03 = 3,
        RT_CUT_MOVE_A = 3,
        REG_USER_04 = 4,
        RT_CUT_TURN_SPEED = 4,
        REG_USER_05 = 5,
        RT_CUT_TURN_A = 5,
        REG_USER_06 = 6,
        RT_CUT_WORK_A = 6,
        REG_USER_07 = 7,
        RT_CUT_MOVE_SPEED_PER = 7,
        REG_USER_08 = 8,
        RT_CUT_WORK_SPEED_PER = 8,
        REG_USER_09 = 9,
        RT_CUT_SPOT_SIZE = 9,
        REG_USER_10 = 10,
        RT_SCAN_X_START_SPEED = 10,
        REG_USER_11 = 11,
        RT_SCAN_Y_START_SPEED = 11,
        REG_USER_12 = 12,
        RT_SCAN_X_A = 12,
        REG_USER_13 = 13,
        RT_SCAN_Y_A = 13,
        REG_USER_14 = 14,
        RT_SCAN_ROW_SPEED = 14,
        REG_USER_15 = 15,
        RT_SCAN_ROW_SPACING = 15,
        REG_USER_16 = 16,
        RT_SCAN_RETURN_ERR = 16,
        REG_USER_17 = 17,
        RT_SCAN_LASER_POWER = 17,
        REG_USER_18 = 18,
        RT_USER_X_RESET_ENABLED = 18,
        REG_USER_19 = 19,
        RT_USER_Y_RESET_ENABLED = 19,
        REG_USER_20 = 20,
        RT_USER_Z_RESET_ENABLED = 20,
        REG_USER_21 = 21,
        RT_RETURN_POS = 21,
        REG_USER_22 = 22,
        RT_BACKLASH_X = 22,
        REG_USER_23 = 23,
        RT_BACKLASH_Y = 23
    };


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

    typedef wchar_t* (__stdcall *FNActivationMainCard)(wchar_t* name, wchar_t* address, wchar_t* phone, wchar_t* qq,
        wchar_t* wx, wchar_t* email, wchar_t* country, wchar_t* distributor, wchar_t* trademark, wchar_t* model,
        wchar_t* cardId);

    typedef wchar_t* (__stdcall* FN_WCHART_BOOL)(bool reload);

    typedef bool(__stdcall* FN_BOOL_WCHART)(wchar_t* licenseCode);

    typedef void(__stdcall* FN_BOOL_WCHART_INT_WCHART)(bool, wchar_t*, int, wchar_t*);
    typedef int(__stdcall* FN_INT_INT_BOOL)(int, bool);

    typedef wchar_t* (__stdcall* FN_WCHART_WCHART)(wchar_t*);

public:
    explicit LaserDriver(QObject* parent = nullptr);
    ~LaserDriver();

    static void ProgressCallBackHandler(void* ptr, int position, int totalCount);
    static void SysMessageCallBackHandler(void* ptr, int sysMsgIndex, int sysMsgCode, wchar_t* sysEventData);
    static void ProcDataProgressCallBackHandler(void* ptr, int position, int totalCount);

    bool load();
    void unload();
    void setDevice(LaserDevice* device);
    LaserDevice* device() const;

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

    QString getMainCardID();
    QString activateMainCard(const QString& name,
        const QString& address,
        const QString& phone,
        const QString& qq,
        const QString& wx,
        const QString& email,
        const QString& country,
        const QString& distributor,
        const QString& trademark,
        const QString& model,
        const QString& cardId
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

    FN_WCHART_WCHART m_fnRegisteMainCard;

    wchar_t m_wcharBuffer[2048];

    LaserDevice* m_device;
    bool m_isClosed;
};

#endif // LASERCONTROLLER_H
