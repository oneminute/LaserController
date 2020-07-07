#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QLibrary>
#include <QDebug>

#include "svg/svgview.h"
#include "laser/LaserDriver.h"
#include "state/StateController.h"
#include "AboutDialog.h"

static inline QString picturesLocation()
{
    return QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).value(0, QDir::currentPath());
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_svgView(new SvgView)
    , m_aboutDialog(new AboutDialog)
{
    ui->setupUi(this);
    //this->setWindowModality(Qt::WindowModality::WindowModal);

    setCentralWidget(m_svgView);

    connect(ui->actionFileOpen, &QAction::triggered, this, &MainWindow::onActionFileOpen);

    StateController::instance().fsm().start();

    //openLibrary("");
    LaserDriver::instance().load();
    LaserDriver::instance().init(this->winId());
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openFile(const QString& filename)
{
    return m_svgView->openFile(filename);
}

bool MainWindow::openLibrary(const QString& dllname)
{
    
    return true;
}

void MainWindow::onActionFileOpen(bool checked)
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters(QStringList() << "image/svg+xml" << "image/svg+xml-compressed");
    dialog.setWindowTitle(tr("Open SVG File"));
    if (m_currentPath.isEmpty())
        dialog.setDirectory(picturesLocation());

    while (dialog.exec() == QDialog::Accepted && !openFile(dialog.selectedFiles().constFirst()))
        ;
}
