#ifndef ACTIVATIONDIALOG_H
#define ACTIVATIONDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QRegularExpression>
#include <QTimer>

namespace Ui
{
    class ActivationDialog;
}

class ActivationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ActivationDialog(QWidget* parent = nullptr);
    ~ActivationDialog();

protected:
    void onPushButtonSendClicked(bool checked);
    void onPushButtonActivateClicked(bool checked);
    void onPushButtonCloseClicked(bool checked);

    void onActivationResponse(bool activated);

    bool validateMail();
    bool validateCode();
    bool validateUserName();

protected slots:
    void onFieldTextEdited(const QString& text);

    void onSendTimerTimeout();
    void onUpdateTimerTimeout();

    void onActiveFailed(int reason);

private:
    QScopedPointer<Ui::ActivationDialog> m_ui;

    QTimer m_sendTimer;
    QTimer m_updateTimer;
    QRegularExpression m_reEmail;
    QRegularExpression m_reCode;
    QRegularExpression m_reUserName;

    QStringList m_errorMsgs;
};

#endif // ACTIVATIONDIALOG_H