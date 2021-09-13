#include "common/Config.h"
#include "LaserApplication.h"
#include "laser/LaserDriver.h"

#ifdef _MSC_VER
#    ifdef NDEBUG
#        pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#    else
#        pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#    endif
#endif

int main(int argc, char *argv[])
{
    LaserApplication *app = new LaserApplication(argc, argv);
    qRegisterMetaType<QPainterPath>("QPainterPath");
    if (!app->initialize())
    {
        return -1;
    }

    int ret = app->exec();

    app->destroy();
    delete app;

    return ret;
}
