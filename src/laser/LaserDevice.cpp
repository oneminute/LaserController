#include "LaserDevice.h"

#include <QDate>

#include "LaserApplication.h"
#include "LaserDriver.h"
#include "common/common.h"
#include "common/Config.h"
#include "exception/LaserException.h"
#include "state/StateController.h"
#include "ui/LaserControllerWindow.h"

class LaserDevicePrivate
{
    Q_DECLARE_PUBLIC(LaserDevice)
public:
    LaserDevicePrivate(LaserDevice* ptr)
        : q_ptr(ptr)
        , driver(nullptr)
        , isInit(false)
        , connected(false)
        , portName("")
        , layoutRect(0, 0, 320, 210)
        , printerDrawUnit(1016)
    {}

    LaserDevice* q_ptr;
    LaserDriver* driver;
    bool isInit;
    bool connected;

    QString portName;
    QRectF layoutRect;      // 加工的幅面宽
    int printerDrawUnit;    // 绘图仪单位，这里值的意思是一英寸分为多少个单位
    QString mainCard;
    QString mainCardRegisteredDate;
    QString mainCardActivatedDate;
    QString boundDongle;
    QString boundDongleRegisteredDate;
    QString boundDongleActivatedDate;
    QString boundDongleBindingTimes;
    QString dongle;
    QString dongleRegisteredDate;
    QString dongleActivatedDate;
    QString dongleBindingTimes;
    QString hardwareRegisteredDate;
    QString hardwareActivatedDate;
    QString hardwareMaintainingTimes;
};

LaserDevice::LaserDevice(QObject* parent)
    : QObject(parent)
    , m_ptr(new LaserDevicePrivate(this))
{
    ADD_TRANSITION(deviceUnconnectedState, deviceConnectedState, this, &LaserDevice::connected);
    ADD_TRANSITION(deviceConnectedState, deviceUnconnectedState, this, &LaserDevice::disconnected);

    connect(this, &LaserDevice::comPortsFetched, this, &LaserDevice::onComPortsFetched);
    connect(this, &LaserDevice::connected, this, &LaserDevice::onConnected);
    connect(this, &LaserDevice::mainCardRegistered, this, &LaserDevice::onMainCardRegistered);
    connect(this, &LaserDevice::mainCardActivated, this, &LaserDevice::onMainCardActivated);
}

LaserDevice::~LaserDevice()
{
    qDebug() << "device destroyed";
}

bool LaserDevice::resetDriver(LaserDriver* driver)
{
    Q_D(LaserDevice);
    unbindDriver();
    d->driver = driver;
    connect(d->driver, &LaserDriver::raiseError, this, &LaserDevice::handleError, Qt::ConnectionType::QueuedConnection);
    connect(d->driver, &LaserDriver::sendMessage, this, &LaserDevice::handleMessage, Qt::ConnectionType::QueuedConnection);
    connect(d->driver, &LaserDriver::libraryLoaded, this, &LaserDevice::onLibraryLoaded);
    connect(d->driver, &LaserDriver::libraryInitialized, this, &LaserDevice::onLibraryInitialized);

    return load();
}

bool LaserDevice::isInit() const
{
    Q_D(const LaserDevice);
    return d->isInit;
}

bool LaserDevice::isConnected() const
{
    Q_D(const LaserDevice);
    return d->connected;
}

QString LaserDevice::portName() const
{
    Q_D(const LaserDevice);
    return d->portName;
}

bool LaserDevice::load()
{
    Q_D(LaserDevice);
    if (d->driver->load())
        return true;

    unbindDriver();
    //d->driver->showAboutWindow();
    return false;
}

qreal LaserDevice::layoutWidth() const
{
    Q_D(const LaserDevice);
    return d->layoutRect.width();
}

qreal LaserDevice::layoutHeight() const
{
    Q_D(const LaserDevice);
    return d->layoutRect.height();
}

void LaserDevice::setLayoutRect(const QRectF& rect, bool toCard)
{
    Q_D(LaserDevice);
    d->layoutRect = rect;
    if (d->driver && toCard && d->layoutRect.isValid())
    {
        d->driver->setSoftwareInitialization(
            d->printerDrawUnit,
            d->layoutRect.left(),
            d->layoutRect.right(),
            d->layoutRect.width(),
            d->layoutRect.height());
    }
}

int LaserDevice::printerDrawUnit() const
{
    Q_D(const LaserDevice);
    return d->printerDrawUnit;
}

void LaserDevice::setPrinterDrawUnit(int unit, bool toCard)
{
    Q_D(LaserDevice);
    d->printerDrawUnit = unit;
    setLayoutRect(d->layoutRect, toCard);
}

QString LaserDevice::requestHardwareId() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        d->driver->getDeviceId(true);
        QString id = d->driver->getDeviceId(false);
        int count = 0;
        while (id.isEmpty())
        {
            QThread::sleep(0);
            id = d->driver->getDeviceId(false);
            count++;
        }
        qLogD << "get device id count: " << count;
        return id;
    }
    return "";
}

QString LaserDevice::requestMainCardId() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        return d->driver->getMainCardID();
    }
    return "";
}

QString LaserDevice::requestDongleId() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        return d->driver->getDongleId();
    }
    return "";
}

void LaserDevice::requestMainCardInfo()
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        QString infoString = d->driver->getMainCardInfo();
        /*if (infoString.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Main card info incomplete."));
        }
        QStringList items = infoString.split(";");
        if (items.length() != 13)
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Main card info incomplete."));
        }
        QMap<QString, QString> info;
        info.insert("mainCard", items[0]);
        info.insert("mainCardRegisteredDate", items[1]);
        info.insert("mainCardActivatedDate", items[2]);
        info.insert("boundDongle", items[3]);
        info.insert("boundDongleRegisteredDate", items[4]);
        info.insert("boundDongleActivatedDate", items[5]);
        info.insert("boundDongleBindingTimes", items[6]);
        info.insert("dongle", items[7]);
        info.insert("dongleRegisteredDate", items[8]);
        info.insert("dongleActivatedDate", items[9]);
        info.insert("dongleBindingTimes", items[10]);
        info.insert("hardwareRegisteredDate", items[11]);
        info.insert("hardwareActivatedDate", items[12]);
        info.insert("hardwareMaintainingTimes", items[13]);
        emit mainCardInfoFetched(info);*/

    }
}

QString LaserDevice::mainCardId() const
{
    Q_D(const LaserDevice);
    return d->mainCard;
}

QString LaserDevice::mainCardRegisteredDate() const
{
    Q_D(const LaserDevice);
    return d->mainCardRegisteredDate;
}

QString LaserDevice::mainCardActivatedDate() const
{
    Q_D(const LaserDevice);
    return d->mainCardActivatedDate;
}

QString LaserDevice::boundDongleId() const
{
    Q_D(const LaserDevice);
    return d->boundDongle;
}

QString LaserDevice::boundDongleRegisteredDate() const
{
    Q_D(const LaserDevice);
    return d->boundDongleRegisteredDate;
}

QString LaserDevice::boundDongleActivatedDate() const
{
    Q_D(const LaserDevice);
    return d->boundDongleActivatedDate;
}

QString LaserDevice::boundDongleBindingTimes() const
{
    Q_D(const LaserDevice);
    return d->boundDongleBindingTimes;
}

QString LaserDevice::dongleId() const
{
    Q_D(const LaserDevice);
    return d->dongle;
}

QString LaserDevice::dongleRegisteredDate() const
{
    Q_D(const LaserDevice);
    return d->dongleRegisteredDate;
}

QString LaserDevice::dongleActivatedDate() const
{
    Q_D(const LaserDevice);
    return d->dongleActivatedDate;
}

QString LaserDevice::dongleBindingTimes() const
{
    Q_D(const LaserDevice);
    return d->dongleBindingTimes;
}

QString LaserDevice::hardwareRegisteredDate() const
{
    Q_D(const LaserDevice);
    return d->hardwareRegisteredDate;
}

QString LaserDevice::hardwareActivatedDate() const
{
    Q_D(const LaserDevice);
    return d->hardwareActivatedDate;
}

QString LaserDevice::hardwareMaintainingTimes() const
{
    Q_D(const LaserDevice);
    return d->hardwareMaintainingTimes;
}

bool LaserDevice::verifyManufacturePassword(const QString& password)
{
    Q_D(LaserDevice);
    //return d->driver->checkFactoryPassword(password);
    return true;
}

bool LaserDevice::writeUserRegisters()
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->writeUserParamToCard(Config::UserRegister::group->keyValuePairs());
}

bool LaserDevice::writeSystemRegisters()
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->writeUserParamToCard(Config::SystemRegister::group->keyValuePairs());
}

bool LaserDevice::readUserRegisters()
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->readAllUserParamFromCard();
}

bool LaserDevice::readSystemRegisters()
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    return d->driver->readAllSysParamFromCard();
}

bool LaserDevice::readUserRegister(int address)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    QList<int> addresses;
    addresses << address;
    return d->driver->readUserParamFromCard(addresses);
}

bool LaserDevice::writeUserReigister(int address, const QVariant& value)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    LaserRegister::RegistersMap pair;
    pair.insert(address, value);
    return d->driver->writeUserParamToCard(pair);
}

bool LaserDevice::readSystemRegister(int address)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    QList<int> addresses;
    addresses << address;
    return d->driver->readSysParamFromCard(addresses);
}

bool LaserDevice::writeSystemReigister(int address, const QVariant& value)
{
    Q_D(LaserDevice);
    if (!isConnected())
        return false;
    LaserRegister::RegistersMap pair;
    pair.insert(address, value);
    return d->driver->writeSysParamToCard(pair);
}

void LaserDevice::showLibraryVersion()
{
    Q_D(LaserDevice);
    //QString compileInfo = d->driver->getCompileInfo();
    //qLogD << "compile info: " << compileInfo;
    QString laserLibraryInfo = d->driver->getLaserLibraryInfo();
    qLogD << "laser library info: " << laserLibraryInfo;
    QString mainCardId = d->driver->getMainCardID();
    qLogD << "main card id: " << mainCardId;
    //d->driver->showAboutWindow();
}

void LaserDevice::checkVersionUpdate(bool hardware, const QString& flag, int currentVersion, const QString& versionNoteToJsonFile)
{
    Q_D(LaserDevice);
    QString systemDate(__DATE__);
    qLogD << "system date: " << systemDate;
    QDate compileDate = QLocale("en_US").toDate(systemDate.simplified(), "MMM d yyyy");
    int year = compileDate.year() % 100;
    int month = compileDate.month();
    int day = compileDate.day();
    int version = year * 10000 + month * 100 + day;
    d->driver->checkVersionUpdate(hardware, flag, 0, versionNoteToJsonFile);
}

void LaserDevice::unload()
{
    Q_D(LaserDevice);
}

void LaserDevice::connectDevice(const QString& portName)
{
    Q_D(LaserDevice);
    qLogD << "connecting to device";
    d->portName = portName;
    d->driver->initComPort(portName);
}

void LaserDevice::disconnectDevice()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->uninitComPort();
    }
}

QString LaserDevice::activateMainCard(const QString& name, const QString& address, const QString& phone, const QString& qq, const QString& wx, const QString& email, const QString& country, const QString& distributor, const QString& trademark, const QString& model)
{
    Q_D(LaserDevice);
    QString cardId = requestMainCardId();
    qLogD << "cardId: " << cardId;
    QString result = d->driver->activateMainCard(name, address, phone, qq, wx, email, country, distributor, trademark, model, cardId);
    qLogD << "activation result: " << result;
    return result;
}

bool LaserDevice::requestTemporaryLicense()
{
    return createLicenseFile("{FFFD38EB-DC3A-45A8-A06D-B10671CF18B3}");
}

bool LaserDevice::createLicenseFile(const QString& licenseCode)
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        return d->driver->createLicenseFile(licenseCode);
    }
    return false;
}

void LaserDevice::moveToOrigin(qreal speed)
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->lPenMoveToOriginalPoint(speed);
    }
}

void LaserDevice::unbindDriver()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        disconnect(d->driver, &LaserDriver::raiseError, this, &LaserDevice::handleError);
        disconnect(d->driver, &LaserDriver::sendMessage, this, &LaserDevice::handleMessage);
        disconnect(d->driver, &LaserDriver::libraryLoaded, this, &LaserDevice::onLibraryLoaded);
        disconnect(d->driver, &LaserDriver::libraryInitialized, this, &LaserDevice::onLibraryInitialized);
        delete d->driver;
        d->driver = nullptr;
    }
}

void LaserDevice::handleError(int code, const QString& message)
{
    LaserException* exception = nullptr;
    switch (code)
    {
    case E_SystemFatalError:
        throw new LaserDeviceFatalException(code, tr("Laser device fatal error"));
        break;
    case E_UnknownError:
        throw new LaserDeviceUnknownException(code);
        break;
    case E_InitializeError:
        throw new LaserDeviceConnectionException(code, tr("Failed to initialize laser device"));
        break;
    case E_UninitializeError:
        throw new LaserDeviceConnectionException(code, tr("Failed to uninitialize laser device normally"));
        break;
    case E_ComPortNotAvailable:
        throw new LaserDeviceConnectionException(code, tr("Com port not available"));
        break;
    case E_GetComPortListError:
        throw new LaserDeviceConnectionException(code, tr("Failed to get COM port list"));
        break;
    case E_DongleNotExists:
        throw new LaserDeviceSecurityException(code, tr("Dongle does not exist"));
        break;
    case E_DongleActiveDisabled:
        throw new LaserDeviceSecurityException(code, tr("Dongle activation is disabled"));
        break;
    case E_MainCardRegisterError:
        throw new LaserDeviceSecurityException(code, tr("Failed to register main card"));
        break;
    case E_MainCardInactivated:
        throw new LaserDeviceSecurityException(code, tr("Main card inactivated"));
        break;
    case E_InvalidMainCardId:
        throw new LaserDeviceSecurityException(code, tr("Invalid main card ID"));
        break;
    case E_InvalidDongleId:
        throw new LaserDeviceSecurityException(code, tr("Invalid dongle ID"));
        break;
    case E_CardBindDongleError:
        throw new LaserDeviceSecurityException(code, tr("Failed to bind card with dongle"));
        break;
    case E_CardBindDongleRepeatedly:
        throw new LaserDeviceSecurityException(code, tr("The card is repeatedly bound to the dongle"));
        break;
    case E_CardDongleBoundExceedsTimes:
        throw new LaserDeviceSecurityException(code, tr("The number of times the card is bound to the dongle exceeds the allowable range"));
        break;
    case E_CardDongleBoundIllegal:
        throw new LaserDeviceSecurityException(code, tr("The card is illegally bound to the dongle"));
        break;
    case E_ClearLaserTubeDurationError:
        throw new LaserDeviceException(code, tr("Failed to clear duration of laser tube"));
        break;
    case E_FactoryPasswordIncorrect:
        throw new LaserDeviceSecurityException(code, tr("Incorrect factory password"));
        emit manufacturePasswordVerified(false);
        break;
    case E_FactoryPasswordLengthError:
        throw new LaserDeviceSecurityException(code, tr("Invalid length of factory password"));
        break;
    case E_FactoryPasswordExpired:
        throw new LaserDeviceSecurityException(code, tr("Factory password expired"));
        break;
    case E_PasswordIncorrectTooManyTimes:
        throw new LaserDeviceSecurityException(code, tr("Input incorrect factory password too many times"));
        break;
    case E_ChangeFactoryPasswordError:
        throw new LaserDeviceSecurityException(code, tr("Failed to change factory password"));
        break;
    case E_ReadSysParamFromCardError:
        throw new LaserDeviceIOException(code, tr("Failed to read parameters from device"));
        break;
    case E_WriteSysParamToCardError:
        throw new LaserDeviceIOException(code, tr("Failed to write parameters to device"));
        break;
    case E_ReadUserParamFromCardError:
        throw new LaserDeviceIOException(code, tr("Failed to read parameters from device"));
        break;
    case E_WriteUserParamToCardError:
        throw new LaserDeviceIOException(code, tr("Failed to write parameters to device"));
        break;
    case E_SaveParamsToServerError:
        throw new LaserNetworkException(code, tr("Failed to save parameters to server"));
        break;
    case E_LoadParamsFromServerError:
        throw new LaserNetworkException(code, tr("Failed to load parameters from server"));
        break;
    case E_FileNotExistsError:
        throw new LaserDeviceDataException(code, tr("File does not exist"));
        break;
    case E_InvalidDataFormat:
        throw new LaserDeviceDataException(code, tr("Invalid data format"));
        break;
    case E_DecryptCommandError:
        throw new LaserDeviceDataException(code, tr("Failed to decrypt data"));
        break;
    case E_InvalidImageData:
        throw new LaserDeviceDataException(code, tr("Invalid image data"));
        break;
    case E_ImageMinSizeTooSmall:
        throw new LaserDeviceDataException(code, tr("Min size of image is too small"));
        break;
    case E_ImageMaxSizeTooLarge:
        throw new LaserDeviceDataException(code, tr("Max size of image is too large"));
        break;
    case E_NoDataError:
        throw new LaserDeviceIOException(code, tr("No data transfered"));
        break;
    case E_TransferDataTimeout:
        throw new LaserDeviceIOException(code, tr("Transfering data timeout"));
        break;
    case E_RetransferAfterTimeout:
        throw new LaserDeviceIOException(code, tr("Retransfer data after timeout"));
        break;
    case E_RetransferTooManyTimes:
        throw new LaserDeviceIOException(code, tr("Retransfer data too many times"));
        break;
    case E_TransferDataError:
        throw new LaserDeviceIOException(code, tr("Failed to transfer data"));
        break;
    case E_ReceiveInvalidDataTooManyTimes:
        throw new LaserDeviceIOException(code, tr("Receive invalid data too many times"));
        break;
    case E_BreakpointDataError:
        throw new LaserDeviceIOException(code, tr("Failed to transfer data with breakpoint"));
        break;
    case E_CanNotDoOnWorking:
        throw new LaserDeviceMachiningException(code, tr("This operation is not supported during machining"));
        break;
    case E_PingServerFail:
        throw new LaserNetworkException(code, tr("Failed to connect to server"));
        break;
    case E_ConnectServerError:
        throw new LaserNetworkException(code, tr("Failed to log in to server"));
        break;
    case E_ConnectFrequently:
        throw new LaserNetworkException(code, tr("Login too frequently"));
        break;
    case E_SubmitToServerError:
        throw new LaserNetworkException(code, tr("Failed to submit data"));
        break;
    case E_UpdateInfoFileNotExists:
        throw new LaserNetworkException(code, tr("Updating info file does not exist"));
        break;
    case E_UpdateFileNotExists:
        throw new LaserNetworkException(code, tr("Updating file dose not exist"));
        break;
    case E_UpdateFailed:
        throw new LaserNetworkException(code, tr("Failed to update"));
        break;
    case E_DownloadFirmwareDataError:
        throw new LaserNetworkException(code, tr("Failed to download firmware"));
        break;
    case E_UpdateFirmwareTimeout:
        throw new LaserNetworkException(code, tr("Update firmware timeout"));
        break;
    }
    /*if (exception)
    {
        throw exception;
    }*/
}

void LaserDevice::handleMessage(int code, const QString& message)
{
    Q_D(LaserDevice);
    switch (code)
    {
    case M_GetComPortListOK:
    {
        QStringList portNames = message.split(";");
        emit comPortsFetched(portNames);
        break;
    }
    case M_ComPortOpened:
    {
        d->connected = true;
        emit comPortConnected(portName());
        emit connected();
        break;
    }
    case M_ComPortClosed:
    {
        d->connected = false;
        emit disconnected();
        break;
    }
    case M_USBArrival:
        break;
    case M_USBRemove:
        break;
    case M_MainCardRegisterOK:
    {
        emit mainCardRegistered();
        break;
    }
    case M_MainCardIsGenuine:
    {
        emit mainCardActivated(false);
        break;
    }
    case M_MainCardIsGenuineEx:
    {
        emit mainCardActivated(true);
        break;
    }
    case M_MainCardMachineMoreInfo:
    {
        if (message.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Main card info incomplete."));
        }
        QStringList items = message.split(";");
        if (items.length() != 13)
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Main card info incomplete."));
        }
        d->mainCard = items[0];
        d->mainCardRegisteredDate = items[1];
        d->mainCardActivatedDate = items[2];
        d->boundDongle = items[3];
        d->boundDongleRegisteredDate = items[4];
        d->boundDongleActivatedDate = items[5];
        d->boundDongleBindingTimes = items[6];
        d->dongle = items[7];
        d->dongleRegisteredDate = items[8];
        d->dongleActivatedDate = items[9];
        d->dongleBindingTimes = items[10];
        d->hardwareRegisteredDate = items[11];
        d->hardwareActivatedDate = items[12];
        d->hardwareMaintainingTimes = items[13];
        emit mainCardInfoFetched();
        break;
    }
    case M_CardDongleBindOK:
    {
        break;
    }
    case M_LaserTubeZeroClearingOK:
    {
        break;
    }
    case M_ReadSysParamFromCardOK:
    {
        if (message.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Registers data incomplete."));
        }
        LaserRegister::batchParse(message, true);
        break;
    }
    case M_WriteSysParamToCardOK:
    {
        break;
    }
    case M_ReadUserParamFromCardOK:
    {
        if (message.isEmpty())
        {
            throw new LaserDeviceDataException(E_TransferDataError, tr("Registers data incomplete."));
        }
        LaserRegister::batchParse(message, false);
        break;
    }
    case M_WriteUserParamToCardOK:
    {
        break;
    }
    case M_ReadComputerParamFromCardOK:
    {
        break;
    }
    case M_WriteComputerParamToCardOK:
    {
        break;
    }
    case M_FactoryPasswordValid:
    {
        emit manufacturePasswordVerified(true);
        break;
    }
    case M_ChangeFactoryPasswordOK:
    {
        break;
    }
    case M_ReturnTextMsgFromCallback:
    {
        break;
    }
    case M_ImportFromFile:
    {
        break;
    }
    case M_CancelCurrentWork:
    {
        break;
    }
    case M_TimeConsuming:
    {
        break;
    }
    case M_EstimatedWorkTime:
    {
        break;
    }
    case M_StartProcData:
    {
        break;
    }
    case M_DataTransCompleted:
    {
        break;
    }
    case M_RequestAndContinue:
    {
        break;
    }
    case M_MotorLock:
    {
        break;
    }
    case M_MotorUnlock:
    {
        break;
    }
    case M_LaserLightOn:
    {
        break;
    }
    case M_LaserLightOff:
    {
        break;
    }
    case M_StartWorking:
    {
        break;
    }
    case M_PauseWorking:
    {
        break;
    }
    case M_ContinueWorking:
    {
        break;
    }
    case M_StopWorking:
    {
        break;
    }
    case M_MachineWorking:
    {
        break;
    }
    case M_Idle:
    {
        break;
    }
    case M_WorkFinished:
    {
        break;
    }
    case M_DeviceIdInfo:
    {
        break;
    }
    case M_ClientAddressInfo:
    {
        break;
    }
    case M_ConnectedServer:
    {
        break;
    }
    case M_DisconnectServer:
    {
        break;
    }
    case M_ConnectServerOK:
    {
        break;
    }
    case M_SubmitToServerOK:
    {
        break;
    }
    case M_DownloadBegin:
    {
        break;
    }
    case M_DownloadEnd:
    {
        break;
    }
    case M_NewVersionChecking:
    {
        break;
    }
    case M_NewVersionCheckFinished:
    {
        break;
    }
    case M_IsLatestVersion:
    {
        break;
    }
    case M_ReadyToUpdateFile:
    {
        break;
    }
    case M_DownloadUpdateInfoFile:
    {
        break;
    }
    case M_FoundSoftNewVersion:
    {
        break;
    }
    case M_DownloadFileCounts:
    {
        break;
    }
    case M_DownloadFileIndex:
    {
        break;
    }
    case M_DownloadSoftDataStart:
    {
        break;
    }
    case M_StartSoftUpdate:
    {
        break;
    }
    case M_CancelSoftUpdate:
    {
        break;
    }
    case M_SoftUpdateFinished:
    {
        break;
    }
    case M_FoundFirmwareNewVersion:
    {
        break;
    }
    case M_DownloadFirmwareDataStart:
    {
        break;
    }
    case M_DownloadFirmwareDataEnd:
    {
        break;
    }
    case M_SendFirmwareDataStart:
    {
        break;
    }
    case M_SendFirmwareDataEnd:
    {
        break;
    }
    case M_UpdateFirmwareStart:
    {
        break;
    }
    case M_UpdateFirmwareEnd:
    {
        break;
    }
    case M_UpdateFirmwareAbort:
    {
        break;
    }
    case M_SaveParamsToServerOK:
    {
        break;
    }
    case M_ReadParamsFromServerOK:
    {
        break;
    }
    case M_UpdateComplete:
    {
        LaserApplication::mainWindow->close();
        break;
    }
    }

}

void LaserDevice::onLibraryLoaded(bool success)
{
    Q_D(LaserDevice);
    qLogD << "LaserDevice::onLibraryLoaded: success = " << success;
    try {
        d->driver->init(LaserApplication::mainWindow->winId());
        QString systemDate(__DATE__);
        qLogD << "system date: " << systemDate;
        QDate compileDate = QLocale("en_US").toDate(systemDate.simplified(), "MMM d yyyy");
        int year = compileDate.year() % 100;
        int month = compileDate.month();
        int day = compileDate.day();
        int version = year * 10000 + month * 100 + day;
        int winId = d->driver->getUpdatePanelHandle(version, LaserApplication::mainWindow->winId());
        LaserApplication::mainWindow->createUpdateDockPanel(winId);
    }
    catch (...) {

    }
}

void LaserDevice::onLibraryInitialized()
{
    Q_D(LaserDevice);
    qLogD << "LaserDevice::onLibraryInitialized";
    d->driver->getPortList();
}

void LaserDevice::onComPortsFetched(const QStringList& portNames)
{
    Q_D(LaserDevice);
    if (portNames.length() == 1)
    {
        connectDevice(portNames[0]);
    }
    else if (portNames.length() > 1)
    {
        if (Config::Device::autoConnectFirst())
        {
            connectDevice(portNames[0]);
        }
    }
}

void LaserDevice::onConnected()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        d->driver->readAllSysParamFromCard();
        d->driver->setFactoryType("LaserController");
        //d->driver->getMainCardRegisterState();
        //QString compileInfo = d->driver->getCompileInfo();
        //qLogD << "compile info: " << compileInfo;
        //QString laserLibraryInfo = d->driver->getLaserLibraryInfo();
        //qLogD << "laser library info: " << laserLibraryInfo;
        //QString mainCardId = d->driver->getMainCardID();
        //qLogD << "main card id: " << mainCardId;
    }
}

void LaserDevice::onMainCardRegistered()
{
    Q_D(LaserDevice);
    d->mainCard = requestMainCardId();
    qLogD << "Hardware id: " << d->mainCard;
    qLogD << "Main card id: " << requestMainCardId();
    qLogD << "Dongle id: " << requestDongleId();
    requestMainCardInfo();
}

void LaserDevice::onMainCardActivated(bool temp)
{
    Q_D(LaserDevice);
    qLogD << "main card activated. temp? " << temp;
}

