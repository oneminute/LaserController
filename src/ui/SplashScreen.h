#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QDialog>
#include <QTimer>
#include <QScopedPointer>

class QProgressBar;
class QLabel;
namespace Ui
{
    class SplashScreen;
}
class SplashScreen : public QDialog
{
    Q_OBJECT
public:
    explicit SplashScreen(QWidget* parent = nullptr);
    ~SplashScreen();

    void setProgress(qreal progress, bool immediate = true);
    qreal progress() { return m_progress; }

    void setShowProgress(bool showProgress) { m_showProgress = showProgress; }
    bool showProgress() { return m_showProgress; }

    void setMessage(const QString& message);
    QString message() { return m_message; }

    void setProgressTick(qreal tick) { m_progressTick = tick; }
    qreal progressTick() { return m_progressTick; }

    void show(int milliseconds = 10000);
    void hide(bool immediate = false);

    void delayedHide(int milliseconds = 1000);

protected slots:
    void progressTimerTimeout();
    void visualTimerTimeout();

private:
    QScopedPointer<Ui::SplashScreen> m_ui;
    bool m_showProgress;
    qreal m_progress;
    qreal m_targetProgress;
    qreal m_delay;
    qreal m_progressTick;
    bool m_close;
    QString m_message;
    QTimer m_visualTimer;
    QTimer m_progressTimer;
    QLabel* m_labelBanner;
    QLabel* m_labelMessage;
    //QProgressBar* m_progressBar;

    Q_DISABLE_COPY(SplashScreen)
};

#endif // SPLASHSCREEN_H
