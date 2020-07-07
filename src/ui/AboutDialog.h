#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class AboutDialog; }
QT_END_NAMESPACE

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();

private:
    QScopedPointer<Ui::AboutDialog> m_ui;
};


#endif // ABOUNTDIALOG_H