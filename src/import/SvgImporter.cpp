#include "SvgImporter.h"

#include <QDebug>

#include "svg/qsvgtinydocument.h"
#include "ui/ImportSVGDialog.h"

SvgImporter::SvgImporter(QObject* parent)
    : Importer(parent)
{

}

SvgImporter::~SvgImporter()
{
}

LaserDocument SvgImporter::import(const QString & filename)
{
    LaserDocument ldoc;
    QSvgTinyDocument* doc = QSvgTinyDocument::load(filename);
    QSize svgSize = doc->size();
    qDebug() << svgSize;
    
    return ldoc;
}
