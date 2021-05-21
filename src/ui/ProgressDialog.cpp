#include "ProgressDialog.h"
#include "ui_ProgressDialog.h"
#include "common/common.h"

class ProgressDialogPrivate
{
    Q_DECLARE_PUBLIC(ProgressDialog)
public:
    ProgressDialogPrivate(ProgressDialog* ptr)
        : q_ptr(ptr)
    {

    }

    ProgressDialog* q_ptr;
};

ProgressDialog::ProgressDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ProgressDialog)
    , m_ptr(new ProgressDialogPrivate(this))
{
    qLogD << "ProgressDialog";
    m_ui->setupUi(this);
    setProgress(0);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::addMessage(const QString& msg)
{
    Q_D(ProgressDialog);
    qLogD << msg;
    m_ui->plainTextEditDetails->appendPlainText(msg);
}

void ProgressDialog::setTitle(const QString& msg)
{
    Q_D(ProgressDialog);
    m_ui->labelInfo->setText(msg);
    m_ui->plainTextEditDetails->appendPlainText(msg);
}

void ProgressDialog::setProgress(float progress)
{
    Q_D(ProgressDialog);
    m_ui->progressBar->setValue(progress);
}

void ProgressDialog::finished()
{
    m_ui->buttonBox->setEnabled(true);
    close();
}
