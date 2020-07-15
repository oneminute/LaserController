#include "LaserLayerDialog.h"
#include "HorizontalEditSlider.h"
#include "ui_LaserLayerDialog.h"

LaserLayerDialog::LaserLayerDialog(const QString& id, LaserLayer::LayerType type, QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::LaserLayerDialog)
    , m_layer(id, type)
{
    initUi(false);
}

LaserLayerDialog::LaserLayerDialog(const LaserLayer& layer, QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::LaserLayerDialog)
    , m_layer(layer)
{
    initUi(true);
}

LaserLayerDialog::~LaserLayerDialog()
{
}

void LaserLayerDialog::initUi(bool editing)
{
    m_ui->setupUi(this);

    m_ui->lineEditLayerName->setText(m_layer.id());
    if (m_layer.type() == LaserLayer::LLT_ENGRAVING)
    {
        m_ui->groupBoxEngraving->setVisible(false);
    }
    else if (m_layer.type() == LaserLayer::LLT_CUTTING)
    {
        m_ui->groupBoxCutting->setVisible(false);
    }

    if (editing)
    {
        m_ui->horizontalEditSliderMinSpeed->setValue(m_layer.minSpeed());
        
        
    }

    /*connect(m_ui->lineEditLayerName, &QLineEdit::textChanged, this, &LaserLayerDialog::onLayerNameChanged);
    connect(m_ui->horizontalEditSliderMinSpeed, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onMinSpeedChanged);
    connect(m_ui->horizontalEditSliderRunSpeed, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onRunSpeedChanged);
    connect(m_ui->horizontalEditSliderLaserPower, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onLaserPowerChanged);
    connect(m_ui->checkBoxEngravingForward, &QCheckBox::stateChanged, this, &LaserLayerDialog::onEngravingForwardChanged);
    connect(m_ui->radioButtonByRow, &QRadioButton::toggled, this, &LaserLayerDialog::onEngravingStyleChanged);
    connect(m_ui->horizontalEditSliderLineSpacing, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onLineSpacingChanged);
    connect(m_ui->horizontalEditSliderColumnSpacing, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onColumnSpacingChanged);
    connect(m_ui->horizontalEditSliderStartX, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onStartXChanged);
    connect(m_ui->horizontalEditSliderStartY, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onStartYChanged);
    connect(m_ui->horizontalEditSliderErrorX, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onErrorXChanged);
    connect(m_ui->horizontalEditSliderMoveSpeed, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onMoveSpeedChanged);
    connect(m_ui->horizontalEditSliderMinSpeedPower, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onMinSpeedPowerChanged);
    connect(m_ui->horizontalEditSliderRunSpeedPower, &HorizontalEditSlider::valueChanged, this, &LaserLayerDialog::onRunSpeedPowerChanged);*/
}

void LaserLayerDialog::accept()
{
    m_layer.setId(m_ui->lineEditLayerName->text());
    m_layer.setMinSpeed(m_ui->horizontalEditSliderMinSpeed->value());
    m_layer.setRunSpeed(m_ui->horizontalEditSliderRunSpeed->value());
    m_layer.setLaserPower(m_ui->horizontalEditSliderLaserPower->value());
    m_layer.setEngravingForward(m_ui->checkBoxEngravingForward->isChecked());
    m_layer.setEngravingStyle(m_ui->radioButtonByRow->isChecked());
    m_layer.setLineSpacing(m_ui->horizontalEditSliderLineSpacing->value());
    m_layer.setColumnSpacing(m_ui->horizontalEditSliderColumnSpacing->value());
    m_layer.setStartX(m_ui->horizontalEditSliderStartX->value());
    m_layer.setStartY(m_ui->horizontalEditSliderStartY->value());
    m_layer.setErrorX(m_ui->horizontalEditSliderErrorX->value());
    m_layer.setErrorY(m_ui->horizontalEditSliderErrorY->value());
    m_layer.setMoveSpeed(m_ui->horizontalEditSliderMoveSpeed->value());
    m_layer.setMinSpeedPower(m_ui->horizontalEditSliderMinSpeedPower->value());
    m_layer.setRunSpeedPower(m_ui->horizontalEditSliderRunSpeedPower->value());

    QDialog::accept();
}

void LaserLayerDialog::onLayerNameChanged(const QString& name)
{
    m_layer.setId(name);
}

void LaserLayerDialog::onMinSpeedChanged(int value)
{
    m_layer.setMinSpeed(value);
}

void LaserLayerDialog::onRunSpeedChanged(int value)
{
    m_layer.setRunSpeed(value);
}

void LaserLayerDialog::onLaserPowerChanged(int value)
{
    m_layer.setLaserPower(value);
}

void LaserLayerDialog::onEngravingForwardChanged(int state)
{
    m_layer.setEngravingForward(state == Qt::CheckState::Checked);
}

void LaserLayerDialog::onEngravingStyleChanged(bool checked)
{
    m_layer.setEngravingStyle(m_ui->radioButtonByRow->isChecked() ? 0 : 1);
}

void LaserLayerDialog::onLineSpacingChanged(int value)
{
    m_layer.setLineSpacing(value);
}

void LaserLayerDialog::onColumnSpacingChanged(int value)
{
    m_layer.setColumnSpacing(value);
}

void LaserLayerDialog::onStartXChanged(int value)
{
    m_layer.setStartX(value);
}

void LaserLayerDialog::onStartYChanged(int value)
{
    m_layer.setStartY(value);
}

void LaserLayerDialog::onErrorXChanged(int value)
{
    m_layer.setErrorX(value);
}

void LaserLayerDialog::onMoveSpeedChanged(int value)
{
    m_layer.setMoveSpeed(value);
}

void LaserLayerDialog::onMinSpeedPowerChanged(int value)
{
    m_layer.setMinSpeed(value);
}

void LaserLayerDialog::onRunSpeedPowerChanged(int value)
{
    m_layer.setRunSpeedPower(value);
}
