#include "UpdateDialog.h"
#include <ui_UpdateDialog.h>

#include <common/common.h>
#include <common/Config.h>
#include <LaserApplication.h>
#include <laser/LaserDevice.h>
#include <QCloseEvent>

#include <QWindow>

UpdateDialog::UpdateDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::UpdateDialog)
{
    m_ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    int winId = LaserApplication::device->showUpdateDialog();
    QWindow* externalWindow = QWindow::fromWinId(winId);
    QWidget* externalWidget = createWindowContainer(externalWindow);
    m_ui->verticalLayout->insertWidget(0, externalWidget);
}

UpdateDialog::~UpdateDialog()
{
}

void UpdateDialog::closeEvent(QCloseEvent* ev)
{
    ev->ignore();
    this->hide();
}
