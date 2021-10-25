#include "ActivationDialog.h"
#include "ui_ActivationDialog.h"

#include <QMessageBox>
#include <QMovie>
#include <QRegularExpression>
#include <QtMath>

#include "common/common.h"
#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "laser/LaserDriver.h"

ActivationDialog::ActivationDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ActivationDialog)
    , m_reEmail("([a-z0-9]+(?:[._-][a-z0-9]+)*)@([a-z0-9]+(?:[.-][a-z0-9]+)*\.[a-z]{2,})")
    , m_reCode("^[0-9]{6}$")
    , m_reUserName("[a-zA-Z0-9!@#$%-_\\w]{6,12}", QRegularExpression::UseUnicodePropertiesOption)
{
    m_ui->setupUi(this);

    connect(m_ui->pushButtonSend, &QPushButton::clicked, this, &ActivationDialog::onPushButtonSendClicked);
    connect(m_ui->pushButtonActivate, &QPushButton::clicked, this, &ActivationDialog::onPushButtonActivateClicked);
    connect(m_ui->pushButtonClose, &QPushButton::clicked, this, &ActivationDialog::onPushButtonCloseClicked);
    connect(m_ui->lineEditMail, &QLineEdit::textEdited, this, &ActivationDialog::onFieldTextEdited);
    connect(m_ui->lineEditCode, &QLineEdit::textEdited, this, &ActivationDialog::onFieldTextEdited);
    connect(m_ui->lineEditUserName, &QLineEdit::textEdited, this, &ActivationDialog::onFieldTextEdited);

    connect(&m_sendTimer, &QTimer::timeout, this, &ActivationDialog::onSendTimerTimeout);
    connect(&m_updateTimer, &QTimer::timeout, this, &ActivationDialog::onUpdateTimerTimeout);

    //QMovie* loading = new QMovie(":/ui/icons/images/loading_anim.gif");
    //m_ui->labelIcon->setMovie(loading);
    //LaserApplication::device->autoActivateMainCard();

    if (LaserApplication::device->isMainCardActivated())
    {
        m_ui->labelIcon->setPixmap(QPixmap(":/ui/icons/images/Image-dialog-info.ico"));
        m_ui->labelStatus->setText(tr("Activated"));
        m_ui->labelDescription->setText(tr("Your main board is already activated at %1")
            .arg(LaserApplication::device->hardwareActivatedDate()));
        m_ui->pushButtonActivate->hide();
    }
    else
    {
        m_ui->labelIcon->setPixmap(QPixmap(":/ui/icons/images/Image-dialog-warning.ico"));
        m_ui->labelStatus->setText(tr("Inactivated"));
        m_ui->labelDescription->setText(tr("1. Networking is required to activate the board, please confirm that you have a vlid connection;\n2. To continue activation, please click the \"Activate\" button below."));
        m_ui->pushButtonActivate->show();
    }

    m_ui->stackedWidget->setCurrentIndex(0);
}

ActivationDialog::~ActivationDialog()
{
}

void ActivationDialog::onPushButtonSendClicked(bool checked)
{
    QString email = m_ui->lineEditMail->text().trimmed();
    if (LaserApplication::device->sendAuthenticationEmail(email))
    {
        m_sendTimer.setInterval(1000 * 5);
        m_sendTimer.start();
        m_updateTimer.setInterval(250);
        m_updateTimer.start();
        m_ui->pushButtonSend->setEnabled(false);
        QMessageBox::information(this, tr("Sent successfully"), tr("Sent successfully. Please check your mail box to receive the activation code."));
    }
    else
    {
        QMessageBox::warning(this, tr("Sent failure"), tr("Please check your email or network connection."));
    }
}

void ActivationDialog::onPushButtonActivateClicked(bool checked)
{
    if (m_ui->stackedWidget->currentIndex() == 0)
    {
        switch (LaserApplication::device->autoActivateMainCard())
        {
        case MAR_Activated:
        {
            m_ui->labelIcon->setPixmap(QPixmap(":/ui/icons/images/Image-dialog-info.ico"));
            m_ui->labelStatus->setText(tr("Activated"));
            m_ui->labelDescription->setText(tr("Your main board is already activated"));
            m_ui->pushButtonActivate->hide();
            QMessageBox::information(this, tr("Activated"), tr("Activated"));
            this->close();
            break;
        }
        case MAR_Inactivated:
        {
            m_ui->labelIcon->setPixmap(QPixmap(":/ui/icons/images/Image-dialog-warning.ico"));
            m_ui->labelStatus->setText(tr("Inactivated"));
            m_ui->labelDescription->setText(tr("1. Networking is required to activate the board, please confirm that you have a vlid connection;\n2. To continue activation, please click the \"Activate\" button below."));
            m_ui->pushButtonActivate->show();
            QMessageBox::warning(this, tr("Activate failure"), tr("You should use the email to fetch activation code and activate your main card online."));
            m_ui->stackedWidget->setCurrentIndex(1);
            onFieldTextEdited("");
            m_ui->pushButtonSend->setEnabled(false);
            m_ui->pushButtonActivate->setEnabled(false);
            break;
        }
        case MAR_Error:
        case MAR_Other:
        {
            m_ui->labelIcon->setPixmap(QPixmap(":/ui/icons/images/Image-dialog-warning.ico"));
            m_ui->labelStatus->setText(tr("Error"));
            m_ui->labelDescription->setText(tr("An error occurred. Please communicate your administartor."));
            break;
        }
        }
    }
    else if (m_ui->stackedWidget->currentIndex() == 1)
    {
        QString result = LaserApplication::device->activateMainCard(
            m_ui->lineEditMail->text(),
            m_ui->lineEditCode->text(),
            m_ui->lineEditUserName->text(),
            m_ui->lineEditTelephone->text(),
            m_ui->lineEditAddress->text(),
            m_ui->lineEditQQ->text(),
            m_ui->lineEditWechat->text(),
            m_ui->lineEditCountry->text(),
            m_ui->comboBoxDistributor->currentText(),
            m_ui->comboBoxBrand->currentText(),
            m_ui->comboBoxModel->currentText()
        );
        m_ui->pushButtonActivate->setEnabled(false);
        if (!result.isEmpty())
        {
            m_ui->labelTips->setStyleSheet("color: rgb(0, 255, 0);font: 12pt;");
            m_ui->labelTips->setText(tr("Activation successful!"));
            emit LaserApplication::device->mainCardActivationChanged(true);
        }
        else
        {
            m_ui->labelTips->setStyleSheet("color: rgb(255, 0, 0);font: 12pt;");
            m_ui->labelTips->setText(tr("Activation failure!"));
            m_ui->pushButtonActivate->setEnabled(true);
            emit LaserApplication::device->mainCardActivationChanged(false);
        }
    }
}

void ActivationDialog::onPushButtonCloseClicked(bool checked)
{
    this->close();
}

void ActivationDialog::onActivationResponse(bool activated)
{
}

bool ActivationDialog::validateMail()
{
    QString email = m_ui->lineEditMail->text();
    QRegularExpressionMatch match = m_reEmail.match(email);
    qLogD << "validate mail state: " << match;
    return match.hasMatch();
}

bool ActivationDialog::validateCode()
{
    QRegularExpressionMatch match = m_reCode.match(m_ui->lineEditCode->text());
    qLogD << "vaildate code state: " << match;
    return match.hasMatch();
}

bool ActivationDialog::validateUserName()
{
    QRegularExpressionMatch match = m_reUserName.match(m_ui->lineEditUserName->text());
    qLogD << "vaildate username state: " << match;
    return match.hasMatch();
}

void ActivationDialog::onFieldTextEdited(const QString& text)
{
    bool mailValid = validateMail();
    bool codeValid = validateCode();
    bool userNameValid = validateUserName();

    QString errorMsg;
    if (mailValid && !m_sendTimer.isActive())
    {
        m_ui->pushButtonSend->setEnabled(true);
    }
    else
    {
        errorMsg.append(tr("Invalid email!\n"));
    }

    if (!codeValid)
    {
        errorMsg.append(tr("Invalid Activation Code!\n"));
    }

    if (!userNameValid)
    {
        errorMsg.append(tr("Invalid User Name!"));
    }

    m_ui->labelTips->setText(errorMsg);

    if (mailValid && codeValid && userNameValid)
    {
        m_ui->pushButtonActivate->setEnabled(true);
    }
    else
    {
        m_ui->pushButtonActivate->setEnabled(false);
    }
}

void ActivationDialog::onSendTimerTimeout()
{
    m_sendTimer.stop();
    m_updateTimer.stop();
    m_ui->pushButtonSend->setText(tr("Send"));
    validateMail();
}

void ActivationDialog::onUpdateTimerTimeout()
{
    if (m_sendTimer.isActive())
    {
        m_ui->pushButtonSend->setText(tr("Send(%1)").arg(m_sendTimer.remainingTime() / 1000));
    }
}

