#include "common/Config.h"
#include "LaserApplication.h"
#include "laser/LaserDriver.h"

#include <QPainterPath>

#include <opencv2/opencv.hpp>
#include <glog/logging.h>

#ifdef _MSC_VER
#    ifdef NDEBUG
#        pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#    else
#        pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#    endif
#endif
//#        pragma comment(linker, "/SUBSYSTEM:CONSOLE")

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    LaserApplication *app = new LaserApplication(argc, argv);
    app->arg0 = QString::fromLatin1(argv[0]);
    qLogD << "arg0 = " << app->arg0;

    qRegisterMetaType<QPainterPath>("QPainterPath");
    qRegisterMetaType<QList<QPointF>>("QList<QPointF>");
    qRegisterMetaType<cv::Mat>("cv::Mat");

    if (!app->initialize())
    {
        return -1;
    }

    int ret = app->exec();

    app->destroy();
    delete app;

    return ret;
}
