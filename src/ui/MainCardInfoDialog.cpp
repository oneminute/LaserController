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

void MainCardInfoDialog::onGetMainCardInfo(QMap<QString, QString> info)
{
    m_ui->labelMainCard->setText(info["mainCard"]);
    m_ui->labelMainCardRegisteredDate->setText(info["mainCardRegisteredDate"]);
    m_ui->labelMainCardActivatedDate->setText(info["mainCardActivatedDate"]);
    m_ui->labelBoundDongle->setText(info["boundDongle"]);
    m_ui->labelBoundDongleRegisteredDate->setText(info["boundDongleRegisteredDate"]);
    m_ui->labelBoundDongleActivatedDate->setText(info["boundDongleActivatedDate"]);
    m_ui->labelBoundDongleBindingTimes->setText(info["boundDongleBindingTimes"]);
    m_ui->labelCurrentDongle->setText(info["dongle"]);
    m_ui->labelCurrentDongleRegisteredDate->setText(info["dongleRegisteredDate"]);
    m_ui->labelCurrentDongleActivatedDate->setText(info["dongleActivatedDate"]);
    m_ui->labelCurrentDongleBindingTimes->setText(info["dongleBindingTimes"]);
    m_ui->labelHardwareRegisteredDate->setText(info["hardwareRegisteredDate"]);
    m_ui->labelHardwareActivatedDate->setText(info["hardwareActivatedDate"]);
    m_ui->labelHardwareMaintainingTimes->setText(info["hardwareMaintainingTimes"]);
}

MainCardInfoDialog::~MainCardInfoDialog()
{
}
