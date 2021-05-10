#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QObject>
#include <QDialog>
#include <QScopedPointer>

namespace Ui { class ProgressDialog; }

class ProgressDialogPrivate;
class ProgressDialog : public QDialog
{
    Q_OBJECT
public:
    ProgressDialog(QWidget* parent = nullptr);
    ~ProgressDialog();

public slots:
    void addMessage(const QString& msg);
    void setTitle(const QString& msg);
    void setProgress(float progress);
    void finished();

private:
    QScopedPointer<Ui::ProgressDialog> m_ui;
    QScopedPointer<ProgressDialogPrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, ProgressDialog);
    Q_DISABLE_COPY(ProgressDialog);
};

#endif //PROGRESSDIALOG_H