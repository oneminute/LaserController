#include "Importer.h"

#include <QMessageBox>

#include "LaserApplication.h"
#include "SvgImporter.h"
#include "DxfImporter.h"
#include "CorelDrawImporter.h"
#include "task/ProgressItem.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include "ui/LaserControllerWindow.h"

Importer::Importer(QObject* parent)
    : QObject(parent)
{}

void Importer::import(const QString & filename, LaserScene * scene, ProgressItem * parentProgress, const QVariantMap & params)
{
    QList<LaserPrimitive*> unavailables;
    importImpl(filename, scene, unavailables, parentProgress, params);
    if (!unavailables.isEmpty())
    {
        QMessageBox::warning(LaserApplication::mainWindow, tr("Warning"),
            tr("Found unavailable primitives, count is %1. These primitives will not be loaded.").arg(unavailables.count()));
    }
    scene->document()->open();
}

QSharedPointer<Importer> Importer::getImporter(QWidget* parentWnd, Types type)
{
    switch (type)
    {
    case Types::SVG:
        return QSharedPointer<Importer>(new SvgImporter);
    case Types::CORELDRAW:
        return QSharedPointer<Importer>(new CorelDrawImporter);
    default:
        break;
    }
    return nullptr;
}

QSharedPointer<Importer> Importer::getImporter(QWidget* parentWnd, const QString& type)
{
    QString lowerType = type.trimmed().toLower();
    if (lowerType == "svg")
    {
        return QSharedPointer<Importer>(new SvgImporter);
    }
    else if (lowerType == "dxf")
    {
        return QSharedPointer<Importer>(new DxfImporter);
    }
    return nullptr;
}
