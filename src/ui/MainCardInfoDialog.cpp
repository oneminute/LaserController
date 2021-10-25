#include "MainCardInfoDialog.h"
#include "ui_MainCardInfoDialog.h"

#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "common/common.h"
#include "common/Config.h"

MainCardInfoDialog::MainCardInfoDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::MainCardInfoDialog)
{
    m_ui->setupUi(this);

    connect(LaserApplication::device, &LaserDevice::mainCardInfoFetched, this, &MainCardInfoDialog::onGetMainCardInfo);

    LaserApplication::device->requestMainCardInfo();
}

void MainCardInfoDialog::onGetMainCardInfo()
{
    m_ui->lineEditFirmwareVersion->setText(LaserApplication::device->firmwareVersion());
    m_ui->lineEditSoftwareVersion->setText(LaserApplication::applicationVersion());
    m_ui->lineEditTotalRunningTime->setText(QString::number(Config::SystemRegister::sysRunTime()));
    m_ui->lineEditLaserTubeRunningTime->setText(QString::number(Config::SystemRegister::laserRunTime()));
    m_ui->lineEditMachiningTimes->setText(QString::number(Config::SystemRegister::sysRunNum()));
    m_ui->lineEditDongleId->setText(LaserApplication::device->dongleId());
    m_ui->lineEditMainCardId->setText(LaserApplication::device->hardwareIdentID());
}

MainCardInfoDialog::~MainCardInfoDialog()
{
}
