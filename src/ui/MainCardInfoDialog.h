#ifndef MAINCARDINFODIALOG_H
#define MAINCARDINFODIALOG_H

#include <QDialog>

namespace Ui
{
    class MainCardInfoDialog;
}

class MainCardInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MainCardInfoDialog(QWidget* parent = nullptr);
    virtual ~MainCardInfoDialog();

protected slots:
    void onGetMainCardInfo(QMap<QString, QString> info);

private:
    QScopedPointer<Ui::MainCardInfoDialog> m_ui;
    Q_DISABLE_COPY(MainCardInfoDialog)
};

#endif // MAINCARDINFODIALOG_H