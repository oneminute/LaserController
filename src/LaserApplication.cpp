#include "LaserApplication.h"

#include <QDateTime>
#include <QDir>
#include <QStyleFactory>
#include <QTranslator>

#include "version.h"
#include "common/common.h"
#include "common/Config.h"
#include "exception/LaserException.h"
#include "laser/LaserDevice.h"
#include "laser/LaserDriver.h"
#include "state/StateController.h"
#include "ui/LaserControllerWindow.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glog/logging.h>
#include <glog/log_severity.h>
#include <iostream>

LaserApplication* LaserApplication::app(nullptr);
LaserControllerWindow* LaserApplication::mainWindow(nullptr);
LaserDevice* LaserApplication::device(nullptr);
LaserDriver* LaserApplication::driver(nullptr);

LaserApplication::LaserApplication(int argc, char** argv)
    : QApplication(argc, argv)
{
    if (app)
    {
        throw LaserFatalException(tr("Application startup failure."));
    }
    app = this;
}

LaserApplication::~LaserApplication()
{
    qDebug() << "application destroyed";
}

bool LaserApplication::initialize()
{
    QDir dir(LaserApplication::applicationDirPath());
    LaserApplication::addLibraryPath(dir.absoluteFilePath("bin"));
    LaserApplication::setApplicationName(QObject::tr("LaserController"));
    LaserApplication::setApplicationDisplayName(QObject::tr("Laser Controller"));
    LaserApplication::setOrganizationName("OneMinute");
    LaserApplication::setApplicationVersion(QString("%1.%2.%3.%4").arg(LC_VERSION_MAJOR).arg(LC_VERSION_MINOR).arg(LC_VERSION_BUILD).arg(LC_VERSION_REVISION));
    LaserApplication::setStyle(QStyleFactory::create("Fusion"));
    qDebug() << "product name:" << LaserApplication::applicationName() << ", version:" << LaserApplication::applicationVersion();

    checkCrash();
    initLog();
    qInstallMessageHandler(LaserApplication::handleLogOutput);

    Config::load();
	Global::unit = static_cast<SizeUnit>(Config::General::unit());

    QTranslator translator;
    //QLocale locale(QLocale::Config::GeneralLanguage());
    QLocale locale(QLocale::Chinese);
    qLogD << "language name: " << QLocale::languageToString(locale.language());
    if (translator.load(locale, applicationName(), QLatin1String("_"), QLatin1String("translations")))
    {
        qDebug() << "load translation file." << applicationName() << locale.name();
        installTranslator(&translator);
    }

    QTranslator qtTranslator;
    if (qtTranslator.load(locale, QLatin1String("qt"), QLatin1String("_"), QLatin1String("translations")))
    {
        qDebug() << "load translation file." << "qt" << locale.name();
        installTranslator(&qtTranslator);
    }

    QLocale::setDefault(locale);

    QFile file("theme/Dark.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        //QTextStream stream(&file);
        //app.setStyleSheet(stream.readAll());
    }

    driver = new LaserDriver;
    device = new LaserDevice;

    //driver->moveToThread(&m_deviceThread);
    //device->moveToThread(&m_deviceThread);

    connect(StateController::instance().deviceUnconnectedState(), &QState::entered, this, &LaserApplication::onEnterDeviceUnconnectedState);

    StateController::start();
    mainWindow = new LaserControllerWindow;
    mainWindow->show();

    device->resetDriver(driver);

    m_deviceThread.start();

    return true;
}

void LaserApplication::destroy()
{
    delete mainWindow;
    StateController::stop();

    if (device)
    {
        device->unload();
        delete device;
    }

    if (driver)
    {
        driver->unload();
        delete driver;
    }
    m_deviceThread.exit();
    m_deviceThread.wait();

    Config::save();
}

bool LaserApplication::notify(QObject * receiver, QEvent * event)
{
    bool done = true;
    try {
        done = QApplication::notify(receiver, event);
    }
    catch (LaserDeviceSecurityException* e)
    {
        qLogW << "laser security exception: " << e->toString();
        //mainWindow->handleSecurityException(e->errorCode(), e->errorMessage());
    }
    catch (LaserException* e)
    {
        qLogD << "laser exception: " << e->toString();
    }
    catch (QException* e)
    {
        qLogD << "QException: " << e;
    }
    catch (std::exception* e)
    {
        qLogD << "std::exception";
    }
    catch (...)
    {
        qLogD << "some exception else";

    }
    return done;
}

void LaserApplication::initLog()
{
    QDir rootDir(".");
	if (!rootDir.exists("log"))
	{
		rootDir.mkdir("log");
	}
    google::InitGoogleLogging(this->arguments()[0].toStdString().c_str());
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

void LaserApplication::clearCrash()
{
	QFile crash("crash.bin");
	if (crash.exists())
	{
		crash.remove();
	}
}

void LaserApplication::checkCrash()
{
    QFile crash("crash.bin");
	if (crash.exists())
	{
		// last running with a crash.
		qWarning() << "Found crash info.";
		crash.open(QIODevice::ReadOnly);
		QDataStream stream(&crash);
		QDateTime datetime;
		stream >> datetime;
		qWarning() << "crashed launching at " << datetime;
		crash.close();
		clearCrash();
	}
	crash.open(QIODevice::WriteOnly);
	QDataStream stream(&crash);
	stream << QDateTime::currentDateTime();
	crash.close();
}

void LaserApplication::handleLogOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
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

void LaserApplication::onEnterDeviceUnconnectedState()
{
    static bool first = true;
    if (first)
    {
        device->load();
        first = false;
    }
}
