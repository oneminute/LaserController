#ifndef LASERDEFINES_H
#define LASERDEFINES_H

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
    E_UpdateInfoFileNotExists = 1046,
    E_UpdateFileNotExists = 1047,
    E_UpdateFailed = 1048,
    E_DownloadFirmwareDataError = 1049,
    E_UpdateFirmwareTimeout = 1050
};

enum LaserEventType
{
    M_Base = 2000,
    M_GetComPortListOK = 2000,
    M_ComPortOpened = 2001,
    M_ComPortClosed = 2002,
    M_USBArrival = 2003,
    M_USBRemove = 2004,
    M_MainCardRegisterOK = 2005,
    M_MainCardIsGenuine = 2006,
    M_MainCardIsGenuineEx = 2007,
    M_MainCardMachineMoreInfo = 2008,
    M_CardDongleBindOK = 2009,
    M_LaserTubeZeroClearingOK = 2010,
    M_ReadSysParamFromCardOK = 2011,
    M_WriteSysParamToCardOK = 2012,
    M_ReadUserParamFromCardOK = 2013,
    M_WriteUserParamToCardOK = 2014,
    M_ReadComputerParamFromCardOK = 2015,
    M_WriteComputerParamToCardOK = 2016,
    M_FactoryPasswordValid = 2017,
    M_ChangeFactoryPasswordOK = 2018,
    M_ReturnTextMsgFromCallback = 2019,
    M_ImportFromFile = 2020,
    M_CancelCurrentWork = 2021,
    M_TimeConsuming = 2022,
    M_EstimatedWorkTime = 2023,
    M_StartProcData = 2024,
    M_DataTransCompleted = 2025,
    M_RequestAndContinue = 2026,
    M_MotorLock = 2027,
    M_MotorUnlock = 2028,
    M_LaserLightOn = 2029,
    M_LaserLightOff = 2030,
    M_StartWorking = 2031,
    M_PauseWorking = 2032,
    M_ContinueWorking = 2033,
    M_StopWorking = 2034,
    M_MachineWorking = 2035,
    M_NotWorking = 2036,
    M_Idle = 2036,
    M_WorkFinished = 2037,
    M_DeviceIdInfo = 2038,
    M_ClientAddressInfo = 2039,
    M_ConnectedServer = 2040,
    M_DisconnectServer = 2041,
    M_ConnectServerOK = 2042,
    M_SubmitToServerOK = 2043,
    M_DownloadBegin = 2044,
    M_DownloadEnd = 2045,
    M_NewVersionChecking = 2046,
    M_NewVersionCheckFinished = 2047,
    M_IsLatestVersion = 2048,
    M_ReadyToUpdateFile = 2049,
    M_DownloadUpdateInfoFile = 2050,
    M_FoundSoftNewVersion = 2051,
    M_DownloadFileCounts = 2052,
    M_DownloadFileIndex = 2053,
    M_DownloadSoftDataStart = 2054,
    M_StartSoftUpdate = 2055,
    M_CancelSoftUpdate = 2056,
    M_SoftUpdateFinished = 2057,
    M_FoundFirmwareNewVersion = 2058,
    M_DownloadFirmwareDataStart = 2059,
    M_DownloadFirmwareDataEnd = 2060,
    M_SendFirmwareDataStart = 2061,
    M_SendFirmwareDataEnd = 2062,
    M_UpdateFirmwareStart = 2063,
    M_UpdateFirmwareEnd = 2064,
    M_UpdateFirmwareAbort = 2065,
    M_SaveParamsToServerOK = 2066,
    M_ReadParamsFromServerOK = 2067,
    M_UpdateComplete = 2070
};
#endif // LASERDEFINES_H