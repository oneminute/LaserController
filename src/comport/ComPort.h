#pragma once

#include <QMutex>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QThread>
#include <QWaitCondition>

class ComPort : public QObject
{
    Q_OBJECT

public:
    explicit ComPort(QObject *parent = nullptr);
    ~ComPort();

    QString errorString() const { return m_errorString; }

public slots:
    int openSync(int port, const QSerialPortInfo& portInfo, int baudRate);
    void openAsync(int port, const QSerialPortInfo& portInfo, int baudRate);

    void closeSync();
    void closeAsync();

    int writeSync(const QByteArray& data);
    void writeAsync(const QByteArray& data);

    QByteArray readSync();
    void readAsync();
    int readBuffer(char*& buf, int& length);

    bool isOpen();
    int port() const;

protected slots:
    void open(int port, const QSerialPortInfo& portInfo, int baudRate);
    void close();
    void write(const QByteArray& data);
    void read();

protected slots:
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void onReadyRead();

private:
    bool readDataInternal();

signals:
    void openSyncSignal(int port, const QSerialPortInfo& portInfo, int baudRate);
    void openAsyncSignal(int port, const QSerialPortInfo& portInfo, int baudRate);
    void closeSyncSignal();
    void closeAsyncSignal();
    void writeSyncSignal(const QByteArray& data);
    void writeAsyncSignal(const QByteArray& data);
    void readSyncSignal();
    void readAsyncSignal();

    void error(const QString &s);
    void dataArrived(int port);

private:
    QString m_portName;
    QMutex m_mutex;
    int m_port = -1;
    QSerialPort* m_serial;
    QThread* m_thread;
    int m_byteWritten = 0;
    QByteArray m_dataRead;
    QString m_errorString;
};

