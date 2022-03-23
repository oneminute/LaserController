#include "LaserApplication.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTranslator>

#include "version.h"
#include "common/common.h"
#include "common/Config.h"
#include "exception/LaserException.h"
#include "laser/LaserDevice.h"
#include "laser/LaserDriver.h"
#include "scene/LaserDocument.h"
#include "state/StateController.h"
#include "ui/LaserControllerWindow.h"
#include "ui/SplashScreen.h"
#include "task/ProgressModel.h"
#include "task/ProgressItem.h"

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#endif // _WINDOWS
#include <glog/logging.h>
#include <glog/log_severity.h>
#include <iostream>

LaserApplication* LaserApplication::app(nullptr);
LaserControllerWindow* LaserApplication::mainWindow(nullptr);
SplashScreen* LaserApplication::splashScreen(nullptr);
ProgressItem* LaserApplication::globalProgress(nullptr);
LaserDevice* LaserApplication::device(nullptr);
LaserDriver* LaserApplication::driver(nullptr);
QString LaserApplication::appShortName("CNELaser");
QMap<QString, QString> LaserApplication::stringMap;
QMap<QString, QTranslator*> LaserApplication::translators;
QTranslator* LaserApplication::currentTranslator(nullptr);
QThread* LaserApplication::mainThread(nullptr);

LaserApplication::LaserApplication(int argc, char** argv)
    : QApplication(argc, argv)
{
    if (app)
    {
        throw LaserFatalException(tr("Application startup failure."));
    }
    app = this;

    if (!antiDebugger())
    {
        //QMessageBox::warning(nullptr, tr("Debugging"), tr("Debugging"));
        //closeParent();
    }
}

LaserApplication::~LaserApplication()
{
    qDebug() << "application destroyed";
}

bool LaserApplication::initialize()
{
    mainThread = QThread::currentThread();
    QDir dir(LaserApplication::applicationDirPath());
    LaserApplication::addLibraryPath(dir.absoluteFilePath("bin"));
    LaserApplication::setApplicationName(QObject::tr("CNE Laser"));
    LaserApplication::setApplicationDisplayName(QObject::tr("CNE Laser %1").arg(LC_VERSION_STR_WITH_TAG));
    LaserApplication::setOrganizationName(tr(""));
    LaserApplication::setApplicationVersion(QString("%1").arg(LC_VERSION_STR_WITH_TAG));
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
    splashScreen = new SplashScreen();
    splashScreen->setMessage(tr("Loading settings..."));
    splashScreen->show();

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
    splashScreen->setProgress(10);

    splashScreen->setMessage(tr("Loading laser library..."));
    driver = new LaserDriver;
    device = new LaserDevice(driver);

    connect(StateController::instance().deviceUnconnectedState(), &QState::entered, this, &LaserApplication::onEnterDeviceUnconnectedState);
    connect(Config::General::languageItem(), &ConfigItem::valueChanged, this, &LaserApplication::onLanguageChanged);
    splashScreen->setProgress(40);

    splashScreen->setMessage(tr("Loading state machine..."));
    StateController::start();
    splashScreen->setProgress(50);

    splashScreen->setMessage(tr("Loading main window..."));
    //progressModel = new ProgressModel;
    globalProgress = new ProgressItem(tr("Total Progress"), ProgressItem::PT_Complex);
    mainWindow = new LaserControllerWindow;
    //previewWindow = new PreviewWindow(mainWindow);
    mainWindow->showMaximized();
    splashScreen->setProgress(85);

    g_deviceThread.start();

    return true;
}

void LaserApplication::destroy()
{
    SAFE_DELETE(mainWindow);
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
    
    //SAFE_DELETE(progressModel)
    SAFE_DELETE(globalProgress)

    g_deviceThread.exit();
    g_deviceThread.wait();

    clearCrash();
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
    catch (std::exception* /*e*/)
    {
        qLogD << "std::exception";
    }
    catch (...)
    {
        qLogD << "some exception else";

    }
    return done;
}

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

    stringMap["Absolute Coords"] = tr("Absolute Coords");
    stringMap["Align X"] = tr("Align X");
    stringMap["Align Y"] = tr("Align Y");
    stringMap["Asymmetric Circles Grid"] = tr("Asymmetric Circles Grid");
    stringMap["Back to origin"] = tr("Back to origin");
    stringMap["Back to user origin"] = tr("Back to user origin");
    stringMap["Back to user origin 1"] = tr("Back to user origin 1");
    stringMap["Back to user origin 2"] = tr("Back to user origin 2");
    stringMap["Back to user origin 3"] = tr("Back to user origin 3");
    stringMap["Bold"] = tr("Bold");
    stringMap["Bottom"] = tr("Bottom");
    stringMap["Bottom Left"] = tr("Bottom Left");
    stringMap["Bottom Right"] = tr("Bottom Right");
    stringMap["Chessboard"] = tr("Chessboard");
    stringMap["Charuco Board"] = tr("Charuco Board");
    stringMap["Chuck Rotary"] = tr("Chuck Rotary");
    stringMap["Circles Grid"] = tr("Circles Grid");
    stringMap["Config Dialog"] = tr("Config Dialog");
    stringMap["Current Position"] = tr("Current Position");
    stringMap["Current location"] = tr("Current location");
    stringMap["Cutting"] = tr("Cutting");
    stringMap["Chinese"] = tr("Chinese");
    stringMap["English"] = tr("English");
    stringMap["Engraving"] = tr("Engraving");
    stringMap["Filling"] = tr("Filling");
    stringMap["Font"] = tr("Font");
    stringMap["Height"] = tr("Height");
    stringMap["High Quality"] = tr("High Quality");
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
    stringMap["Normal Quality"] = tr("Normal Quality");
    stringMap["Off"] = tr("Off");
    stringMap["PageSize"] = tr("Page Size(mm): %1x%2");
    stringMap["Perfect Quality"] = tr("Perfect Quality");
    stringMap["Power"] = tr("Power");
    stringMap["Release motor"] = tr("Release motor");
    stringMap["Right"] = tr("Right");
    stringMap["Roller Rotary"] = tr("Roller Rotary");
    stringMap["Rotate"] = tr("Rotate");
    stringMap["Size"] = tr("Size");
    stringMap["Spacing X"] = tr("Spacing X");
    stringMap["Spacing Y"] = tr("Spacing Y");
    stringMap["Speed"] = tr("Speed");
    stringMap["Tips"] = tr("Tips");
    stringMap["Top"] = tr("Top");
    stringMap["Top Left"] = tr("Top Left");
    stringMap["Top Right"] = tr("Top Right");
    stringMap["User Origin"] = tr("User Origin");
    stringMap["Unload motor"] = tr("Unload motor");
    stringMap["Unregistered"] = tr("Unregistered");
    stringMap["Upper Case"] = tr("Upper Case");
    stringMap["Vertical"] = tr("Vertical");
    stringMap["WargingOverstepTitle"] = tr("Out max valid region.");
    stringMap["WargingOverstepText"] = tr("You had oversteped the valid region, Please don't operate out the red line.");
    stringMap["Welcome!"] = tr("Welcome!");
    stringMap["Width"] = tr("Width");
    stringMap["X Pos"] = tr("X Pos");
    stringMap["Y Pos"] = tr("Y Pos");

    stringMap["deviceState"] = tr("Device State");
    stringMap["deviceConnectedState"] = tr("Connected");
    stringMap["deviceErrorState"] = tr("Device Error");
    stringMap["deviceIdleState"] = tr("Device Idle");
    stringMap["deviceMachiningState"] = tr("Machining");
    stringMap["devicePausedState"] = tr("Paused");
    stringMap["deviceUnconnectedState"] = tr("Unconnected");
    stringMap["documentState"] = tr("Document State");
    stringMap["documentEmptyState"] = tr("Empty Document");
    stringMap["documentIdleState"] = tr("Document Idle");
    stringMap["documentPrimitiveState"] = tr("Primitive State");
    stringMap["documentPrimitiveArcTextState"] = tr("Arc Text State");
    stringMap["documentPrimitiveEllipseState"] = tr("Ellipse State");
    stringMap["documentPrimitiveEllipseCreatingState"] = tr("Creating Ellipse");
    stringMap["documentPrimitiveEllipseReadyState"] = tr("Ellipse Ready");
    stringMap["documentPrimitiveFrameState"] = tr("Frame State");
    stringMap["documentPrimitiveHorizontalTextState"] = tr("Horizontal Text");
    stringMap["documentPrimitiveLineState"] = tr("Line State");
    stringMap["documentPrimitiveLineCreatingState"] = tr("Creating Line");
    stringMap["documentPrimitiveLineReadyState"] = tr("Line Ready");
    stringMap["documentPrimitivePolygonState"] = tr("Polygon State");
    stringMap["documentPrimitivePolygonCreatingState"] = tr("Creating Polygon");
    stringMap["documentPrimitivePolygonReadyState"] = tr("Polygon Ready");
    stringMap["documentPrimitivePolygonStartRectState"] = tr("Polygon Start Rect");
    stringMap["documentPrimitiveRectState"] = tr("Rect State");
    stringMap["documentPrimitiveRectCreatingState"] = tr("Creaing Rect");
    stringMap["documentPrimitiveRectReadyState"] = tr("Rect Ready");
    stringMap["documentPrimitiveRingState"] = tr("Ring State");
    stringMap["documentPrimitiveRingEllipseState"] = tr("Ring Ellipse State");
    stringMap["documentPrimitiveSplineState"] = tr("Spline State");
    stringMap["documentPrimitiveSplineCreatingState"] = tr("Creating Spline");
    stringMap["documentPrimitiveSplineEditState"] = tr("Editing Spline");
    stringMap["documentPrimitiveSplineReadyState"] = tr("Spline Ready");
    stringMap["documentPrimitiveStarState"] = tr("Star State");
    stringMap["documentPrimitiveTextState"] = tr("Text State");
    stringMap["documentPrimitiveTextCreatingState"] = tr("Creating Text");
    stringMap["documentPrimitiveTextReadyState"] = tr("Text Ready");
    stringMap["documentPrimitiveVerticalTextState"] = tr("Vertical Text State");
    stringMap["documentPrintAndCutSelectingState"] = tr("Selecting In Canvas");
    stringMap["documentPrintAndCutAligningState"] = tr("Aligning");
    stringMap["documentSelectedState"] = tr("Selected");
    stringMap["documentSelectedEditingState"] = tr("Selected Editing");
    stringMap["documentSelectingState"] = tr("Selecting");
    stringMap["documentSelectionState"] = tr("Selection");
    stringMap["documentViewDragState"] = tr("Drag State");
    stringMap["documentViewDragReadyState"] = tr("Ready Draging");
    stringMap["documentViewDragingState"] = tr("Draging");
    stringMap["documentWorkingState"] = tr("Document Working");
    stringMap["finishState"] = tr("Finished");
    stringMap["initState"] = tr("Initialized");
    stringMap["workingState"] = tr("Working");

    Config::updateTitlesAndDescriptions();

    emit app->languageChanged();
}

QString LaserApplication::str(const QString& key)
{
    if (stringMap.contains(key))
    {
        if (stringMap[key].isEmpty())
            return key;
        return stringMap[key];
    }
    else
    {
        return key;
    }
}

int LaserApplication::exec()
{
    try {
        int result = QApplication::exec();
        return result;
    }
    catch (std::exception e)
    {
        std::cout << "error: " << e.what() << std::endl;
        return -1;
    }
}

void LaserApplication::restart()
{
    mainWindow->close();
    LaserApplication::quit();
    QProcess::startDetached(LaserApplication::instance()->arguments()[0], LaserApplication::instance()->arguments());
}

bool LaserApplication::antiDebugger()
{
#if _WINDOWS
    STARTUPINFO info;
    GetStartupInfo(&info);
    if (info.dwX || info.dwY || info.dwXCountChars || info.dwYCountChars ||
        info.dwFillAttribute || info.dwXSize || info.dwYSize ||
        info.dwFlags & STARTF_FORCEOFFFEEDBACK)
        return false;
    else
        return true;
#endif
}

void LaserApplication::closeParent()
{
#if _WINDOWS
    int pid = -1;
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32);

    //assume first arg is the PID to get the PPID for, or use own PID
    pid = GetCurrentProcessId();

    if (Process32First(h, &pe)) {
        do {
            if (pe.th32ProcessID == pid) {
                printf("PID: %i; PPID: %i\n", pid, pe.th32ParentProcessID);
            }
        } while (Process32Next(h, &pe));
    }

    CloseHandle(h);
#endif
}

LaserDocument* LaserApplication::createDocument()
{
    LaserScene* scene = mainWindow->scene();
	LaserDocument* doc = new LaserDocument(scene);
	mainWindow->initDocument(doc);
    doc->open();
    return doc;
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

void LaserApplication::onLanguageChanged(const QVariant& value, void* senderPtr)
{
    retranslate();
    device->updateDriverLanguage();
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

//void LaserApplication::closeProgressWindow()
//{
//}
//
//void LaserApplication::showProgressWindow()
//{
//    if (previewWindow->isVisible())
//    {
//        previewWindow->hide();
//    }
//    else
//    {
//        previewWindow->show();
//        previewWindow->raise();
//        previewWindow->activateWindow();
//    }
//}
//
//void LaserApplication::resetProgressWindow()
//{
//    previewWindow->reset();
//    //progressModel->clear();
//}

ProgressItem* LaserApplication::resetProcess()
{
    globalProgress->reset();
    device->clearProgress();
    return globalProgress;
}
