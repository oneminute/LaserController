#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QDialog>

class SplashScreen : public QDialog
{
    Q_OBJECT
public:
    explicit SplashScreen(QWidget* parent = nullptr);
    ~SplashScreen();

    void setProgress(qreal progress) { m_progress = progress; }
    qreal progress() { return m_progress; }

    void setShowProgress(bool showProgress) { m_showProgress = showProgress; }
    bool showProgress() { return m_showProgress; }

    void setMessage(const QString& message) { m_message = message; }
    QString message() { return m_message; }

private:
    bool m_showProgress;
    qreal m_progress;
    QString m_message;

    Q_DISABLE_COPY(SplashScreen)
};

#endif // SPLASHSCREEN_H
