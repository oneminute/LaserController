#include "ConnectionDialog.h"
#include "ui_ConnectionDialog.h"

#include "laser/LaserDriver.h"
#include "util/Utils.h"

ConnectionDialog::ConnectionDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ConnectionDialog)
{
    m_ui->setupUi(this);

    QStringList ports = LaserDriver::instance().getPortList();
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
    QString portName = m_ui->comboBoxCOMs->currentText();
    LaserDriver::instance().initComPort(portName);
    QDialog::accept();
}
