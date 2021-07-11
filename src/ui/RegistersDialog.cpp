#include "RegistersDialog.h"
#include "ui_RegistersDialog.h"

#include <QTableWidgetItem>

RegistersDialog::RegistersDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::RegistersDialog)
{
    m_ui->setupUi(this);

    m_ui->tableWidgetRegisters->setColumnWidth(0, 60);
    m_ui->tableWidgetRegisters->setColumnWidth(1, 120);
    m_ui->tableWidgetRegisters->setColumnWidth(2, 400);

    m_ui->toolButtonReload->setDefaultAction(m_ui->actionReload);
    m_ui->toolButtonSave->setDefaultAction(m_ui->actionSave);
    m_ui->toolButtonDiscard->setDefaultAction(m_ui->actionDiscard);
    m_ui->toolButtonBackup->setDefaultAction(m_ui->actionBackup);
    m_ui->toolButtonRestore->setDefaultAction(m_ui->actionRestore);

    connect(&LaserDriver::instance(), &LaserDriver::registersFectched, this, &RegistersDialog::registersFetched);
    connect(m_ui->tableWidgetRegisters, &QTableWidget::itemChanged, this, &RegistersDialog::onRegistersItemChanged);
    connect(m_ui->actionReload, &QAction::triggered, this, &RegistersDialog::onActionReload);
    connect(m_ui->actionSave, &QAction::triggered, this, &RegistersDialog::onActionSave);

    LaserDriver::instance().readAllSysParamFromCard();
}

RegistersDialog::~RegistersDialog()
{

}

void RegistersDialog::onActionReload(bool checked)
{
    LaserDriver::instance().readAllSysParamFromCard();
}

void RegistersDialog::onActionSave(bool checked)
{
    LaserDriver::instance().writeSysParamToCard(m_changedRegisters);
    m_changedRegisters.clear();
    LaserDriver::instance().readAllSysParamFromCard();
}

void RegistersDialog::onRegistersItemChanged(QTableWidgetItem * item)
{
    int addr = item->data(Qt::UserRole).toInt();
    QVariant value = item->data(Qt::UserRole + 1);

    QString newText = item->text();
    QString oldText = value.toString();

    if (newText != oldText)
    {
        item->setBackgroundColor(Qt::red);
        m_changedRegisters[addr] = newText;
    }
    else
    {
        item->setBackgroundColor(Qt::white);
        if (m_changedRegisters.contains(addr))
        {
            m_changedRegisters.remove(addr);
        }
    }
}

void RegistersDialog::registersFetched(const LaserRegister::RegistersMap& datas)
{
    m_ui->tableWidgetRegisters->blockSignals(true);
    m_ui->tableWidgetRegisters->setRowCount(datas.count());

    int i = 0;
    for (LaserRegister::RegistersMap::ConstIterator it = datas.constBegin(); it != datas.constEnd(); it++)
    {
        int addr = it.key();
        QVariant value = it.value();

        Qt::ItemFlags flags;
        QTableWidgetItem* addrItem = new QTableWidgetItem;
        addrItem->setText(QString::number(addr));
        m_ui->tableWidgetRegisters->setItem(i, 0, addrItem);
        flags = addrItem->flags();
        flags &= ~Qt::ItemIsEditable;
        addrItem->setFlags(flags);

        QTableWidgetItem* valueItem = new QTableWidgetItem;
        valueItem->setText(value.toString());
        valueItem->setData(Qt::UserRole, addr);
        valueItem->setData(Qt::UserRole + 1, value);
        m_ui->tableWidgetRegisters->setItem(i, 1, valueItem);

        QTableWidgetItem* commentItem = new QTableWidgetItem;
        //commentItem->setText(LaserDriver::instance().registerComment(addr));
        m_ui->tableWidgetRegisters->setItem(i, 2, commentItem);
        flags = commentItem->flags();
        flags &= ~Qt::ItemIsEditable;
        commentItem->setFlags(flags);

        i++;
    }
    m_ui->tableWidgetRegisters->blockSignals(false);
}