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

    if (m_layer->type() == LLT_ENGRAVING)
        m_ui->radioButtonEngraving->setChecked(true);
    else if (m_layer->type() == LLT_CUTTING)
        m_ui->radioButtonCutting->setChecked(true);
    else if (m_layer->type() == LLT_FILLING)
        m_ui->radioButtonFilling->setChecked(true);

    m_ui->comboBoxFillingType->addItem(tr("Line"), 0);
    m_ui->comboBoxFillingType->addItem(tr("Pixel"), 1);

    m_ui->editSliderCuttingRunSpeed->setStep(0.001);
    m_ui->editSliderCuttingRunSpeed->setMinimum(1);
    m_ui->editSliderCuttingRunSpeed->setMaximum(2000);
    m_ui->editSliderCuttingRunSpeed->setPage(10);
    m_ui->editSliderCuttingRunSpeed->setDecimals(3);

    m_ui->editSliderCuttingMinSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderCuttingMinSpeedPower->setStep(0.1);
    m_ui->editSliderCuttingMinSpeedPower->setPage(10);
    m_ui->editSliderCuttingMinSpeedPower->setDecimals(1);
    m_ui->editSliderCuttingMinSpeedPower->setMaximum(100);

    m_ui->editSliderCuttingRunSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderCuttingRunSpeedPower->setStep(0.1);
    m_ui->editSliderCuttingRunSpeedPower->setPage(10);
    m_ui->editSliderCuttingRunSpeedPower->setMaximum(100);
    m_ui->editSliderCuttingRunSpeedPower->setDecimals(1);

    m_ui->editSliderEngravingRunSpeed->setStep(0.001);
    m_ui->editSliderEngravingRunSpeed->setMinimum(1);
    m_ui->editSliderEngravingRunSpeed->setMaximum(2000);
    m_ui->editSliderEngravingRunSpeed->setPage(10);
    m_ui->editSliderEngravingRunSpeed->setDecimals(3);

    m_ui->editSliderEngravingLaserPower->setMaximum(100);
    m_ui->editSliderEngravingLaserPower->setTextTemplate("%1%");
    m_ui->editSliderEngravingLaserPower->setStep(0.1);
    m_ui->editSliderEngravingLaserPower->setPage(10);
    m_ui->editSliderEngravingLaserPower->setDecimals(1);

    m_ui->editSliderEngravingMinSpeedPower->setMaximum(100);
    m_ui->editSliderEngravingMinSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderEngravingMinSpeedPower->setStep(0.1);
    m_ui->editSliderEngravingMinSpeedPower->setPage(10);
    m_ui->editSliderEngravingMinSpeedPower->setDecimals(1);

    m_ui->editSliderEngravingRunSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderEngravingRunSpeedPower->setMaximum(100);
    m_ui->editSliderEngravingRunSpeedPower->setDecimals(0);
    m_ui->editSliderEngravingRunSpeedPower->setPage(10);
    m_ui->editSliderEngravingRunSpeedPower->setStep(0.1);

    m_ui->editSliderEngravingRowInterval->setMinimum(1);
    m_ui->editSliderEngravingRowInterval->setMaximum(10000);
    m_ui->editSliderEngravingRowInterval->setPageStep(10);

    m_ui->editSliderDPI->setMinimum(1);
    m_ui->editSliderDPI->setMaximum(1200);
    m_ui->editSliderDPI->setPageStep(10);

    m_ui->editSliderLPI->setMinimum(1);
    m_ui->editSliderLPI->setMaximum(1200);
    m_ui->editSliderLPI->setPageStep(10);

    m_ui->editSliderFillingRunSpeed->setStep(0.001);
    m_ui->editSliderFillingRunSpeed->setMinimum(1);
    m_ui->editSliderFillingRunSpeed->setMaximum(2000);
    m_ui->editSliderFillingRunSpeed->setPage(10);
    m_ui->editSliderFillingRunSpeed->setDecimals(3);

    m_ui->editSliderFillingMinSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderFillingMinSpeedPower->setMaximum(100);
    m_ui->editSliderFillingMinSpeedPower->setStep(0.1);
    m_ui->editSliderFillingMinSpeedPower->setPage(10);
    m_ui->editSliderFillingMinSpeedPower->setDecimals(1);

    m_ui->editSliderFillingRunSpeedPower->setStep(0.1);
    m_ui->editSliderFillingRunSpeedPower->setMaximum(100);
    m_ui->editSliderFillingRunSpeedPower->setPage(10);
    m_ui->editSliderFillingRunSpeedPower->setTextTemplate("%1%");
    m_ui->editSliderFillingRunSpeedPower->setDecimals(0);

    m_ui->editSliderFillingRowInterval->setMinimum(1);
    m_ui->editSliderFillingRowInterval->setMaximum(1000);
    m_ui->editSliderFillingRowInterval->setPageStep(10);

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

    connect(m_ui->radioButtonCutting, &QRadioButton::toggled, this, &LaserLayerDialog::onCuttingToggled);
    connect(m_ui->radioButtonEngraving, &QRadioButton::toggled, this, &LaserLayerDialog::onEngravingToggled);
    connect(m_ui->radioButtonFilling, &QRadioButton::toggled, this, &LaserLayerDialog::onFillingToggled);
    connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &LaserLayerDialog::onButtonClicked);
    connect(m_ui->checkBoxEngravingEnableCutting, &QCheckBox::toggled, this, &LaserLayerDialog::onEngravingEnableCuttingToggled);
    connect(m_ui->checkBoxFillingEnableCutting, &QCheckBox::toggled, this, &LaserLayerDialog::onFillingEnableCuttingToggled);
    connect(m_ui->comboBoxFillingType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LaserLayerDialog::onFillingTypeChanged);

	resetParameters();
}

void LaserLayerDialog::resetParameters()
{
    m_ui->editSliderCuttingRunSpeed->setIntValue(m_layer->cuttingRunSpeed());
    m_ui->editSliderCuttingMinSpeedPower->setIntValue(m_layer->cuttingMinSpeedPower());
    m_ui->editSliderCuttingRunSpeedPower->setIntValue(m_layer->cuttingRunSpeedPower());

    m_ui->checkBoxEngravingEnableCutting->setChecked(m_layer->engravingEnableCutting());
    m_ui->editSliderEngravingRunSpeed->setIntValue(m_layer->engravingRunSpeed());
    m_ui->editSliderEngravingLaserPower->setIntValue(m_layer->engravingLaserPower());
    m_ui->editSliderEngravingMinSpeedPower->setIntValue(m_layer->engravingMinSpeedPower());
    m_ui->editSliderEngravingRunSpeedPower->setIntValue(m_layer->engravingRunSpeedPower());
    m_ui->editSliderEngravingRowInterval->setValue(m_layer->engravingRowInterval());
    m_ui->checkBoxUseHalftone->setChecked(m_layer->useHalftone());
    m_ui->doubleSpinBoxHalftoneAngles->setValue(m_layer->halftoneAngles());
    m_ui->editSliderDPI->setValue(m_layer->dpi());
    m_ui->editSliderLPI->setValue(m_layer->lpi());

    m_ui->comboBoxFillingType->setCurrentIndex(m_layer->fillingType());
    m_ui->checkBoxFillingEnableCutting->setChecked(m_layer->fillingEnableCutting());
    m_ui->editSliderFillingRunSpeed->setIntValue(m_layer->fillingRunSpeed());
    m_ui->editSliderFillingMinSpeedPower->setIntValue(m_layer->fillingMinSpeedPower());
    m_ui->editSliderFillingRunSpeedPower->setIntValue(m_layer->fillingRunSpeedPower());
    m_ui->editSliderFillingRowInterval->setValue(m_layer->fillingRowInterval());

    updateControls();
}

void LaserLayerDialog::restoreParameters()
{	
    m_ui->editSliderCuttingRunSpeed->setIntValue(Config::CuttingLayer::runSpeed());
    m_ui->editSliderCuttingMinSpeedPower->setIntValue(Config::CuttingLayer::minPower());
    m_ui->editSliderCuttingRunSpeedPower->setIntValue(Config::CuttingLayer::maxPower());

    m_ui->checkBoxEngravingEnableCutting->setChecked(Config::EngravingLayer::enableCutting());
    m_ui->editSliderEngravingRunSpeed->setIntValue(Config::EngravingLayer::runSpeed());
    m_ui->editSliderEngravingLaserPower->setIntValue(Config::EngravingLayer::laserPower());
    m_ui->editSliderEngravingMinSpeedPower->setIntValue(Config::EngravingLayer::minPower());
    m_ui->editSliderEngravingRunSpeedPower->setIntValue(Config::EngravingLayer::maxPower());
    m_ui->editSliderEngravingRowInterval->setValue(Config::EngravingLayer::rowInterval());
    m_ui->checkBoxUseHalftone->setChecked(Config::EngravingLayer::useHalftone());
    m_ui->doubleSpinBoxHalftoneAngles->setValue(Config::EngravingLayer::halftoneAngles());
    m_ui->editSliderDPI->setValue(Config::EngravingLayer::DPI());
    m_ui->editSliderLPI->setValue(Config::EngravingLayer::LPI());

    m_ui->comboBoxFillingType->setCurrentIndex(Config::FillingLayer::fillingType());
    m_ui->checkBoxFillingEnableCutting->setChecked(Config::FillingLayer::enableCutting());
    m_ui->editSliderFillingRunSpeed->setIntValue(Config::FillingLayer::runSpeed());
    m_ui->editSliderFillingMinSpeedPower->setIntValue(Config::FillingLayer::minPower());
    m_ui->editSliderFillingRunSpeedPower->setIntValue(Config::FillingLayer::maxPower());
    m_ui->editSliderFillingRowInterval->setValue(Config::FillingLayer::rowInterval());
}

void LaserLayerDialog::onCuttingToggled(bool checked)
{
    if (checked)
    {
        m_type = LLT_CUTTING;
        updateControls();
    }
}

void LaserLayerDialog::onEngravingToggled(bool checked)
{
    if (checked)
    {
        m_type = LLT_ENGRAVING;
        updateControls();
    }
}

void LaserLayerDialog::onFillingToggled(bool checked)
{
    if (checked)
    {
        m_type = LLT_FILLING;
        updateControls();
    }
}

void LaserLayerDialog::onEngravingEnableCuttingToggled(bool checked)
{
    updateControls();
}

void LaserLayerDialog::onFillingEnableCuttingToggled(bool checked)
{
    updateControls();
}

void LaserLayerDialog::onFillingTypeChanged(int index)
{
    updateControls();
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
		Config::CuttingLayer::runSpeedItem()->setValue(m_ui->editSliderCuttingRunSpeed->intValue(), SS_DIRECTLY, this);
		Config::CuttingLayer::minPowerItem()->setValue(m_ui->editSliderCuttingMinSpeedPower->intValue(), SS_DIRECTLY, this);
		Config::CuttingLayer::maxPowerItem()->setValue(m_ui->editSliderCuttingRunSpeedPower->intValue(), SS_DIRECTLY, this);

        Config::EngravingLayer::enableCuttingItem()->setValue(m_ui->checkBoxEngravingEnableCutting->isChecked(), SS_DIRECTLY, this);
		Config::EngravingLayer::runSpeedItem()->setValue(m_ui->editSliderEngravingRunSpeed->intValue(), SS_DIRECTLY, this);
		Config::EngravingLayer::laserPowerItem()->setValue(m_ui->editSliderEngravingLaserPower->intValue(), SS_DIRECTLY, this);
		Config::EngravingLayer::minPowerItem()->setValue(m_ui->editSliderEngravingMinSpeedPower->intValue(), SS_DIRECTLY, this);
		Config::EngravingLayer::maxPowerItem()->setValue(m_ui->editSliderEngravingRunSpeedPower->intValue(), SS_DIRECTLY, this);
        Config::EngravingLayer::rowIntervalItem()->setValue(m_ui->editSliderEngravingRowInterval->value(), SS_DIRECTLY, this);
        Config::EngravingLayer::halftoneAnglesItem()->setValue(m_ui->doubleSpinBoxHalftoneAngles->value(), SS_DIRECTLY, this);
		Config::EngravingLayer::LPIItem()->setValue(m_ui->editSliderLPI->value(), SS_DIRECTLY, this);
		Config::EngravingLayer::DPIItem()->setValue(m_ui->editSliderDPI->value(), SS_DIRECTLY, this);
		Config::EngravingLayer::useHalftoneItem()->setValue(m_ui->checkBoxUseHalftone->isChecked(), SS_DIRECTLY, this);

        Config::FillingLayer::enableCuttingItem()->setValue(m_ui->checkBoxFillingEnableCutting->isChecked(), SS_DIRECTLY, this);
        Config::FillingLayer::fillingTypeItem()->setValue(m_ui->comboBoxFillingType->currentData().toInt(), SS_DIRECTLY, this);
		Config::FillingLayer::runSpeedItem()->setValue(m_ui->editSliderFillingRunSpeed->intValue(), SS_DIRECTLY, this);
		Config::FillingLayer::minPowerItem()->setValue(m_ui->editSliderFillingMinSpeedPower->intValue(), SS_DIRECTLY, this);
		Config::FillingLayer::maxPowerItem()->setValue(m_ui->editSliderFillingRunSpeedPower->intValue(), SS_DIRECTLY, this);
        Config::FillingLayer::rowIntervalItem()->setValue(m_ui->editSliderFillingRowInterval->value(), SS_DIRECTLY, this);

		if (Config::isModified())
		{
            Config::FillingLayer::group->save(true, true);
            Config::EngravingLayer::group->save(true, true);
            Config::CuttingLayer::group->save(true, true);
		}
	}
}

void LaserLayerDialog::updateControls()
{
    switch (m_type)
    {
    case LLT_ENGRAVING:
    {
        m_ui->groupBoxEngraving->setEnabled(true);
        m_ui->groupBoxCutting->setEnabled(false);
        m_ui->groupBoxFilling->setEnabled(false);
        m_ui->groupBoxFillingLines->setEnabled(false);
        m_ui->checkBoxEngravingEnableCutting->setEnabled(true);

        if (m_ui->checkBoxEngravingEnableCutting->isChecked())
        {
            m_ui->groupBoxCutting->setEnabled(true);
        }
    }
    break;
    case LLT_CUTTING:
    {
        m_ui->groupBoxEngraving->setEnabled(false);
        m_ui->groupBoxCutting->setEnabled(true);
        m_ui->groupBoxFilling->setEnabled(false);
        m_ui->groupBoxFillingLines->setEnabled(false);
    }
    break;
    case LLT_FILLING:
    {
        m_ui->groupBoxEngraving->setEnabled(false);
        m_ui->groupBoxCutting->setEnabled(false);
        m_ui->groupBoxFilling->setEnabled(true);
        m_ui->groupBoxFillingLines->setEnabled(true);
        m_ui->checkBoxEngravingEnableCutting->setEnabled(false);

        if (m_ui->checkBoxFillingEnableCutting->isChecked())
        {
            m_ui->groupBoxCutting->setEnabled(true);
        }

        if (m_ui->comboBoxFillingType->currentData().toInt() == FT_Pixel)
        {
            m_ui->groupBoxEngraving->setEnabled(true);
            m_ui->groupBoxFillingLines->setEnabled(false);
        }
        else if (m_ui->comboBoxFillingType->currentData().toInt() == FT_Line)
        {
            m_ui->groupBoxEngraving->setEnabled(false);
            m_ui->groupBoxFillingLines->setEnabled(true);
        }
    }
    break;
    }
}

void LaserLayerDialog::accept()
{
    m_layer->setCuttingRunSpeed(m_ui->editSliderCuttingRunSpeed->intValue());
    m_layer->setCuttingMinSpeedPower(m_ui->editSliderCuttingMinSpeedPower->intValue());
    m_layer->setCuttingRunSpeedPower(m_ui->editSliderCuttingRunSpeedPower->intValue());

    m_layer->setEngravingEnableCutting(m_ui->checkBoxEngravingEnableCutting->isChecked());
    m_layer->setEngravingRunSpeed(m_ui->editSliderEngravingRunSpeed->intValue());
    m_layer->setEngravingLaserPower(m_ui->editSliderEngravingLaserPower->intValue());
    m_layer->setEngravingMinSpeedPower(m_ui->editSliderEngravingMinSpeedPower->intValue());
    m_layer->setEngravingRunSpeedPower(m_ui->editSliderEngravingRunSpeedPower->intValue());
    m_layer->setEngravingRowInterval(m_ui->editSliderEngravingRowInterval->value());
    m_layer->setHalftoneAngles(m_ui->doubleSpinBoxHalftoneAngles->value());
    m_layer->setLpi(m_ui->editSliderLPI->value());
    m_layer->setDpi(m_ui->editSliderDPI->value());
    m_layer->setUseHalftone(m_ui->checkBoxUseHalftone->isChecked());

    m_layer->setFillingEnableCutting(m_ui->checkBoxFillingEnableCutting->isChecked());
    m_layer->setFillingType(m_ui->comboBoxFillingType->currentData().toInt());
    m_layer->setFillingRunSpeed(m_ui->editSliderFillingRunSpeed->intValue());
    m_layer->setFillingMinSpeedPower(m_ui->editSliderFillingMinSpeedPower->intValue());
    m_layer->setFillingRunSpeedPower(m_ui->editSliderFillingRunSpeedPower->intValue());
    m_layer->setFillingRowInterval(m_ui->editSliderFillingRowInterval->value());

    m_layer->setType(m_type);

    QDialog::accept();
}

