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
    {

    }

    DxfNode* q_ptr;

    int groupCode;
    QString variable;
    QList<DxfNode*> children;
    QVariantMap values;
};

DxfNode::DxfNode(int groupCode, const QString& variable)
    : m_ptr(new DxfNodePrivate(this))
{
    Q_D(DxfNode);
    d->groupCode = groupCode;
    d->variable = variable;
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
    {

    }

    ~DxfImporterPrivate()
    {
        qDeleteAll(nodes);
    }

    DxfImporter* q_ptr;

    QStack<DxfNode*> nodes;
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
    qLogD << "import from dxf";

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return nullptr;

    //bool foundGroup = false;
    QRegularExpression reGroupCode("^\\s{2}(\\d+)");
    QRegularExpression reContent("^(\\s*)(.*)");
    int currentGroupCode = std::numeric_limits<int>::min();

    QTextStream stream(&file);
    QString line = file.readLine();
    while (!line.isEmpty() && !line.isNull())
    {
        //qLogD << "line: " << line;
        // 先判断当前是不是已经找到一个组代码了
        QRegularExpressionMatch matchGroupCode = reGroupCode.match(line);
        if (matchGroupCode.hasMatch())
        {
            // 当前行为组代码
            QString groupCodeString = matchGroupCode.captured(1);
            bool ok;
            currentGroupCode = groupCodeString.toInt(&ok);
            if (ok)
            {
                qLogD << "group code: " << groupCodeString;
            }
            else
            {
                currentGroupCode = std::numeric_limits<int>::min();
            }
        }
        else
        {
            // 当前行为内容
            QRegularExpressionMatch matchContent = reContent.match(line);
            if (matchContent.hasMatch())
            {
                int capturedLength = matchContent.lastCapturedIndex();
                QString spaces;
                QString content;
                if (capturedLength == 1)
                {
                    content = matchContent.captured(1);
                }
                else if (capturedLength == 2)
                {
                    spaces = matchContent.captured(1);
                    content = matchContent.captured(2);
                }
                qLogD << "content: spaces length = " << spaces.size() << ", content = " << content;

                
            }
        }

        line = file.readLine();
    }

    return nullptr;
}

DxfNode* DxfImporter::readLines(QTextStream* stream)
{
    Q_D(DxfImporter);
    QString line = stream->readLine().trimmed();
    bool ok;
    int groupCode = line.toInt(&ok);
    if (!ok)
    {
        qLogW << "read dxf line\"" << line << "\" error";
        return nullptr;
    }
    line = stream->readLine().trimmed();
    if (line.isEmpty() || line.isNull())
    {
        qLogW << "read dxf line\"" << line << "\" error";
        return nullptr;
    }
    DxfNode* node = new DxfNode(groupCode, line);
    return node;
}
