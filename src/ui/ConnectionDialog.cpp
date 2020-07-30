#include "ConnectionDialog.h"
#include "ui_ConnectionDialog.h"

#include "laser/LaserDriver.h"

ConnectionDialog::ConnectionDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ConnectionDialog)
{
    m_ui->setupUi(this);

    QList<int> ports = LaserDriver::instance().getPortList();
    for (int i = 0; i < ports.size(); i++)
    {
        m_ui->comboBoxCOMs->addItem(QString("COM %1").arg(ports[i]), ports[i]);
    }

}

ConnectionDialog::~ConnectionDialog()
{

}

void ConnectionDialog::accept()
{
    int comPort = m_ui->comboBoxCOMs->currentData(Qt::UserRole).toInt();
    LaserDriver::instance().initComPort(comPort);
    QDialog::accept();
}
