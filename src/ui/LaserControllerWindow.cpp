#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"

#include <QFileDialog>

#include "widget/LaserViewer.h"
#include "scene/LaserScene.h"
#include "import/Importer.h"

LaserControllerWindow::LaserControllerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::LaserControllerWindow)
    , m_scene(new LaserScene)
{
    m_ui->setupUi(this);

    m_viewer = new LaserViewer(m_scene.data());
    this->setCentralWidget(m_viewer);

    connect(m_ui->actionImportSVG, &QAction::triggered, this, &LaserControllerWindow::onActionImportSVG);
}

LaserControllerWindow::~LaserControllerWindow()
{

}

QString LaserControllerWindow::getFilename(const QString& title, const QStringList & mime)
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters(QStringList() << "image/svg+xml" << "image/svg+xml-compressed");
    dialog.setWindowTitle(tr("Open SVG File"));
    if (dialog.exec() == QDialog::Accepted)
        return dialog.selectedFiles().constFirst();
    else
        return "";
}

void LaserControllerWindow::onActionImportSVG(bool checked)
{
    QString filename = getFilename(tr("Open SVG File"), QStringList() << "image/svg+xml" << "image/svg+xml-compressed");
    if (filename.isEmpty())
        return;
    QSharedPointer<Importer> importer = Importer::getImporter(Importer::SVG);
    LaserDocument* doc = importer->import(filename);
    m_scene->updateDocument(doc);
}