#include "SplashScreen.h"
#include "ui_SplashScreen.h"
#include"LaserControllerWindow.h"
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QProgressBar>
#include <QLabel>
#include <QtMath>
#include <QPalette>

#include "LaserApplication.h"


SplashScreen::SplashScreen(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::SplashScreen)
    , m_showProgress(true)
    , m_progress(0)
    , m_targetProgress(100)
    , m_delay(10000)
    , m_progressTick(0.5)
    , m_close(false)
{
//#ifdef _DEBUG
//    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
//#else
//    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
//#endif
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    m_ui->setupUi(this);   
    QRect screenGeometry = LaserApplication::desktop()->screenGeometry();
    int w = screenGeometry.width()*0.5;
    int h = screenGeometry.height() * 0.5;
    resize(w, h);
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    /*QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setMargin(1);

    m_labelBanner = new QLabel;
    m_labelBanner->setPixmap(QPixmap(":/ui/icons/images/CNE_banner.png"));

    m_labelMessage = new QLabel;
    //m_labelMessage->setAlignment(Qt::AlignRight);

    m_progressBar = new QProgressBar;

    mainLayout->addWidget(m_labelBanner, Qt::AlignHCenter);
    //mainLayout->addWidget(m_labelMessage);
    QHBoxLayout* processLayout = new QHBoxLayout;
    //mainLayout->addLayout(processLayout);
    mainLayout->addWidget(m_progressBar, Qt::AlignLeft);

    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 0);
    mainLayout->setStretch(2, 0);

    setLayout(mainLayout);
    

    
    */
    //m_progressBar = m_ui->progressBar;
    m_progressTimer.setInterval(100);
    connect(&m_progressTimer, &QTimer::timeout, this, &SplashScreen::progressTimerTimeout, Qt::ConnectionType::DirectConnection);
    connect(&m_visualTimer, &QTimer::timeout, this, &SplashScreen::visualTimerTimeout);
}

SplashScreen::~SplashScreen()
{
    m_progressTimer.stop();
    m_visualTimer.stop();
}

void SplashScreen::setProgress(qreal progress, bool immediate) 
{
    if (immediate)
    {
        m_progress = qBound(0.0, progress, 100.0); 
        m_ui->progressBar->setValue(qRound(m_progress));
    }
    else
    {
        m_targetProgress = progress;
        if (!m_progressTimer.isActive())
        m_progressTimer.start();
    }
}

void SplashScreen::setMessage(const QString& message) 
{
    m_message = message; 
    //m_labelMessage->setText(" " + message);
}

void SplashScreen::show(int milliseconds)
{
    if (milliseconds > 0)
        m_progressTimer.start();
    
    QDialog::show();
    
}

void SplashScreen::hide(bool immediate)
{
    QDialog::hide();
    //LaserApplication::mainWindow->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    //LaserApplication::mainWindow->show();
}

void SplashScreen::delayedHide(int milliseconds)
{
    QTimer::singleShot(1000, [=] () 
        {
            hide();
        }
    );
}

void SplashScreen::progressTimerTimeout()
{
    m_progress += m_progressTick;
    m_ui->progressBar->setValue(qRound(m_progress));
}

void SplashScreen::visualTimerTimeout()
{
    
    m_visualTimer.stop();
    delayedHide();
}

