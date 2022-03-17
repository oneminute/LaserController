#include "SelectOriginDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>

#include "LaserApplication.h"
#include "common/Config.h"
#include "laser/LaserDevice.h"
#include "widget/RadioButtonGroup.h"

SelectOriginDialog::SelectOriginDialog(QWidget* parent)
    : QDialog(parent)
{
    resize(300, 100);

    QVBoxLayout* mainLayout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    m_buttonGroupOrigin = new RadioButtonGroup(2, 2);
    m_buttonGroupOrigin->setValues(QList<int>() << 0 << 3 << 1 << 2);
    m_buttonGroupOrigin->setValue(Config::SystemRegister::deviceOrigin());
    formLayout->addRow(tr("Origin"), m_buttonGroupOrigin);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 0);
    
    this->setLayout(mainLayout);
}

SelectOriginDialog::~SelectOriginDialog()
{
}

int SelectOriginDialog::origin() const
{
    return m_buttonGroupOrigin->value();
}
