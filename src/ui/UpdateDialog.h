#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>

namespace Ui
{
    class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UpdateDialog(QWidget* parent = nullptr);
    ~UpdateDialog();

protected:
    virtual void closeEvent(QCloseEvent* ev) override;

private:
    QScopedPointer<Ui::UpdateDialog> m_ui;

    Q_DISABLE_COPY(UpdateDialog)
};

#endif // UPDATEDIALOG_H