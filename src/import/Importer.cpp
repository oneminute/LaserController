#include "Importer.h"
#include "SvgImporter.h"
#include "DxfImporter.h"
#include "CorelDrawImporter.h"
#include "task/ProgressItem.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"

Importer::Importer(QObject* parent)
    : QObject(parent)
{}

void Importer::import(const QString & filename, LaserScene * scene, ProgressItem * parentProgress, const QVariantMap & params)
{
    importImpl(filename, scene, parentProgress, params);
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
