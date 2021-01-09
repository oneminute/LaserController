#include "common/Config.h"
#include "laser/LaserDriver.h"
#include "scene/LaserPrimitive.h"
#include "state/StateController.h"
#include "state/StateController.h"
#include "ui/LaserControllerWindow.h"
#include "version.h"

#include <QApplication>
#include <qstylefactory.h>
#include <qtranslator.h>
#include <QDebug>
#include <QDataStream>
#include <QTextStream>

//#define GLOG_NO_ABBREVIATED_SEVERITIES
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glog/logging.h>
#include <glog/log_severity.h>
#include <iostream>

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    QFileInfo fileInfo(QString::fromLocal8Bit(file));
    const char* function = context.function ? context.function : "";
    switch (type)
    {
    case QtDebugMsg:
        LOG(INFO) << "[" << fileInfo.fileName().toStdString() << ":" << context.line << /*" " << function <<*/ "] " << localMsg.constData();
        break;
    case QtInfoMsg:
        LOG(INFO) << "[" << fileInfo.fileName().toStdString() << ":" << context.line << /*" " << function <<*/ "] " << localMsg.constData();
        break;
    case QtWarningMsg:
        LOG(WARNING) << "[" << fileInfo.fileName().toStdString() << ":" << context.line << /*" " << function <<*/ "] " << localMsg.constData();
        break;
    case QtCriticalMsg:
        LOG(ERROR) << "[" << fileInfo.fileName().toStdString() << ":" << context.line << /*" " << function <<*/ "] " << localMsg.constData();
        break;
    case QtFatalMsg:
        LOG(FATAL) << "[" << fileInfo.fileName().toStdString() << ":" << context.line << /*" " << function <<*/ "] " << localMsg.constData();
        break;
    }
}

void initLog(char* argv)
{
	QDir rootDir(".");
	if (!rootDir.exists("log"))
	{
		rootDir.mkdir("log");
	}
    std::cout << argv << std::endl;
    google::InitGoogleLogging(argv);
    google::InstallFailureSignalHandler();

    FLAGS_stderrthreshold = google::GLOG_ERROR;
    FLAGS_alsologtostderr = true; 
    FLAGS_colorlogtostderr = true; 
    FLAGS_max_log_size = 100;
    FLAGS_logbufsecs = 0; 
    FLAGS_stop_logging_if_full_disk = true; 

    //set log path;
    google::SetLogDestination(google::GLOG_INFO,   "log/logs");
    google::SetLogDestination(google::GLOG_WARNING,"log/logs");
    google::SetLogDestination(google::GLOG_ERROR,  "log/logs");
    google::SetLogDestination(google::GLOG_FATAL,  "log/logs");
    google::SetLogFilenameExtension("lc_");
}

void checkCrash()
{

}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDir dir(QApplication::applicationDirPath());
    QApplication::addLibraryPath(dir.absoluteFilePath("bin"));
    QApplication::setApplicationName(QObject::tr("LaserController"));
    QApplication::setApplicationDisplayName(QObject::tr("Laser Controller"));
    QApplication::setOrganizationName("OneMinute");
    QApplication::setApplicationVersion(QString("%1.%2.%3.%4").arg(LC_VERSION_MAJOR).arg(LC_VERSION_MINOR).arg(LC_VERSION_BUILD).arg(LC_VERSION_REVISION));
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    initLog(argv[0]);
    qInstallMessageHandler(messageOutput);

    qDebug() << "product name:" << QApplication::applicationName() << ", version:" << QApplication::applicationVersion();

    Config::load();

    QTranslator translator;
    QLocale locale(QLocale::Chinese);
    //qDebug() << "load translation file." << a.applicationName() << locale.name();
    if (translator.load(locale, app.applicationName(), QLatin1String("_"), QLatin1String("translations")))
    {
        qDebug() << "load translation file." << app.applicationName() << locale.name();
        //app.installTranslator(&translator);
    }

    QTranslator qtTranslator;
    if (qtTranslator.load(locale, QLatin1String("qt"), QLatin1String("_"), QLatin1String("translations")))
    {
        qDebug() << "load translation file." << "qt" << locale.name();
        //app.installTranslator(&qtTranslator);
    }

    QFile file("theme/Dark.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        //QTextStream stream(&file);
        //app.setStyleSheet(stream.readAll());
    }

    StateController::start();

    LaserControllerWindow w;
    w.showMaximized();

    int ret = app.exec();

    StateController::stop();
    return ret;
}
