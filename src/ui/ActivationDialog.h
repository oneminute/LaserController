#ifndef ACTIVATIONDIALOG_H
#define ACTIVATIONDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QRegularExpressionValidator>
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
    void onLineEditEmailEditingFinished();
    void onLineEditCodeEditingFinished();
    void onLineEditUserNameEditingFinished();

    void onSendTimerTimeout();
    void onUpdateTimerTimeout();

private:
    QScopedPointer<Ui::ActivationDialog> m_ui;

    QTimer m_sendTimer;
    QTimer m_updateTimer;
    QRegularExpressionValidator m_validatorEmail;
    QRegularExpressionValidator m_validatorCode;
    QRegularExpressionValidator m_validatorUserName;
};

#endif // ACTIVATIONDIALOG_H