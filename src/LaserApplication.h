#ifndef LASERAPPLICATION_H
#define LASERAPPLICATION_H

#include <QApplication>
#include <QThread>
#include "common/common.h"

class LaserControllerWindow;
class PreviewWindow;
class LaserDevice;
class LaserDriver;

class LaserApplication : public QApplication
{
    Q_OBJECT
public:
    LaserApplication(int argc, char** argv);
    ~LaserApplication();

    bool initialize();
    void destroy();

    bool checkEnvironment();

    //bool notify(QObject* receiver, QEvent* event) override;

    static void retranslate();
    static QString str(const QString& key);

protected:
    void initLog();
    void clearCrash();
    void checkCrash();
    static void handleLogOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

protected slots:
    void onEnterDeviceUnconnectedState();
    void onLanguageChanged(const QVariant& value, ModifiedBy modifiedBy);

public:
    static LaserApplication* app;
    static LaserControllerWindow* mainWindow;
    static PreviewWindow* previewWindow;
    static LaserDevice* device;
    static LaserDriver* driver;

    static void closeProgressWindow();
    static void showProgressWindow();

    static QMap<QString, QString> stringMap;

private:
    QThread g_deviceThread;
};

#define ltr(key) LaserApplication::str(key)

#endif // LASERAPPLICATION_H