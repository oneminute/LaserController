#include "HalftoneDialog.h"
#include "ui_HalftoneDialog.h"

HalftoneDialog::HalftoneDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::HalftoneDialog)
{
    m_ui->setupUi(this);

    m_ui->editSliderLpi->setMinimum(1);
    m_ui->editSliderLpi->setMaximum(100);
    m_ui->editSliderLpi->setValue(60);

    m_ui->editSliderDegrees->setMinimum(0);
    m_ui->editSliderDegrees->setMaximum(45);
    m_ui->editSliderDegrees->setValue(45);

    m_ui->editSliderDpi->setMinimum(0);
    m_ui->editSliderDpi->setMaximum(1200);
    m_ui->editSliderDpi->setValue(600);

    m_ui->editSliderPixelInterval->setMinimum(0);
    m_ui->editSliderPixelInterval->setMaximum(10);
    m_ui->editSliderPixelInterval->setValue(7);
}

HalftoneDialog::~HalftoneDialog()
{
}

void HalftoneDialog::accept()
{
    m_lpi = m_ui->editSliderLpi->value();
    m_degrees = m_ui->editSliderDegrees->value();
    m_dpi = m_ui->editSliderDpi->value();
    m_pixelInterval = m_ui->editSliderPixelInterval->value();
    m_yPulseLength = m_ui->labelYPulseLength->text().toFloat();
    QDialog::accept();
}
