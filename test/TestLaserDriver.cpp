#include "TestLaserDriver.h"

#include <QWidget>
#include <QDebug>
#include <QThread>

void TestLaserDriver::laserDriverTestCase()
{
    LaserDriver& driver = LaserDriver::instance();
    QVERIFY(driver.load());
    QWidget w;
    w.setWindowTitle("test");
    w.show();
    QString version = driver.getVersion();
    QString compileInfo = driver.getCompileInfo();
    qDebug() << version;
    qDebug() << compileInfo;
    driver.init(w.winId());
    QList<int> ports = driver.getPortList();
    qDebug() << "ports:" << ports;
    QVERIFY(ports.length() > 0);
    //QVERIFY(driver.initComPort(ports[0]));
    //QVERIFY(driver.unInitComPort());
    driver.setTransTimeOutInterval(20);
    driver.setSoftwareInitialization(1016, 10, 10, 210, 297);
    driver.setHardwareInitialization(0.2, 300, 2, 1);
    QList<int> addrs;
    QList<double> values;
    addrs << 3 << 5 << 6;
    values << 0.003 << 0.004 << 0.005;
    QVERIFY(driver.writeSysParamToCard(addrs, values));
    QVERIFY(driver.readSysParamFromCard(addrs));
    //driver.showAboutWindow();
    driver.lPenMoveToOriginalPoint(0.1);
    driver.lPenQuickMoveTo(0, true, 30, 30, 1, 0.1, 0.1);
    driver.controlHDAction(1);
    QVERIFY(!driver.getMainCardID().isEmpty());
    qDebug() << "GetCurrentLaserPos:" << driver.GetCurrentLaserPos();
    driver.smallScaleMovement(true, false, 2, 100, 200, 300);
    driver.loadDataFromFile("D:/LaserController/install/shapes.svg");
    driver.startMachining(true);
    driver.pauseContinueMachining(true);
    //driver.stopMachining();
    driver.controlMotor(true);
    driver.testLaserLight(true);
    driver.unload();
}

