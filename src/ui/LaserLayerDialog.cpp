#include "LaserLayerDialog.h"
#include "widget/EditSlider.h"
#include "ui_LaserLayerDialog.h"

#include "common/Config.h"
#include "laser/LaserDriver.h"
#include "scene/LaserLayer.h"
#include "scene/LaserDocument.h"
#include "scene/PageInformation.h"
#include "widget/InputWidgetWrapper.h"

#include <QPushButton>

LaserLayerDialog::LaserLayerDialog(LaserLayer* layer, QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::LaserLayerDialog)
    , m_doc(nullptr)
    , m_layer(layer)
{
    Q_ASSERT(layer);
    m_doc = layer->document();
    m_type = layer->type();
    initUi();
}

LaserLayerDialog::~LaserLayerDialog()
{
}

void LaserLayerDialog::initUi()
{
    m_ui->setupUi(this);

    connect(m_ui->radioButtonCutting, &QRadioButton::toggled, this, &LaserLayerDialog::onCuttingToggled);
    connect(m_ui->radioButtonEngraving, &QRadioButton::toggled, this, &LaserLayerDialog::onEngravingToggled);
    connect(m_ui->radioButtonFilling, &QRadioButton::toggled, this, &LaserLayerDialog::onBothToggled);
    connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &LaserLayerDialog::onButtonClicked);
    connect(m_ui->checkBoxEngravingEnableCutting, &QCheckBox::toggled, this, &LaserLayerDialog::onEngravingEnableCuttingToggled);
    connect(m_ui->checkBoxFillingEnableCutting, &QCheckBox::toggled, this, &LaserLayerDialog::onFillingEnableCuttingToggled);

    if (m_type == LLT_ENGRAVING)
    {
        m_ui->radioButtonCutting->setChecked(false);
        m_ui->radioButtonEngraving->setChecked(true);
        m_ui->radioButtonFilling->setChecked(false);
    }
    else if (m_type == LLT_CUTTING)
    {
        m_ui->radioButtonCutting->setChecked(true);
        m_ui->radioButtonEngraving->setChecked(false);
        m_ui->radioButtonFilling->setChecked(false);
    }
    else if (m_type == LLT_FILLING)
    {
        m_ui->radioButtonCutting->setChecked(true);
        m_ui->radioButtonEngraving->setChecked(false);
        m_ui->radioButtonFilling->setChecked(true);
    }

    m_ui->editSliderCuttingRunSpeed->setMaximum(2000);
    m_ui->editSliderCuttingMinSpeedPower->setMaximum(100);
    m_ui->editSliderCuttingMinSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderCuttingMinSpeedPower->setStep(0.1);
    m_ui->editSliderCuttingRunSpeedPower->setMaximum(100);
    m_ui->editSliderCuttingRunSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderCuttingRunSpeedPower->setStep(0.1);

    m_ui->editSliderEngravingRunSpeed->setMaximum(2000);
    m_ui->editSliderEngravingLaserPower->setMaximum(100);
    m_ui->editSliderEngravingLaserPower->setTextTemplate("%1%");
    m_ui->editSliderEngravingLaserPower->setStep(0.1);
    m_ui->editSliderEngravingMinSpeedPower->setMaximum(100);
    m_ui->editSliderEngravingMinSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderEngravingMinSpeedPower->setStep(0.1);
    m_ui->editSliderEngravingRunSpeedPower->setMaximum(100);
    m_ui->editSliderEngravingRunSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderEngravingRunSpeedPower->setStep(0.1);
    m_ui->editSliderEngravingRowInterval->setMinimum(1);
    m_ui->editSliderEngravingRowInterval->setMaximum(1000);

    m_ui->editSliderDPI->setMinimum(1);
    m_ui->editSliderDPI->setMaximum(1200);
    m_ui->editSliderLPI->setMinimum(1);
    m_ui->editSliderLPI->setMaximum(1200);

    m_ui->editSliderFillingRunSpeed->setMaximum(2000);
    m_ui->editSliderFillingMinSpeedPower->setMaximum(100);
    m_ui->editSliderFillingMinSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderFillingMinSpeedPower->setStep(0.1);
    m_ui->editSliderFillingRunSpeedPower->setMaximum(100);
    m_ui->editSliderFillingRunSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderFillingRunSpeedPower->setStep(0.1);
    m_ui->editSliderFillingRowInterval->setMinimum(1);
    m_ui->editSliderFillingRowInterval->setMaximum(1000);

    m_ui->lineEditLayerName->setText(m_layer->name());

    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    m_ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));
    m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText(tr("Restore Defaults"));
    m_ui->buttonBox->button(QDialogButtonBox::Reset)->setText(tr("Reset"));

    m_ui->labelCuttingSpeed->setText(Config::CuttingLayer::runSpeedItem()->title());
    m_ui->labelCuttingMinPower->setText(Config::CuttingLayer::minPowerItem()->title());
    m_ui->labelCuttingMaxPower->setText(Config::CuttingLayer::maxPowerItem()->title());

    m_ui->labelEngravingEnableCutting->setText(Config::EngravingLayer::enableCuttingItem()->title());
    m_ui->labelEngravingSpeed->setText(Config::EngravingLayer::runSpeedItem()->title());
    m_ui->labelEngravingPower->setText(Config::EngravingLayer::laserPowerItem()->title());
    m_ui->labelEngravingMinPower->setText(Config::EngravingLayer::minPowerItem()->title());
    m_ui->labelEngravingMaxPower->setText(Config::EngravingLayer::maxPowerItem()->title());
    m_ui->labelEngravingRowInterval->setText(Config::EngravingLayer::rowIntervalItem()->title());
    m_ui->labelUseHalftone->setText(Config::EngravingLayer::useHalftoneItem()->title());
    m_ui->labelAngles->setText(Config::EngravingLayer::halftoneAnglesItem()->title());
    m_ui->labelLPI->setText(Config::EngravingLayer::LPIItem()->title());
    m_ui->labelDPI->setText(Config::EngravingLayer::DPIItem()->title());

    m_ui->labelFillingEnableCutting->setText(Config::FillingLayer::enableCuttingItem()->title());
    m_ui->labelFillingSpeed->setText(Config::FillingLayer::runSpeedItem()->title());
    m_ui->labelFillingMinPower->setText(Config::FillingLayer::minPowerItem()->title());
    m_ui->labelFillingMaxPower->setText(Config::FillingLayer::maxPowerItem()->title());
    m_ui->labelFillingRowInterval->setText(Config::FillingLayer::rowIntervalItem()->title());

	resetParameters();
}

void LaserLayerDialog::resetParameters()
{
    m_ui->editSliderCuttingRunSpeed->setValue(m_layer->cuttingRunSpeed());
    m_ui->editSliderCuttingMinSpeedPower->setValue(m_layer->cuttingMinSpeedPower());
    m_ui->editSliderCuttingRunSpeedPower->setValue(m_layer->cuttingRunSpeedPower());

    m_ui->checkBoxEngravingEnableCutting->setChecked(m_layer->engravingEnableCutting());
    m_ui->editSliderEngravingRunSpeed->setValue(m_layer->engravingRunSpeed());
    m_ui->editSliderEngravingLaserPower->setValue(m_layer->engravingLaserPower());
    m_ui->editSliderEngravingMinSpeedPower->setValue(m_layer->engravingMinSpeedPower());
    m_ui->editSliderEngravingRunSpeedPower->setValue(m_layer->engravingRunSpeedPower());
    m_ui->editSliderEngravingRowInterval->setValue(m_layer->engravingRowInterval());
    m_ui->checkBoxUseHalftone->setChecked(m_layer->useHalftone());
    m_ui->doubleSpinBoxHalftoneAngles->setValue(m_layer->halftoneAngles());
    m_ui->editSliderDPI->setValue(m_layer->dpi());
    m_ui->editSliderLPI->setValue(m_layer->lpi());

    m_ui->editSliderFillingRunSpeed->setValue(m_layer->fillingRunSpeed());
    m_ui->editSliderFillingMinSpeedPower->setValue(m_layer->fillingMinSpeedPower());
    m_ui->editSliderFillingRunSpeedPower->setValue(m_layer->fillingRunSpeedPower());
    m_ui->editSliderFillingRowInterval->setValue(m_layer->fillingRowInterval());
}

void LaserLayerDialog::restoreParameters()
{	
    m_ui->editSliderCuttingRunSpeed->setValue(Config::CuttingLayer::runSpeed());
    m_ui->editSliderCuttingMinSpeedPower->setValue(Config::CuttingLayer::minPower());
    m_ui->editSliderCuttingRunSpeedPower->setValue(Config::CuttingLayer::maxPower());

    m_ui->checkBoxEngravingEnableCutting->setChecked(Config::EngravingLayer::enableCutting());
    m_ui->editSliderEngravingRunSpeed->setValue(Config::EngravingLayer::runSpeed());
    m_ui->editSliderEngravingLaserPower->setValue(Config::EngravingLayer::laserPower());
    m_ui->editSliderEngravingMinSpeedPower->setValue(Config::EngravingLayer::minPower());
    m_ui->editSliderEngravingRunSpeedPower->setValue(Config::EngravingLayer::maxPower());
    m_ui->editSliderEngravingRowInterval->setValue(Config::EngravingLayer::rowInterval());
    m_ui->checkBoxUseHalftone->setChecked(Config::EngravingLayer::useHalftone());
    m_ui->doubleSpinBoxHalftoneAngles->setValue(Config::EngravingLayer::halftoneAngles());
    m_ui->editSliderDPI->setValue(Config::EngravingLayer::DPI());
    m_ui->editSliderLPI->setValue(Config::EngravingLayer::LPI());

    m_ui->editSliderFillingRunSpeed->setValue(Config::FillingLayer::runSpeed());
    m_ui->editSliderFillingMinSpeedPower->setValue(Config::FillingLayer::minPower());
    m_ui->editSliderFillingRunSpeedPower->setValue(Config::FillingLayer::maxPower());
    m_ui->editSliderFillingRowInterval->setValue(Config::FillingLayer::rowInterval());
}

void LaserLayerDialog::onCuttingToggled(bool checked)
{
    if (checked)
    {
		m_ui->groupBoxCutting->setVisible(true);
		m_ui->groupBoxEngraving->setVisible(false);
        m_ui->groupBoxFilling->setVisible(false);
		adjustSize();
        m_type = LLT_CUTTING;
    }
}

void LaserLayerDialog::onEngravingToggled(bool checked)
{
    if (checked)
    {
		m_ui->groupBoxCutting->setVisible(m_ui->checkBoxEngravingEnableCutting->isChecked());
		m_ui->groupBoxEngraving->setVisible(true);
        m_ui->groupBoxFilling->setVisible(false);
		adjustSize();
        m_type = LLT_ENGRAVING;
    }
}

void LaserLayerDialog::onBothToggled(bool checked)
{
    if (checked)
    {
		m_ui->groupBoxCutting->setVisible(m_ui->checkBoxFillingEnableCutting->isChecked());
		m_ui->groupBoxEngraving->setVisible(false);
        m_ui->groupBoxFilling->setVisible(true);
		adjustSize();
        m_type = LLT_FILLING;
    }
}

void LaserLayerDialog::onEngravingEnableCuttingToggled(bool checked)
{
    m_ui->groupBoxCutting->setVisible(checked);
	adjustSize();
}

void LaserLayerDialog::onFillingEnableCuttingToggled(bool checked)
{
    m_ui->groupBoxCutting->setVisible(checked);
	adjustSize();
}

void LaserLayerDialog::onButtonClicked(QAbstractButton * button)
{
    QDialogButtonBox::StandardButton stdButton = m_ui->buttonBox->standardButton(button);
	if (stdButton == QDialogButtonBox::Reset)
	{
		resetParameters();
	}
	else if (stdButton == QDialogButtonBox::RestoreDefaults)
	{
		restoreParameters();
	}
	else if (stdButton == QDialogButtonBox::Save)
	{
		Config::CuttingLayer::runSpeedItem()->setValue(m_ui->editSliderCuttingRunSpeed->value());
		Config::CuttingLayer::minPowerItem()->setValue(m_ui->editSliderCuttingMinSpeedPower->value());
		Config::CuttingLayer::maxPowerItem()->setValue(m_ui->editSliderCuttingRunSpeedPower->value());

        Config::EngravingLayer::enableCuttingItem()->setValue(m_ui->checkBoxEngravingEnableCutting->isChecked());
		Config::EngravingLayer::runSpeedItem()->setValue(m_ui->editSliderEngravingRunSpeed->value());
		Config::EngravingLayer::laserPowerItem()->setValue(m_ui->editSliderEngravingLaserPower->value());
		Config::EngravingLayer::minPowerItem()->setValue(m_ui->editSliderEngravingMinSpeedPower->value());
		Config::EngravingLayer::maxPowerItem()->setValue(m_ui->editSliderEngravingRunSpeedPower->value());
        Config::EngravingLayer::rowIntervalItem()->setValue(m_ui->editSliderEngravingRowInterval->value());
        Config::EngravingLayer::halftoneAnglesItem()->setValue(m_ui->doubleSpinBoxHalftoneAngles->value());
		Config::EngravingLayer::LPIItem()->setValue(m_ui->editSliderLPI->value());
		Config::EngravingLayer::DPIItem()->setValue(m_ui->editSliderDPI->value());
		Config::EngravingLayer::useHalftoneItem()->setValue(m_ui->checkBoxUseHalftone->isChecked());

		Config::FillingLayer::runSpeedItem()->setValue(m_ui->editSliderFillingRunSpeed->value());
		Config::FillingLayer::minPowerItem()->setValue(m_ui->editSliderFillingMinSpeedPower->value());
		Config::FillingLayer::maxPowerItem()->setValue(m_ui->editSliderFillingRunSpeedPower->value());
        Config::FillingLayer::rowIntervalItem()->setValue(m_ui->editSliderFillingRowInterval->value());

		if (Config::isModified())
		{
			Config::save();
		}
	}
}

void LaserLayerDialog::accept()
{
    m_layer->setCuttingRunSpeed(m_ui->editSliderCuttingRunSpeed->value());
    m_layer->setCuttingMinSpeedPower(m_ui->editSliderCuttingMinSpeedPower->value());
    m_layer->setCuttingRunSpeedPower(m_ui->editSliderCuttingRunSpeedPower->value());

    m_layer->setEngravingRunSpeed(m_ui->editSliderEngravingRunSpeed->value());
    m_layer->setEngravingLaserPower(m_ui->editSliderEngravingLaserPower->value());
    m_layer->setEngravingMinSpeedPower(m_ui->editSliderEngravingMinSpeedPower->value());
    m_layer->setEngravingRunSpeedPower(m_ui->editSliderEngravingRunSpeedPower->value());
    m_layer->setEngravingRowInterval(m_ui->editSliderEngravingRowInterval->value());
    m_layer->setHalftoneAngles(m_ui->doubleSpinBoxHalftoneAngles->value());
    m_layer->setLpi(m_ui->editSliderLPI->value());
    m_layer->setDpi(m_ui->editSliderDPI->value());
    m_layer->setUseHalftone(m_ui->checkBoxUseHalftone->isChecked());

    m_layer->setFillingRunSpeed(m_ui->editSliderFillingRunSpeed->value());
    m_layer->setFillingMinSpeedPower(m_ui->editSliderFillingMinSpeedPower->value());
    m_layer->setFillingRunSpeedPower(m_ui->editSliderFillingRunSpeedPower->value());
    m_layer->setFillingRowInterval(m_ui->editSliderFillingRowInterval->value());

    m_layer->setType(m_type);

    QDialog::accept();
}

