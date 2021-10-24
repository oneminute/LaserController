#include "UserInfoDialog.h"

#include "common/common.h"
#include "LaserApplication.h"
#include "laser/LaserDefines.h"
#include "laser/LaserDevice.h"

#include "ui_UserInfoDialog.h"

UserInfoDialog::UserInfoDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::UserInfoDialog)
{
    m_ui->setupUi(this);
}

UserInfoDialog::~UserInfoDialog()
{
}
