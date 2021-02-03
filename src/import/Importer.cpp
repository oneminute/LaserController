#include "Importer.h"
#include "SvgImporter.h"
#include "CorelDrawImporter.h"

Importer::Importer(QObject* parent)
    : QObject(parent)
{}

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
