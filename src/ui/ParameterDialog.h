#ifndef PARAMETER_DIALOG_H
#define PARAMETER_DIALOG_H

#include <qobject.h>
#include <QDialog>
#include <QScopedPointer>

#include "laser/LaserDriver.h"

namespace Ui
{
    class ParameterDialog;
}

class ParameterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ParameterDialog(QWidget* parent = nullptr);
    virtual ~ParameterDialog();

    void makeDefault();

protected slots:
    void registersFetched(const QMap<LaserDriver::RegisterType, QVariant>& datas);
    void rightManufactorPassword();
    void wrongManufactorPassword();

    void onPushButtonReadClicked(bool checked = false);

    void onPushButtonWriteClicked(bool checked = false);

    void onPushButtonDefaultClicked(bool checked = false);

    void onPushButtonVerifyClicked(bool checked = false);

    void onPushButtonClearClicked(bool checked = false);

    void onPushButtonResetClicked(bool checked = false);

private:
    QScopedPointer<Ui::ParameterDialog> m_ui;
};

#endif // PARAMETER_DIALOG_H