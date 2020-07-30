//#include "gtest/gtest.h"
#include <QApplication>

#include "TestLaserDriver.h"
#include "TestStateController.h"
#include "TestLaserDocument.h"

QT_BEGIN_NAMESPACE 
QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS 
QT_END_NAMESPACE 
int main(int argc, char *argv[]) 
{ 
    QApplication app(argc, argv); 
    app.setAttribute(Qt::AA_Use96Dpi, true); 
    QTEST_DISABLE_KEYPAD_NAVIGATION 
    QTEST_ADD_GPU_BLACKLIST_SUPPORT 

    TestLaserDriver testLaserDriver; 
    TestStateController testStateController;
    TestLaserDocument testLaserDocument;

    QTEST_SET_MAIN_SOURCE_PATH

    int ret = 0;
    ret += QTest::qExec(&testLaserDriver, argc, argv);
    ret += QTest::qExec(&testStateController, argc, argv);
    ret += QTest::qExec(&testLaserDocument, argc, argv);

    //ret += app.exec();
    QTest::qWait(1000);
    return ret;
}

//int main(int argc, char **argv)
//{
//    QApplication a(argc, argv);
//    testing::InitGoogleTest(&argc, argv);
//    return RUN_ALL_TESTS();
//}
