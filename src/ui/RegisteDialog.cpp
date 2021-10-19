#include "RegisteDialog.h" 
#include "ui_RegisteDialog.h"

#include <QClipBoard>
#include <QMessageBox>

#include "common/common.h"
#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "laser/LaserDriver.h"

RegisteDialog::RegisteDialog(QWidget* parent) 
    : QDialog(parent) 
    , m_ui(new Ui::RegisteDialog)
{ 
    m_ui->setupUi(this);

    QString dongleId = LaserApplication::device->requestDongleId();
    QString mainCardId = LaserApplication::device->requestMainCardId();
    QString registeId = LaserApplication::device->requestRegisteId();

    m_ui->lineEditCardId->setText(mainCardId);
    m_ui->lineEditRegisteId->setText(registeId);
    m_ui->lineEditDongleId->setText(dongleId);

    m_ui->labelAPIVersion->setText(LaserApplication::device->apiLibVersion());
    m_ui->labelEXEVersion->setText(LaserApplication::applicationVersion());

    connect(m_ui->pushButtonCopyCardId, &QPushButton::clicked, this, &RegisteDialog::onButtonCopyCardIdClicked);
    connect(m_ui->pushButtonCopyDongleId, &QPushButton::clicked, this, &RegisteDialog::onButtonCopyDongleIdClicked);
    connect(m_ui->pushButtonCopyRegisteId, &QPushButton::clicked, this, &RegisteDialog::onButtonCopyRegisteIdClicked);
    connect(m_ui->pushButtonRegisteCode, &QPushButton::clicked, this, &RegisteDialog::onButtonRegiste);
} 
 
RegisteDialog::~RegisteDialog() 
{ 
}

void RegisteDialog::onButtonCopyCardIdClicked(bool checked)
{
    LaserApplication::clipboard()->setText(m_ui->lineEditCardId->text());
}

void RegisteDialog::onButtonCopyDongleIdClicked(bool checked)
{
    LaserApplication::clipboard()->setText(m_ui->lineEditDongleId->text());
}

void RegisteDialog::onButtonCopyRegisteIdClicked(bool checked)
{
    LaserApplication::clipboard()->setText(m_ui->lineEditRegisteId->text());
}

void RegisteDialog::onButtonRegiste(bool checked)
{
    QString code = m_ui->lineEditRegisterCode->text().trimmed();
    if (code.length() != 24)
    {
        QMessageBox::warning(this, tr("Invalide registe code"), tr("The registration code contains at least 24 valid characters. Please check your input."));
        return;
    }

    LaserApplication::device->registeMainCard(code, this);
}

