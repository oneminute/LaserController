#include "LaserDevice.h"

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
        , portName("")
    {}

    LaserDriver* driver;
    QString portName;
    LaserDevice* q_ptr;
};

LaserDevice::LaserDevice(QObject* parent)
    : QObject(parent)
    , m_ptr(new LaserDevicePrivate(this))
{
    ADD_TRANSITION(deviceUnconnectedState, deviceConnectedState, this, &LaserDevice::connected);
    ADD_TRANSITION(deviceConnectedState, deviceUnconnectedState, this, &LaserDevice::disconnected);
}

LaserDevice::~LaserDevice()
{
    qDebug() << "device destroyed";
}

void LaserDevice::resetDriver(LaserDriver* driver)
{
    Q_D(LaserDevice);
    unbindDriver();
    d->driver = driver;
    connect(d->driver, &LaserDriver::raiseError, this, &LaserDevice::handleError, Qt::ConnectionType::QueuedConnection);
    connect(d->driver, &LaserDriver::sendMessage, this, &LaserDevice::handleMessage, Qt::ConnectionType::QueuedConnection);

    connect(d->driver, &LaserDriver::libraryLoaded, this, &LaserDevice::onLibraryLoaded);
    connect(d->driver, &LaserDriver::libraryInitialized, this, &LaserDevice::onLibraryInitialized);

    connect(this, &LaserDevice::comPortsFetched, this, &LaserDevice::onComPortsFetched);
    load();
}

QString LaserDevice::portName() const
{
    Q_D(const LaserDevice);
    return d->portName;
}

void LaserDevice::load()
{
    Q_D(LaserDevice);
    d->driver->load();
}

void LaserDevice::unload()
{
    Q_D(LaserDevice);
    //d->driver->unload();
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

void LaserDevice::unbindDriver()
{
    Q_D(LaserDevice);
    if (d->driver)
    {
        disconnect(d->driver, &LaserDriver::raiseError, this, &LaserDevice::handleError);
        disconnect(d->driver, &LaserDriver::sendMessage, this, &LaserDevice::handleMessage);
    }
}


void LaserDevice::handleError(int code, const QString& message)
{
    LaserException* exception = nullptr;
    switch (code)
    {
    case E_SystemFatalError:
        exception = new LaserDeviceFatalException(code, tr("Laser device fatal error"));
        break;
    case E_UnknownError:
        exception = new LaserDeviceUnknownException(code);
        break;
    case E_InitializeError:
        exception = new LaserDeviceConnectionException(code, tr("Failed to initialize laser device"));
        break;
    case E_UninitializeError:
        exception = new LaserDeviceConnectionException(code, tr("Failed to uninitialize laser device normally"));
        break;
    case E_ComPortNotAvailable:
        exception = new LaserDeviceConnectionException(code, tr("Com port not available"));
        break;
    case E_GetComPortListError:
        exception = new LaserDeviceConnectionException(code, tr("Failed to get COM port list"));
        break;
    case E_DongleNotExists:
        exception = new LaserDeviceSecurityException(code, tr("Dongle does not exist"));
        break;
    case E_DongleActiveDisabled:
        exception = new LaserDeviceSecurityException(code, tr("Dongle activation is disabled"));
        break;
    case E_MainCardRegisterError:
        exception = new LaserDeviceSecurityException(code, tr("Failed to register main card"));
        break;
    case E_MainCardInactivated:
        exception = new LaserDeviceSecurityException(code, tr("Main card inactivated"));
        break;
    case E_InvalidMainCardId:
        exception = new LaserDeviceSecurityException(code, tr("Invalid main card ID"));
        break;
    case E_InvalidDongleId:
        exception = new LaserDeviceSecurityException(code, tr("Invalid dongle ID"));
        break;
    case E_CardBindDongleError:
        exception = new LaserDeviceSecurityException(code, tr("Failed to bind card with dongle"));
        break;
    case E_CardBindDongleRepeatedly:
        exception = new LaserDeviceSecurityException(code, tr("The card is repeatedly bound to the dongle"));
        break;
    case E_CardDongleBoundExceedsTimes:
        exception = new LaserDeviceSecurityException(code, tr("The number of times the card is bound to the dongle exceeds the allowable range"));
        break;
    case E_CardDongleBoundIllegal:
        exception = new LaserDeviceSecurityException(code, tr("The card is illegally bound to the dongle"));
        break;
    case E_ClearLaserTubeDurationError:
        exception = new LaserDeviceException(code, tr("Failed to clear duration of laser tube"));
        break;
    case E_FactoryPasswordIncorrect:
        exception = new LaserDeviceSecurityException(code, tr("Incorrect factory password"));
        break;
    case E_FactoryPasswordLengthError:
        exception = new LaserDeviceSecurityException(code, tr("Invalid length of factory password"));
        break;
    case E_FactoryPasswordExpired:
        exception = new LaserDeviceSecurityException(code, tr("Factory password expired"));
        break;
    case E_PasswordIncorrectTooManyTimes:
        exception = new LaserDeviceSecurityException(code, tr("Input incorrect factory password too many times"));
        break;
    case E_ChangeFactoryPasswordError:
        exception = new LaserDeviceSecurityException(code, tr("Failed to change factory password"));
        break;
    case E_ReadSysParamFromCardError:
        exception = new LaserDeviceIOException(code, tr("Failed to read parameters from device"));
        break;
    case E_WriteSysParamToCardError:
        exception = new LaserDeviceIOException(code, tr("Failed to write parameters to device"));
        break;
    case E_ReadUserParamFromCardError:
        exception = new LaserDeviceIOException(code, tr("Failed to read parameters from device"));
        break;
    case E_WriteUserParamToCardError:
        exception = new LaserDeviceIOException(code, tr("Failed to write parameters to device"));
        break;
    case E_SaveParamsToServerError:
        exception = new LaserNetworkException(code, tr("Failed to save parameters to server"));
        break;
    case E_LoadParamsFromServerError:
        exception = new LaserNetworkException(code, tr("Failed to load parameters from server"));
        break;
    case E_FileNotExistsError:
        exception = new LaserDeviceDataException(code, tr("File does not exist"));
        break;
    case E_InvalidDataFormat:
        exception = new LaserDeviceDataException(code, tr("Invalid data format"));
        break;
    case E_DecryptCommandError:
        exception = new LaserDeviceDataException(code, tr("Failed to decrypt data"));
        break;
    case E_InvalidImageData:
        exception = new LaserDeviceDataException(code, tr("Invalid image data"));
        break;
    case E_ImageMinSizeTooSmall:
        exception = new LaserDeviceDataException(code, tr("Min size of image is too small"));
        break;
    case E_ImageMaxSizeTooLarge:
        exception = new LaserDeviceDataException(code, tr("Max size of image is too large"));
        break;
    case E_NoDataError:
        exception = new LaserDeviceIOException(code, tr("No data transfered"));
        break;
    case E_TransferDataTimeout:
        exception = new LaserDeviceIOException(code, tr("Transfering data timeout"));
        break;
    case E_RetransferAfterTimeout:
        exception = new LaserDeviceIOException(code, tr("Retransfer data after timeout"));
        break;
    case E_RetransferTooManyTimes:
        exception = new LaserDeviceIOException(code, tr("Retransfer data too many times"));
        break;
    case E_TransferDataError:
        exception = new LaserDeviceIOException(code, tr("Failed to transfer data"));
        break;
    case E_ReceiveInvalidDataTooManyTimes:
        exception = new LaserDeviceIOException(code, tr("Receive invalid data too many times"));
        break;
    case E_BreakpointDataError:
        exception = new LaserDeviceIOException(code, tr("Failed to transfer data with breakpoint"));
        break;
    case E_CanNotDoOnWorking:
        exception = new LaserDeviceMachiningException(code, tr("This operation is not supported during machining"));
        break;
    case E_PingServerFail:
        exception = new LaserNetworkException(code, tr("Failed to connect to server"));
        break;
    case E_ConnectServerError:
        exception = new LaserNetworkException(code, tr("Failed to log in to server"));
        break;
    case E_ConnectFrequently:
        exception = new LaserNetworkException(code, tr("Login too frequently"));
        break;
    case E_SubmitToServerError:
        exception = new LaserNetworkException(code, tr("Failed to submit data"));
        break;
    case E_UpdateInfoFileNotExists:
        exception = new LaserNetworkException(code, tr("Updating info file does not exist"));
        break;
    case E_UpdateFileNotExists:
        exception = new LaserNetworkException(code, tr("Updating file dose not exist"));
        break;
    case E_UpdateFailed:
        exception = new LaserNetworkException(code, tr("Failed to update"));
        break;
    case E_DownloadFirmwareDataError:
        exception = new LaserNetworkException(code, tr("Failed to download firmware"));
        break;
    case E_UpdateFirmwareTimeout:
        exception = new LaserNetworkException(code, tr("Update firmware timeout"));
        break;
    }
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
        emit comPortConnected(portName());
        emit connected();
        break;
    }
    case M_ComPortClosed:
    {
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
        emit mainCardActivated();
        break;
    }
    }
}

void LaserDevice::onLibraryLoaded(bool success)
{
    Q_D(LaserDevice);
    qLogD << "LaserDevice::onLibraryLoaded: success = " << success;
    d->driver->init(LaserApplication::mainWindow->winId());
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
        if (Config::DeviceAutoConnectFirst())
        {
            connectDevice(portNames[0]);
        }
    }
}

