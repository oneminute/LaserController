#include "RegisterDialog.h" 
#include "ui_RegisterDialog.h"

#include <QClipBoard>
#include <QMessageBox>

#include "common/common.h"
#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "laser/LaserDriver.h"

RegisterDialog::RegisterDialog(QWidget* parent) 
    : QDialog(parent) 
    , m_ui(new Ui::RegisterDialog)
    , m_reCode("^[0-9a-zA-Z]{24}$")
{ 
    m_ui->setupUi(this);

    QString dongleId = LaserApplication::device->requestDongleId();
    QString mainCardId = LaserApplication::device->requestMainCardId();
    QString registerId = LaserApplication::device->requestRegisteId();
    qLogD << "dongleId: " << dongleId;
    qLogD << "mainCardId: " << mainCardId;
    qLogD << "registerId: " << registerId;

    m_ui->lineEditRegisteId->setText(registerId);
    m_ui->lineEditDongleId->setText(dongleId);

    QString mainCardModal = LaserApplication::device->getMainCardModal();
    qLogD << "main card modal: " << mainCardModal;
    m_ui->labelMainCardModal->setText(mainCardModal);
    m_ui->labelFirmwareVersion->setText(LaserApplication::device->firmwareVersion());
    m_ui->labelAPIVersion->setText(LaserApplication::device->apiLibVersion());
    m_ui->labelEXEVersion->setText(LaserApplication::applicationVersion());

    connect(m_ui->pushButtonCopyDongleId, &QPushButton::clicked, this, &RegisterDialog::onButtonCopyDongleIdClicked);
    connect(m_ui->pushButtonCopyRegisteId, &QPushButton::clicked, this, &RegisterDialog::onButtonCopyRegisteIdClicked);
    connect(m_ui->pushButtonRegisteCode, &QPushButton::clicked, this, &RegisterDialog::onButtonRegiste);
    connect(m_ui->pushButtonStatus, &QPushButton::clicked, this, &RegisterDialog::onButtonStatusClicked);
    connect(LaserApplication::device, &LaserDevice::mainCardRegistrationChanged, this, &RegisterDialog::onRegistrationChanged);
} 
 
RegisterDialog::~RegisterDialog() 
{ 
}

void RegisterDialog::onButtonCopyDongleIdClicked(bool checked)
{
    LaserApplication::clipboard()->setText(m_ui->lineEditDongleId->text());
}

void RegisterDialog::onButtonCopyRegisteIdClicked(bool checked)
{
    LaserApplication::clipboard()->setText(m_ui->lineEditRegisteId->text());
}

void RegisterDialog::onButtonRegiste(bool checked)
{
    QString code = m_ui->lineEditRegisterCode->text().trimmed();
    QRegularExpressionMatch match = m_reCode.match(code);
    if (!match.hasMatch())
    {
        QMessageBox::warning(this, tr("Invalide registe code"), tr("The registration code contains at least 24 valid characters. Please check your input."));
        return;
    }

    LaserApplication::device->registerMainCard(code, this);
    //LaserApplication::device->requestMainCardRegInfo();
}

void RegisterDialog::onButtonStatusClicked(bool checked)
{
    LaserApplication::device->requestMainCardRegInfo();
}

void RegisterDialog::onRegistrationChanged(bool registered)
{
    if (registered)
    {
        m_ui->labelStatus->setText(tr("Registered"));
    }
    else
    {
        m_ui->labelStatus->setText(tr("Unregistered"));
    }
}

