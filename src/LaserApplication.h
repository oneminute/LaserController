#ifndef LASERAPPLICATION_H
#define LASERAPPLICATION_H

#include <QApplication>
#include <QThread>

class LaserControllerWindow;
class PreviewWindow;
class LaserDevice;
class LaserDriver;

class LaserApplication : public QApplication
{
public:
    LaserApplication(int argc, char** argv);
    ~LaserApplication();

    bool initialize();
    void destroy();

    bool checkEnvironment();

    bool notify(QObject* receiver, QEvent* event) override;

protected:
    void initLog();
    void clearCrash();
    void checkCrash();
    static void handleLogOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

protected slots:
    void onEnterDeviceUnconnectedState();

public:
    static LaserApplication* app;
    static LaserControllerWindow* mainWindow;
    static PreviewWindow* previewWindow;
    static LaserDevice* device;
    static LaserDriver* driver;

    static void closeProgressWindow();
    static void showProgressWindow();

private:
    QThread g_deviceThread;
};

#endif // LASERAPPLICATION_H