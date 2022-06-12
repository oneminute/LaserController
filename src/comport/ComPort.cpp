#include "ComPort.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMutexLocker>
#include <QSerialPort>
#include <QTime>

ComPort::ComPort(QObject *parent) 
    : QObject(parent)
    , m_serial(nullptr)
    , m_thread(new QThread)
{
    connect(this, &ComPort::openSyncSignal, this, &ComPort::open, Qt::BlockingQueuedConnection);
    connect(this, &ComPort::openAsyncSignal, this, &ComPort::open, Qt::QueuedConnection);
    connect(this, &ComPort::closeSyncSignal, this, &ComPort::close, Qt::BlockingQueuedConnection);
    connect(this, &ComPort::closeAsyncSignal, this, &ComPort::close, Qt::QueuedConnection);
    connect(this, &ComPort::writeSyncSignal, this, &ComPort::write, Qt::BlockingQueuedConnection);
    connect(this, &ComPort::writeAsyncSignal, this, &ComPort::write, Qt::QueuedConnection);
    connect(this, &ComPort::readSyncSignal, this, &ComPort::read, Qt::BlockingQueuedConnection);
    connect(this, &ComPort::readAsyncSignal, this, &ComPort::read, Qt::QueuedConnection);

    this->moveToThread(m_thread);
    m_thread->start();
}

ComPort::~ComPort()
{
    QMutexLocker lock(&m_mutex);
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
    m_thread = nullptr;
}

void ComPort::open(int port, const QSerialPortInfo& portInfo, int baudRate)
{
    QMutexLocker lock(&m_mutex);
    qDebug() << QThread::currentThread() << "ComPort::open()";
    if (!m_serial)
    {
        m_serial = new QSerialPort;
        connect(m_serial, &QSerialPort::errorOccurred, this, &ComPort::onErrorOccurred);
        connect(m_serial, &QSerialPort::readyRead, this, &ComPort::onReadyRead);
    }
    if (m_serial->isOpen())
    {
        m_serial->close();
    }
    m_serial->setPort(portInfo);
    if (!m_serial->setBaudRate(baudRate))
    {
        m_errorString = QString("Port %1 does not surpport baud rate of %2.").arg(port).arg(baudRate);
        qWarning() << m_errorString;
        emit error(m_errorString);
        return;
    }
    if (!m_serial->open(QIODevice::ReadWrite))
    {
        m_errorString = QString("Can't open port %1").arg(port);
        qWarning() << m_errorString;
        m_port = -1;
        emit error(m_errorString);
        return;
    }
    qDebug() << "Port" << port << "opened";
    m_port = port;
}

int ComPort::openSync(int port, const QSerialPortInfo& portInfo, int baudRate)
{
    emit openSyncSignal(port, portInfo, baudRate);
    return m_port;
}

void ComPort::openAsync(int port, const QSerialPortInfo& portInfo, int baudRate)
{
    emit openAsyncSignal(port, portInfo, baudRate);
}

void ComPort::close()
{
    QMutexLocker lock(&m_mutex);
    qDebug() << "ComPort::close()";
    if (m_serial)
    {
        m_serial->close();
        delete m_serial;
        m_serial = nullptr;
        qDebug() << "com port closed";
    }
}

void ComPort::closeSync()
{
    if (isOpen())
        emit closeSyncSignal();
}

void ComPort::closeAsync()
{
    if (isOpen())
        emit closeAsyncSignal();
}

void ComPort::write(const QByteArray& data)
{
    qDebug() << "ComPort::write(): data writing:" << data.toHex();
    QMutexLocker lock(&m_mutex);
    if (data.isEmpty())
        return;
    m_byteWritten = m_serial->write(data);
}

int ComPort::writeSync(const QByteArray& data)
{
    qDebug() << "ComPort::writeSync(): data writing:" << data.toHex();
    emit writeSyncSignal(data);
    return m_byteWritten;
}

void ComPort::writeAsync(const QByteArray& data)
{
    emit writeAsyncSignal(data);
}

void ComPort::read()
{
    QMutexLocker locker(&m_mutex);
    qDebug() << "ComPort::read(): empty: " << m_dataRead.isEmpty() << m_dataRead.size();
    if (m_dataRead.isEmpty())
    {
    qDebug() << "ComPort::read() start to reading:" << m_dataRead.toHex();
        readDataInternal();
    }
}

QByteArray ComPort::readSync()
{
    emit readSyncSignal();
    qDebug() << "ComPort::readSync() data ptr:" << (void*)m_dataRead.data();
    return m_dataRead;
}

void ComPort::readAsync()
{
    emit readAsyncSignal();
}

int ComPort::readBuffer(char*& buf, int& length)
{
    length = m_dataRead.length();
    buf = m_dataRead.data();
    return length;
}

bool ComPort::isOpen()
{
    if (!m_serial)
        return false;
    //qDebug() << QThread::currentThread() << "ComPort::isOpen()";
    return m_serial->isOpen();
}
int ComPort::port() const
{
    return m_port;
}

void ComPort::onErrorOccurred(QSerialPort::SerialPortError error)
{
    qWarning() << "error:" << error << m_serial->errorString();
}

void ComPort::onReadyRead()
{
    QMutexLocker locker(&m_mutex);
    qDebug() << "ComPort::onReadyRead() available bytes to read:" << m_serial->bytesAvailable();
    readDataInternal();
    emit dataArrived(m_port);
}

bool ComPort::readDataInternal()
{
    if (m_serial->waitForReadyRead(100))
    {
        m_dataRead = m_serial->readAll();
        while (m_serial->waitForReadyRead(10))
            m_dataRead += m_serial->readAll();
        qDebug() << "ComPort::readDataInternal():" << m_dataRead.toHex();
        return true;
    }
    else
    {
        qWarning() << "No data";
        return false;
    }
}

