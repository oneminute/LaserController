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
    explicit ConnectionDialog(const QStringList& ports, QWidget* parent = nullptr);
    virtual ~ConnectionDialog();

    QString portName() const { return m_portName; }

protected slots:
    virtual void accept();

private:
    QScopedPointer<Ui::ConnectionDialog> m_ui;
    QString m_portName;
};

#endif // CONNECTIONDIALOG_H