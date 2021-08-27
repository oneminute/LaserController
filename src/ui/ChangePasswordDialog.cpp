#include "ChangePasswordDialog.h"
#include "ui_ChangePasswordDialog.h"

#include <QMessageBox>

ChangePasswordDialog::ChangePasswordDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ChangePasswordDialog)
{
    m_ui->setupUi(this);

    connect(&LaserDriver::instance(), &LaserDriver::wrongManufacturerPassword, this, &ChangePasswordDialog::wrongManufacturerPassword);
    connect(&LaserDriver::instance(), &LaserDriver::rightManufacturerPassword, this, &ChangePasswordDialog::rightManufacturerPassword);
    connect(&LaserDriver::instance(), &LaserDriver::changeManufacturerPasswordOk, this, &ChangePasswordDialog::changeManufacturerPasswordOk);
    connect(&LaserDriver::instance(), &LaserDriver::changeManufacturerPasswordFailure, this, &ChangePasswordDialog::changeManufacturerPasswordFailure);
    connect(m_ui->pushButtonChange, &QPushButton::clicked, this, &ChangePasswordDialog::onPushButtonChangeClicked);
    connect(m_ui->pushButtonCancel, &QPushButton::clicked, this, &ChangePasswordDialog::onPushButtonCancelClicked);
}

ChangePasswordDialog::~ChangePasswordDialog()
{

}

void ChangePasswordDialog::onPushButtonChangeClicked(bool checked)
{
    if (m_ui->lineEditOldPassword->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid password"), tr("Old password cannot be empty!"));
        return;
    }

    if (m_ui->lineEditNewPassword->text().size() < 6)
    {
        QMessageBox::warning(this, tr("Invalid password"), tr("New password should contain at least 6 characters!"));
        return;
    }

    //LaserDriver::instance().checkFactoryPassword(m_ui->lineEditOldPassword->text());
}

void ChangePasswordDialog::onPushButtonCancelClicked(bool checked)
{
    this->close();
}

void ChangePasswordDialog::rightManufacturerPassword()
{
    LaserDriver::instance().changeFactoryPassword(m_ui->lineEditOldPassword->text(), m_ui->lineEditNewPassword->text());
}

void ChangePasswordDialog::wrongManufacturerPassword()
{
    QMessageBox::warning(this, tr("Invalid password"), tr("Wrong old password!"));
}

void ChangePasswordDialog::changeManufacturerPasswordOk()
{
    this->close();
}

void ChangePasswordDialog::changeManufacturerPasswordFailure()
{
    QMessageBox::warning(this, tr("Change Failure"), tr("Change password failure!"));
    this->close();
}
