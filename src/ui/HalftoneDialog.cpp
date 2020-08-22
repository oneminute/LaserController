#include "HalftoneDialog.h"
#include "ui_HalftoneDialog.h"

HalftoneDialog::HalftoneDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::HalftoneDialog)
{
    m_ui->setupUi(this);
}

HalftoneDialog::~HalftoneDialog()
{
}

void HalftoneDialog::accept()
{
    m_lpi = m_ui->horizontalEditSliderLpi->value();
    m_degrees = m_ui->horizontalEditSliderDegrees->value();
    m_dpi = m_ui->horizontalEditSliderDpi->value();
    QDialog::accept();
}
