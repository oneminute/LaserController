#include <QtTest>
#include <QCoreApplication>
#include <QTest>

#include <iostream>

#include "DllTest.h"

class DllTestTesting : public QObject
{
    Q_OBJECT

public:
    DllTestTesting();
    ~DllTestTesting();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_fn1();
    void test_callback();

private:
};

DllTestTesting::DllTestTesting()
{
}

DllTestTesting::~DllTestTesting()
{
}

void DllTestTesting::initTestCase()
{
    //QVERIFY(InitComPortLib());
}

void DllTestTesting::cleanupTestCase()
{
    //QVERIFY(UninitComPortLib());
}

void DllTestTesting::test_fn1()
{
    testFn1();
}

void DllTestTesting::test_callback()
{
    setCallback([]()
        {
            qDebug() << "callback done.";
        }
    );
    doCallback();
}

//QTEST_MAIN(ComPortTesting)
QTEST_APPLESS_MAIN(DllTestTesting)

#include "DllTest.moc""

