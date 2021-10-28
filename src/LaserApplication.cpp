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
#include "task/ProgressModel.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glog/logging.h>
#include <glog/log_severity.h>
#include <iostream>

LaserApplication* LaserApplication::app(nullptr);
LaserControllerWindow* LaserApplication::mainWindow(nullptr);
PreviewWindow* LaserApplication::previewWindow(nullptr);
ProgressModel* LaserApplication::progressModel(nullptr);
LaserDevice* LaserApplication::device(nullptr);
LaserDriver* LaserApplication::driver(nullptr);
QString LaserApplication::appShortName("CNELaser");
QMap<QString, QString> LaserApplication::stringMap;
QMap<QString, QTranslator*> LaserApplication::translators;
QTranslator* LaserApplication::currentTranslator(nullptr);

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
    LaserApplication::setApplicationName(QObject::tr("CNE Laser"));
    LaserApplication::setApplicationDisplayName(QObject::tr("CNE Laser %1").arg(LC_VERSION_STR));
    LaserApplication::setOrganizationName(tr(""));
    LaserApplication::setApplicationVersion(QString("%1").arg(LC_VERSION_STR));
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

    loadLanguages();
    checkEnvironment();
    checkCrash();
    initLog();
    qInstallMessageHandler(LaserApplication::handleLogOutput);

    Config::init();
    Config::load();
	Global::unit = static_cast<SizeUnit>(Config::General::unit());

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
    progressModel = new ProgressModel;
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
    
    SAFE_DELETE(previewWindow)
    SAFE_DELETE(progressModel)
    g_deviceThread.exit();
    g_deviceThread.wait();

    Config::destroy();

    qDeleteAll(translators);
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

QString LaserApplication::softwareVersion()
{
    return LC_VERSION_STR;
}

void LaserApplication::loadLanguages()
{
    qDeleteAll(translators);
    translators.clear();
    QDir translationDir("translations");
    QStringList nameFilters;
    nameFilters << "*.qm";
    QFileInfoList entries = translationDir.entryInfoList(nameFilters);
    for (QFileInfo& fileInfo : entries)
    {
        QTranslator* translator = new QTranslator;
        translator->load(fileInfo.absoluteFilePath());
        translators.insert(fileInfo.baseName(), translator);
    }
}

void LaserApplication::changeLanguage()
{
    QLocale locale(static_cast<QLocale::Language>(Config::General::language()));
    qLogD << "language code: " << Config::General::language() << ", " << locale;
    qLogD << "language name: " << QLocale::languageToString(locale.language());
    qLogD << "language bcp47Name: " << locale.bcp47Name();
    QStringList languages = locale.uiLanguages();
    for (QString localeName : qAsConst(languages)) {
        localeName.replace(QLatin1Char('-'), QLatin1Char('_'));
        QString realName = appShortName + "_" + localeName;

        if (translators.contains(realName))
        {
            qDebug() << "load translation file." << applicationName() << locale.name();
            if (currentTranslator)
                removeTranslator(currentTranslator);
            currentTranslator = translators[realName];
            installTranslator(currentTranslator);
        }
    }
    

    /*translator = new QTranslator;
    if (translator->load(locale, QLatin1String("qt"), QLatin1String("_"), QLatin1String("translations")))
    {
        qDebug() << "load translation file." << "qt" << locale.name();
        installTranslator(translator);
        translators.append(translator);
    }
    else
    {
        delete translator;
    }*/

    QLocale::setDefault(locale);
}

void LaserApplication::retranslate()
{
    changeLanguage();
    QLocale locale(static_cast<QLocale::Language>(Config::General::language()));
    QLocale::setDefault(locale);

    /*stringMap["Absolute Coords"] = translate("LaserApplication", "Absolute Coords", nullptr);
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
    stringMap["Vertical"] = translate("LaserApplication", "Vertical", nullptr);*/

    stringMap["Absolute Coords"] = tr("Absolute Coords");
    stringMap["Align X"] = tr("Align X");
    stringMap["Align Y"] = tr("Align Y");
    stringMap["Bold"] = tr("Bold");
    stringMap["Bottom"] = tr("Bottom");
    stringMap["Config Dialog"] = tr("Config Dialog");
    stringMap["Current Position"] = tr("Current Position");
    stringMap["Cutting"] = tr("Cutting");
    stringMap["Chinese"] = tr("Chinese");
    stringMap["English"] = tr("English");
    stringMap["Engraving"] = tr("Engraving");
    stringMap["Filling"] = tr("Filling");
    stringMap["Font"] = tr("Font");
    stringMap["Height"] = tr("Height");
    stringMap["High Contrast"] = tr("High Contrast");
    stringMap["Horizontal"] = tr("Horizontal");
    stringMap["Inactivated"] = tr("Inactivated");
    stringMap["Italic"] = tr("Italic");
    stringMap["Laser Power"] = tr("Laser Power");
    stringMap["Left"] = tr("Left");
    stringMap["Low Contrast"] = tr("Low Contrast");
    stringMap["Medium Contrast"] = tr("Medium Contrast");
    stringMap["Middle"] = tr("Middle");
    stringMap["Movement"] = tr("Movement");
    stringMap["Off"] = tr("Off");
    stringMap["PageSize"] = tr("Page Size(mm): %1x%2");
    stringMap["Power"] = tr("Power");
    stringMap["Right"] = tr("Right");
    stringMap["Rotate"] = tr("Rotate");
    stringMap["Size"] = tr("Size");
    stringMap["Spacing X"] = tr("Spacing X");
    stringMap["Spacing Y"] = tr("Spacing Y");
    stringMap["Speed"] = tr("Speed");
    stringMap["Tips"] = tr("Tips");
    stringMap["Top"] = tr("Top");
    stringMap["Top Left"] = tr("Top Left");
    stringMap["User Origin"] = tr("User Origin");
    stringMap["Unregistered"] = tr("Unregistered");
    stringMap["Upper Case"] = tr("Upper Case");
    stringMap["Vertical"] = tr("Vertical");
    stringMap["WargingOverstepTitle"] = tr("Out max valid region.");
    stringMap["WargingOverstepText"] = tr("You had oversteped the valid region, Please don't operate out the red line.");
    stringMap["Welcome!"] = tr("Welcome!");
    stringMap["Width"] = tr("Width");
    stringMap["X Pos"] = tr("X Pos");
    stringMap["Y Pos"] = tr("Y Pos");

    Config::updateTitlesAndDescriptions();

    emit app->languageChanged();
}

QString LaserApplication::str(const QString& key)
{
    if (stringMap.contains(key))
        return stringMap[key];
    else
        return key;
}

int LaserApplication::exec()
{
    try {
        int result = QApplication::exec();
        return result;
    }
    catch (...)
    {
        qLogW << "error: ";
        return -1;
    }
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
