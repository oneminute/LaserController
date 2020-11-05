#include "LaserLayerDialog.h"
#include "widget/EditSlider.h"
#include "ui_LaserLayerDialog.h"

#include "scene/LaserLayer.h"
#include "scene/LaserDocument.h"
#include "scene/PageInformation.h"
#include "laser/LaserDriver.h"

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

    if (m_type == LLT_ENGRAVING)
    {
        initEngravingParameters();
        m_ui->radioButtonCutting->setChecked(false);
        m_ui->radioButtonEngraving->setChecked(true);
        m_ui->radioButtonBoth->setChecked(false);
    }
    else if (m_type == LLT_CUTTING)
    {
        initCuttingParameters();
        m_ui->radioButtonCutting->setChecked(true);
        m_ui->radioButtonEngraving->setChecked(false);
        m_ui->radioButtonBoth->setChecked(false);
    }
    else if (m_type == LLT_BOTH)
    {
        initEngravingParameters();
        m_ui->radioButtonCutting->setChecked(false);
        m_ui->radioButtonEngraving->setChecked(false);
        m_ui->radioButtonBoth->setChecked(true);
    }

    m_ui->editSliderMinSpeed->setMaximum(600);
    m_ui->editSliderRunSpeed->setMaximum(600);
    m_ui->editSliderLaserPower->setMaximum(1000);
    m_ui->editSliderLineSpacing->setMaximum(50);
    m_ui->editSliderColumnSpacing->setMaximum(50);
    m_ui->editSliderMinSpeedPower->setMaximum(1000);
    m_ui->editSliderRunSpeedPower->setMaximum(1000);
    m_ui->editSliderDPI->setMinimum(0);
    m_ui->editSliderDPI->setMaximum(1200);
    m_ui->editSliderLPI->setMinimum(1);
    m_ui->editSliderLPI->setMaximum(100);

    m_ui->lineEditLayerName->setText(m_layer->name());
    m_ui->editSliderMinSpeed->setValue(m_layer->minSpeed());
    m_ui->editSliderRunSpeed->setValue(m_layer->runSpeed());
    m_ui->editSliderLaserPower->setValue(m_layer->laserPower());
    m_ui->checkBoxEngravingForward->setChecked(m_layer->engravingForward());
    m_ui->editSliderLineSpacing->setValue(m_layer->lineSpacing());
    m_ui->editSliderColumnSpacing->setValue(m_layer->columnSpacing());
    m_ui->editSliderMinSpeedPower->setValue(m_layer->minSpeedPower());
    m_ui->editSliderRunSpeedPower->setValue(m_layer->runSpeedPower());
    m_ui->checkBoxUseHalftone->setChecked(m_layer->useHalftone());
    m_ui->editSliderDPI->setValue(m_layer->dpi());
    m_ui->editSliderLPI->setValue(m_layer->lpi());
}

void LaserLayerDialog::initCuttingParameters()
{
    m_ui->editSliderMinSpeed->setValue(15);
    m_ui->editSliderRunSpeed->setValue(60);
    m_ui->editSliderLaserPower->setValue(80);
    m_ui->editSliderMinSpeedPower->setValue(700);
    m_ui->editSliderRunSpeedPower->setValue(1000);
    m_ui->groupBoxGeneral->setVisible(true);
    m_ui->groupBoxEngraving->setVisible(false);
    m_ui->groupBoxBitmap->setVisible(false);
    setFixedHeight(300);
}

void LaserLayerDialog::initEngravingParameters()
{
    m_ui->editSliderMinSpeed->setValue(60);
    m_ui->editSliderRunSpeed->setValue(300);
    m_ui->editSliderLaserPower->setValue(115);
    m_ui->editSliderLineSpacing->setValue(7);
    m_ui->editSliderColumnSpacing->setValue(0);
    m_ui->editSliderMinSpeedPower->setValue(0);
    m_ui->editSliderRunSpeedPower->setValue(900);
    m_ui->editSliderDPI->setValue(600);
    m_ui->editSliderLPI->setValue(600);
    m_ui->groupBoxGeneral->setVisible(true);
    m_ui->groupBoxEngraving->setVisible(true);
    m_ui->groupBoxBitmap->setVisible(true);
    setFixedHeight(600);
}

void LaserLayerDialog::initBothParameters()
{
    m_ui->editSliderMinSpeed->setValue(60);
    m_ui->editSliderRunSpeed->setValue(300);
    m_ui->editSliderLaserPower->setValue(115);
    m_ui->editSliderLineSpacing->setValue(7);
    m_ui->editSliderColumnSpacing->setValue(0);
    m_ui->editSliderMinSpeedPower->setValue(0);
    m_ui->editSliderRunSpeedPower->setValue(900);
    m_ui->groupBoxGeneral->setVisible(true);
    m_ui->groupBoxEngraving->setVisible(true);
    m_ui->groupBoxBitmap->setVisible(true);
    setFixedHeight(600);
}

void LaserLayerDialog::onCuttingToggled(bool checked)
{
    if (checked)
    {
        initCuttingParameters();
        m_type = LLT_CUTTING;
    }
}

void LaserLayerDialog::onEngravingToggled(bool checked)
{
    if (checked)
    {
        initEngravingParameters();
        m_type = LLT_ENGRAVING;
    }
}

void LaserLayerDialog::onBothToggled(bool checked)
{
    if (checked)
    {
        initEngravingParameters();
        m_type = LLT_BOTH;
    }
}

void LaserLayerDialog::accept()
{
    m_layer->setMinSpeed(m_ui->editSliderMinSpeed->value());
    m_layer->setRunSpeed(m_ui->editSliderRunSpeed->value());
    m_layer->setLaserPower(m_ui->editSliderLaserPower->value());
    m_layer->setEngravingForward(m_ui->checkBoxEngravingForward->isChecked());
    m_layer->setEngravingStyle(m_ui->radioButtonByRow->isChecked());
    m_layer->setLineSpacing(m_ui->editSliderLineSpacing->value());
    m_layer->setColumnSpacing(m_ui->editSliderColumnSpacing->value());
    m_layer->setMinSpeedPower(m_ui->editSliderMinSpeedPower->value());
    m_layer->setRunSpeedPower(m_ui->editSliderRunSpeedPower->value());
    m_layer->setLpi(m_ui->editSliderLPI->value());
    m_layer->setDpi(m_ui->editSliderDPI->value());
    m_layer->setUseHalftone(m_ui->checkBoxUseHalftone->isChecked());
    m_layer->setType(m_type);

    QDialog::accept();
}

