#include "ui/MainWindow.h"
#include "ui/LaserControllerWindow.h"
#include "state/StateController.h"
#include "laser/LaserDriver.h"
#include "state/StateController.h"

#include <QApplication>
#include <qstylefactory.h>
#include <qtranslator.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication::addLibraryPath("./bin");
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QTranslator translator;
    qDebug() << "load translation file." << a.applicationName() << QLocale::system().name();
    if (translator.load(QLocale(), a.applicationName(), QLatin1String("_")))
    {
        qDebug() << "load translation file." << a.applicationName() << QLocale::system().bcp47Name();
    }

    StateController::start();

    LaserControllerWindow w;
    LaserDriver::instance().load();
    LaserDriver::instance().init(w.winId());
    w.showMaximized();
    int ret = a.exec();

    StateController::instance().fsm().stop();
    return ret;
}
