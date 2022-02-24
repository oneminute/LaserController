#include "ConnectionDialog.h"
#include "ui_ConnectionDialog.h"

#include <QPushButton>

#include "laser/LaserDriver.h"
#include "util/Utils.h"

ConnectionDialog::ConnectionDialog(const QStringList& ports, QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ConnectionDialog)
{
    m_ui->setupUi(this);

    Q_ASSERT(!ports.empty());

    for (int i = 0; i < ports.size(); i++)
    {
        m_ui->comboBoxCOMs->addItem(ports[i], utils::parsePortName(ports[i]));
    }
}

ConnectionDialog::~ConnectionDialog()
{

}

void ConnectionDialog::accept()
{
    m_portName = m_ui->comboBoxCOMs->currentText();
    QDialog::accept();
}
