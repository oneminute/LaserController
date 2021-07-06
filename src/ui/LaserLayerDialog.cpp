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
    connect(m_ui->radioButtonBoth, &QRadioButton::toggled, this, &LaserLayerDialog::onBothToggled);
    connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &LaserLayerDialog::onButtonClicked);

    if (m_type == LLT_ENGRAVING)
    {
        m_ui->radioButtonCutting->setChecked(false);
        m_ui->radioButtonEngraving->setChecked(true);
        m_ui->radioButtonBoth->setChecked(false);
    }
    else if (m_type == LLT_CUTTING)
    {
        m_ui->radioButtonCutting->setChecked(true);
        m_ui->radioButtonEngraving->setChecked(false);
        m_ui->radioButtonBoth->setChecked(false);
    }
    else if (m_type == LLT_BOTH)
    {
        m_ui->radioButtonCutting->setChecked(false);
        m_ui->radioButtonEngraving->setChecked(false);
        m_ui->radioButtonBoth->setChecked(true);
    }

    m_ui->editSliderCuttingMinSpeed->setMaximum(600);
    m_ui->editSliderCuttingRunSpeed->setMaximum(600);
    m_ui->editSliderCuttingLaserPower->setMaximum(1000);
    m_ui->editSliderCuttingMinSpeedPower->setMaximum(1000);
    m_ui->editSliderCuttingRunSpeedPower->setMaximum(1000);
    m_ui->editSliderEngravingMinSpeed->setMaximum(600);
    m_ui->editSliderEngravingRunSpeed->setMaximum(600);
    m_ui->editSliderEngravingLaserPower->setMaximum(1000);
    m_ui->editSliderEngravingMinSpeedPower->setMaximum(1000);
    m_ui->editSliderEngravingRunSpeedPower->setMaximum(1000);
    m_ui->editSliderDPI->setMinimum(0);
    m_ui->editSliderDPI->setMaximum(1200);
    m_ui->editSliderLPI->setMinimum(1);
    m_ui->editSliderLPI->setMaximum(100);

    m_ui->lineEditLayerName->setText(m_layer->name());

	resetParameters();
}

void LaserLayerDialog::resetParameters()
{
	m_ui->editSliderCuttingMinSpeed->setValue(m_layer->minSpeed());
    m_ui->editSliderCuttingRunSpeed->setValue(m_layer->runSpeed());
    m_ui->editSliderCuttingLaserPower->setValue(m_layer->laserPower());
    m_ui->editSliderCuttingMinSpeedPower->setValue(m_layer->minSpeedPower());
    m_ui->editSliderCuttingRunSpeedPower->setValue(m_layer->runSpeedPower());

    m_ui->editSliderEngravingMinSpeed->setValue(m_layer->minSpeed());
    m_ui->editSliderEngravingRunSpeed->setValue(m_layer->runSpeed());
    m_ui->editSliderEngravingLaserPower->setValue(m_layer->laserPower());
    m_ui->editSliderEngravingMinSpeedPower->setValue(m_layer->minSpeedPower());
    m_ui->editSliderEngravingRunSpeedPower->setValue(m_layer->runSpeedPower());
    m_ui->checkBoxUseHalftone->setChecked(m_layer->useHalftone());
    m_ui->editSliderDPI->setValue(m_layer->dpi());
    m_ui->editSliderLPI->setValue(m_layer->lpi());
}

void LaserLayerDialog::restoreParameters()
{	
	/*m_ui->editSliderCuttingMinSpeed->setValue(Config::defaultCuttingLayerMinSpeed());
    m_ui->editSliderCuttingRunSpeed->setValue(Config::defaultCuttingLayerRunSpeed());
    m_ui->editSliderCuttingLaserPower->setValue(Config::defaultCuttingLayerLaserPower());
    m_ui->editSliderCuttingMinSpeedPower->setValue(Config::defaultCuttingLayerMinSpeedPower());
    m_ui->editSliderCuttingRunSpeedPower->setValue(Config::defaultCuttingLayerRunSpeedPower());

    m_ui->editSliderEngravingMinSpeed->setValue(Config::defaultEngravingLayerMinSpeed());
    m_ui->editSliderEngravingRunSpeed->setValue(Config::defaultEngravingLayerRunSpeed());
    m_ui->editSliderEngravingLaserPower->setValue(Config::defaultEngravingLayerLaserPower());
    m_ui->editSliderEngravingMinSpeedPower->setValue(Config::defaultEngravingLayerMinSpeedPower());
    m_ui->editSliderEngravingRunSpeedPower->setValue(Config::defaultEngravingLayerRunSpeedPower());
    m_ui->checkBoxUseHalftone->setChecked(Config::defaultEngravingLayerUseHalftone());
    m_ui->editSliderDPI->setValue(Config::defaultEngravingLayerDPI());
    m_ui->editSliderLPI->setValue(Config::defaultEngravingLayerLPI());*/
}

void LaserLayerDialog::onCuttingToggled(bool checked)
{
    if (checked)
    {
		m_ui->groupBoxCutting->setVisible(true);
		m_ui->groupBoxEngraving->setVisible(false);
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
		adjustSize();
        m_type = LLT_ENGRAVING;
    }
}

void LaserLayerDialog::onBothToggled(bool checked)
{
    if (checked)
    {
        m_type = LLT_BOTH;
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
		/*Config::setCuttingLayerMinSpeed(m_ui->editSliderCuttingMinSpeed->value());
		Config::setCuttingLayerRunSpeed(m_ui->editSliderCuttingRunSpeed->value());
		Config::setCuttingLayerLaserPower(m_ui->editSliderCuttingLaserPower->value());
		Config::setCuttingLayerMinSpeedPower(m_ui->editSliderCuttingMinSpeedPower->value());
		Config::setCuttingLayerRunSpeedPower(m_ui->editSliderCuttingRunSpeedPower->value());

		Config::setEngravingLayerMinSpeed(m_ui->editSliderEngravingMinSpeed->value());
		Config::setEngravingLayerRunSpeed(m_ui->editSliderEngravingRunSpeed->value());
		Config::setEngravingLayerLaserPower(m_ui->editSliderEngravingLaserPower->value());
		Config::setEngravingLayerMinSpeedPower(m_ui->editSliderEngravingMinSpeedPower->value());
		Config::setEngravingLayerRunSpeedPower(m_ui->editSliderEngravingRunSpeedPower->value());
		Config::setEngravingLayerLPI(m_ui->editSliderLPI->value());
		Config::setEngravingLayerDPI(m_ui->editSliderDPI->value());
		Config::setEngravingLayerUseHalftone(m_ui->checkBoxUseHalftone->isChecked());*/

		if (Config::isModified())
		{
			Config::save();
		}
	}
}

void LaserLayerDialog::accept()
{
	if (m_type == LLT_CUTTING)
	{
		m_layer->setMinSpeed(m_ui->editSliderCuttingMinSpeed->value());
		m_layer->setRunSpeed(m_ui->editSliderCuttingRunSpeed->value());
		m_layer->setLaserPower(m_ui->editSliderCuttingLaserPower->value());
		m_layer->setMinSpeedPower(m_ui->editSliderCuttingMinSpeedPower->value());
		m_layer->setRunSpeedPower(m_ui->editSliderCuttingRunSpeedPower->value());
	}
	else if (m_type == LLT_ENGRAVING)
	{
		m_layer->setMinSpeed(m_ui->editSliderEngravingMinSpeed->value());
		m_layer->setRunSpeed(m_ui->editSliderEngravingRunSpeed->value());
		m_layer->setLaserPower(m_ui->editSliderEngravingLaserPower->value());
		m_layer->setMinSpeedPower(m_ui->editSliderEngravingMinSpeedPower->value());
		m_layer->setRunSpeedPower(m_ui->editSliderEngravingRunSpeedPower->value());
		m_layer->setLpi(m_ui->editSliderLPI->value());
		m_layer->setDpi(m_ui->editSliderDPI->value());
		m_layer->setUseHalftone(m_ui->checkBoxUseHalftone->isChecked());
	}
	m_layer->setType(m_type);

    QDialog::accept();
}

