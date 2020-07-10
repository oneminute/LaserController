#include "ImportSVGDialog.h"
#include "ui_ImportSVGDialog.h"

ImportSVGDialog::ImportSVGDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ImportSVGDialog)
{
    m_ui->setupUi(this);
}

ImportSVGDialog::~ImportSVGDialog()
{

}