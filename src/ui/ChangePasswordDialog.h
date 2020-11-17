#ifndef CHANGE_PASSWORD_DIALOG_H
#define CHANGE_PASSWORD_DIALOG_H

#include <QDialog>
#include <QScopedPointer>
#include "laser/LaserDriver.h"

namespace Ui
{
    class ChangePasswordDialog;
}

class ChangePasswordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChangePasswordDialog(QWidget* parent = nullptr);
    virtual ~ChangePasswordDialog();

private:
    void onPushButtonChangeClicked(bool checked = false);
    void onPushButtonCancelClicked(bool checked = false);

    void rightManufacturerPassword();
    void wrongManufacturerPassword();
    void changeManufacturerPasswordOk();
    void changeManufacturerPasswordFailure();

private:
    QScopedPointer<Ui::ChangePasswordDialog> m_ui;
};

#endif // CHANGE_PASSWORD_DIALOG_H