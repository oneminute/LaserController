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
        , layoutRect(0, 0, 320, 210)
        , printerDrawUnit(1016)
    {}

    LaserDevice* q_ptr;
    LaserDriver* driver;

    QString portName;
    QRectF layoutRect;      // 加工的幅面宽
    int printerDrawUnit;    // 绘图仪单位，这里值的意思是一英寸分为多少个单位
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

void LaserDevice::resetDriver(LaserDriver* driver)
{
    Q_D(LaserDevice);
    unbindDriver();
    d->driver = driver;
    connect(d->driver, &LaserDriver::raiseError, this, &LaserDevice::handleError, Qt::ConnectionType::QueuedConnection);
    connect(d->driver, &LaserDriver::sendMessage, this, &LaserDevice::handleMessage, Qt::ConnectionType::QueuedConnection);

    connect(d->driver, &LaserDriver::libraryLoaded, this, &LaserDevice::onLibraryLoaded);
    connect(d->driver, &LaserDriver::libraryInitialized, this, &LaserDevice::onLibraryInitialized);

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

QString LaserDevice::hardwareId() const
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

QString LaserDevice::mainCardId() const
{
    Q_D(const LaserDevice);
    if (d->driver)
    {
        return d->driver->getMainCardID();
    }
    return "";
}

QString LaserDevice::dongleId() const
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
    QString cardId = mainCardId();
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
        emit mainCardInfoFetched(info);
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
        d->driver->getMainCardRegisterState();
    }
}

void LaserDevice::onMainCardRegistered()
{
    Q_D(LaserDevice);
    qLogD << "Hardware id: " << hardwareId();
    qLogD << "Main card id: " << mainCardId();
    qLogD << "Dongle id: " << dongleId();
}

void LaserDevice::onMainCardActivated(bool temp)
{
    Q_D(LaserDevice);
    qLogD << "main card activated. temp? " << temp;
}

