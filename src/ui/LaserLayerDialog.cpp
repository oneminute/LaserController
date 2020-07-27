#include "LaserLayerDialog.h"
#include "HorizontalEditSlider.h"
#include "ui_LaserLayerDialog.h"

#include "scene/LaserLayer.h"
#include "scene/LaserDocument.h"

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
    initUi(true);
}

LaserLayerDialog::~LaserLayerDialog()
{
}

void LaserLayerDialog::initUi(bool editing)
{
    m_ui->setupUi(this);

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
        m_ui->horizontalEditSliderErrorY->setValue(m_layer->errorY());
        m_ui->horizontalEditSliderMoveSpeed->setValue(m_layer->moveSpeed());
        m_ui->horizontalEditSliderMinSpeedPower->setValue(m_layer->minSpeedPower());
        m_ui->horizontalEditSliderRunSpeedPower->setValue(m_layer->runSpeedPower());
    }
    else
    {
        m_ui->lineEditLayerName->setText(m_doc->newLayerName(m_type));
    }

    if (m_type == LLT_ENGRAVING)
    {
        m_ui->groupBoxEngraving->setVisible(false);
    }
    else if (m_type == LLT_CUTTING)
    {
        m_ui->groupBoxCutting->setVisible(false);
    }
}

void LaserLayerDialog::accept()
{
    if (!m_layer)
        m_layer = new LaserLayer(m_ui->lineEditLayerName->text(), m_type, m_doc);
    else
        m_layer->setName(m_ui->lineEditLayerName->text());
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

