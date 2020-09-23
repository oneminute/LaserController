#include "Importer.h"
#include "SvgImporter.h"
#include "CorelDrawImporter.h"

Importer::Importer(QWidget* parentWnd, QObject* parent)
    : QObject(parent)
    , m_parentWnd(parentWnd)
{}

QSharedPointer<Importer> Importer::getImporter(QWidget* parentWnd, Types type)
{
    switch (type)
    {
    case Types::SVG:
        return QSharedPointer<Importer>(new SvgImporter(parentWnd));
    case Types::CORELDRAW:
        return QSharedPointer<Importer>(new CorelDrawImporter(parentWnd));
    default:
        break;
    }
    return nullptr;
}
