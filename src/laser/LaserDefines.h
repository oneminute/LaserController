#ifndef LASERDEFINES_H
#define LASERDEFINES_H

#include <QString>
#include <QStringList>
#include <QVector3D>

enum LaserErrorCode
{
    E_Base = 0,
    E_FileException = 1,

    // laser device exception
    E_SystemFatalError = 1000,
    E_UnknownError = 1001,

    // connection exception
    E_InitializeError = 1002,
    E_UninitializeError = 1003,
    E_ComPortNotAvailable = 1004,
    E_GetComPortListError = 1005,

    // security exception
    E_DongleNotExists = 1006,
    E_DongleActiveDisabled = 1007,
    E_MainCardRegisterError = 1008,
    E_MainCardInactivated = 1009,
    E_InvalidMainCardId = 1010,
    E_InvalidDongleId = 1011,
    E_CardBindDongleError = 1012,
    E_CardBindDongleRepeatedly = 1013,
    E_CardDongleBoundExceedsTimes = 1014,
    E_CardDongleBoundIllegal = 1015,

    /// <summary>
    /// 激光管清零失败
    /// </summary>
    E_ClearLaserTubeDurationError = 1016,

    // security exception
    E_FactoryPasswordIncorrect = 1017,
    E_FactoryPasswordLengthError = 1018,
    E_FactoryPasswordExpired = 1019,
    E_PasswordIncorrectTooManyTimes = 1020,
    E_ChangeFactoryPasswordError = 1021,

    // IO exception
    E_ReadSysParamFromCardError = 1022,
    E_WriteSysParamToCardError = 1023,
    E_ReadUserParamFromCardError = 1024,
    E_WriteUserParamToCardError = 1025,

    // Network exception
    E_SaveParamsToServerError = 1026,
    E_LoadParamsFromServerError = 1027,

    // data exception
    E_FileNotExistsError = 1028,
    E_InvalidDataFormat = 1029,
    E_DecryptCommandError = 1030,
    E_InvalidImageData = 1031,
    E_ImageMinSizeTooSmall = 1032,
    E_ImageMaxSizeTooLarge = 1033,

    // IO exception
    E_NoDataError = 1034,
    E_TransferDataTimeout = 1035,
    E_RetransferAfterTimeout = 1036,
    E_RetransferTooManyTimes = 1037,
    E_TransferDataError = 1038,
    E_ReceiveInvalidDataTooManyTimes = 1039,
    E_BreakpointDataError = 1040,

    // Machining exception
    E_CanNotDoOnWorking = 1041,

    // Network exception
    E_PingServerFail = 1042,
    E_ConnectServerError = 1043,
    E_ConnectFrequently = 1044,
    E_SubmitToServerError = 1045,

    E_ServerAccessDenied = 1046,
    E_ServerReturnEmpty = 1047,

    E_UpdateInfoFileNotExists,
    E_UpdateFileNotExists,
    E_UpdateFailed,
    E_DownloadFirmwareDataError,
    E_UpdateFirmwareTimeout,

    E_InadequatePermissions,

    E_SendEmailFailed,
    E_MailboxInvalid,
    E_MailboxAccountError,
    E_ActiveCodeInvalid,
    E_ValidateCodeInvalid,
    E_MailboxNameInvalid
};

enum LaserEventType
{
    M_Base = 2000,
    M_GetComPortListOK = 2000,
    M_ComPortOpened = 2001,
    M_ComPortClosed = 2002,

    M_USBArrival = 2003,
    M_USBRemove,

    M_DongleArrival,
    M_DongleRemove,

    M_MainCardRegisterOK,
    M_MainCardIsGenuine,
    M_MainCardIsGenuineEx,
    M_MainCardMachineMoreInfo = 2010,

    M_CardDongleBindOK,
    M_LaserTubeZeroClearingOK,

    M_ReadSysParamFromCardOK,
    M_WriteSysParamToCardOK,

    M_ReadUserParamFromCardOK,
    M_WriteUserParamToCardOK,

    M_ReadComputerParamFromCardOK,
    M_WriteComputerParamToCardOK,

    M_SaveParamsToServerOK,
    M_ReadParamsFromServerOK,

    M_FactoryPasswordValid,
    M_ChangeFactoryPasswordOK,

    M_ReturnTextMsgFromCallback,

    M_ImportFromFile = 2024,
    M_CancelCurrentWork,
    M_TimeConsuming,
    M_EstimatedWorkTime,
    M_StartProcData,
    M_DataTransCompleted,
    M_RequestAndContinue,

    M_MotorLock = 2031,
    M_MotorUnlock,
    M_LaserLightOn,
    M_LaserLightOff,

    M_StartWorking,
    M_PauseWorking,
    M_ContinueWorking,
    M_StopWorking,

    M_MachineWorking = 2039,
    M_NotWorking,
    M_Idle = 2041,
    M_WorkFinished = 2041,

    M_DeviceIdInfo,
    M_ClientAddressInfo,

    M_ConnectedServer,
    M_DisconnectServer,
    M_ConnectServerOK,
    M_SubmitToServerOK,

    M_DownloadBegin = 2048,
    M_DownloadEnd,

    M_NewVersionChecking,
    M_NewVersionCheckFinished,
    M_IsLatestVersion,
    M_ReadyToUpdateFile,
    M_DownloadUpdateInfoFile,

    M_FoundSoftNewVersion = 2055,
    M_DownloadFileCounts,
    M_DownloadFileIndex,
    M_DownloadSoftDataStart,
    M_StartSoftUpdate,
    M_CancelSoftUpdate,
    M_SoftUpdateFinished,

    M_FoundFirmwareNewVersion = 2062,
    M_DownloadFirmwareDataStart,
    M_DownloadFirmwareDataEnd,
    M_SendFirmwareDataStart,
    M_SendFirmwareDataEnd,
    M_UpdateFirmwareStart,
    M_UpdateFirmwareEnd,
    M_UpdateFirmwareAbort,

    M_UpdateComplete = 2070,

    M_SendEmailComplete,
    M_MainlBoxValid,
    M_MainlBoxAccountOK,

    m_RequestProcessing = 2074, //面板通过板卡返回请求:开始加工
    m_RequestLensFocus = 2075, //面板通过板卡返回请求:镜头对焦
    m_RequestDrawBounding = 2076 //面板通过板卡返回请求:走边框
};

enum LaserWorkMode
{
    LWM_STOP,
    LWM_WORKING,
    LWM_PAUSE
};

enum MainCardActivateResult
{
    MAR_Activated = 0,
    MAR_Inactivated = 1,
    MAR_Error,
    MAR_Other
};

struct DeviceState
{
public:
    DeviceState()
        : operation(0)
        , packageNo(0)
        , workingMode(LaserWorkMode::LWM_STOP)
    {}
    int operation;
    int packageNo;
    LaserWorkMode workingMode;
    int operationId;
    QVector3D pos;

    bool parse(const QString& data)
    {
        QStringList values = data.split(";", QString::SkipEmptyParts);
        if (values.size() != 7)
            return false;

        bool ok = true;
        operation = values.at(0).toInt(&ok);
        packageNo = values.at(1).toInt(&ok);
        workingMode = static_cast<LaserWorkMode>(values.at(2).toInt(&ok));
        operationId = values.at(3).toInt(&ok);
        pos.setX(values.at(4).toDouble(&ok));
        pos.setY(values.at(5).toDouble(&ok));
        pos.setZ(values.at(6).toDouble(&ok));
        return true;
    }
};
Q_DECLARE_METATYPE(DeviceState)


#endif // LASERDEFINES_H