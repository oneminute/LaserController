#include "common/Config.h"
#include "LaserApplication.h"
#include "laser/LaserDriver.h"

#include <opencv2/opencv.hpp>

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
    LaserApplication *app = new LaserApplication(argc, argv);

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
