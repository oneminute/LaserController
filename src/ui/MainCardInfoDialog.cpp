#include "MainCardInfoDialog.h"
#include "ui_MainCardInfoDialog.h"

#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "common/common.h"

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
    m_ui->labelMainCard->setText(LaserApplication::device->mainCardId());;
    m_ui->labelMainCardRegisteredDate->setText(LaserApplication::device->mainCardRegisteredDate());;
    m_ui->labelMainCardActivatedDate->setText(LaserApplication::device->mainCardActivatedDate());;
    m_ui->labelBoundDongle->setText(LaserApplication::device->boundDongleId());;
    m_ui->labelBoundDongleRegisteredDate->setText(LaserApplication::device->boundDongleRegisteredDate());;
    m_ui->labelBoundDongleActivatedDate->setText(LaserApplication::device->boundDongleActivatedDate());;
    m_ui->labelBoundDongleBindingTimes->setText(LaserApplication::device->boundDongleBindingTimes());;
    m_ui->labelCurrentDongle->setText(LaserApplication::device->dongleId());;
    m_ui->labelCurrentDongleRegisteredDate->setText(LaserApplication::device->dongleRegisteredDate());;
    m_ui->labelCurrentDongleActivatedDate->setText(LaserApplication::device->dongleActivatedDate());;
    m_ui->labelCurrentDongleBindingTimes->setText(LaserApplication::device->dongleBindingTimes());;
    m_ui->labelHardwareRegisteredDate->setText(LaserApplication::device->hardwareRegisteredDate());;
    m_ui->labelHardwareActivatedDate->setText(LaserApplication::device->hardwareActivatedDate());;
    m_ui->labelHardwareMaintainingTimes->setText(LaserApplication::device->hardwareMaintainingTimes());;
}

MainCardInfoDialog::~MainCardInfoDialog()
{
}
