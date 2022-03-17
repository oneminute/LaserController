#include "DxfNode.h"

#include "LaserApplication.h"
#include "common/common.h"
#include "scene/LaserDocument.h"
#include "scene/LaserPrimitive.h"
#include "task/ProgressItem.h"
#include "task/ProgressModel.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

class DxfStreamPrivate
{
    Q_DECLARE_PUBLIC(DxfStream)
public:
    DxfStreamPrivate(DxfStream* ptr)
    {}

    DxfStream* q_ptr;

};

DxfStream::DxfStream(QIODevice* device, ProgressItem* parentProgress)
    : QTextStream(device)
    , m_lineNumber(0)
    , m_cached(false)
    , m_progress(new ProgressItem(QObject::tr("Parse Dxf file"), ProgressItem::PT_Simple, parentProgress))
{
    m_progress->setMaximum(device->bytesAvailable());
}

bool DxfStream::readGroup(DxfGroup& pair)
{
    if (isCached())
    {
        m_cached = false;
    }
    else
    {
        m_line = readLine().trimmed();
        m_progress->setProgress(pos());
        m_lineNumber++;
        bool ok;
        m_cachedGroupCode = m_line.toInt(&ok);
        m_lineNumber++;
        if (!ok)
            return false;
        m_line = readLine().trimmed();
        if (m_line == "EOF")
            qLogD << "READ EOF";
        m_cachedVariable = m_line;
    }
    pair.groupCode = m_cachedGroupCode;
    pair.variable = m_cachedVariable;

    m_progress->finish();
    return true;
}

bool DxfStream::isCached() const
{
    return m_cached;
}

void DxfStream::setCached()
{
    m_cached = true;
}

int DxfStream::lineNumber() const
{
    return m_lineNumber;
}

DxfNode::DxfNode(DxfDocumentNode* doc, int groupCode, const QString& variable, DxfNodePrivate* privateData)
    : m_ptr(privateData)
{
    Q_D(DxfNode);
    d->document = doc;
    d->groupCode = groupCode;
    d->variable = variable;
}

void DxfNode::setName(const QString& name)
{
    Q_D(DxfNode);
    d->name = name;
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

QString DxfNode::name() const
{
    Q_D(const DxfNode);
    return d->name;
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

DxfDocumentNode* DxfNode::document() const
{
    Q_D(const DxfNode);
    return d->document;
}

DxfNodeType DxfNode::nodeType() const
{
    Q_D(const DxfNode);
    return d->nodeType;
}

void DxfNode::debugPrint() const
{
    Q_D(const DxfNode);
    qLogD << QString("groupCode = %1, variable = %2\n").arg(d->groupCode).arg(d->variable);
}

class DxfDocumentNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfDocumentNode)
public:
    DxfDocumentNodePrivate(DxfDocumentNode* ptr)
        : DxfNodePrivate(ptr, NT_Document)
        , header(nullptr)
        , classes(nullptr)
        , tables(nullptr)
        , blocks(nullptr)
        , entities(nullptr)
        , objects(nullptr)
        , thumbnailImage(nullptr)
    {

    }

    DxfHeaderNode* header;
    DxfClassesNode* classes;
    DxfTablesNode* tables;
    DxfBlocksNode* blocks;
    DxfEntitiesNode* entities;
    DxfObjectsNode* objects;
    DxfThumbnailImageNode* thumbnailImage;

    QMap<quint32, DxfItemNode*> itemsMap;
};

DxfDocumentNode::DxfDocumentNode()
    : DxfNode(this, 0, "DOCUMENT", new DxfDocumentNodePrivate(this))
{

}

const DxfEntitiesNode& DxfDocumentNode::entities() const
{
    Q_D(const DxfDocumentNode);
    return *(d->entities);
}

void DxfDocumentNode::addItem(DxfItemNode* node)
{
    Q_D(DxfDocumentNode);
    d->itemsMap[node->handle()] = node;
}

DxfItemNode* DxfDocumentNode::item(int handle) const
{
    Q_D(const DxfDocumentNode);
    if (d->itemsMap.contains(handle))
    {
        return d->itemsMap[handle];
    }
    else
    {
        return nullptr;
    }
}

bool DxfDocumentNode::parse(DxfStream* stream)
{
    Q_D(DxfDocumentNode);
    while (true)
    {
        DxfGroup group;

        if (!stream->readGroup(group))
        {
            qLogW << "Parse dxf file error at line " << stream->lineNumber();
            return false;
        }

        DxfNode* childNode = nullptr;
        if (group.match("SECTION"))
        {
            if (!stream->readGroup(group))
            {
                qLogW << "Parse dxf file error.";
                return false;
            }
            qLogD << stream->lineNumber() << ": " << group.groupCode << ", " << group.variable;
            if (group.match("HEADER"))
            {
                childNode = d->header = new DxfHeaderNode(this, group.groupCode);
            }
            else if (group.match("CLASSES"))
            {
                childNode = d->classes = new DxfClassesNode(this, group.groupCode);
            }
            else if (group.match("TABLES"))
            {
                childNode = d->tables = new DxfTablesNode(this, group.groupCode);
            }
            else if (group.match("BLOCKS"))
            {
                childNode = d->blocks = new DxfBlocksNode(this, group.groupCode);
            }
            else if (group.match("ENTITIES"))
            {
                childNode = d->entities = new DxfEntitiesNode(this, group.groupCode);
            }
            else if (group.match("OBJECTS"))
            {
                childNode = d->objects = new DxfObjectsNode(this, group.groupCode);
            }
            else if (group.match("THUMBNAILIMAGE"))
            {
                childNode = d->thumbnailImage = new DxfThumbnailImageNode(this, group.groupCode);
            }
        }
        else if (group.match("EOF"))
        {
            break;
        }

        if (childNode)
        {
            addChildNode(childNode);
            if (!childNode->parse(stream))
                qLogW << "parse error.";
        }
    }

    // post progress
    QStack<DxfNode*> stack;
    stack.push(this);
    while (!stack.isEmpty())
    {
        DxfNode* node = stack.pop();

        node->postProcess();

        for (DxfNode* child : node->children())
        {
            stack.push(child);
        }
    }

    return true;
}

void DxfDocumentNode::debugPrint() const
{
    Q_D(const DxfDocumentNode);
    qLogD << "\n";
    DxfNode::debugPrint();
    if (d->header)
        d->header->debugPrint();
    if (d->classes)
        d->classes->debugPrint();
    if (d->tables)
        d->tables->debugPrint();
    if (d->blocks)
        d->blocks->debugPrint();
    if (d->entities)
        d->entities->debugPrint();
    if (d->objects)
        d->objects->debugPrint();
}

class DxfHeaderNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfHeaderNode)
public:
    DxfHeaderNodePrivate(DxfHeaderNode* ptr)
        : DxfNodePrivate(ptr, NT_Header)
    {}

    DxfNode::PropertyMap properties;
};

DxfHeaderNode::DxfHeaderNode(DxfDocumentNode* doc, int groupCode)
    : DxfNode(doc, groupCode, "HEADER", new DxfHeaderNodePrivate(this))
{
}

DxfNode::PropertyMap& DxfHeaderNode::properties()
{
    Q_D(DxfHeaderNode);
    return d->properties;
}

void DxfHeaderNode::setProperty(const QString& name, const QString& value)
{
    Q_D(DxfHeaderNode);
    if (d->properties.contains(name))
    {
        d->properties[name] = value;
    }
    else
    {
        d->properties.insert(name, value);
    }
}

bool DxfHeaderNode::parse(DxfStream* stream)
{
    Q_D(DxfHeaderNode);
    while (true)
    {
        DxfGroup group;
        if (!stream->readGroup(group))
        {
            return false;
        }

        if (group.match(9))
        {
            QString propertyName = group.variable;

            if (!stream->readGroup(group))
            {
                return false;
            }
            
            setProperty(propertyName, group.variable);
        }
        else if (group.match(0, "ENDSEC"))
        {
            break;
        }
    }
    return true;
}

void DxfHeaderNode::debugPrint() const
{
    Q_D(const DxfHeaderNode);
    qLogD << "HEADER" << "\n";
    for (PropertyMap::ConstIterator i = d->properties.constBegin(); i != d->properties.constEnd(); i++)
    {
        qLogD << "  " << i.key() << " = " << i.value() << "\n";
    }
}

class DxfClassNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfClassNode)
public:
    DxfClassNodePrivate(DxfClassNode* ptr)
        : DxfNodePrivate(ptr, NT_Class)
        , proxyCapabilitiesFlag(0)
        , instanceCount(0)
        , wasAProxy(false)
        , isAnEntity(false)
    {}

    QString dxfRecordName;
    QString applicationName;
    int proxyCapabilitiesFlag;
    int instanceCount;
    bool wasAProxy;
    bool isAnEntity;
};

DxfClassNode::DxfClassNode(DxfDocumentNode* doc, int groupCode)
    : DxfNode(doc, groupCode, "CLASS", new DxfClassNodePrivate(this))
{

}

bool DxfClassNode::parse(DxfStream* stream)
{
    Q_D(DxfClassNode);
    while (true)
    {
        DxfGroup group;
        if (!stream->readGroup(group))
        {
            return false;
        }

        if (group.match(1))
        {
            d->dxfRecordName = group.variable;
        }
        else if (group.match(2))
        {
            setName(group.variable);
        }
        else if (group.match(3))
        {
            d->applicationName = group.variable;
        }
        else if (group.match(90))
        {
            bool ok;
            d->proxyCapabilitiesFlag = group.variable.toInt(&ok);
            if (!ok)
                return false;
        }
        else if (group.match(91))
        {
            bool ok;
            d->instanceCount = group.variable.toInt(&ok);
            if (!ok)
                return false;
        }
        else if (group.match(280))
        {
            d->wasAProxy = "1" == group.variable.trimmed();
        }
        else if (group.match(281))
        {
            d->isAnEntity = "1" == group.variable.trimmed();
        }
        else if (group.match(0))
        {
            stream->setCached();
            break;
        }
    }
    return true;
}

void DxfClassNode::debugPrint() const
{
    Q_D(const DxfClassNode);
    qLogD << "CLASS " << name() << "\n";
    DxfNode::debugPrint();
    qLogD << "    dxfRecordName: " << d->dxfRecordName << "\n";
    qLogD << "    applicationName: " << d->applicationName << "\n";
    qLogD << "    proxyCapabilitiesFlag: " << d->proxyCapabilitiesFlag << "\n";
    qLogD << "    instanceCount: " << d->instanceCount << "\n";
    qLogD << "    wasAProxy: " << d->wasAProxy << "\n";
    qLogD << "    isAnEntity: " << d->isAnEntity << "\n";
}

class DxfItemNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfItemNode)
public:
    DxfItemNodePrivate(DxfItemNode* ptr, DxfNodeType nodeType)
        : DxfNodePrivate(ptr, nodeType)
        , handle(-1)
    {}

    int handle;
    QStringList appDefinedGroups;
    int softPointerId;
    QStringList subClassMarkers;
};

DxfItemNode::DxfItemNode(DxfDocumentNode* doc, int groupCode, const QString& variable, DxfItemNodePrivate* ptr)
    : DxfNode(doc, groupCode, variable, ptr)
{

}

int DxfItemNode::handle() const
{
    Q_D(const DxfItemNode);
    return d->handle;
}

bool DxfItemNode::parse(DxfStream* stream)
{
    Q_D(DxfItemNode);
    QString currentAppDefinedGroup;
    bool group102Begin = false;
    while (true)
    {
        DxfGroup group;
        if (!stream->readGroup(group))
        {
            return false;
        }

        if (group102Begin && !group.match(102))
        {
            currentAppDefinedGroup.append(group.variable);
        }
        else if (isEnd(group))
        {
            if (group.needCache)
                stream->setCached();
            break;
        }
        else if (group.match(5))
        {
            if (!group.assign(d->handle, 16))
                return false;
            d->document->addItem(this);
        }
        else if (group.match(102))
        {
            if (group102Begin)
            {
                currentAppDefinedGroup.append(group.variable);
                d->appDefinedGroups.append(currentAppDefinedGroup);
                currentAppDefinedGroup = QString();
                group102Begin = false;
            }
            else
            {
                currentAppDefinedGroup.append(group.variable);
                group102Begin = true;
            }
        }
        else if (group.match(330))
        {
            if (!group.assign(d->softPointerId, 16))
                return false;
        }
        else if (group.match(100))
        {
            d->subClassMarkers.append(group.variable);
        }
        else if (group.match(0, "ENDSEC"))
        {
            break;
        }
        else if (group.match(0, "SECTION"))
        {
            stream->setCached();
            break;
        }
        else
        {
            if (!parseItem(group))
                return false;
        }
        
    }
    return true;
}

void DxfItemNode::debugPrint() const
{
    Q_D(const DxfItemNode);
    DxfNode::debugPrint();
    qLogD << "    handle = " << d->handle << "\n";
    qLogD << "    appDefinedGroups = \n";
    for (QString def : d->appDefinedGroups)
    { 
        qLogD << "      " << def << "\n";
    }
    qLogD << "    softPointerId = " << d->softPointerId << "\n";
    qLogD << "    subClassMarker = \n";
    for (QString marker : d->subClassMarkers)
    {
        qLogD << "      " << marker << "\n";
    }
}

class DxfTableNodePrivate : public DxfItemNodePrivate
{
    Q_DECLARE_PUBLIC(DxfTableNode)
public:
    DxfTableNodePrivate(DxfTableNode* ptr)
        : DxfItemNodePrivate(ptr, NT_Table)
        , maximumEntries(0)
    {}
    
    QString entityName;
    int maximumEntries;
};

DxfTableNode::DxfTableNode(DxfDocumentNode* doc, int groupCode)
    : DxfItemNode(doc, groupCode, "TABLE", new DxfTableNodePrivate(this))
{

}

void DxfTableNode::debugPrint() const
{
    Q_D(const DxfTableNode);
    qLogD << "TABLE " << name() << "\n";
    DxfItemNode::debugPrint();
    qLogD << "    entityName: " << d->entityName << "\n";
    qLogD << "    maximumEntries: " << d->maximumEntries << "\n";
}

bool DxfTableNode::parseItem(DxfGroup& group)
{
    Q_D(DxfTableNode);
    if (group.match(-1))
    {
        d->entityName = group.variable;
    }
    else if (group.match(2) && name().isEmpty())
    {
        setName(group.variable);
    }
    else if (group.match(70))
    {
        bool ok;
        d->maximumEntries = group.variable.toInt(&ok);
        if (!ok)
            return false;
    }
    
    return true;
}

bool DxfTableNode::isEnd(DxfGroup& group)
{
    if (group.match(0, "ENDTAB"))
    {
        return true;
    }
    return false;
}

class DxfBlockNodePrivate : public DxfItemNodePrivate
{
    Q_DECLARE_PUBLIC(DxfBlockNode)
public:
    DxfBlockNodePrivate(DxfBlockNode* ptr)
        : DxfItemNodePrivate(ptr, NT_Block)
        , blockTypeFlags(0)
    {}

    QString layerName;
    QString subClassMarkerBlockBegin;

    /// <summary>
    ///	Block - type flags(bit - coded values, may be combined) :
    /// 0 = Indicates none of the following flags apply
    /// 1 = This is an anonymous block generated by hatching, associative dimensioning, other internal operations, or an application
    /// 2 = This block has non - constant attribute definitions(this bit is not set if the block has any attribute definitions that are constant, or has no attribute definitions at all)
    /// 4 = This block is an external reference(xref)
    /// 8 = This block is an xref overlay
    /// 16 = This block is externally dependent
    /// 32 = This is a resolved external reference, or dependent of an external reference(ignored on input)
    /// 64 = This definition is a referenced external reference(ignored on input)
    /// </summary>
    int blockTypeFlags;
    QVector3D basePoint;
    QString blockName3;
    QString xrefPathName;
    QString blockDescription;
};

DxfBlockNode::DxfBlockNode(DxfDocumentNode* doc, int groupCode)
    : DxfItemNode(doc, groupCode, "BLOCK", new DxfBlockNodePrivate(this))
{

}

void DxfBlockNode::debugPrint() const
{
    Q_D(const DxfBlockNode);
    qLogD << "BLOCK " << name() << "\n";
    DxfItemNode::debugPrint();
    qLogD << "    layerName = " << d->layerName << "\n";
    qLogD << "    subClassMarkerBlockBegin = " << d->subClassMarkerBlockBegin << "\n";
    qLogD << "    blockTypeFlags = " << d->blockTypeFlags << "\n";
    qLogD << "    basePoint = " << d->basePoint.x() << ", " << d->basePoint.y() << ", " << d->basePoint.z() << "\n";
    qLogD << "    blockName3 = " << d->blockName3 << "\n";
    qLogD << "    xrefPathName = " << d->xrefPathName << "\n";
    qLogD << "    blockDescription = " << d->blockDescription << "\n";
    d->children[0]->debugPrint();
}

bool DxfBlockNode::parseItem(DxfGroup& group)
{
    Q_D(DxfBlockNode);
    if (group.match(8))
    {
        d->layerName = group.variable;
    }
    else if (group.match(2))
    {
        setName(group.variable);
    }
    else if (group.match(100))
    {
        d->subClassMarkerBlockBegin = group.variable;
    }
    else if (group.match(70))
    {
        bool ok;
        d->blockTypeFlags = group.variable.toInt(&ok);
        if (!ok)
            return false;
    }
    else if (group.match(10))
    {
        qreal x;
        if (!group.assign(x))
            return false;
        d->basePoint.setX(x);
    }
    else if (group.match(20))
    {
        qreal y;
        if (!group.assign(y))
            return false;
        d->basePoint.setY(y);
    }
    else if (group.match(30))
    {
        qreal z;
        if (!group.assign(z))
            return false;
        d->basePoint.setZ(z);
    }
    else if (group.match(3))
    {
        d->blockName3 = group.variable;
    }
    else if (group.match(1))
    {
        d->xrefPathName = group.variable;
    }
    else if (group.match(4))
    {
        d->blockDescription = group.variable;
    }
    return true;
}

bool DxfBlockNode::isEnd(DxfGroup& group)
{
    if (group.match(0, "ENDBLK"))
    {
        group.needCache = true;
        return true;
    }
    return false;
}

class DxfEndBlockNodePrivate : public DxfItemNodePrivate
{
    Q_DECLARE_PUBLIC(DxfEndBlockNode)
public:
    DxfEndBlockNodePrivate(DxfEndBlockNode* ptr)
        : DxfItemNodePrivate(ptr, NT_EndBlock)
    {}

    QString layerName;
};

DxfEndBlockNode::DxfEndBlockNode(DxfDocumentNode* doc, int groupCode)
    : DxfItemNode(doc, groupCode, "ENDBLK", new DxfEndBlockNodePrivate(this))
{
}

void DxfEndBlockNode::debugPrint() const
{
    Q_D(const DxfEndBlockNode);
    qLogD << "ENDBLK " << name() << "\n";
    DxfItemNode::debugPrint();
    qLogD << "    layerName = " << d->layerName << "\n";
}

bool DxfEndBlockNode::parseItem(DxfGroup& group)
{
    Q_D(DxfEndBlockNode);
    if (group.match(8))
    {
        d->layerName = group.variable;
    }
    return true;
}

bool DxfEndBlockNode::isEnd(DxfGroup& group)
{
    if (group.match(0))
    {
        group.needCache = true;
        return true;
    }
    return false;
}

class DxfEntityNodePrivate : public DxfItemNodePrivate
{
    Q_DECLARE_PUBLIC(DxfEntityNode)
public:
    DxfEntityNodePrivate(DxfEntityNode* ptr, DxfNodeType nodeType)
        : DxfItemNodePrivate(ptr, nodeType)
        , spaceType(0)
        , materialObjectId(0)
        , colorNumber(0)
        , lineweight(0)
        , linetypeScale(0)
        , visibility(0)
        , proxyGraphicsDataLength(0)
        , colorValue(0)
        , transparency(0)
        , plotStyleObjectId(0)
        , shadowMode(0)
        , extrusionDirection(0, 0, 1)
    {}

    /// <summary>
    /// APP: entity name (changes each time a drawing is opened)
    /// </summary>
    QString entityName;

    QString entityType;

    /// <summary>
    /// Absent or zero indicates entity is in model space. 1 indicates entity 
    /// is in paper space (optional).
    /// </summary>
    int spaceType;
    QString layoutTabName;
    QString layerName;

    /// <summary>
    /// Linetype name (present if not BYLAYER). The special 
    /// name BYBLOCK indicates a floating linetype
    /// </summary>
    QString lineTypeName;

    /// <summary>
    /// Hard-pointer ID/handle to material object (present if not BYLAYER)
    /// Note: Handle number is in a hex value
    /// </summary>
    int materialObjectId;

    /// <summary>
    /// Color number (present if not BYLAYER); zero indicates the BYBLOCK (floating) 
    /// color; 256 indicates BYLAYER; a negative value indicates that the layer is 
    /// turned off (optional)
    /// </summary>
    int colorNumber;

    /// <summary>
    /// Lineweight enum value. Stored and moved around as a 16-bit integer.
    /// </summary>
    int lineweight;
    qreal linetypeScale;

    /// <summary>
    /// The original value from dxf is as following:
    /// Object visibility (optional):
    /// 0 = Visible
    /// 1 = Invisible
    /// We use boolean value instead.
    /// </summary>
    bool visibility;

    /// <summary>
    /// Number of bytes in the proxy entity graphics represented in the subsequent 
    /// 310 groups, which are binary chunk records
    /// </summary>
    int proxyGraphicsDataLength;

    /// <summary>
    /// Proxy entity graphics data (multiple lines; 256 characters max. per line) 
    /// </summary>
    QByteArray proxyGraphicsData;

    /// <summary>
    /// A 24 - bit color value that should be dealt with in terms of bytes with values 
    /// of 0 to 255. The lowest byte is the blue value, the middle byte is the green value, 
    /// and the third byte is the red value.The top byte is always 0. The group code cannot 
    /// be used by custom entities for their own data because the group code is reserved 
    /// for AcDbEntity, class - level color data and AcDbEntity, class - level transparency 
    /// data
    /// </summary>
    int colorValue;
    QColor color;

    /// <summary>
    /// Color name. The group code cannot be used by custom entities for their own data 
    /// because the group code is reserved for AcDbEntity, class-level color data and 
    /// AcDbEntity, class-level transparency data
    /// </summary>
    QString colorName;

    /// <summary>
    /// Transparency value. The group code cannot be used by custom entities for their own
    ///  data because the group code is reserved for AcDbEntity, class-level color data 
    /// and AcDbEntity, class-level transparency data
    /// </summary>
    int transparency;

    /// <summary>
    /// Hard-pointer ID/handle to the plot style object
    /// </summary>
    int plotStyleObjectId;

    /// <summary>
    /// Shadow mode
    /// 0 = Casts and receives shadows
    /// 1 = Casts shadows
    /// 2 = Receives shadows
    /// 3 = Ignores shadows
    /// Note : Starting with AutoCAD 2016 - based products, this property is obsolete but
    /// still supported for backwards compatibility.
    /// </summary>
    int shadowMode;

    /// <summary>
    ///	Extrusion direction(optional; default = 0, 0, 1)
    /// DXF: X value; APP: 3D vector
    /// DXF : Y and Z values of extrusion direction(optional)
    /// </summary>
    QVector3D extrusionDirection;
};

DxfEntityNode::DxfEntityNode(DxfDocumentNode* doc, int groupCode, const QString& variable, DxfEntityNodePrivate* ptr)
    : DxfItemNode(doc, groupCode, variable, ptr)
{
    setName(variable);
}

DxfEntityNode::DxfEntityNode(DxfDocumentNode* doc, int groupCode, const QString& variable)
    : DxfItemNode(doc, groupCode, variable, new DxfEntityNodePrivate(this, NT_Entity))
{
    setName(variable);
}

void DxfEntityNode::debugPrint() const
{
    Q_D(const DxfEntityNode);
    qLogD << "ENTITY " << name() << "\n";
    DxfItemNode::debugPrint();
    qLogD << "    entityName = " << d->entityName << "\n";
    qLogD << "    entityType = " << d->entityType << "\n";
    qLogD << "    spaceType = " << d->spaceType << "\n";
    qLogD << "    layerName = " << d->layerName << "\n";
    qLogD << "    lineTypeName = " << d->lineTypeName << "\n";
    qLogD << "    materialObjectId = " << d->materialObjectId << "\n";
    qLogD << "    colorNumber = " << d->colorNumber << "\n";
    qLogD << "    lineweight = " << d->lineweight << "\n";
    qLogD << "    linetypeScale = " << d->linetypeScale << "\n";
    qLogD << "    visibility = " << d->visibility << "\n";
    qLogD << "    proxyGraphicsDataLength = " << d->proxyGraphicsDataLength << "\n";
    qLogD << "    proxyGraphicsData = " << d->proxyGraphicsData << "\n";
    qLogD << "    colorValue = " << d->colorValue << "\n";
    qLogD << "    transparency = " << d->transparency << "\n";
    qLogD << "    plotStyleObjectId = " << d->plotStyleObjectId << "\n";
    qLogD << "    shadowMode = " << d->shadowMode << "\n";
    qLogD << "    extrusionDirection = " << d->extrusionDirection.x() << ", " << d->extrusionDirection.y() << ", " << d->extrusionDirection.z() << " \n";
}

bool DxfEntityNode::parseItem(DxfGroup& group)
{
    Q_D(DxfEntityNode);
    if (group.match(-1))
    {
        group.assign(d->entityName);
    }
    else if (group.match(6))
    {
        group.assign(d->lineTypeName);
    }
    else if (group.match(8))
    {
        group.assign(d->layerName);
    }
    else if (group.match(48))
    {
        if (!group.assign(d->linetypeScale))
            return false;
    }
    else if (group.match(60))
    {
        int value;
        if (!group.assign(value))
            return false;
        d->visibility = value == 0;
    }
    else if (group.match(62))
    {
        if (!group.assign(d->colorNumber))
            return false;
    }
    else if (group.match(67))
    {
        if (!group.assign(d->spaceType))
            return false;
    }
    else if (group.match(92))
    {
        if (!group.assign(d->proxyGraphicsDataLength))
            return false;
    }
    else if (group.match(210))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.setX(value);
    }
    else if (group.match(220))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.setY(value);
    }
    else if (group.match(230))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.setZ(value);
    }
    else if (group.match(284))
    {
        if (!group.assign(d->shadowMode))
            return false;
    }
    else if (group.match(310))
    {
        d->proxyGraphicsData.append(QByteArray::fromHex(group.variable.toUtf8()));
    }
    else if (group.match(347))
    {
        if (!group.assign(d->materialObjectId, 16))
            return false;
    }
    else if (group.match(370))
    {
        if (!group.assign(d->lineweight))
            return false;
    }
    else if (group.match(390))
    {
        if (!group.assign(d->plotStyleObjectId, 16))
            return false;
    }
    else if (group.match(410))
    {
        group.assign(d->layoutTabName);
    }
    else if (group.match(420))
    {
        if (!group.assign(d->colorValue))
            return false;
        d->color = QColor::fromRgba64(qRgba64(d->colorValue));
    }
    else if (group.match(430))
    {
        group.assign(d->colorName);
    }
    else if (group.match(440))
    {
        if (!group.assign(d->transparency))
            return false;
    }
    return true;
}

bool DxfEntityNode::isEnd(DxfGroup& group)
{
    if (group.match(0, "ENDSEC"))
    {
        return true;
    }
    else if (group.match(0))
    {
        group.needCache = true;
        return true;
    }
    return false;
}

class DxfLWPolylineNodePrivate : public DxfEntityNodePrivate
{
    Q_DECLARE_PUBLIC(DxfLWPolylineNode)
public:
    DxfLWPolylineNodePrivate(DxfLWPolylineNode* ptr)
        : DxfEntityNodePrivate(ptr, NT_LWPolyline)
        , verticesCount(0)
        , polylineFlag(0)
        , constantWidth(0)
        , elevation(0)
        , thickness(0)
        , vertexIdentifier(0)
    {}

    int verticesCount;

    /// <summary>
    /// Polyline flag (bit-coded); default is 0:
    /// 1 = Closed; 128 = Plinegen
    /// </summary>
    int polylineFlag;

    /// <summary>
    /// Constant m_width (optional; default = 0). Not used if variable m_width 
    /// (codes 40 and/or 41) is set 
    /// </summary>
    qreal constantWidth;

    /// <summary>
    /// Elevation (optional; default = 0)
    /// </summary>
    qreal elevation;

    /// <summary>
    /// Thickness(optional; default = 0)
    /// </summary>
    qreal thickness;

    /// <summary>
    /// Vertex coordinates (in OCS), multiple entries; one entry for each vertex
    /// DXF: X value; APP: 2D point
    /// DXF: Y value of vertex coordinates(in OCS), multiple entries; one entry 
    /// for each vertex
    /// </summary>
    QList<QPointF> points;

    int vertexIdentifier;

    /// <summary>
    /// Starting m_width(multiple entries; one entry for each vertex) (optional; 
    /// default = 0; multiple entries).Not used if constant m_width(code 43) is set
    /// End m_width (multiple entries; one entry for each vertex) (optional; 
    /// default = 0; multiple entries). Not used if constant m_width (code 43) is 
    /// set
    /// </summary>
    QList<QVector2D> verticesWidth;

    /// <summary>
    /// Bulge (multiple entries; one entry for each vertex) (optional; default = 0)
    /// </summary>
    QList<qreal> bulges;
};


DxfLWPolylineNode::DxfLWPolylineNode(DxfDocumentNode* doc, int groupCode)
    : DxfEntityNode(doc, groupCode, "LWPOLYLINE", new DxfLWPolylineNodePrivate(this))
{
}

void DxfLWPolylineNode::debugPrint() const
{
    Q_D(const DxfLWPolylineNode);
    DxfEntityNode::debugPrint();
    qLogD << "    verticesCount = " << d->verticesCount << "\n";
    qLogD << "    polylineFlag = " << d->polylineFlag << "\n";
    qLogD << "    constantWidth = " << d->constantWidth << "\n";
    qLogD << "    elevation = " << d->elevation << "\n";
    qLogD << "    thickness = " << d->thickness << "\n";
    qLogD << "    points = \n";
    for (QPointF point : d->points)
    {
        qLogD << "      " << point.x() << ", " << point.y() << "\n";
    }
    qLogD << "    vertexIdentifier = " << d->vertexIdentifier << "\n";
    qLogD << "    verticesWidth = \n";
    for (QVector2D point : d->verticesWidth)
    {
        qLogD << "      " << point.x() << ", " << point.y() << "\n";
    }
    qLogD << "    bulges = \n";
    for (qreal bulge : d->bulges)
    {
        qLogD << "      " << bulge << "\n";
    }
}

LaserPrimitive* DxfLWPolylineNode::convertTo(LaserDocument* doc, const QTransform& t) const
{
    Q_D(const DxfLWPolylineNode);
    QPolygonF polygon;
    LaserPrimitive* primitive;
    for (QPointF point : d->points)
    {
        polygon.append(point);
    }
    if (d->polylineFlag == 0)
    {
        primitive = new LaserPolyline(t.map(polygon).toPolygon(), doc);
    }
    else if (d->polylineFlag == 1)
    {
        primitive = new LaserPolygon(t.map(polygon).toPolygon(), doc);
    }
    return primitive;
}

bool DxfLWPolylineNode::parseItem(DxfGroup& group)
{
    Q_D(DxfLWPolylineNode);
    if (!DxfEntityNode::parseItem(group))
        return false;
    if (group.matched)
        return true;

    if (group.match(10))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->points.append(QPointF(value, 0));
    }
    else if (group.match(20))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->points.last().setY(value);
    }
    else if (group.match(38))
    {
        if (!group.assign(d->elevation))
            return false;
    }
    else if (group.match(39))
    {
        if (!group.assign(d->thickness))
            return false;
    }
    else if (group.match(40))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->verticesWidth.append(QVector2D(value, 0));
    }
    else if (group.match(41))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->verticesWidth.last().setY(value);
    }
    else if (group.match(42))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->bulges.append(value);
    }
    else if (group.match(43))
    {
        if (!group.assign(d->constantWidth))
            return false;
    }
    else if (group.match(70))
    {
        if (!group.assign(d->polylineFlag))
            return false;
    }
    else if (group.match(90))
    {
        if (!group.assign(d->verticesCount))
            return false;
    }
    else if (group.match(91))
    {
        if (!group.assign(d->vertexIdentifier, 16))
            return false;
    }
    

    return true;
}

class DxfCircleNodePrivate : public DxfEntityNodePrivate
{
    Q_DECLARE_PUBLIC(DxfCircleNode)
public:
    DxfCircleNodePrivate(DxfCircleNode* ptr)
        : DxfEntityNodePrivate(ptr, NT_Circle)
    {}

    qreal thickness;

    QVector3D center;
    qreal radius;
};

DxfCircleNode::DxfCircleNode(DxfDocumentNode* doc, int groupCode)
    : DxfEntityNode(doc, groupCode, "CIRCLE", new DxfCircleNodePrivate(this))
{

}

void DxfCircleNode::debugPrint() const
{
    Q_D(const DxfCircleNode);
    DxfEntityNode::debugPrint();
    qLogD << "    thickness = " << d->thickness << "\n";
    qLogD << "    center = " << d->center.x() << ", " << d->center.y() << ", " << d->center.z() << "\n";
    qLogD << "    radius = " << d->radius << "\n";
}

LaserPrimitive* DxfCircleNode::convertTo(LaserDocument* doc, const QTransform& t) const
{
    Q_D(const DxfCircleNode);
    QPointF center = d->center.toPointF();
    QRectF rect(center.x() - d->radius, center.y() - d->radius, d->radius * 2, d->radius * 2);
    LaserEllipse* primitive = new LaserEllipse(t.mapRect(rect).toRect(), doc);
    return primitive;
}

bool DxfCircleNode::parseItem(DxfGroup& group)
{
    Q_D(DxfCircleNode);
    if (group.match(10))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->center.setX(value);
    }
    else if (group.match(20))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->center.setY(value);
    }
    else if (group.match(30))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->center.setZ(value);
    }
    else if (group.match(39))
    {
        if (!group.assign(d->thickness))
            return false;
    }
    else if (group.match(40))
    {
        if (!group.assign(d->radius))
            return false;
    }
    
    return true;
}

class DxfEllipseNodePrivate : public DxfEntityNodePrivate
{
    Q_DECLARE_PUBLIC(DxfEllipseNode)
public:
    DxfEllipseNodePrivate(DxfEllipseNode* ptr)
        : DxfEntityNodePrivate(ptr, NT_Ellipse)
    {}

    qreal thickness;

    QVector3D center;

    /// <summary>
    /// 该向量是从中心点指向长轴一个端点的向量，该向量长度即为长轴的半长
    /// </summary>
    QVector3D endPoint;

    /// <summary>
    /// Ratio of minor axis to major axis
    /// </summary>
    qreal ratio;

    /// <summary>
    /// Start parameter (this value is 0.0 for a full ellipse)
    /// </summary>
    qreal startParameter;

    /// <summary>
    /// End parameter (this value is 2pi for a full ellipse)
    /// </summary>
    qreal endParameter;
};

DxfEllipseNode::DxfEllipseNode(DxfDocumentNode* doc, int groupCode)
    : DxfEntityNode(doc, groupCode, "ELLIPSE", new DxfEllipseNodePrivate(this))
{

}

void DxfEllipseNode::debugPrint() const
{
    Q_D(const DxfEllipseNode);
    DxfEntityNode::debugPrint();
    qLogD << "    thickness = " << d->thickness << "\n";
    qLogD << "    center = " << d->center.x() << ", " << d->center.y() << ", " << d->center.z() << "\n";
    qLogD << "    endPoint = " << d->endPoint.x() << ", " << d->endPoint.y() << ", " << d->endPoint.z() << "\n";
    qLogD << "    ratio = " << d->ratio << "\n";
    qLogD << "    startParameter = " << d->startParameter << "\n";
    qLogD << "    endParameter = " << d->endParameter << "\n";
}

LaserPrimitive* DxfEllipseNode::convertTo(LaserDocument* doc, const QTransform& t) const
{
    Q_D(const DxfEllipseNode);
    QVector2D halfMajor = d->endPoint.toVector2D();
    qreal halfMajorLength = halfMajor.length();
    qreal halfMinorLength = halfMajorLength * d->ratio;

    qreal angle = qAtan2(halfMajor.y(), halfMajor.x());
    QTransform savedT;
    savedT.rotateRadians(angle);
    savedT = QTransform(savedT.m11(), savedT.m12(), savedT.m21(), savedT.m22(), d->center.x(), d->center.y());
    savedT = savedT * t;

    QRect rect(-halfMajorLength, -halfMinorLength, halfMajorLength * 2, halfMinorLength * 2);

    LaserEllipse* primitive = new LaserEllipse(rect, doc, savedT);
    return primitive;
}

bool DxfEllipseNode::parseItem(DxfGroup& group)
{
    Q_D(DxfEllipseNode);
    if (group.match(10))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->center.setX(value);
    }
    else if (group.match(20))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->center.setY(value);
    }
    else if (group.match(30))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->center.setZ(value);
    }
    if (group.match(11))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->endPoint.setX(value);
    }
    else if (group.match(21))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->endPoint.setY(value);
    }
    else if (group.match(31))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->endPoint.setZ(value);
    }
    else if (group.match(39))
    {
        if (!group.assign(d->thickness))
            return false;
    }
    else if (group.match(40))
    {
        if (!group.assign(d->ratio))
            return false;
    }
    else if (group.match(41))
    {
        if (!group.assign(d->startParameter))
            return false;
    }
    else if (group.match(42))
    {
        if (!group.assign(d->endParameter))
            return false;
    }
    
    return true;
}

class DxfLineNodePrivate : public DxfEntityNodePrivate
{
    Q_DECLARE_PUBLIC(DxfLineNode)
public:
    DxfLineNodePrivate(DxfLineNode* ptr)
        : DxfEntityNodePrivate(ptr, NT_Line)
        , thickness(0)
    {}

    qreal thickness;
    QVector3D startPoint;
    QVector3D endPoint;
};

DxfLineNode::DxfLineNode(DxfDocumentNode* doc, int groupCode)
    : DxfEntityNode(doc, groupCode, "LINE", new DxfLineNodePrivate(this))
{

}

void DxfLineNode::debugPrint() const
{
    Q_D(const DxfLineNode);
    DxfEntityNode::debugPrint();
    qLogD << "    thickness = " << d->thickness << "\n";
    qLogD << "    startPoint = " << d->startPoint.x() << ", " << d->startPoint.y() << ", " << d->startPoint.z() << "\n";
    qLogD << "    endPoint = " << d->endPoint.x() << ", " << d->endPoint.y() << ", " << d->endPoint.z() << "\n";
}

LaserPrimitive* DxfLineNode::convertTo(LaserDocument* doc, const QTransform& t) const
{
    Q_D(const DxfLineNode);
    QLineF line(d->startPoint.toPointF(), d->endPoint.toPointF());
    LaserLine* primitive = new LaserLine(t.map(line).toLine(), doc);
    return primitive;
}

bool DxfLineNode::parseItem(DxfGroup& group)
{
    Q_D(DxfLineNode);
    if (group.match(10))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->startPoint.setX(value);
    }
    else if (group.match(20))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->startPoint.setY(value);
    }
    else if (group.match(30))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->startPoint.setZ(value);
    }
    else if (group.match(11))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->endPoint.setX(value);
    }
    else if (group.match(21))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->endPoint.setY(value);
    }
    else if (group.match(31))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->endPoint.setZ(value);
    }
    else if (group.match(39))
    {
        if (!group.assign(d->thickness))
            return false;
    }
    return true;
}

class DxfSplineNodePrivate : public DxfEntityNodePrivate
{
    Q_DECLARE_PUBLIC(DxfSplineNode)
public:
    DxfSplineNodePrivate(DxfSplineNode* ptr)
        : DxfEntityNodePrivate(ptr, NT_Spline)
        , splineFlag(0)
        , curveDegree(0)
        , knotsCount(0)
        , controlPointsCount(0)
        , fitPointsCount(0)
        , knotTolerance(0.0000001)
        , controlPointTolerance(0.0000001)
        , fitTolerance(0.0000000001)
    {}

    /// <summary>
    /// Normal vector(omitted if the spline is nonplanar)
    /// DXF: X value; APP: 3D vector
    /// DXF: Y and Z values of normal vector (optional)
    /// </summary>
    QVector3D normalVector;

    /// <summary>
    /// Spline flag (bit coded):
    /// 1 = Closed spline
    /// 2 = Periodic spline
    /// 4 = Rational spline
    /// 8 = Planar
    /// 16 = Linear(planar bit is also set)
    /// </summary>
    quint32 splineFlag;

    /// <summary>
    /// Degree of the spline curve
    /// </summary>
    quint32 curveDegree;

    quint32 knotsCount;

    quint32 controlPointsCount;

    /// <summary>
    /// Number of fit points (if any)
    /// </summary>
    quint32 fitPointsCount;

    /// <summary>
    /// Knot tolerance (default = 0.0000001)
    /// </summary>
    qreal knotTolerance;

    /// <summary>
    /// Control-point tolerance (default = 0.0000001)
    /// </summary>
    qreal controlPointTolerance;

    /// <summary>
    /// Fit tolerance (default = 0.0000000001)
    /// </summary>
    qreal fitTolerance;

    QVector3D startTangent;
    QVector3D endTangent;

    QList<qreal> knots;

    QList<qreal> weights;

    QList<QVector3D> controlPoints;
    QList<QVector3D> fitPoints;
};


DxfSplineNode::DxfSplineNode(DxfDocumentNode* doc, int groupCode)
    : DxfEntityNode(doc, groupCode, "SPLINE", new DxfSplineNodePrivate(this))
{
}

void DxfSplineNode::debugPrint() const
{
    Q_D(const DxfSplineNode);
    DxfEntityNode::debugPrint();
    qLogD << "    normalVector = " << d->normalVector.x() << ", " << d->normalVector.y() << ", " << d->normalVector.z() << "\n";
    qLogD << "    splineFlag = " << d->splineFlag << "\n";
    qLogD << "    curveDegree = " << d->curveDegree << "\n";
    qLogD << "    knotsCount = " << d->knotsCount << "\n";
    qLogD << "    controlPointsCount = " << d->controlPointsCount << "\n";
    qLogD << "    fitPointsCount = " << d->fitPointsCount << "\n";
    qLogD << "    knotTolerance = " << d->knotTolerance << "\n";
    qLogD << "    controlPointTolerance = " << d->controlPointTolerance << "\n";
    qLogD << "    fitTolerance = " << d->fitTolerance << "\n";
    qLogD << "    startTangent = " << d->startTangent.x() << ", " << d->startTangent.y() << ", " << d->startTangent.z() << "\n";
    qLogD << "    endTangent = " << d->endTangent.x() << ", " << d->endTangent.y() << ", " << d->endTangent.z() << "\n";
    qLogD << "    knots = \n";
    for (qreal knot : d->knots)
    {
        qLogD << "      " << knot << "\n";
    }
    qLogD << "    weights = \n";
    for (qreal weight : d->weights)
    {
        qLogD << "      " << weight << "\n";
    }
    qLogD << "    controlPoints = \n";
    for (QVector3D point : d->controlPoints)
    {
        qLogD << "      " << point.x() << ", " << point.y() << ", " << point.z() << "\n";
    }
    qLogD << "    fitPoints = \n";
    for (QVector3D point : d->fitPoints)
    {
        qLogD << "      " << point.x() << ", " << point.y() << ", " << point.z() << "\n";
    }
}

LaserPrimitive* DxfSplineNode::convertTo(LaserDocument* doc, const QTransform& t) const
{
    Q_D(const DxfSplineNode);
    if (d->controlPoints.count() == 0)
        return nullptr;
    QList<QPointF> controlPoints;
    for (int i = 0; i < d->controlPoints.count(); i++)
    {
        QPointF point = t.map(d->controlPoints[i].toPointF());
        controlPoints.append(point);
    }
    QList<qreal> knots(d->knots);

    QList<qreal> weights(d->weights);
    if (d->weights.isEmpty())
    {
        for (int i = 0; i < controlPoints.count(); i++)
        {
            weights.append(1);
        }
    }

    quint32 p = d->curveDegree;
    quint32 n = d->controlPointsCount;

    if (isClosed())
    {
        //controlPoints.append(controlPoints[0]);
        /*for (int i = 0; i < p + 2; i++)
        {
            knots.append(knots[i]);
        }*/
        /*for (int i = 0; i < p; i++)
        {
            controlPoints[n - p + i] = controlPoints[i];
        }*/
    }

    //controlPoints.append(controlPoints[0]);
    LaserNurbs* primitive = new LaserNurbs(controlPoints, knots, weights, LaserNurbs::BasisType::BT_BSPLINE, doc);
    return primitive;
}

bool DxfSplineNode::isClosed() const
{
    Q_D(const DxfSplineNode);
    return d->splineFlag & 0x00000001;
}

bool DxfSplineNode::parseItem(DxfGroup& group)
{
    Q_D(DxfSplineNode);
    if (!DxfEntityNode::parseItem(group))
        return false;
    if (group.matched)
        return true;

    if (group.match(10))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->controlPoints.append(QVector3D(value, 0, 0));
    }
    else if (group.match(20))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->controlPoints.last().setY(value);
    }
    else if (group.match(30))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->controlPoints.last().setZ(value);
    }
    else if (group.match(11))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->fitPoints.append(QVector3D(value, 0, 0));
    }
    else if (group.match(21))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->fitPoints.last().setY(value);
    }
    else if (group.match(31))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->fitPoints.last().setZ(value);
    }
    else if (group.match(12))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->startTangent.setX(value);
    }
    else if (group.match(22))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->startTangent.setY(value);
    }
    else if (group.match(32))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->startTangent.setZ(value);
    }
    else if (group.match(13))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->endTangent.setX(value);
    }
    else if (group.match(23))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->endTangent.setY(value);
    }
    else if (group.match(33))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->endTangent.setZ(value);
    }
    else if (group.match(40))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->knots.append(value);
    }
    else if (group.match(41))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->weights.append(value);
    }
    else if (group.match(42))
    {
        if (!group.assign(d->knotTolerance))
            return false;
    }
    else if (group.match(43))
    {
        if (!group.assign(d->controlPointTolerance))
            return false;
    }
    else if (group.match(44))
    {
        if (!group.assign(d->fitTolerance))
            return false;
    }
    else if (group.match(70))
    {
        if (!group.assign(d->splineFlag))
            return false;
    }
    else if (group.match(71))
    {
        if (!group.assign(d->curveDegree))
            return false;
    }
    else if (group.match(72))
    {
        if (!group.assign(d->knotsCount))
            return false;
    }
    else if (group.match(73))
    {
        if (!group.assign(d->controlPointsCount))
            return false;
    }
    else if (group.match(74))
    {
        if (!group.assign(d->fitPointsCount))
            return false;
    }
    else if (group.match(210))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->normalVector.setX(value);
    }
    else if (group.match(220))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->normalVector.setY(value);
    }
    else if (group.match(230))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->normalVector.setZ(value);
    }

    return true;
}

class DxfArcNodePrivate : public DxfEntityNodePrivate
{
    Q_DECLARE_PUBLIC(DxfArcNode)
public:
    DxfArcNodePrivate(DxfArcNode* ptr)
        : DxfEntityNodePrivate(ptr, NT_Line)
        , thickness(0)
        , radius(0)
        , startAngle(0)
        , endAngle(0)
    {}

    qreal thickness;
    QVector3D centerPoint;
    qreal radius;
    qreal startAngle;
    qreal endAngle;
};

DxfArcNode::DxfArcNode(DxfDocumentNode* doc, int groupCode)
    : DxfEntityNode(doc, groupCode, "ARC", new DxfArcNodePrivate(this))
{

}

void DxfArcNode::debugPrint() const
{
    Q_D(const DxfArcNode);
    DxfEntityNode::debugPrint();
    qLogD << "    thickness = " << d->thickness << "\n";
    qLogD << "    centerPoint = " << d->centerPoint.x() << ", " << d->centerPoint.y() << ", " << d->centerPoint.z() << "\n";
    qLogD << "    radius = " << d->radius << "\n";
    qLogD << "    startAngle = " << d->startAngle << "\n";
    qLogD << "    endAngle = " << d->endAngle << "\n";
}

LaserPrimitive* DxfArcNode::convertTo(LaserDocument* doc, const QTransform& t) const
{
    Q_D(const DxfArcNode);
    QPainterPath path;
    QRect rect(d->centerPoint.x() - d->radius, d->centerPoint.y() - d->radius, d->radius * 2, d->radius * 2);
    //path.moveTo(d->centerPoint.toPointF());
    path.arcMoveTo(rect, d->startAngle);

    // 因为cad是在第一象限而本软件是在第四象限，所以传入的t做了一个垂直镜像翻转，在这种情况下，
    // 画弧的时候，需要返着画，即顺时针。否则正好与cad原图逆着方向。
    path.arcTo(rect, d->startAngle, d->startAngle - d->endAngle);
    LaserPath* primitive = new LaserPath(t.map(path), doc);
    return primitive;
}

bool DxfArcNode::parseItem(DxfGroup& group)
{
    Q_D(DxfArcNode);
    if (group.match(10))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->centerPoint.setX(value);
    }
    else if (group.match(20))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->centerPoint.setY(value);
    }
    else if (group.match(30))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->centerPoint.setZ(value);
    }
    else if (group.match(40))
    {
        if (!group.assign(d->radius))
            return false;
    }
    else if (group.match(50))
    {
        if (!group.assign(d->startAngle))
            return false;
    }
    else if (group.match(51))
    {
        if (!group.assign(d->endAngle))
            return false;
    }
    return true;
}

class DxfImageNodePrivate : public DxfEntityNodePrivate
{
    Q_DECLARE_PUBLIC(DxfImageNode)
public:
    DxfImageNodePrivate(DxfImageNode* ptr)
        : DxfEntityNodePrivate(ptr, NT_Line)
        , classVersion(0)
        , displayProperties(0)
        , clippingState(0)
        , brightness(50)
        , contrast(50)
        , fade(0)
        , imageObjectHandle(0)
        , imageReactorHandle(0)
        , clipBoundaryVerticesCount(0)
        , clipMode(0)
        , defObject(nullptr)
    {}

    int classVersion;

    QVector3D insertionPoint;

    /// <summary>
    /// U-vector of a single pixel (points along the visual bottom of the image, 
    /// starting at the insertion point) (in WCS)
    /// </summary>
    QVector3D uVector;

    /// <summary>
    /// V-vector of a single pixel (points along the visual left side of the 
    /// image, starting at the insertion point) (in WCS)
    /// </summary>
    QVector3D vVector;

    QSize imageSize;

    int imageObjectHandle;

    /// <summary>
    /// Image display properties:
    /// 1 = Show image
    /// 2 = Show image when not aligned with screen
    /// 4 = Use clipping boundary
    /// 8 = Transparency is on
    /// </summary>
    int displayProperties;

    /// <summary>
    /// Clipping state:
    /// 0 = Off
    /// 1 = On
    /// </summary>
    int clippingState;

    /// <summary>
    /// Brightness value (0-100; default = 50)
    /// </summary>
    int brightness;

    /// <summary>
    /// Contrast value (0-100; default = 50)
    /// </summary>
    int contrast;

    /// <summary>
    /// Fade value(0 - 100; default = 0)
    /// </summary>
    int fade;

    int imageReactorHandle;

    /// <summary>
    /// Clipping boundary type. 1 = Rectangular; 2 = Polygonal
    /// </summary>
    int clippingBoundaryType;

    /// <summary>
    /// Number of clip boundary vertices that follow
    /// </summary>
    int clipBoundaryVerticesCount;

    /// <summary>
    /// Clip boundary vertex (in OCS)
    /// DXF: X value; APP: 2D point(multiple entries)
    /// NOTE 1) For rectangular clip boundary type, two opposite corners must be
    /// specified.Default is(-0.5, -0.5), (size.x - 0.5, size.y - 0.5). 2) For 
    /// polygonal clip boundary type, three or more vertices must be specified.
    /// Polygonal vertices must be listed sequentially
    /// </summary>
    QList<QPointF> clipBoundaryVertices;

    /// <summary>
    /// Clip Mode:
    /// 0 = Outside Mode
    /// 1 = Inside Mode
    /// </summary>
    int clipMode;

    DxfImageDefNode* defObject;
};

DxfImageNode::DxfImageNode(DxfDocumentNode* doc, int groupCode)
    : DxfEntityNode(doc, groupCode, "IMAGE", new DxfImageNodePrivate(this))
{

}

QString DxfImageNode::fileName() const
{
    Q_D(const DxfImageNode);
    if (d->defObject)
    {
        return d->defObject->fileName();
    }
    return "";
}

void DxfImageNode::postProcess()
{
    Q_D(DxfImageNode);
    DxfItemNode* obj = d->document->item(d->imageObjectHandle);
    DxfImageDefNode* imageDef = dynamic_cast<DxfImageDefNode*>(obj);
    if (!imageDef)
        return;

    d->defObject = imageDef;
}

void DxfImageNode::debugPrint() const
{
    Q_D(const DxfImageNode);
    DxfEntityNode::debugPrint();
    qLogD << "    classVersion = " << d->classVersion << "\n";
    qLogD << "    insertionPoint = " << d->insertionPoint.x() << ", " << d->insertionPoint.y() << ", " << d->insertionPoint.z() << "\n";
    qLogD << "    uVector = " << d->uVector.x() << ", " << d->uVector.y() << ", " << d->uVector.z() << "\n";
    qLogD << "    vVector = " << d->vVector.x() << ", " << d->vVector.y() << ", " << d->vVector.z() << "\n";
    qLogD << "    imageSize = " << d->imageSize.width() << ", " << d->imageSize.height() << "\n";
    qLogD << "    imageObjectHandle = " << d->imageObjectHandle << "\n";
    qLogD << "    displayProperties = " << d->displayProperties << "\n";
    qLogD << "    clippingState = " << d->clippingState << "\n";
    qLogD << "    brightness = " << d->brightness << "\n";
    qLogD << "    contrast = " << d->contrast << "\n";
    qLogD << "    fade = " << d->fade << "\n";
    qLogD << "    imageReactorHandle = " << d->imageReactorHandle << "\n";
    qLogD << "    clippingBoundaryType = " << d->clippingBoundaryType << "\n";
    qLogD << "    clipBoundaryVerticesCount = " << d->clipBoundaryVerticesCount << "\n";
    qLogD << "    clipBoundaryVertices = \n";
    for (QPointF point : d->clipBoundaryVertices)
    {
        qLogD << "      " << point.x() << ", " << point.y() << "\n";
    }
    qLogD << "    clipMode = " << d->clipMode << "\n";
}

LaserPrimitive* DxfImageNode::convertTo(LaserDocument* doc, const QTransform& t) const
{
    Q_D(const DxfImageNode);

    QFileInfo fileInfo(fileName());
    if (!fileInfo.exists())
        return nullptr;

    QImage image;
    if (!image.load(fileName()))
        return nullptr;

    QRectF bounding(d->insertionPoint.toPointF(), 
        QSize(d->uVector.x() * d->imageSize.width(), d->vVector.y() * d->imageSize.height()));
    LaserBitmap* primitive = new LaserBitmap(image, t.mapRect(bounding).toRect(), doc);
    return primitive;
}


bool DxfImageNode::parseItem(DxfGroup& group)
{
    Q_D(DxfImageNode);
    if (group.match(10))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->insertionPoint.setX(value);
    }
    else if (group.match(20))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->insertionPoint.setY(value);
    }
    else if (group.match(30))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->insertionPoint.setZ(value);
    }
    else if (group.match(11))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->uVector.setX(value);
    }
    else if (group.match(21))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->uVector.setY(value);
    }
    else if (group.match(31))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->uVector.setZ(value);
    }
    else if (group.match(12))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->vVector.setX(value);
    }
    else if (group.match(22))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->vVector.setY(value);
    }
    else if (group.match(32))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->vVector.setZ(value);
    }
    else if (group.match(13))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->imageSize.setWidth(value);
    }
    else if (group.match(23))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->imageSize.setHeight(value);
    }
    else if (group.match(14))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->clipBoundaryVertices.append(QPointF(value, 0));
    }
    else if (group.match(24))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->clipBoundaryVertices.last().setY(value);
    }
    else if (group.match(70))
    {
        if (!group.assign(d->displayProperties))
            return false;
    }
    else if (group.match(71))
    {
        if (!group.assign(d->clippingBoundaryType))
            return false;
    }
    else if (group.match(91))
    {
        if (!group.assign(d->clipBoundaryVerticesCount))
            return false;
    }
    else if (group.match(280))
    {
        if (!group.assign(d->clippingState))
            return false;
    }
    else if (group.match(281))
    {
        if (!group.assign(d->brightness))
            return false;
    }
    else if (group.match(282))
    {
        if (!group.assign(d->contrast))
            return false;
    }
    else if (group.match(283))
    {
        if (!group.assign(d->fade))
            return false;
    }
    else if (group.match(290))
    {
        if (!group.assign(d->clipMode))
            return false;
    }
    else if (group.match(340))
    {
        if (!group.assign(d->imageObjectHandle, 16))
            return false;
    }
    else if (group.match(360))
    {
        if (!group.assign(d->imageReactorHandle, 16))
            return false;
    }
    
    return true;
}

class DxfObjectNodePrivate : public DxfItemNodePrivate
{
    Q_DECLARE_PUBLIC(DxfObjectNode)
public:
    DxfObjectNodePrivate(DxfObjectNode* ptr, DxfNodeType nodeType)
        : DxfItemNodePrivate(ptr, nodeType)
    {}
};

DxfObjectNode::DxfObjectNode(DxfDocumentNode* doc, int groupCode, const QString& variable, DxfObjectNodePrivate* ptr)
    : DxfItemNode(doc, groupCode, variable, ptr)
{}

DxfObjectNode::DxfObjectNode(DxfDocumentNode* doc, int groupCode, const QString & variable)
    : DxfItemNode(doc, groupCode, variable, new DxfObjectNodePrivate(this, NT_Object))
{
}

void DxfObjectNode::debugPrint() const
{
    Q_D(const DxfObjectNode);
    DxfItemNode::debugPrint();
    qLogD << "OBJECT " << name() << "\n";
    DxfItemNode::debugPrint();
}

bool DxfObjectNode::parseItem(DxfGroup& group)
{
    return true;
}

bool DxfObjectNode::isEnd(DxfGroup& group)
{
    if (group.match(0, "ENDSEC"))
    {
        return true;
    }
    else if (group.match(0))
    {
        group.needCache = true;
        return true;
    }
    return false;
}

class DxfImageDefNodePrivate : public DxfObjectNodePrivate
{
    Q_DECLARE_PUBLIC(DxfImageDefNode)
public:
    DxfImageDefNodePrivate(DxfImageDefNode* ptr)
        : DxfObjectNodePrivate(ptr, NT_ImageDef)
        , version(0)
        , imageSize(0, 0)
        , unitsPerPixel(0, 0)
        , loadedFlag(0)
        , resolutionUnits(0)
    {}

    int version;
    QString fileName;
    QSizeF imageSize;
    QSizeF unitsPerPixel;

    /// <summary>
    /// Image-is-loaded flag. 0 = Unloaded; 1 = Loaded
    /// </summary>
    int loadedFlag;

    /// <summary>
    /// Resolution units. 0 = No units; 2 = Centimeters; 5 = Inch
    /// </summary>
    int resolutionUnits;
};

DxfImageDefNode::DxfImageDefNode(DxfDocumentNode* doc, int groupCode)
    : DxfObjectNode(doc, groupCode, "IMAGEDEF", new DxfImageDefNodePrivate(this))
{

}

QString DxfImageDefNode::fileName() const
{
    Q_D(const DxfImageDefNode);
    return d->fileName;
}

void DxfImageDefNode::debugPrint() const
{
    Q_D(const DxfImageDefNode);
    DxfObjectNode::debugPrint();
    qLogD << "    version = " << d->version << "\n";
    qLogD << "    fileName = " << d->fileName << "\n";
    qLogD << "    imageSize = " << d->imageSize.width() << ", " << d->imageSize.height() << "\n";
    qLogD << "    unitsPerPixel = " << d->unitsPerPixel.width() << ", " << d->unitsPerPixel.height() << "\n";
    qLogD << "    loadedFlag = " << d->loadedFlag << "\n";
    qLogD << "    resolutionUnits = " << d->resolutionUnits << "\n";
}

bool DxfImageDefNode::parseItem(DxfGroup& group)
{
    Q_D(DxfImageDefNode);
    if (group.match(1))
    {
        group.assign(d->fileName);
    }
    else if (group.match(10))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->imageSize.setWidth(value);
    }
    else if (group.match(20))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->imageSize.setHeight(value);
    }
    if (group.match(11))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->unitsPerPixel.setWidth(value);
    }
    else if (group.match(21))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->unitsPerPixel.setHeight(value);
    }
    else if (group.match(90))
    {
        if (!group.assign(d->version))
            return false;
    }
    else if (group.match(280))
    {
        if (!group.assign(d->loadedFlag))
            return false;
    }
    else if (group.match(51))
    {
        if (!group.assign(d->resolutionUnits))
            return false;
    }
    return true;
}

class DxfThumbnailImageNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfThumbnailImageNode)
public:
    DxfThumbnailImageNodePrivate(DxfThumbnailImageNode* ptr)
        : DxfNodePrivate(ptr, NT_ThumbnailImage)
    {}
};

DxfThumbnailImageNode::DxfThumbnailImageNode(DxfDocumentNode* doc, int groupCode)
    : DxfNode(doc, groupCode, "THUMBNAILIMAGE", new DxfThumbnailImageNodePrivate(this))
{

}

bool DxfThumbnailImageNode::parse(DxfStream* stream)
{
    return true;
}

class DxfCollectionNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfCollectionNode)
public:
    DxfCollectionNodePrivate(DxfCollectionNode* ptr, DxfNodeType nodeType)
        : DxfNodePrivate(ptr, nodeType)
    {

    }

    QList<DxfNode*> items;
};

DxfCollectionNode::DxfCollectionNode(DxfDocumentNode* doc, int groupCode, const QString& variable, DxfCollectionNodePrivate* ptr)
    : DxfNode(doc, groupCode, variable, ptr)
{

}

class DxfClassesNodePrivate : public DxfCollectionNodePrivate
{
    Q_DECLARE_PUBLIC(DxfClassesNode)
public:
    DxfClassesNodePrivate(DxfClassesNode* ptr)
        : DxfCollectionNodePrivate(ptr, NT_Classes)
    {}
};

DxfClassesNode::DxfClassesNode(DxfDocumentNode* doc, int groupCode)
    : DxfCollectionNode(doc, groupCode, "CLASSES", new DxfClassesNodePrivate(this))
{

}

bool DxfClassesNode::parse(DxfStream* stream)
{
    Q_D(DxfClassesNode);
    while (true)
    {
        DxfGroup group;
        if (!stream->readGroup(group))
        {
            return false;
        }

        if (group.match(0, "CLASS"))
        {
            DxfClassNode* childNode = new DxfClassNode(d->document, group.groupCode);
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
        else if (group.match(0, "ENDSEC"))
        {
            break;
        }
    }
    return true;
}

void DxfClassesNode::debugPrint() const
{
    qLogD << "CLASSES\n";
    DxfNode::debugPrint();
    for (DxfClassNode* node : *this)
    {
        node->debugPrint();
    }
}

class DxfTablesNodePrivate : public DxfCollectionNodePrivate
{
    Q_DECLARE_PUBLIC(DxfTablesNode)
public:
    DxfTablesNodePrivate(DxfTablesNode* ptr)
        : DxfCollectionNodePrivate(ptr, NT_Tables)
    {}
};

DxfTablesNode::DxfTablesNode(DxfDocumentNode* doc, int groupCode)
    : DxfCollectionNode(doc, groupCode, "TABLES", new DxfTablesNodePrivate(this))
{

}

bool DxfTablesNode::parse(DxfStream* stream)
{
    Q_D(DxfTablesNode);
    while (true)
    {
        DxfGroup group;
        if (!stream->readGroup(group))
        {
            return false;
        }

        if (group.match(0, "TABLE"))
        {
            DxfTableNode* childNode = new DxfTableNode(d->document, group.groupCode);
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
        else if (group.match(0, "ENDSEC"))
        {
            break;
        }
    }
    return true;
}

void DxfTablesNode::debugPrint() const
{
    Q_D(const DxfTablesNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "TABLES\n";
    DxfNode::debugPrint();
    for (DxfTableNode* node : *this)
    {
        node->debugPrint();
    }
}

class DxfBlocksNodePrivate : public DxfCollectionNodePrivate
{
    Q_DECLARE_PUBLIC(DxfBlocksNode)
public:
    DxfBlocksNodePrivate(DxfBlocksNode* ptr)
        : DxfCollectionNodePrivate(ptr, NT_Blocks)
    {}
};

DxfBlocksNode::DxfBlocksNode(DxfDocumentNode* doc, int groupCode)
    : DxfCollectionNode(doc, groupCode, "BLOCKS", new DxfBlocksNodePrivate(this))
{

}

bool DxfBlocksNode::parse(DxfStream* stream)
{
    Q_D(DxfBlocksNode);
    while (true)
    {
        DxfGroup group;
        if (!stream->readGroup(group))
        {
            return false;
        }

        if (group.match(0, "BLOCK"))
        {
            DxfBlockNode* childNode = new DxfBlockNode(d->document, group.groupCode);
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
        else if (group.match(0, "ENDBLK"))
        {
            DxfEndBlockNode* childNode = new DxfEndBlockNode(d->document, group.groupCode);
            last()->addChildNode(childNode);
            if (!childNode->parse(stream))
                return false;
        }
        else if (group.match(0, "ENDSEC"))
        {
            break;
        }
    }
    return true;
}

void DxfBlocksNode::debugPrint() const
{
    Q_D(const DxfBlocksNode);
    qLogD << "BLOCKS\n";
    DxfNode::debugPrint();
    for (DxfBlockNode* node : *this)
    {
        node->debugPrint();
    }
}

class DxfEntitiesNodePrivate : public DxfCollectionNodePrivate
{
    Q_DECLARE_PUBLIC(DxfEntitiesNode)
public:
    DxfEntitiesNodePrivate(DxfEntitiesNode* ptr)
        : DxfCollectionNodePrivate(ptr, NT_Entities)
    {}
};

DxfEntitiesNode::DxfEntitiesNode(DxfDocumentNode* doc, int groupCode)
    : DxfCollectionNode(doc, groupCode, "ENTITIES", new DxfEntitiesNodePrivate(this))
{

}

bool DxfEntitiesNode::parse(DxfStream* stream)
{
    Q_D(DxfEntitiesNode);
    while (true)
    {
        DxfGroup group;
        if (!stream->readGroup(group))
        {
            return false;
        }

        DxfEntityNode* childNode = nullptr;
        if (group.match(0, "ENDSEC"))
        {
            break;
        }
        else if (group.match(0, "ARC"))
        {
            childNode = new DxfArcNode(d->document, group.groupCode);
        }
        else if (group.match(0, "LWPOLYLINE"))
        {
            childNode = new DxfLWPolylineNode(d->document, group.groupCode);
        }
        else if (group.match(0, "CIRCLE"))
        {
            childNode = new DxfCircleNode(d->document, group.groupCode);
        }
        else if (group.match(0, "ELLIPSE"))
        {
            childNode = new DxfEllipseNode(d->document, group.groupCode);
        }
        else if (group.match(0, "LINE"))
        {
            childNode = new DxfLineNode(d->document, group.groupCode);
        }
        else if (group.match(0, "SPLINE"))
        {
            childNode = new DxfSplineNode(d->document, group.groupCode);
        }
        else if (group.match(0, "IMAGE"))
        {
            childNode = new DxfImageNode(d->document, group.groupCode);
        }
        else if (group.match(0))
        {
            childNode = new DxfEntityNode(d->document, group.groupCode, group.variable);
        }

        if (childNode)
        {
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
    }
    return true;
}

void DxfEntitiesNode::debugPrint() const
{
    Q_D(const DxfEntitiesNode);
    qLogD << "ENTITIES\n";
    DxfNode::debugPrint();
    for (DxfEntityNode* node : *this)
    {
        node->debugPrint();
    }
}

class DxfObjectsNodePrivate : public DxfCollectionNodePrivate
{
    Q_DECLARE_PUBLIC(DxfObjectsNode)
public:
    DxfObjectsNodePrivate(DxfObjectsNode* ptr)
        : DxfCollectionNodePrivate(ptr, NT_Objects)
    {}
};

DxfObjectsNode::DxfObjectsNode(DxfDocumentNode* doc, int groupCode)
    : DxfCollectionNode(doc, groupCode, "OBJECTS", new DxfObjectsNodePrivate(this))
{

}

bool DxfObjectsNode::parse(DxfStream* stream)
{
    Q_D(DxfObjectsNode);
    while (true)
    {
        DxfObjectNode* childNode = nullptr;
        DxfGroup group;
        if (!stream->readGroup(group))
        {
            return false;
        }

        if (group.match(0, "ENDSEC"))
        {
            break;
        }
        else if (group.match(0, "IMAGEDEF"))
        {
            childNode = new DxfImageDefNode(d->document);
        }
        else if (group.match(0))
        {
            childNode = new DxfObjectNode(d->document, group.groupCode, group.variable);
        }

        if (childNode)
        {
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
    }
    return true;
}

void DxfObjectsNode::debugPrint() const
{
    Q_D(const DxfObjectsNode);
    qLogD << "OBJECTS\n";
    DxfNode::debugPrint();
    for (DxfObjectNode* node : *this)
    {
        node->debugPrint();
    }
}

