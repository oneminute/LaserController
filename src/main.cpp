#include "common/Config.h"
#include "LaserApplication.h"
#include "laser/LaserDriver.h"

int main(int argc, char *argv[])
{
    LaserApplication *app = new LaserApplication(argc, argv);
    if (!app->initialize())
    {
        return -1;
    }

    int ret = app->exec();

    app->destroy();
    delete app;

    return ret;
}
