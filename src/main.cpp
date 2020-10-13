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
    std::cout << argv << std::endl;
    google::InitGoogleLogging(argv);
    google::InstallFailureSignalHandler();

    FLAGS_stderrthreshold = google::GLOG_ERROR; //INFO WARNING ERROR FATAL, 默认输出到stderr(app Output/cli)的阀值是ERROR
    FLAGS_alsologtostderr = true; //当这个全局变量为真时，忽略FLAGS_stderrthreshold的限制，所有信息打印到终端
    FLAGS_colorlogtostderr = true; //设置输出到屏幕的日志显示相应颜色,错误等级有颜色区分
    FLAGS_max_log_size = 100; //Maximum log size: 100MB
    FLAGS_logbufsecs = 0;        //缓冲日志输出，默认为30秒，此处改为立即输出
    FLAGS_stop_logging_if_full_disk = true;     //当磁盘被写满时，停止日志输出

    //set log path;第一个参数为日志级别设置,级别高于 google::INFO 的日志同时输出到屏幕，第二个参数表示输出目录及日志文件名前缀,log目录我是事先在build-prj目录下创建好.
    google::SetLogDestination(google::GLOG_INFO,   "log/INFO_");
    google::SetLogDestination(google::GLOG_WARNING,"log/WARNING_");   //设置 google::WARNING 级别的日志存储路径和文件名前缀
    google::SetLogDestination(google::GLOG_ERROR,  "log/ERROR_");    //设置 google::ERROR 级别的日志存储路径和文件名前缀
    google::SetLogDestination(google::GLOG_FATAL,  "log/FATAL_");    //设置 google::FATAL 级别的日志存储路径和文件名前缀
    google::SetLogFilenameExtension("lc_");     //设置文件名扩展，如平台？或其它需要区分的信息
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

    QTranslator translator;
    QLocale locale(QLocale::Chinese);
    //qDebug() << "load translation file." << a.applicationName() << locale.name();
    if (translator.load(locale, app.applicationName(), QLatin1String("_"), QLatin1String("translations")))
    {
        qDebug() << "load translation file." << app.applicationName() << locale.name();
        app.installTranslator(&translator);
    }

    QTranslator qtTranslator;
    if (qtTranslator.load(locale, QLatin1String("qt"), QLatin1String("_"), QLatin1String("translations")))
    {
        qDebug() << "load translation file." << "qt" << locale.name();
        app.installTranslator(&qtTranslator);
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
