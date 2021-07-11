#ifndef DEVICE_SETTINGS_DIALOG_H
#define DEVICE_SETTINGS_DIALOG_H

#include <qobject.h>
#include <QDialog>
#include <QScopedPointer>

#include "laser/LaserDriver.h"

namespace Ui
{
    class DeviceSettingsDialog;
}

class DeviceSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DeviceSettingsDialog(QWidget* parent = nullptr);
    virtual ~DeviceSettingsDialog();

    void makeDefault();

protected slots:
    void registersFetched(const LaserRegister::RegistersMap& datas);
    void rightManufactorPassword();
    void wrongManufactorPassword();

    void onPushButtonReadClicked(bool checked = false);

    void onPushButtonWriteClicked(bool checked = false);

    void onPushButtonDefaultClicked(bool checked = false);

    void onPushButtonVerifyClicked(bool checked = false);

    void onPushButtonClearClicked(bool checked = false);

    void onPushButtonResetClicked(bool checked = false);

private:
    QScopedPointer<Ui::DeviceSettingsDialog> m_ui;
};

#endif // DEVICE_SETTINGS_DIALOG_H