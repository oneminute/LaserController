#include "common/Config.h"
#include "LaserApplication.h"
#include "laser/LaserDriver.h"

int main(int argc, char *argv[])
{
    //QApplication app(argc, argv);
    LaserApplication *app = new LaserApplication(argc, argv);
    if (!app->initialize())
    {
        return -1;
    }

    int ret = app->exec();
    //LaserDriver* driver = new LaserDriver;
    //driver->load();
    //driver->init(0);
    //int ret = app.exec();

    app->destroy();
    //QThread::sleep(5);
    delete app;
    //driver->unload();
    //delete driver;

    //return ret;
    return 0;
}
