#include "LaserLayerDialog.h"
#include "HorizontalEditSlider.h"
#include "ui_LaserLayerDialog.h"

#include "scene/LaserLayer.h"

LaserLayerDialog::LaserLayerDialog(const QString& id, LayerType type, QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::LaserLayerDialog)
    , m_layer(nullptr)
{
    initUi(false);
}

LaserLayerDialog::LaserLayerDialog(LaserLayer* layer, QWidget* parent)
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

    m_ui->lineEditLayerName->setText(m_layer->id());
    if (m_layer->type() == LLT_ENGRAVING)
    {
        m_ui->groupBoxEngraving->setVisible(false);
    }
    else if (m_layer->type() == LLT_CUTTING)
    {
        m_ui->groupBoxCutting->setVisible(false);
    }

    if (editing)
    {
        m_ui->horizontalEditSliderMinSpeed->setValue(m_layer->minSpeed());
        // TODO: modify laser
        
    }

}

void LaserLayerDialog::accept()
{
    m_layer->setId(m_ui->lineEditLayerName->text());
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
    m_layer->setErrorY(m_ui->horizontalEditSliderErrorY->value());
    m_layer->setMoveSpeed(m_ui->horizontalEditSliderMoveSpeed->value());
    m_layer->setMinSpeedPower(m_ui->horizontalEditSliderMinSpeedPower->value());
    m_layer->setRunSpeedPower(m_ui->horizontalEditSliderRunSpeedPower->value());

    QDialog::accept();
}

