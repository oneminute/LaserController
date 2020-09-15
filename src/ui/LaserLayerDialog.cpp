#include "LaserLayerDialog.h"
#include "HorizontalEditSlider.h"
#include "ui_LaserLayerDialog.h"

#include "scene/LaserLayer.h"
#include "scene/LaserDocument.h"
#include "scene/PageInformation.h"
#include "laser/LaserDriver.h"

LaserLayerDialog::LaserLayerDialog(LaserDocument* doc, LaserLayerType type, QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::LaserLayerDialog)
    , m_doc(doc)
    , m_layer(nullptr)
    , m_type(type)
{
    initUi(false);
}

LaserLayerDialog::LaserLayerDialog(LaserLayer* layer, QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::LaserLayerDialog)
    , m_doc(nullptr)
    , m_layer(layer)
{
    Q_ASSERT(layer);
    m_doc = layer->document();
    m_type = layer->type();
    initUi(true);
}

LaserLayerDialog::~LaserLayerDialog()
{
}

void LaserLayerDialog::initUi(bool editing)
{
    m_ui->setupUi(this);

    connect(m_ui->radioButtonCutting, &QRadioButton::toggled, this, &LaserLayerDialog::onCuttingToggled);
    connect(m_ui->radioButtonEngraving, &QRadioButton::toggled, this, &LaserLayerDialog::onEngravingToggled);

    /*QVariant value;
    if (LaserDriver::instance().getRegister(LaserDriver::RT_ENGRAVING_ROW_STEP, value))
    {
        m_ui->horizontalEditSliderLineSpacing->setValue(value.toInt());
    }
    if (LaserDriver::instance().getRegister(LaserDriver::RT_ENGRAVING_COLUMN_STEP, value))
    {
        m_ui->horizontalEditSliderColumnSpacing->setValue(value.toInt());
    }
    if (LaserDriver::instance().getRegister(LaserDriver::RT_CUSTOM_1_X, value))
    {
        m_ui->horizontalEditSliderStartX->setValue(value.toInt());
    }
    if (LaserDriver::instance().getRegister(LaserDriver::RT_CUSTOM_1_Y, value))
    {
        m_ui->horizontalEditSliderStartY->setValue(value.toInt());
    }
    if (LaserDriver::instance().getRegister(LaserDriver::RT_X_AXIS_BACKLASH, value))
    {
        m_ui->horizontalEditSliderErrorX->setValue(value.toInt());
    }*/
    /*if (LaserDriver::instance().getRegister(LaserDriver::RT_CUTTING_LAUNCHING_SPEED_RATIO, value))
    {
        m_ui->horizontalEditSliderMinSpeedPower->setValue(value.toInt());
    }
    if (LaserDriver::instance().getRegister(LaserDriver::RT_CUTTING_RUNNING_SPEED_RATIO, value))
    {
        m_ui->horizontalEditSliderRunSpeedPower->setValue(value.toInt());
    }*/

    m_ui->horizontalEditSliderStartX->setMaximum(m_doc->pageInformation().width());
    m_ui->horizontalEditSliderStartY->setMaximum(m_doc->pageInformation().height());
    if (m_type == LLT_ENGRAVING)
    {
        m_ui->horizontalEditSliderMinSpeed->setValue(60);
        m_ui->horizontalEditSliderRunSpeed->setValue(300);
        m_ui->horizontalEditSliderLaserPower->setValue(115);
        m_ui->horizontalEditSliderLineSpacing->setValue(7);
        m_ui->horizontalEditSliderColumnSpacing->setValue(0);
        m_ui->horizontalEditSliderStartX->setValue(25);
        m_ui->horizontalEditSliderStartY->setValue(0);
        m_ui->horizontalEditSliderErrorX->setValue(0);
        m_ui->horizontalEditSliderMinSpeedPower->setValue(0);
        m_ui->horizontalEditSliderRunSpeedPower->setValue(900);

        m_ui->radioButtonCutting->setChecked(false);
        m_ui->radioButtonEngraving->setChecked(true);
    }
    else if (m_type == LLT_CUTTING)
    {
        m_ui->horizontalEditSliderMinSpeed->setValue(15);
        m_ui->horizontalEditSliderRunSpeed->setValue(60);
        m_ui->horizontalEditSliderLaserPower->setValue(80);
        m_ui->horizontalEditSliderMinSpeedPower->setValue(700);
        m_ui->horizontalEditSliderRunSpeedPower->setValue(1000);

        m_ui->radioButtonCutting->setChecked(true);
        m_ui->radioButtonEngraving->setChecked(false);

        setFixedHeight(300);
    }

    if (editing)
    {
        m_type = m_layer->type();
        m_ui->lineEditLayerName->setText(m_layer->name());
        m_ui->horizontalEditSliderMinSpeed->setValue(m_layer->minSpeed());
        m_ui->horizontalEditSliderRunSpeed->setValue(m_layer->runSpeed());
        m_ui->horizontalEditSliderLaserPower->setValue(m_layer->laserPower());
        m_ui->checkBoxEngravingForward->setChecked(m_layer->engravingForward());
        m_ui->horizontalEditSliderLineSpacing->setValue(m_layer->lineSpacing());
        m_ui->horizontalEditSliderColumnSpacing->setValue(m_layer->columnSpacing());
        m_ui->horizontalEditSliderStartX->setValue(m_layer->startX());
        m_ui->horizontalEditSliderStartY->setValue(m_layer->startY());
        m_ui->horizontalEditSliderErrorX->setValue(m_layer->errorX());
        m_ui->horizontalEditSliderMinSpeedPower->setValue(m_layer->minSpeedPower());
        m_ui->horizontalEditSliderRunSpeedPower->setValue(m_layer->runSpeedPower());
    }
    
}

void LaserLayerDialog::onCuttingToggled(bool checked)
{
    if (checked)
    {
        m_ui->groupBoxGeneral->setVisible(true);
        m_ui->groupBoxEngraving->setVisible(false);
        m_ui->groupBoxBitmap->setVisible(false);

        setFixedHeight(300);
    }
}

void LaserLayerDialog::onEngravingToggled(bool checked)
{
    if (checked)
    {
        m_ui->groupBoxGeneral->setVisible(true);
        m_ui->groupBoxEngraving->setVisible(true);
        m_ui->groupBoxBitmap->setVisible(true);

        setFixedHeight(600);
    }
}

void LaserLayerDialog::accept()
{
    //if (!m_layer)
        //m_layer = new LaserLayer(m_ui->lineEditLayerName->text(), m_type, m_doc);
    //else
        //m_layer->setName(m_ui->lineEditLayerName->text());
    m_layer->setMinSpeed(m_ui->horizontalEditSliderMinSpeed->value());
    m_layer->setRunSpeed(m_ui->horizontalEditSliderRunSpeed->value());
    m_layer->setLaserPower(m_ui->horizontalEditSliderLaserPower->value());
    m_layer->setEngravingForward(m_ui->checkBoxEngravingForward->isChecked());
    m_layer->setEngravingStyle(m_ui->radioButtonByRow->isChecked());
    m_layer->setLineSpacing(m_ui->horizontalEditSliderLineSpacing->value());
    m_layer->setColumnSpacing(m_ui->horizontalEditSliderColumnSpacing->value());
    m_layer->setStartX(m_ui->horizontalEditSliderStartX->value());
    m_layer->setStartY(m_ui->horizontalEditSliderStartY->value());
    m_layer->setErrorX(m_ui->horizontalEditSliderErrorX->value());
    m_layer->setMinSpeedPower(m_ui->horizontalEditSliderMinSpeedPower->value());
    m_layer->setRunSpeedPower(m_ui->horizontalEditSliderRunSpeedPower->value());
    m_layer->setLpi(m_ui->horizontalEditSliderLPI->value());
    m_layer->setDpi(m_ui->horizontalEditSliderDPI->value());
    m_layer->setNonlinearCoefficient(m_ui->doubleSpinBoxNonlinearCoefficient->value());

    QDialog::accept();
}

