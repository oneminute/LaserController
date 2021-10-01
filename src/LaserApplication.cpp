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
#include "ui/PreviewWindow.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glog/logging.h>
#include <glog/log_severity.h>
#include <iostream>

LaserApplication* LaserApplication::app(nullptr);
LaserControllerWindow* LaserApplication::mainWindow(nullptr);
PreviewWindow* LaserApplication::previewWindow(nullptr);
LaserDevice* LaserApplication::device(nullptr);
LaserDriver* LaserApplication::driver(nullptr);
QMap<QString, QString> LaserApplication::stringMap;

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
    QString appShortName("CNELaser");
    QDir dir(LaserApplication::applicationDirPath());
    LaserApplication::addLibraryPath(dir.absoluteFilePath("bin"));
    LaserApplication::setApplicationName(QObject::tr("CNE Laser"));
    LaserApplication::setApplicationDisplayName(QObject::tr("CNE Laser %1").arg(LC_VERSION_STR));
    LaserApplication::setOrganizationName(tr(""));
    LaserApplication::setApplicationVersion(QString("version: %1").arg(LC_VERSION_STR));
    //LaserApplication::setStyle(QStyleFactory::create("Fusion"));
    
    qLogD << "product name:" << LaserApplication::applicationName() << ", version:" << LaserApplication::applicationVersion();
    qLogD << "Styles that current operation system supported:";
    for (QString style : QStyleFactory::keys())
    {
        qLogD << "  style: " << style;
    }

    QLocale currentLocale = QLocale::system();
    qLogD << "Current locale: " << currentLocale;
    QStringList currentDisplayLanguages = QLocale::system().uiLanguages();
    QLocale displayLocale = QLocale(currentDisplayLanguages.first());
    qLogD << "Display languages: " << displayLocale << ", code: " << displayLocale.language();

    checkEnvironment();

    checkCrash();
    initLog();
    qInstallMessageHandler(LaserApplication::handleLogOutput);

    Config::init();
    Config::load();
	Global::unit = static_cast<SizeUnit>(Config::General::unit());

    QTranslator translator;
    QLocale locale(static_cast<QLocale::Language>(Config::General::language()));
    qLogD << "language code: " << Config::General::language() << ", " << QLocale::Chinese;
    qLogD << "language name: " << QLocale::languageToString(locale.language());
    if (translator.load(locale, appShortName, QLatin1String("_"), QLatin1String("translations")))
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
    //Config::load();
    retranslate();

    QFile file("theme/Dark.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        //QTextStream stream(&file);
        //setStyleSheet(stream.readAll());
    }

    driver = new LaserDriver;
    device = new LaserDevice(driver);

    connect(StateController::instance().deviceUnconnectedState(), &QState::entered, this, &LaserApplication::onEnterDeviceUnconnectedState);
    connect(Config::General::languageItem(), &ConfigItem::valueChanged, this, &LaserApplication::onLanguageChanged);

    StateController::start();
    previewWindow = new PreviewWindow(mainWindow);
    mainWindow = new LaserControllerWindow;
    mainWindow->showMaximized();

    //connect(StateController::instance().deviceState(), &QState::entered, device, &LaserDevice::load);
    //connect(mainWindow, &LaserControllerWindow::windowCreated, device, &LaserDevice::load);

    //device->load();


    g_deviceThread.start();

    return true;
}

void LaserApplication::destroy()
{
    delete mainWindow;
    StateController::stop();

    if (driver)
    {
        driver->unload();
        delete driver;
    }
    
    if (device)
    {
        device->unload();
        delete device;
    }
    
    if (previewWindow)
    {
        delete previewWindow;
    }
    g_deviceThread.exit();
    g_deviceThread.wait();

    Config::destroy();
}

bool LaserApplication::checkEnvironment()
{
    QDir baseDir = QDir::current();
    qLogD << baseDir;
    if (!baseDir.exists("tmp"))
    {
        baseDir.mkdir("tmp");
    }
	if (!baseDir.exists("log"))
	{
		baseDir.mkdir("log");
	}
    if (!baseDir.exists("config"))
    {
        baseDir.mkdir("config");
    }
    return true;
}

//bool LaserApplication::notify(QObject * receiver, QEvent * event)
//{
//    bool done = true;
//    try {
//        done = QApplication::notify(receiver, event);
//    }
//    catch (LaserDeviceSecurityException* e)
//    {
//        qLogW << "laser security exception: " << e->toString();
//        //mainWindow->handleSecurityException(e->errorCode(), e->errorMessage());
//    }
//    catch (LaserException* e)
//    {
//        qLogD << "laser exception: " << e->toString();
//    }
//    catch (QException* e)
//    {
//        qLogD << "QException: " << e;
//    }
//    catch (std::exception* e)
//    {
//        qLogD << "std::exception";
//    }
//    catch (...)
//    {
//        qLogD << "some exception else";
//
//    }
//    return done;
//}

void LaserApplication::retranslate()
{
    stringMap["Absolute Coords"] = translate("LaserApplication", "Absolute Coords", nullptr);
    stringMap["Config Dialog"] = translate("LaserApplication", "Config Dialog", nullptr);
    stringMap["Current Position"] = translate("LaserApplication", "Current Position", nullptr);
    stringMap["Cutting"] = translate("LaserApplication", "Cutting", nullptr);
    stringMap["Chinese"] = translate("LaserApplication", "Chinese", nullptr);
    stringMap["English"] = translate("LaserApplication", "English", nullptr);
    stringMap["Engraving"] = translate("LaserApplication", "Engraving", nullptr);
    stringMap["Filling"] = translate("LaserApplication", "Filling", nullptr);
    stringMap["High Contrast"] = translate("LaserApplication", ("High Contrast"), nullptr);
    stringMap["Horizontal"] = translate("LaserApplication", "Horizontal", nullptr);
    stringMap["Low Contrast"] = translate("LaserApplication", ("Low Contrast"), nullptr);
    stringMap["Medium Contrast"] = translate("LaserApplication", ("Medium Contrast"), nullptr);
    stringMap["Off"] = translate("LaserApplication", ("Off"), nullptr);
    stringMap["Power"] = translate("LaserApplication", ("Power"), nullptr);
    stringMap["Size"] = translate("LaserApplication", ("Size"), nullptr);
    stringMap["Speed"] = translate("LaserApplication", ("Speed"), nullptr);
    stringMap["User Origin"] = translate("LaserApplication", "User Origin", nullptr);
    stringMap["Vertical"] = translate("LaserApplication", "Vertical", nullptr);

    Config::updateTitlesAndDescriptions();
}

QString LaserApplication::str(const QString& key)
{
    if (stringMap.contains(key))
        return stringMap[key];
    else
        return key;
}

void LaserApplication::initLog()
{
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

void LaserApplication::onLanguageChanged(const QVariant& value, ModifiedBy modifiedBy)
{
    QLocale locale(static_cast<QLocale::Language>(Config::General::language()));
    QLocale::setDefault(locale);
    //Config::load();
    retranslate();
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

void LaserApplication::closeProgressWindow()
{
}

void LaserApplication::showProgressWindow()
{
    previewWindow->setWindowModality(Qt::ApplicationModal);
    previewWindow->show();
}
