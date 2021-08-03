#include "DxfImporter.h"

#include "common/common.h"
#include "DxfNode.h"
#include <QFile>
#include <QList>
#include <QRegularExpression>
#include <QRegularExpressionMatch> 
#include <QStack>
#include <QTextStream>


class DxfImporterPrivate
{
    Q_DECLARE_PUBLIC(DxfImporter)
public:
    DxfImporterPrivate(DxfImporter* ptr)
        : q_ptr(ptr)
    {

    }

    ~DxfImporterPrivate()
    {
    }

    DxfImporter* q_ptr;
    DxfNode* documentNode;
};

//QMap<QString, DxfNode::NodeType> DxfImporter::sectionClassMap
//    {
//        {"SECTION", DxfNode::NT_Section},
//        {"HEADER", DxfNode::NT_Section},
//        {"CLASSES", DxfNode::NT_Collection},
//        {"CLASS", DxfNode::NT_Section},
//        {"TABLES", DxfNode::NT_Collection},
//        {"TABLE", DxfNode::NT_Section}
//    }
//;

DxfImporter::DxfImporter(QObject* parent)
    : Importer(parent)
    , m_ptr(new DxfImporterPrivate(this))
{

}

DxfImporter::~DxfImporter()
{
}

LaserDocument* DxfImporter::import(const QString& filename, LaserScene* scene, const QVariantMap& params)
{
    Q_D(DxfImporter);
    qLogD << "import from dxf";

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return nullptr;

    DxfStream stream(&file);

    d->documentNode = new DxfDocumentNode;
    if (!d->documentNode->parse(&stream))
    {
    }
    else
    {
    }
    qLogD << *(d->documentNode);
    delete d->documentNode;
    d->documentNode = nullptr;

    return nullptr;
}
