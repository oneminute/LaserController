#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class ConnectionDialog; }
QT_END_NAMESPACE

class ConnectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConnectionDialog(QWidget* parent = nullptr);
    virtual ~ConnectionDialog();

protected slots:
    virtual void accept();

private:
    QScopedPointer<Ui::ConnectionDialog> m_ui;
};

#endif // CONNECTIONDIALOG_H