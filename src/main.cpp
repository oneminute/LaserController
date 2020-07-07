#include "ui/MainWindow.h"
#include "state/StateController.h"

#include <QApplication>
#include <qstylefactory.h>
#include <qtranslator.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QTranslator translator;
    qDebug() << "load translation file." << a.applicationName() << QLocale::system().name();
    if (translator.load(QLocale(), a.applicationName(), QLatin1String("_")))
    {
        qDebug() << "load translation file." << a.applicationName() << QLocale::system().bcp47Name();
    }


    MainWindow w;
    w.show();
    int ret = a.exec();

    StateController::instance().fsm().stop();
    return ret;
}
