#ifndef LASERAPPLICATION_H
#define LASERAPPLICATION_H

#include <QApplication>
#include <QThread>
#include "common/common.h"

class LaserControllerWindow;
class SplashScreen;
class ProgressItem;
class LaserDevice;
class LaserDriver;
class LaserDocument;

class LaserApplication : public QApplication
{
    Q_OBJECT
public:
    LaserApplication(int argc, char** argv);
    ~LaserApplication();

    bool initialize();
    void destroy();

    bool checkEnvironment();

    bool notify(QObject* receiver, QEvent* event) override;

    static QString softwareVersion();
    static void loadLanguages();
    static void changeLanguage();
    static void retranslate();
    static QString str(const QString& key);
    static int exec();
    static void restart();
    static bool antiDebugger();
    static void closeParent();

    static LaserDocument* createDocument();

    static void cleanLogFiles();
    static void cleanCachedFiles();

    static void showSplashScreen(const QString& msg, int progress, QWidget* parentWnd = nullptr);
    static void hideSplashScreen(int ms);

protected:
    void initLog();
    void cleanCrash();
    void checkCrash();
    static void handleLogOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

protected slots:
    void onEnterDeviceUnconnectedState();
    void onLanguageChanged(const QVariant& value, void* senderPtr);

signals:
    void languageChanged();

public:
    static LaserApplication* app;
    static LaserControllerWindow* mainWindow;
    static SplashScreen* splashScreen;
    static ProgressItem* globalProgress;
    static LaserDevice* device;
    static LaserDriver* driver;
    static QString appShortName;
    static QTranslator* currentTranslator;

    static ProgressItem* resetProcess();

    static QMap<QString, QString> stringMap;
    static QMap<QString, QTranslator*> translators;

    static QThread* mainThread;

private:
    QThread g_deviceThread;
};

#define ltr(key) LaserApplication::str(key)

#endif // LASERAPPLICATION_H