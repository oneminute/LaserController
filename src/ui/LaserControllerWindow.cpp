#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"

#include <QFileDialog>

#include "widget/LaserViewer.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include "import/Importer.h"
#include "ui/LaserLayerDialog.h"

LaserControllerWindow::LaserControllerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::LaserControllerWindow)
    , m_scene(new LaserScene)
{
    m_ui->setupUi(this);

    m_viewer = new LaserViewer(m_scene.data());
    this->setCentralWidget(m_viewer);

    connect(m_ui->actionImportSVG, &QAction::triggered, this, &LaserControllerWindow::onActionImportSVG);
    connect(m_ui->toolButtonAddEngravingLayer, &QToolButton::clicked, this, &LaserControllerWindow::onToolButtonAddEngravingLayer);
    connect(m_ui->toolButtonAddCuttingLayer, &QToolButton::clicked, this, &LaserControllerWindow::onToolButtonAddCuttingLayer);
}

LaserControllerWindow::~LaserControllerWindow()
{

}

void LaserControllerWindow::onToolButtonAddEngravingLayer(bool)
{
    QString newName = m_scene->document()->newLayerName(LaserLayer::LLT_ENGRAVING);
    LaserLayerDialog dialog(newName, LaserLayer::LLT_ENGRAVING);
    dialog.exec();

    LaserLayer layer = dialog.layer();
    m_scene->document()->addLayer(layer);
}

void LaserControllerWindow::onToolButtonAddCuttingLayer(bool checked)
{
    QString newName = m_scene->document()->newLayerName(LaserLayer::LLT_CUTTING);
    LaserLayerDialog dialog(newName, LaserLayer::LLT_CUTTING);
    dialog.exec();

    LaserLayer layer = dialog.layer();
    m_scene->document()->addLayer(layer);
}

void LaserControllerWindow::updateLayers()
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