#include "LaserLayerDialog.h"
#include "widget/EditSlider.h"
#include "ui_LaserLayerDialog.h"

#include "common/Config.h"
#include "laser/LaserDriver.h"
#include "scene/LaserLayer.h"
#include "scene/LaserDocument.h"
#include "scene/PageInformation.h"
#include "widget/InputWidgetWrapper.h"

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

    m_ui->editSliderCuttingMinSpeed->setMaximum(2000);
    m_ui->editSliderCuttingRunSpeed->setMaximum(2000);

    m_ui->editSliderCuttingLaserPower->setMaximum(100);
    m_ui->editSliderCuttingLaserPower->setTextTemplate("%1%");
    m_ui->editSliderCuttingLaserPower->setStep(0.1);
    m_ui->editSliderCuttingMinSpeedPower->setMaximum(100);
    m_ui->editSliderCuttingMinSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderCuttingMinSpeedPower->setStep(0.1);
    m_ui->editSliderCuttingRunSpeedPower->setMaximum(100);
    m_ui->editSliderCuttingRunSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderCuttingRunSpeedPower->setStep(0.1);

    m_ui->editSliderEngravingMinSpeed->setMaximum(2000);
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
    //m_ui->editSliderHalftoneGridSize->setMinimum(1);
    //m_ui->editSliderHalftoneGridSize->setMaximum(200);

    m_ui->editSliderDPI->setMinimum(1);
    m_ui->editSliderDPI->setMaximum(1200);
    m_ui->editSliderLPI->setMinimum(1);
    m_ui->editSliderLPI->setMaximum(1200);

    m_ui->editSliderFillingMinSpeed->setMaximum(2000);
    m_ui->editSliderFillingRunSpeed->setMaximum(2000);

    m_ui->editSliderFillingLaserPower->setMaximum(100);
    m_ui->editSliderFillingLaserPower->setTextTemplate("%1%");
    m_ui->editSliderFillingLaserPower->setStep(0.1);
    m_ui->editSliderFillingMinSpeedPower->setMaximum(100);
    m_ui->editSliderFillingMinSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderFillingMinSpeedPower->setStep(0.1);
    m_ui->editSliderFillingRunSpeedPower->setMaximum(100);
    m_ui->editSliderFillingRunSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderFillingRunSpeedPower->setStep(0.1);
    m_ui->editSliderFillingRowInterval->setMinimum(1);
    m_ui->editSliderFillingRowInterval->setMaximum(1000);

    m_ui->lineEditLayerName->setText(m_layer->name());

	resetParameters();
}

void LaserLayerDialog::resetParameters()
{
	m_ui->editSliderCuttingMinSpeed->setValue(m_layer->cuttingMinSpeed());
    m_ui->editSliderCuttingRunSpeed->setValue(m_layer->cuttingRunSpeed());
    m_ui->editSliderCuttingLaserPower->setValue(m_layer->cuttingLaserPower());
    m_ui->editSliderCuttingMinSpeedPower->setValue(m_layer->cuttingMinSpeedPower());
    m_ui->editSliderCuttingRunSpeedPower->setValue(m_layer->cuttingRunSpeedPower());

    m_ui->editSliderEngravingMinSpeed->setValue(m_layer->engravingMinSpeed());
    m_ui->editSliderEngravingRunSpeed->setValue(m_layer->engravingRunSpeed());
    m_ui->editSliderEngravingLaserPower->setValue(m_layer->engravingLaserPower());
    m_ui->editSliderEngravingMinSpeedPower->setValue(m_layer->engravingMinSpeedPower());
    m_ui->editSliderEngravingRunSpeedPower->setValue(m_layer->engravingRunSpeedPower());
    m_ui->editSliderEngravingRowInterval->setValue(m_layer->engravingRowInterval());
    m_ui->checkBoxUseHalftone->setChecked(m_layer->useHalftone());
    m_ui->doubleSpinBoxHalftoneAngles->setValue(m_layer->halftoneAngles());
    //m_ui->editSliderHalftoneGridSize->setValue(m_layer->halftoneGridSize());
    m_ui->editSliderDPI->setValue(m_layer->dpi());
    m_ui->editSliderLPI->setValue(m_layer->lpi());

    m_ui->editSliderFillingMinSpeed->setValue(m_layer->fillingMinSpeed());
    m_ui->editSliderFillingRunSpeed->setValue(m_layer->fillingRunSpeed());
    m_ui->editSliderFillingLaserPower->setValue(m_layer->fillingLaserPower());
    m_ui->editSliderFillingMinSpeedPower->setValue(m_layer->fillingMinSpeedPower());
    m_ui->editSliderFillingRunSpeedPower->setValue(m_layer->fillingRunSpeedPower());
    m_ui->editSliderFillingRowInterval->setValue(m_layer->fillingRowInterval());
}

void LaserLayerDialog::restoreParameters()
{	
	m_ui->editSliderCuttingMinSpeed->setValue(Config::CuttingLayer::minSpeed());
    m_ui->editSliderCuttingRunSpeed->setValue(Config::CuttingLayer::runSpeed());
    m_ui->editSliderCuttingLaserPower->setValue(Config::CuttingLayer::laserPower());
    m_ui->editSliderCuttingMinSpeedPower->setValue(Config::CuttingLayer::minPower());
    m_ui->editSliderCuttingRunSpeedPower->setValue(Config::CuttingLayer::maxPower());

    m_ui->editSliderEngravingMinSpeed->setValue(Config::EngravingLayer::minSpeed());
    m_ui->editSliderEngravingRunSpeed->setValue(Config::EngravingLayer::runSpeed());
    m_ui->editSliderEngravingLaserPower->setValue(Config::EngravingLayer::laserPower());
    m_ui->editSliderEngravingMinSpeedPower->setValue(Config::EngravingLayer::minPower());
    m_ui->editSliderEngravingRunSpeedPower->setValue(Config::EngravingLayer::maxPower());
    m_ui->editSliderEngravingRowInterval->setValue(Config::EngravingLayer::rowInterval());
    m_ui->checkBoxUseHalftone->setChecked(Config::EngravingLayer::useHalftone());
    m_ui->doubleSpinBoxHalftoneAngles->setValue(Config::EngravingLayer::halftoneAngles());
    //m_ui->editSliderHalftoneGridSize->setValue(Config::EngravingLayer::halftoneGridSize());
    m_ui->editSliderDPI->setValue(Config::EngravingLayer::DPI());
    m_ui->editSliderLPI->setValue(Config::EngravingLayer::LPI());

    m_ui->editSliderFillingMinSpeed->setValue(Config::FillingLayer::minSpeed());
    m_ui->editSliderFillingRunSpeed->setValue(Config::FillingLayer::runSpeed());
    m_ui->editSliderFillingLaserPower->setValue(Config::FillingLayer::laserPower());
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
		m_ui->groupBoxCutting->setVisible(false);
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
		m_ui->groupBoxCutting->setVisible(true);
		m_ui->groupBoxEngraving->setVisible(false);
        m_ui->groupBoxFilling->setVisible(true);
		adjustSize();
        m_type = LLT_FILLING;
    }
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
		Config::CuttingLayer::minSpeedItem()->setValue(m_ui->editSliderCuttingMinSpeed->value());
		Config::CuttingLayer::runSpeedItem()->setValue(m_ui->editSliderCuttingRunSpeed->value());
		Config::CuttingLayer::laserPowerItem()->setValue(m_ui->editSliderCuttingLaserPower->value());
		Config::CuttingLayer::minPowerItem()->setValue(m_ui->editSliderCuttingMinSpeedPower->value());
		Config::CuttingLayer::maxPowerItem()->setValue(m_ui->editSliderCuttingRunSpeedPower->value());

		Config::EngravingLayer::minSpeedItem()->setValue(m_ui->editSliderEngravingMinSpeed->value());
		Config::EngravingLayer::runSpeedItem()->setValue(m_ui->editSliderEngravingRunSpeed->value());
		Config::EngravingLayer::laserPowerItem()->setValue(m_ui->editSliderEngravingLaserPower->value());
		Config::EngravingLayer::minPowerItem()->setValue(m_ui->editSliderEngravingMinSpeedPower->value());
		Config::EngravingLayer::maxPowerItem()->setValue(m_ui->editSliderEngravingRunSpeedPower->value());
        Config::EngravingLayer::rowIntervalItem()->setValue(m_ui->editSliderEngravingRowInterval->value());
        Config::EngravingLayer::halftoneAnglesItem()->setValue(m_ui->doubleSpinBoxHalftoneAngles->value());
        //Config::EngravingLayer::halftoneGridSizeItem()->setValue(m_ui->editSliderHalftoneGridSize->value());
		Config::EngravingLayer::LPIItem()->setValue(m_ui->editSliderLPI->value());
		Config::EngravingLayer::DPIItem()->setValue(m_ui->editSliderDPI->value());
		Config::EngravingLayer::useHalftoneItem()->setValue(m_ui->checkBoxUseHalftone->isChecked());

		Config::FillingLayer::minSpeedItem()->setValue(m_ui->editSliderFillingMinSpeed->value());
		Config::FillingLayer::runSpeedItem()->setValue(m_ui->editSliderFillingRunSpeed->value());
		Config::FillingLayer::laserPowerItem()->setValue(m_ui->editSliderFillingLaserPower->value());
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
    //if (m_type == LLT_CUTTING || m_type == LLT_FILLING)
    //{
    m_layer->setCuttingMinSpeed(m_ui->editSliderCuttingMinSpeed->value());
    m_layer->setCuttingRunSpeed(m_ui->editSliderCuttingRunSpeed->value());
    m_layer->setCuttingLaserPower(m_ui->editSliderCuttingLaserPower->value());
    m_layer->setCuttingMinSpeedPower(m_ui->editSliderCuttingMinSpeedPower->value());
    m_layer->setCuttingRunSpeedPower(m_ui->editSliderCuttingRunSpeedPower->value());
    //}
    //else if (m_type == LLT_ENGRAVING)
    //{
    m_layer->setEngravingMinSpeed(m_ui->editSliderEngravingMinSpeed->value());
    m_layer->setEngravingRunSpeed(m_ui->editSliderEngravingRunSpeed->value());
    m_layer->setEngravingLaserPower(m_ui->editSliderEngravingLaserPower->value());
    m_layer->setEngravingMinSpeedPower(m_ui->editSliderEngravingMinSpeedPower->value());
    m_layer->setEngravingRunSpeedPower(m_ui->editSliderEngravingRunSpeedPower->value());
    m_layer->setEngravingRowInterval(m_ui->editSliderEngravingRowInterval->value());
    m_layer->setHalftoneAngles(m_ui->doubleSpinBoxHalftoneAngles->value());
    //m_layer->setHalftoneGridSize(m_ui->editSliderHalftoneGridSize->value());
    m_layer->setLpi(m_ui->editSliderLPI->value());
    m_layer->setDpi(m_ui->editSliderDPI->value());
    m_layer->setUseHalftone(m_ui->checkBoxUseHalftone->isChecked());
    //}

    m_layer->setFillingMinSpeed(m_ui->editSliderFillingMinSpeed->value());
    m_layer->setFillingRunSpeed(m_ui->editSliderFillingRunSpeed->value());
    m_layer->setFillingLaserPower(m_ui->editSliderFillingLaserPower->value());
    m_layer->setFillingMinSpeedPower(m_ui->editSliderFillingMinSpeedPower->value());
    m_layer->setFillingRunSpeedPower(m_ui->editSliderFillingRunSpeedPower->value());
    m_layer->setFillingRowInterval(m_ui->editSliderFillingRowInterval->value());

    m_layer->setType(m_type);

    QDialog::accept();
}

