#pragma once

#include "ComPortExport.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QObject>
#include <QMap>
#include <QSerialPortInfo>
#include <QThread>

#include "ComPort.h"

typedef wchar_t* PCHAR;

/// <summary>
/// 打开/关闭端口后事件回调函数签名。
/// <param name="port">端口号</param>
/// </summary>
typedef void(__cdecl *ComPortOpenCloseHandler)(int port);

/// <summary>
/// 串口数据到达回调函数签名。
/// <param name="port">端口号</param>
/// </summary>
typedef void(__cdecl* ComPortDataArrivedHandler)(int port);

/// <summary>
/// 非C++程序无需直接调用此类。
/// </summary>
class COMPORT_EXPORT ComPortController
{
public:
    explicit ComPortController();
    ~ComPortController();

    QStringList getAvailablePorts();

    int openComPort(int port, int baudRate);

    int closeComPort(int port);

    int write(int port, const QByteArray& data);
    QByteArray read(int port);
    int readBuffer(int port, char*& buf);

    bool exist(int port) const;

    static ComPortController& instance();
    static void destroy();

    void setAfterOpenHandler(ComPortOpenCloseHandler handler);
    void setAfterCloseHandler(ComPortOpenCloseHandler handler);
    void setDataArrivedHandler(ComPortDataArrivedHandler handler);

private:
    QMap<int, QSerialPortInfo> m_comPortInfos;
    QMap<int, ComPort*> m_comPorts;

    ComPortOpenCloseHandler m_fnAfterOpenHandler;
    ComPortOpenCloseHandler m_fnAfterCloseHandler;
    ComPortDataArrivedHandler m_fnDataArrivedHandler;

    static ComPortController* m_instance;
};

/// <summary>
/// 初始化串口通信库。
/// </summary>
COMPORT_C_EXPORT bool InitComPortLib();

/// <summary>
/// 销毁串口通信库。
/// </summary>
COMPORT_C_EXPORT bool UninitComPortLib();

/// <summary>
/// 获取当前可用的串口列表。
/// <returns>返回以分号隔开的可用串口列表，每个列表是串口数字。如："3;4;8"。结尾无分号。</returns>
/// </summary>
COMPORT_C_EXPORT PCHAR GetComPortList();

/// <summary>
/// 打开指定串口。该函数可以在不关闭当前串口的情况下多次调用，但不建议这么做。
/// <param name="port">端口号</param>
/// <param name="baudRate">波特率</param>
/// <returns>如果成功打开指定串口，则返回串口号，否则反回-1。</returns>
/// </summary>
COMPORT_C_EXPORT int OpenComPort(int port, int baudRate);

/// <summary>
/// 关闭指定端口。
/// <param name="port">端口号</param>
/// <returns>如果成功关闭指定串口，则返回串口号，否则反回-1。</returns>
/// </summary>
COMPORT_C_EXPORT int CloseComPort(int port);

/// <summary>
/// 模块内针对每个已打开的端口分别维护了一个端口通信对象及相应的线程。该函数用于检测当前指定端口的通信对象是否已经存在。
/// 如果存在，往往指该端口已经打开。
/// <param name="port">端口号</param>
/// <returns>端口是否存在，存在则为true或1，否则为false或0</returns>
/// </summary>
COMPORT_C_EXPORT bool IsComPortExisted(int port);


/// <summary>
/// 设置端口打开后回调函数。传入的回调函数要保证可重入。
/// <param name="handler">回调函数</param>
/// </summary>
COMPORT_C_EXPORT void SetAfterOpen(ComPortOpenCloseHandler handler);

/// <summary>
/// 设置端口关闭后回调函数。传入的回调函数要保证可重入。
/// <param name="handler">回调函数</param>
/// </summary>
COMPORT_C_EXPORT void SetAfterClose(ComPortOpenCloseHandler handler);

/// <summary>
/// 设置端口数据到达后回调函数。传入的回调函数要保证可重入。
/// <param name="handler">回调函数</param>
/// </summary>
COMPORT_C_EXPORT void SetDataArrived(ComPortDataArrivedHandler handler);

/// <summary>
/// 向指定端口写入数据。
/// <param name="port">端口号</param>
/// <param name="buf">缓冲区指针</param>
/// <param name="length">数据字节长度，该长度为8位字节长度</param>
/// <returns>写入成功则返回写入数据长度，否则返回-1。</returns>
/// </summary>
COMPORT_C_EXPORT int WriteData(int port, char* buf, int length);

/// <summary>
/// 从指定端口读取数据。
/// <param name="port">端口号</param>
/// <param name="buf">缓冲区指针</param>
/// <returns>写入成功则返回写入数据长度，否则返回-1。</returns>
/// </summary>
COMPORT_C_EXPORT int ReadData(int port, char*& buf);

//COMPORT_C_EXPORT void LockReadBuffer();
//COMPORT_C_EXPORT void UnlockReadBuffer();
