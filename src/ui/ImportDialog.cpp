#include "ImportDialog.h"

#include <QComboBox>

#include "ui_ImportDialog.h"

ImportDialog::ImportDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ImportDialog)
{
    m_ui->setupUi(this);

    connect(m_ui->comboBoxPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportDialog::currentIndexChanged);
}

ImportDialog::~ImportDialog()
{

}

qreal ImportDialog::factor() const
{
    return 1.0 / m_ui->doubleSpinBoxFactor->value();
}

bool ImportDialog::disableViewbox() const
{
    return m_ui->checkBoxDisableViewbox->isChecked();
}

void ImportDialog::currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        m_ui->doubleSpinBoxFactor->setValue(1.0);
        m_ui->doubleSpinBoxFactor->setEnabled(false);
        m_ui->checkBoxDisableViewbox->setChecked(false);
        break;
    case 1:
        m_ui->doubleSpinBoxFactor->setValue(2.834645666);
        m_ui->doubleSpinBoxFactor->setEnabled(false);
        m_ui->checkBoxDisableViewbox->setChecked(true);
        break;
    case 2:
        m_ui->doubleSpinBoxFactor->setValue(1.0);
        m_ui->doubleSpinBoxFactor->setEnabled(true);
        break;
    }
}