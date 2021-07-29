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
    DxfNodePrivate(DxfNode* ptr, DxfNode::NodeType _nodeType)
        : q_ptr(ptr)
        , parent(nullptr)
        , sectionType(DxfNode::ST_Section)
        , nodeType(_nodeType)
    {

    }

    ~DxfNodePrivate()
    {
        qDeleteAll(children);
    }

    DxfNode* q_ptr;

    int groupCode;
    QString variable;
    DxfNode* parent;
    QList<DxfNode*> children;
    DxfNode::SectionType sectionType;
    DxfNode::NodeType nodeType;
};

DxfNode::DxfNode(int groupCode, const QString& variable, DxfNodePrivate* privateData)
    : m_ptr(privateData)
{
    Q_D(DxfNode);
    d->groupCode = groupCode;
    d->variable = variable;

    if (variable == "SECTION")
    {
        d->sectionType = ST_Section;
    }
    else if (variable == "HEADER")
    {
        d->sectionType = ST_Header;
    }
    else if (variable == "CLASSES")
    {
        d->sectionType = ST_Classes;
    }
}

DxfNode::~DxfNode()
{
}

int DxfNode::groupCode() const
{
    Q_D(const DxfNode);
    return d->groupCode;
}

QString DxfNode::variable() const
{
    Q_D(const DxfNode);
    return d->variable;
}

DxfNode* DxfNode::parent() const
{
    Q_D(const DxfNode);
    return d->parent;
}

void DxfNode::setParent(DxfNode* node)
{
    Q_D(DxfNode);
    d->parent = node;
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
    node->setParent(this);
    return node;
}

DxfNode::SectionType DxfNode::sectionType() const
{
    Q_D(const DxfNode);
    return d->sectionType;
}

DxfNode::NodeType DxfNode::nodeType() const
{
    Q_D(const DxfNode);
    return d->nodeType;
}

bool DxfNode::check(int groupCode, const QString& variable) const
{
    return this->groupCode() == groupCode && this->variable() == variable;
}

bool DxfNode::check(int groupCode) const
{
    return this->groupCode() == groupCode;
}

bool DxfNode::check(const QString& variable) const
{
    return this->variable() == variable;
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

class DxfSectionNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfSectionNode)
public:
    DxfSectionNodePrivate(DxfSectionNode* ptr, DxfNode::NodeType nodeType)
        : DxfNodePrivate(ptr, nodeType)
    {

    }

    QMap<QString, DxfNode*> propertyNodes;
};

DxfSectionNode::DxfSectionNode(int groupCode, const QString& variable)
    : DxfNode(groupCode, variable, new DxfSectionNodePrivate(this, DxfNode::NT_Section))
{

}

DxfSectionNode::~DxfSectionNode()
{

}

QMap<QString, DxfNode*>& DxfSectionNode::propertyNodes()
{
    Q_D(DxfSectionNode);
    return d->propertyNodes;
}

class DxfCollectionNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfCollectionNode)
public:
    DxfCollectionNodePrivate(DxfCollectionNode* ptr)
        : DxfNodePrivate(ptr, DxfNode::NT_Collection)
    {

    }

    QList<DxfNode*> items;
};

DxfCollectionNode::DxfCollectionNode(int groupCode, const QString& variable)
    : DxfNode(groupCode, variable, new DxfCollectionNodePrivate(this))
{

}

DxfCollectionNode::~DxfCollectionNode()
{

}

class DxfKeyValueNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfKeyValueNode)
public:
    DxfKeyValueNodePrivate(DxfKeyValueNode* ptr)
        : DxfNodePrivate(ptr, DxfNode::NT_KeyValue)
    {

    }

    DxfNode* value;
};

DxfKeyValueNode::DxfKeyValueNode(int groupCode, const QString& variable)
    : DxfNode(groupCode, variable, new DxfKeyValueNodePrivate(this))
{

}

DxfKeyValueNode::~DxfKeyValueNode()
{

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
        qDeleteAll(allNodes);
    }

    DxfImporter* q_ptr;

    //QStack<DxfNode*> nodes;
    QList<DxfNode*> allNodes;
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

    DxfNode* rootNode = nullptr;
    DxfNode* cursorNode = nullptr;
    DxfNode* currentNode = readLines(&stream);
    if (currentNode)
    {
        rootNode = currentNode;
    }
    while (currentNode)
    {
        qLogD << *currentNode;
        if (cursorNode)
        {
            if (cursorNode->check("SECTION"))
            {
                if (currentNode->check("HEADER"))
                {
                    cursorNode->addChildNode(currentNode);
                    cursorNode = currentNode;
                }
            }
            else if (cursorNode->check("HEADER"))
            {
                if (currentNode->check(1, "$ACADVER"))
                {
                    cursorNode->addChildNode(currentNode);
                    cursorNode = currentNode;
                }
            }
            else if (cursorNode->check("$ACADVER"))
            {
                if (currentNode->check(1))
                {

                }
            }
        }

        currentNode = readLines(&stream);
    }

    /*QStack<DxfNode*> stack;
    if (!d->allNodes.isEmpty())
    {
        stack.push(d->allNodes[0]);
    }*/
    //int index = 0;
    /*while (!stack.isEmpty())
    {
        currentNode = stack.pop();
        if (index < d->allNodes.length())
        {
            DxfNode* candidate = d->allNodes[index++];

            if (currentNode->isTopSection())
            {
                currentNode->addChildNode(candidate);
                stack.push(candidate);
            }
        }

    }*/
    

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
        //qLogW << "read dxf line(" << d->lineNumber << ")\"" << line << "\" error";
        return nullptr;
    }
    line = stream->readLine().trimmed();
    d->lineNumber++;
    /*if (line.isEmpty() || line.isNull())
    {
        qLogW << "read dxf line(" << d->lineNumber << ")\"" << line << "\" error";
        return nullptr;
    }*/
    DxfNode* node = nullptr;
    //node= new DxfNode(groupCode, line);
    //d->allNodes.append(node);
    return node;
}
