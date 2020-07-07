#include "AboutDialog.h"
#include "./ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::AboutDialog)
{
    m_ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
}
