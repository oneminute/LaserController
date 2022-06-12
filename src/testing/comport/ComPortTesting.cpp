#include <QtTest>
#include <QCoreApplication>
#include <QTest>

#include <iostream>

#include "ComPortController.h"

class ComPortTesting : public QObject
{
    Q_OBJECT

public:
    ComPortTesting();
    ~ComPortTesting();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_getAvailablePorts();
    void test_open();
    void test_write1();
    void test_write2();
    void test_write3();
    void test_close();

private:
};

ComPortTesting::ComPortTesting()
{
}

ComPortTesting::~ComPortTesting()
{
}

void ComPortTesting::initTestCase()
{
    QVERIFY(InitComPortLib());
}

void ComPortTesting::cleanupTestCase()
{
    QVERIFY(UninitComPortLib());
}

void ComPortTesting::test_getAvailablePorts()
{
    wchar_t* portsString = GetComPortList();
    std::wstring str(portsString);
    QVERIFY(str.length());
}

void ComPortTesting::test_open()
{
    QCOMPARE(OpenComPort(8, 115200), 8);
}

void ComPortTesting::test_write1()
{
    QByteArray data = "ABCDEF01F70B03FFD9010000012EBC700001012EBC700002012EBC700003012EBC700004012EBC700005012EBC700006012EBC70000"
        "7012EBC700008012EBC700009012EBC70000A012EBC70000B012EBC70000C012EBC70000D012EBC70000E012EBC70000F012EBC700010012EBC7000"
        "11012EBC700012012EBC700013012EBC700014012EBC700015012EBC700016012EBC700017012EBC700018012EBC700019012EBC70001A012EBC700"
        "01B012EBC70001C012EBC70001D012EBC70001E012EBC70001F012EBC700020012EBC700021012EBC700022012EBC700023012EBC700024012EBC70"
        "0025012EBC700026012EBC700027012EBC700028012EBC700029012EBC70002A012EBC70002B012EBC70002C012EBC70002D012EBC70002E012EBC7"
        "0002F012EBC700030012EBC700031012EBC700032012EBC700033012EBC700034012EBC700035012EBC700036012EBC700037012EBC700038012EBC7"
        "00039012EBC70003A012EBC70003B012EBC70003C012EBC70003D012EBC70003E012EBC70003F012EBC700040012EBC700041012EBC700042012EBC7"
        "00043012EBC700044012EBC700045012EBC700046012EBC700047012EBC700048012EBC700049012EBC70004A012EBC70004B012EBC70004C012EBC7"
        "0004D012EBC70004E012EBC70004F012EBC701BE694450D7007825D0C71DA43"; //读系统参数
    int length = WriteData(8, data.data(), data.length());
    QCOMPARE(length, data.length());
}

void ComPortTesting::test_write2()
{
    QThread::currentThread()->msleep(100);
    QByteArray data = QByteArray::fromHex("ABCDEF00270502FFD8FDA100000000780000C35079000186A00DA7000000000D963A8EDD5F202E"); //移动至
    int length = WriteData(8, data.data(), data.length());
    QCOMPARE(length, data.length());
    QThread::currentThread()->msleep(100);
    char* buf;
    length = ReadData(8, buf);
    QByteArray result(buf, length);
    qDebug() << "result:" << result.toHex();
    QVERIFY(result.length());
}

void ComPortTesting::test_write3()
{
    QThread::currentThread()->msleep(3000);
    QByteArray data = QByteArray::fromHex("ABCDEF00270502FFD8FFA100000000580000000059000000000DA7000000000D47AC37028D690F"); //回原点
    int length = WriteData(8, data.data(), data.length());
    QCOMPARE(length, data.length());
    QThread::currentThread()->msleep(100);
    char* buf;
    length = ReadData(8, buf);
    QByteArray result(buf, length);
    qDebug() << "result:" << result.toHex();
    QVERIFY(result.length());
}

void ComPortTesting::test_close()
{
    QCOMPARE(CloseComPort(8), 8);
}

//QTEST_MAIN(ComPortTesting)
QTEST_APPLESS_MAIN(ComPortTesting)

#include "ComPortTesting.moc""

