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

    FLAGS_stderrthreshold = google::GLOG_ERROR; //INFO WARNING ERROR FATAL, Ĭ�������stderr(app Output/cli)�ķ�ֵ��ERROR
    FLAGS_alsologtostderr = true; //�����ȫ�ֱ���Ϊ��ʱ������FLAGS_stderrthreshold�����ƣ�������Ϣ��ӡ���ն�
    FLAGS_colorlogtostderr = true; //�����������Ļ����־��ʾ��Ӧ��ɫ,����ȼ�����ɫ����
    FLAGS_max_log_size = 100; //Maximum log size: 100MB
    FLAGS_logbufsecs = 0;        //������־�����Ĭ��Ϊ30�룬�˴���Ϊ�������
    FLAGS_stop_logging_if_full_disk = true;     //�����̱�д��ʱ��ֹͣ��־���

    //set log path;��һ������Ϊ��־��������,������� google::INFO ����־ͬʱ�������Ļ���ڶ���������ʾ���Ŀ¼����־�ļ���ǰ׺,logĿ¼����������build-prjĿ¼�´�����.
    google::SetLogDestination(google::GLOG_INFO,   "log/INFO_");
    google::SetLogDestination(google::GLOG_WARNING,"log/WARNING_");   //���� google::WARNING �������־�洢·�����ļ���ǰ׺
    google::SetLogDestination(google::GLOG_ERROR,  "log/ERROR_");    //���� google::ERROR �������־�洢·�����ļ���ǰ׺
    google::SetLogDestination(google::GLOG_FATAL,  "log/FATAL_");    //���� google::FATAL �������־�洢·�����ļ���ǰ׺
    google::SetLogFilenameExtension("lc_");     //�����ļ�����չ����ƽ̨����������Ҫ���ֵ���Ϣ
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
