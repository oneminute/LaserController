#include "DxfImporter.h"

#include "common/common.h"
#include <QFile>
#include <QList>
#include <QRegularExpression>
#include <QRegularExpressionMatch> 
#include <QStack>
#include <QTextStream>

class DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfNode)
public:
    DxfNodePrivate(DxfNode* ptr)
        : q_ptr(ptr)
        , sectionType(DxfNode::ST_Section)
    {

    }

    ~DxfNodePrivate()
    {
        qDeleteAll(children);
    }

    DxfNode* q_ptr;

    int groupCode;
    QString variable;
    QList<DxfNode*> children;
    DxfNode::SectionType sectionType;
    QVariantMap values;
};

DxfNode::DxfNode(int groupCode, const QString& variable)
    : m_ptr(new DxfNodePrivate(this))
{
    Q_D(DxfNode);
    d->groupCode = groupCode;
    d->variable = variable;

    if (variable == "SECTION")
    {
        d->sectionType = ST_Section;
    }
}

DxfNode::~DxfNode()
{
}

int DxfNode::groupCodes() const
{
    Q_D(const DxfNode);
    return d->groupCode;
}

QString DxfNode::variable() const
{
    Q_D(const DxfNode);
    return d->variable;
}

QList<DxfNode*>& DxfNode::children()
{
    Q_D(DxfNode);
    return d->children;
}

DxfNode* DxfNode::addChildNode(DxfNode* node)
{
    Q_D(DxfNode);
    d->children.append(node);
    return node;
}

DxfNode::SectionType DxfNode::sectionType() const
{
    Q_D(const DxfNode);
    return d->sectionType;
}

QVariantMap& DxfNode::values()
{
    Q_D(DxfNode);
    return d->values;
}

void DxfNode::insertValue(const QString& key, const QVariant& value)
{
    Q_D(DxfNode);
    d->values.insert(key, value);
}

bool DxfNode::contains(const QString& key) const
{
    Q_D(const DxfNode);
    return d->values.contains(key);
}

QString DxfNode::toString() const
{
    Q_D(const DxfNode);
    return QString("groupCode = %1, variable = %2").arg(d->groupCode).arg(d->variable);
}

QDebug operator<<(QDebug debug, const DxfNode& node)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << node.toString();
    return debug;
}

class DxfImporterPrivate
{
    Q_DECLARE_PUBLIC(DxfImporter)
public:
    DxfImporterPrivate(DxfImporter* ptr)
        : q_ptr(ptr)
        , lineNumber(0)
    {

    }

    ~DxfImporterPrivate()
    {
        qDeleteAll(nodes);
    }

    DxfImporter* q_ptr;

    QStack<DxfNode*> nodes;
    int lineNumber;
};

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

    d->lineNumber = 0;

    QTextStream stream(&file);
    DxfNode* lastNode = nullptr;
    DxfNode* currentNode = readLines(&stream);
    while (currentNode)
    {
        if (!lastNode)
        {
            
        }
        d->nodes.push(currentNode);
        qLogD << *currentNode;
        currentNode = readLines(&stream);
    }

    return nullptr;
}

DxfNode* DxfImporter::readLines(QTextStream* stream)
{
    Q_D(DxfImporter);
    QString line = stream->readLine().trimmed();
    d->lineNumber++;
    bool ok;
    int groupCode = line.toInt(&ok);
    if (!ok)
    {
        qLogW << "read dxf line(" << d->lineNumber << ")\"" << line << "\" error";
        return nullptr;
    }
    line = stream->readLine().trimmed();
    d->lineNumber++;
    /*if (line.isEmpty() || line.isNull())
    {
        qLogW << "read dxf line(" << d->lineNumber << ")\"" << line << "\" error";
        return nullptr;
    }*/
    DxfNode* node = new DxfNode(groupCode, line);
    return node;
}
