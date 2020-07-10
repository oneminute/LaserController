#include "Importer.h"
#include "SvgImporter.h"

Importer::Importer(QObject* parent)
    : QObject(parent)
{}

QSharedPointer<Importer> Importer::getImporter(Types type)
{
    switch (type)
    {
    case Types::SVG:
        return QSharedPointer<Importer>(new SvgImporter);
        break;
    default:
        break;
    }
    return nullptr;
}
