#include "ComPortController.h"

#include <QCoreApplication>
#include <QDebug>
#include <QRegularExpression>

QCoreApplication* g_app = nullptr;
ComPortController* ComPortController::m_instance(nullptr);

ComPortController::ComPortController()
    : m_fnAfterOpenHandler(nullptr)
    , m_fnAfterCloseHandler(nullptr)
    , m_fnDataArrivedHandler(nullptr)
{
}

ComPortController::~ComPortController()
{
    for (QMap<int, ComPort*>::Iterator i = m_comPorts.begin(); i != m_comPorts.end(); i++)
    {
        i.value()->closeSync();
    }
    qDeleteAll(m_comPorts);
}

QStringList ComPortController::getAvailablePorts()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QStringList infos;
    QRegularExpression re("((COM)|(com))(\\d+)");
    for (const QSerialPortInfo& info : ports)
    {
        QString portName = QString("%1(%2)").arg(info.description()).arg(info.portName());
        QRegularExpressionMatch match = re.match(info.portName());
        QString portNum = match.captured(4);
        bool ok = false;
        int port = portNum.toInt(&ok);
        if (!ok)
            continue;
        infos << QString::number(port);
        //qDebug() << info.description() << info.portName() << info.manufacturer() << info.serialNumber() << port;
        m_comPortInfos[port] = info;
    }
    return infos;
}

int ComPortController::openComPort(int port, int baudRate)
{
    qDebug() << QThread::currentThread() << "ComPortController::openComPort()" << port << baudRate;
    if (m_comPortInfos.isEmpty())
    {
        getAvailablePorts();
    }
    if (m_comPortInfos.isEmpty())
        return -1;
    if (!m_comPortInfos.contains(port))
    {
        return -1;
    }
    ComPort* comPort = nullptr;
    if (m_comPorts.contains(port))
    {
        comPort = m_comPorts[port];
    }
    else
    {
        comPort = new ComPort;
        m_comPorts.insert(port, comPort);
        QObject::connect(comPort, &ComPort::dataArrived, [=](int port)
            {
                if (m_fnDataArrivedHandler)
                    m_fnDataArrivedHandler(port);
            }
        );
    }
    int result = comPort->openSync(port, m_comPortInfos[port], baudRate);
    qDebug() << "m_fnAfterOpenHandler" << m_fnAfterOpenHandler;
    if (m_fnAfterOpenHandler)
        m_fnAfterOpenHandler(port);
    return result;
}

int ComPortController::closeComPort(int port)
{
    ComPort* comPort = nullptr;
    if (m_comPorts.contains(port))
    {
        comPort = m_comPorts[port];
    }
    else
    {
        return -1;
    }
    comPort->closeSync();
    if (m_fnAfterCloseHandler)
        m_fnAfterCloseHandler(port);
    qDebug() << "close com port" << port;
    return comPort->port();
}

int ComPortController::write(int port, const QByteArray& data)
{
    ComPort* comPort = nullptr;
    qDebug() << "ComPortController::write(): " << port << m_comPorts.contains(port);
    if (m_comPorts.contains(port))
    {
        comPort = m_comPorts[port];
    }
    else
    {
        return -1;
    }
    qDebug() << "ComPortController::write(): isOpen:" << comPort->isOpen();
    if (!comPort->isOpen())
        return -1;
    int result = comPort->writeSync(data);
    return result;
}

QByteArray ComPortController::read(int port)
{
    ComPort* comPort = nullptr;
    if (m_comPorts.contains(port))
    {
        comPort = m_comPorts[port];
    }
    else
    {
        return QByteArray();
    }
    if (!comPort->isOpen())
        return QByteArray();

    return comPort->readSync();
}

int ComPortController::readBuffer(int port, char*& buf)
{
    ComPort* comPort = nullptr;
    if (m_comPorts.contains(port))
    {
        comPort = m_comPorts[port];
    }
    else
    {
        return -1;
    }
    if (!comPort->isOpen())
        return -1;

    int length = 0;
    comPort->readBuffer(buf, length);
    return length;
}

bool ComPortController::exist(int port) const
{
    return m_comPorts.contains(port);
}

ComPortController& ComPortController::instance()
{
    if (m_instance == nullptr)
        m_instance = new ComPortController;
    return *m_instance;
}

void ComPortController::destroy()
{
    if (m_instance)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}

void ComPortController::setAfterOpenHandler(ComPortOpenCloseHandler handler) 
{
    m_fnAfterOpenHandler = handler; 
}

void ComPortController::setAfterCloseHandler(ComPortOpenCloseHandler handler)
{
    m_fnAfterCloseHandler = handler;
}

void ComPortController::setDataArrivedHandler(ComPortDataArrivedHandler handler)
{
    m_fnDataArrivedHandler = handler;
}

COMPORT_C_EXPORT PCHAR GetComPortList()
{
    static std::wstring portsString;
    QStringList ports = ComPortController::instance().getAvailablePorts();
    QString portsStr = ports.join(";");
    portsString.reserve(portsStr.length() + 1);
    portsStr.toWCharArray(const_cast<wchar_t*>(portsString.c_str()));
    return const_cast<wchar_t*>(portsString.c_str());
}

COMPORT_C_EXPORT bool InitComPortLib()
{
    if (g_app == nullptr)
    {
        int argc = 0;
        if (QCoreApplication::instance() == nullptr)
            g_app = new QCoreApplication(argc, nullptr);
    }
    return true;
}

COMPORT_C_EXPORT bool UninitComPortLib()
{
    ComPortController::destroy();
    if (g_app != nullptr)
    {
        delete g_app;
        g_app = nullptr;
    }
    return true;
}

COMPORT_C_EXPORT int OpenComPort(int port, int baudRate)
{
    return ComPortController::instance().openComPort(port, baudRate);
}

COMPORT_C_EXPORT int CloseComPort(int port)
{
    return ComPortController::instance().closeComPort(port);
}

COMPORT_C_EXPORT bool IsComPortExisted(int port)
{
    return ComPortController::instance().exist(port);
}

COMPORT_C_EXPORT void SetAfterOpen(ComPortOpenCloseHandler handler)
{
    ComPortController::instance().setAfterOpenHandler(handler);
}

COMPORT_C_EXPORT void SetAfterClose(ComPortOpenCloseHandler handler)
{
    ComPortController::instance().setAfterCloseHandler(handler);
}

COMPORT_C_EXPORT void SetDataArrived(ComPortDataArrivedHandler handler)
{
    ComPortController::instance().setDataArrivedHandler(handler);
}

COMPORT_C_EXPORT int WriteData(int port, char* buf, int length)
{
    qDebug() << "write to port" << port;
    QByteArray data(buf, length);
    return ComPortController::instance().write(port, data);
}


COMPORT_C_EXPORT int ReadData(int port, char*& buf)
{
    int length = ComPortController::instance().readBuffer(port, buf);
    return length;
}

//COMPORT_C_EXPORT void LockReadBuffer()
//{
//    return void();
//}
//
//COMPORT_C_EXPORT void UnlockReadBuffer()
//{
//    return void();
//}

