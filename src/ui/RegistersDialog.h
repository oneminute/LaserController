#ifndef REGISTERSDIALOG_H
#define REGISTERSDIALOG_H

#include "common/common.h"
#include "laser/LaserDriver.h"

#include <QDialog>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class RegistersDialog; }
QT_END_NAMESPACE

class QTableWidgetItem;

class RegistersDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegistersDialog(QWidget* parent = nullptr);
    virtual ~RegistersDialog();

protected slots:
    void registersFetched(const LaserDriver::RegistersMap& datas);
    void onActionReload(bool checked = false);
    void onActionSave(bool checked = false);
    void onRegistersItemChanged(QTableWidgetItem* item);

private:
    QScopedPointer<Ui::RegistersDialog> m_ui;
    LaserDriver::RegistersMap m_changedRegisters;
};

#endif // REGISTERSDIALOG_H