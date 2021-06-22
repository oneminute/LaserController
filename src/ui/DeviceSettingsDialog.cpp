#include "DeviceSettingsDialog.h"
#include "ui_DeviceSettingsDialog.h"

#include <QMessageBox>
#include "ChangePasswordDialog.h"

DeviceSettingsDialog::DeviceSettingsDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::DeviceSettingsDialog)
{
    m_ui->setupUi(this);

    m_ui->editSliderCuttingStartSpeed->setMaximum(600);
    m_ui->editSliderCuttingRunSpeed->setMaximum(600);
    m_ui->editSliderCuttingPower->setMaximum(1000);
    m_ui->editSliderCuttingStartPowerRate->setMaximum(1000);
    m_ui->editSliderCuttingRunPowerRate->setMaximum(1000);

    m_ui->editSliderEngravingStartSpeed->setMaximum(600);
    m_ui->editSliderEngravingRunSpeed->setMaximum(600);
    m_ui->editSliderEngravingPower->setMaximum(1000);
    m_ui->editSliderEngravingStartPowerRate->setMaximum(1000);
    m_ui->editSliderEngravingRunPowerRate->setMaximum(1000);

    m_ui->editSliderEngravingRowSpacing->setMinimum(1);
    m_ui->editSliderEngravingRowSpacing->setMaximum(1000);
    m_ui->editSliderEngravingColumnSpacing->setMinimum(0);
    m_ui->editSliderEngravingColumnSpacing->setMaximum(1000);

    m_ui->editSliderMoveStartSpeed->setMaximum(600);
    m_ui->editSliderMoveRunSpeed->setMaximum(600);
    m_ui->editSliderXBacklash->setMaximum(3000000);
    m_ui->editSliderYBacklash->setMaximum(3000000);
    m_ui->editSliderCutLaserFrequency->setMaximum(2000);
    m_ui->editSliderCarvingLaserFrequency->setMaximum(6000);

    m_ui->editSliderResetSpeed->setMaximum(600);
    m_ui->editSliderZeroSpeed->setMaximum(600);
    m_ui->comboBoxZeroPosition->addItem(tr("Top Left"), 1);
    m_ui->comboBoxZeroPosition->addItem(tr("Top Right"), 2);
    m_ui->comboBoxZeroPosition->addItem(tr("Bottom Right"), 3);
    m_ui->comboBoxZeroPosition->addItem(tr("Bottom Left"), 4);
    m_ui->comboBoxMotorDirection->addItem(tr("0"), 0);
    m_ui->comboBoxMotorDirection->addItem(tr("1"), 1);
    m_ui->comboBoxMotorDirection->addItem(tr("2"), 2);
    m_ui->comboBoxMotorDirection->addItem(tr("3"), 3);
    m_ui->comboBoxLimitDirection->addItem(tr("0"), 0);
    m_ui->comboBoxLimitDirection->addItem(tr("1"), 1);
    m_ui->comboBoxLimitDirection->addItem(tr("2"), 2);
    m_ui->comboBoxLimitDirection->addItem(tr("3"), 3);
    m_ui->editSliderPageWidth->setMaximum(10000);
    m_ui->editSliderPageHeight->setMaximum(10000);
    m_ui->editSliderDrawingUnit->setMaximum(3000);

    m_ui->groupBoxManufactor->setVisible(false);

    connect(&LaserDriver::instance(), &LaserDriver::registersFectched, this, &DeviceSettingsDialog::registersFetched);
    connect(&LaserDriver::instance(), &LaserDriver::rightManufacturerPassword, this, &DeviceSettingsDialog::rightManufactorPassword);
    connect(&LaserDriver::instance(), &LaserDriver::wrongManufacturerPassword, this, &DeviceSettingsDialog::wrongManufactorPassword);
    connect(m_ui->pushButtonRead, &QPushButton::clicked, this, &DeviceSettingsDialog::onPushButtonReadClicked);
    connect(m_ui->pushButtonWrite, &QPushButton::clicked, this, &DeviceSettingsDialog::onPushButtonWriteClicked);
    connect(m_ui->pushButtonDefault, &QPushButton::clicked, this, &DeviceSettingsDialog::onPushButtonDefaultClicked);
    connect(m_ui->pushButtonVerifyPassword, &QPushButton::clicked, this, &DeviceSettingsDialog::onPushButtonVerifyClicked);
    connect(m_ui->pushButtonClearPassword, &QPushButton::clicked, this, &DeviceSettingsDialog::onPushButtonClearClicked);
    connect(m_ui->pushButtonResetPassword, &QPushButton::clicked, this, &DeviceSettingsDialog::onPushButtonResetClicked);

    LaserDriver::instance().readAllSysParamFromCard();
}

DeviceSettingsDialog::~DeviceSettingsDialog()
{
}

void DeviceSettingsDialog::makeDefault()
{
    m_ui->editSliderCuttingStartSpeed->setValue(15);
    m_ui->editSliderCuttingRunSpeed->setValue(50);
    m_ui->editSliderCuttingPower->setValue(80);
    m_ui->editSliderCuttingStartPowerRate->setValue(600);
    m_ui->editSliderCuttingRunPowerRate->setValue(1000);

    m_ui->editSliderEngravingStartSpeed->setValue(30);
    m_ui->editSliderEngravingRunSpeed->setValue(200);
    m_ui->editSliderEngravingPower->setValue(120);
    m_ui->editSliderEngravingStartPowerRate->setValue(0);
    m_ui->editSliderEngravingRunPowerRate->setValue(1000);

    m_ui->editSliderEngravingRowSpacing->setValue(7);
    m_ui->editSliderEngravingColumnSpacing->setValue(0);

    m_ui->editSliderMoveStartSpeed->setValue(15);
    m_ui->editSliderMoveRunSpeed->setValue(200);
    m_ui->editSliderXBacklash->setValue(0);
    m_ui->editSliderYBacklash->setValue(0);
    m_ui->lineEditTotalRunningTime->setText(0);
    m_ui->lineEditLaserTotalUseTime->setText(0);
    m_ui->editSliderCutLaserFrequency->setValue(2000);
    m_ui->editSliderCarvingLaserFrequency->setValue(6000);

    m_ui->editSliderResetSpeed->setValue(15);
    m_ui->editSliderZeroSpeed->setValue(200);
    m_ui->doubleSpinBoxXStepLength->setValue(6.32914);
    m_ui->doubleSpinBoxYStepLength->setValue(6.32914);

    m_ui->comboBoxZeroPosition->setCurrentIndex(0);
    m_ui->comboBoxMotorDirection->setCurrentIndex(0);
    m_ui->comboBoxLimitDirection->setCurrentIndex(0);

    m_ui->editSliderPageWidth->setValue(210);
    m_ui->editSliderPageHeight->setValue(270);

    m_ui->editSliderDrawingUnit->setValue(1016);
}

void DeviceSettingsDialog::rightManufactorPassword()
{
    m_ui->groupBoxManufactor->setVisible(true);
}

void DeviceSettingsDialog::wrongManufactorPassword()
{
    QMessageBox::warning(this, tr("Wrong password"), tr("Wrong password"));
    m_ui->groupBoxManufactor->setVisible(false);
}

void DeviceSettingsDialog::onPushButtonReadClicked(bool checked)
{
    LaserDriver::instance().readAllSysParamFromCard();
}

void DeviceSettingsDialog::onPushButtonWriteClicked(bool checked)
{
    LaserDriver::RegistersMap values;
    /*values[LaserDriver::REG_18] = QString::number(m_ui->editSliderCuttingPower->value());
    values[LaserDriver::REG_20] = QString::number(m_ui->editSliderCuttingStartPowerRate->value());
    values[LaserDriver::REG_19] = QString::number(m_ui->editSliderCuttingRunPowerRate->value());

    values[LaserDriver::REG_04] = QString::number(m_ui->editSliderEngravingStartSpeed->value());
    values[LaserDriver::REG_15] = QString::number(m_ui->editSliderEngravingPower->value());
    values[LaserDriver::REG_17] = QString::number(m_ui->editSliderEngravingStartPowerRate->value());
    values[LaserDriver::REG_16] = QString::number(m_ui->editSliderEngravingRunPowerRate->value());
    values[LaserDriver::REG_14] = QString::number(m_ui->editSliderEngravingRowSpacing->value());
    values[LaserDriver::REG_13] = QString::number(m_ui->editSliderEngravingColumnSpacing->value());

    values[LaserDriver::REG_40] = QString::number(m_ui->editSliderMoveStartSpeed->value());
    values[LaserDriver::REG_05] = QString::number(m_ui->editSliderMoveRunSpeed->value());
    values[LaserDriver::REG_11] = QString::number(m_ui->editSliderXBacklash->value());
    values[LaserDriver::REG_12] = QString::number(m_ui->editSliderYBacklash->value());

    values[LaserDriver::REG_25] = QString::number(m_ui->editSliderCutLaserFrequency->value());
    values[LaserDriver::REG_27] = QString::number(m_ui->editSliderCarvingLaserFrequency->value());

    values[LaserDriver::REG_03] = QString::number(m_ui->editSliderResetSpeed->value());
    values[LaserDriver::REG_07] = QString::number(m_ui->editSliderZeroSpeed->value());
    values[LaserDriver::REG_09] = QString::number(m_ui->doubleSpinBoxXStepLength->value());
    values[LaserDriver::REG_10] = QString::number(m_ui->doubleSpinBoxYStepLength->value());

    values[LaserDriver::REG_08] = m_ui->comboBoxZeroPosition->currentData().toInt() - 1;
    values[LaserDriver::REG_21] = m_ui->comboBoxMotorDirection->currentData();
    values[LaserDriver::REG_22] = m_ui->comboBoxLimitDirection->currentData();

    int pageSize = 0;
    pageSize = m_ui->editSliderPageWidth->value() << 16 | m_ui->editSliderPageHeight->value();
    values[LaserDriver::REG_38] = pageSize;
    values[LaserDriver::REG_39] = m_ui->editSliderDrawingUnit->value();*/

    LaserDriver::instance().writeSysParamToCard(values);
}

void DeviceSettingsDialog::onPushButtonDefaultClicked(bool checked)
{
    makeDefault();
}

void DeviceSettingsDialog::onPushButtonVerifyClicked(bool checked)
{
    LaserDriver::instance().checkFactoryPassword(m_ui->lineEditManufacturerPassword->text());
}

void DeviceSettingsDialog::onPushButtonClearClicked(bool checked)
{
    m_ui->lineEditManufacturerPassword->setText("");
}

void DeviceSettingsDialog::onPushButtonResetClicked(bool checked)
{
    disconnect(&LaserDriver::instance(), &LaserDriver::rightManufacturerPassword, this, &DeviceSettingsDialog::rightManufactorPassword);
    ChangePasswordDialog dialog;
    dialog.exec();
    connect(&LaserDriver::instance(), &LaserDriver::rightManufacturerPassword, this, &DeviceSettingsDialog::rightManufactorPassword);
}

void DeviceSettingsDialog::registersFetched(const LaserDriver::RegistersMap& datas)
{
    /*m_ui->editSliderCuttingStartSpeed->setValue(15);
    m_ui->editSliderCuttingRunSpeed->setValue(50);
    m_ui->editSliderCuttingPower->setValue(datas[LaserDriver::REG_18].toInt());
    m_ui->editSliderCuttingStartPowerRate->setValue(datas[LaserDriver::REG_20].toInt());
    m_ui->editSliderCuttingRunPowerRate->setValue(datas[LaserDriver::REG_19].toInt());

    m_ui->editSliderEngravingStartSpeed->setValue(datas[LaserDriver::REG_04].toInt());
    m_ui->editSliderEngravingRunSpeed->setValue(200);
    m_ui->editSliderEngravingPower->setValue(datas[LaserDriver::REG_15].toInt());
    m_ui->editSliderEngravingStartPowerRate->setValue(datas[LaserDriver::REG_17].toInt());
    m_ui->editSliderEngravingRunPowerRate->setValue(datas[LaserDriver::REG_16].toInt());

    m_ui->editSliderEngravingRowSpacing->setValue(datas[LaserDriver::REG_14].toInt());
    m_ui->editSliderEngravingColumnSpacing->setValue(datas[LaserDriver::REG_13].toInt());

    m_ui->editSliderMoveStartSpeed->setValue(datas[LaserDriver::REG_40].toInt());
    m_ui->editSliderMoveRunSpeed->setValue(datas[LaserDriver::REG_05].toInt());
    m_ui->editSliderXBacklash->setValue(datas[LaserDriver::REG_11].toInt());
    m_ui->editSliderYBacklash->setValue(datas[LaserDriver::REG_12].toInt());
    m_ui->lineEditTotalRunningTime->setText(datas[LaserDriver::REG_23].toString());
    m_ui->lineEditLaserTotalUseTime->setText(datas[LaserDriver::REG_24].toString());
    m_ui->editSliderCutLaserFrequency->setValue(datas[LaserDriver::REG_25].toInt());
    m_ui->editSliderCarvingLaserFrequency->setValue(datas[LaserDriver::REG_27].toInt());

    m_ui->editSliderResetSpeed->setValue(datas[LaserDriver::REG_03].toInt());
    m_ui->editSliderZeroSpeed->setValue(datas[LaserDriver::REG_07].toInt());
    m_ui->doubleSpinBoxXStepLength->setValue(datas[LaserDriver::REG_09].toDouble());
    m_ui->doubleSpinBoxYStepLength->setValue(datas[LaserDriver::REG_10].toDouble());

    m_ui->comboBoxZeroPosition->setCurrentIndex(datas[LaserDriver::REG_08].toInt() - 1);
    m_ui->comboBoxMotorDirection->setCurrentIndex(datas[LaserDriver::REG_21].toInt());
    m_ui->comboBoxLimitDirection->setCurrentIndex(datas[LaserDriver::REG_22].toInt());

    int pageSize = datas[LaserDriver::REG_38].toInt();
    int pageWidth = (pageSize >> 16) & 0x0000FFFF;
    int pageHeight = pageSize & 0x0000FFFF;
    m_ui->editSliderPageWidth->setValue(pageWidth);
    m_ui->editSliderPageHeight->setValue(pageHeight);

    m_ui->editSliderDrawingUnit->setValue(datas[LaserDriver::REG_39].toInt());*/
}
