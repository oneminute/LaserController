#include "DxfNode.h"

#include "common/common.h"

class DxfStreamPrivate
{
    Q_DECLARE_PUBLIC(DxfStream)
public:
    DxfStreamPrivate(DxfStream* ptr)
    {}

    DxfStream* q_ptr;

};

DxfStream::DxfStream(QIODevice* device)
    : QTextStream(device)
    , m_lineNumber(0)
    , m_cached(false)
{
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

DxfNode::DxfNode(int groupCode, const QString& variable, DxfNodePrivate* privateData)
    : m_ptr(privateData)
{
    Q_D(DxfNode);
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

DxfNodeType DxfNode::nodeType() const
{
    Q_D(const DxfNode);
    return d->nodeType;
}

QString DxfNode::toString() const 
{
    Q_D(const DxfNode);
    return QString("groupCode = %1, variable = %2\n").arg(d->groupCode).arg(d->variable);
}

QDebug operator<<(QDebug debug, const DxfNode& node)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << node.toString();
    return debug;
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
};

DxfDocumentNode::DxfDocumentNode()
    : DxfNode(0, "DOCUMENT", new DxfDocumentNodePrivate(this))
{

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
                childNode = d->header = new DxfHeaderNode(group.groupCode);
            }
            else if (group.match("CLASSES"))
            {
                childNode = d->classes = new DxfClassesNode(group.groupCode);
            }
            else if (group.match("TABLES"))
            {
                childNode = d->tables = new DxfTablesNode(group.groupCode);
            }
            else if (group.match("BLOCKS"))
            {
                childNode = d->blocks = new DxfBlocksNode(group.groupCode);
            }
            else if (group.match("ENTITIES"))
            {
                childNode = d->entities = new DxfEntitiesNode(group.groupCode);
            }
            else if (group.match("OBJECTS"))
            {
                childNode = d->objects = new DxfObjectsNode(group.groupCode);
            }
            else if (group.match("THUMBNAILIMAGE"))
            {
                childNode = d->thumbnailImage = new DxfThumbnailImageNode(group.groupCode);
            }
        }
        else if (group.match("EOF"))
        {
            break;
        }

        if (childNode)
        {
            if (childNode->parse(stream))
            {
                addChildNode(childNode);
            }
        }
    }
    return true;
}

QString DxfDocumentNode::toString() const
{
    Q_D(const DxfDocumentNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "\n";
    stream << DxfNode::toString();
    if (d->header)
        stream << d->header->toString();
    if (d->classes)
        stream << d->classes->toString();
    if (d->tables)
        stream << d->tables->toString();
    if (d->blocks)
        stream << d->blocks->toString();
    if (d->entities)
        stream << d->entities->toString();
    if (d->objects)
        stream << d->objects->toString();
    return buf;
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

DxfHeaderNode::DxfHeaderNode(int groupCode)
    : DxfNode(groupCode, "HEADER", new DxfHeaderNodePrivate(this))
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

QString DxfHeaderNode::toString() const
{
    Q_D(const DxfHeaderNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "HEADER" << "\n";
    for (PropertyMap::ConstIterator i = d->properties.constBegin(); i != d->properties.constEnd(); i++)
    {
        stream << "  " << i.key() << " = " << i.value() << "\n";
    }
    return buf;
    return buf;
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

DxfClassNode::DxfClassNode(int groupCode)
    : DxfNode(groupCode, "CLASS", new DxfClassNodePrivate(this))
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

QString DxfClassNode::toString() const
{
    Q_D(const DxfClassNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "CLASS " << name() << "\n";
    stream << DxfNode::toString();
    stream << "    dxfRecordName: " << d->dxfRecordName << "\n";
    stream << "    applicationName: " << d->applicationName << "\n";
    stream << "    proxyCapabilitiesFlag: " << d->proxyCapabilitiesFlag << "\n";
    stream << "    instanceCount: " << d->instanceCount << "\n";
    stream << "    wasAProxy: " << d->wasAProxy << "\n";
    stream << "    isAnEntity: " << d->isAnEntity << "\n";
    return buf;
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
    QString subClassMarker;
};

DxfItemNode::DxfItemNode(int groupCode, const QString& variable, DxfItemNodePrivate* ptr)
    : DxfNode(groupCode, variable, ptr)
{

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
        else if (group.match(100) && d->subClassMarker.isEmpty())
        {
            d->subClassMarker = group.variable;
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

QString DxfItemNode::toString() const
{
    Q_D(const DxfItemNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "    handle = " << d->handle << "\n";
    stream << "    appDefinedGroups = \n";
    for (QString def : d->appDefinedGroups)
    { 
        stream << "      " << def << "\n";
    }
    stream << "    softPointerId = " << d->softPointerId << "\n";
    stream << "    subClassMarker = " << d->subClassMarker << "\n";

    return buf;
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

DxfTableNode::DxfTableNode(int groupCode)
    : DxfItemNode(groupCode, "TABLE", new DxfTableNodePrivate(this))
{

}

QString DxfTableNode::toString() const
{
    Q_D(const DxfTableNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "TABLE " << name() << "\n";
    stream << DxfItemNode::toString();
    stream << "    entityName: " << d->entityName << "\n";
    stream << "    maximumEntries: " << d->maximumEntries << "\n";
    return buf;
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

DxfBlockNode::DxfBlockNode(int groupCode)
    : DxfItemNode(groupCode, "BLOCK", new DxfBlockNodePrivate(this))
{

}

QString DxfBlockNode::toString() const
{
    Q_D(const DxfBlockNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "BLOCK " << name() << "\n";
    stream << DxfNode::toString();
    stream << DxfItemNode::toString();
    stream << "    layerName = " << d->layerName << "\n";
    stream << "    subClassMarkerBlockBegin = " << d->subClassMarkerBlockBegin << "\n";
    stream << "    blockTypeFlags = " << d->blockTypeFlags << "\n";
    stream << "    basePoint = " << d->basePoint.x() << ", " << d->basePoint.y() << ", " << d->basePoint.z() << "\n";
    stream << "    blockName3 = " << d->blockName3 << "\n";
    stream << "    xrefPathName = " << d->xrefPathName << "\n";
    stream << "    blockDescription = " << d->blockDescription << "\n";
    stream << d->children[0]->toString();
    return buf;
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
    QString subClassMarkerBlockEnd;
};

DxfEndBlockNode::DxfEndBlockNode(int groupCode)
    : DxfItemNode(groupCode, "ENDBLK", new DxfEndBlockNodePrivate(this))
{
}

QString DxfEndBlockNode::toString() const
{
    Q_D(const DxfEndBlockNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "ENDBLK " << name() << "\n";
    stream << DxfNode::toString();
    stream << DxfItemNode::toString();
    stream << "    layerName = " << d->layerName << "\n";
    stream << "    subClassMarkerBlockEnd = " << d->subClassMarkerBlockEnd << "\n";
    return buf;
}

bool DxfEndBlockNode::parseItem(DxfGroup& group)
{
    Q_D(DxfEndBlockNode);
    if (group.match(8))
    {
        d->layerName = group.variable;
    }
    else if (group.match(100))
    {
        d->subClassMarkerBlockEnd = group.variable;
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
    int linetypeScale;

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
};

DxfEntityNode::DxfEntityNode(int groupCode, const QString& variable, DxfEntityNodePrivate* ptr)
    : DxfItemNode(groupCode, variable, ptr)
{
    setName(variable);
}

DxfEntityNode::DxfEntityNode(int groupCode, const QString& variable)
    : DxfItemNode(groupCode, variable, new DxfEntityNodePrivate(this, NT_Entity))
{
    setName(variable);
}

QString DxfEntityNode::toString() const
{
    Q_D(const DxfEntityNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "ENTITY " << name() << "\n";
    stream << DxfNode::toString();
    stream << DxfItemNode::toString();
    stream << "    entityName = " << d->layerName << "\n";
    stream << "    entityType = " << d->entityType << "\n";
    stream << "    spaceType = " << d->spaceType << "\n";
    stream << "    layerName = " << d->layerName << "\n";
    stream << "    lineTypeName = " << d->lineTypeName << "\n";
    stream << "    materialObjectId = " << d->materialObjectId << "\n";
    stream << "    colorNumber = " << d->colorNumber << "\n";
    stream << "    lineweight = " << d->lineweight << "\n";
    stream << "    linetypeScale = " << d->linetypeScale << "\n";
    stream << "    visibility = " << d->visibility << "\n";
    stream << "    proxyGraphicsDataLength = " << d->proxyGraphicsDataLength << "\n";
    stream << "    proxyGraphicsData = " << d->proxyGraphicsData << "\n";
    stream << "    colorValue = " << d->colorValue << "\n";
    stream << "    transparency = " << d->transparency << "\n";
    stream << "    plotStyleObjectId = " << d->plotStyleObjectId << "\n";
    stream << "    shadowMode = " << d->shadowMode << "\n";
    return buf;
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
    /// Constant width (optional; default = 0). Not used if variable width 
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
    /// Starting width(multiple entries; one entry for each vertex) (optional; 
    /// default = 0; multiple entries).Not used if constant width(code 43) is set
    /// End width (multiple entries; one entry for each vertex) (optional; 
    /// default = 0; multiple entries). Not used if constant width (code 43) is 
    /// set
    /// </summary>
    QList<QVector2D> verticesWidth;

    /// <summary>
    /// Bulge (multiple entries; one entry for each vertex) (optional; default = 0)
    /// </summary>
    QList<qreal> bulges;

    /// <summary>
    ///	Extrusion direction(optional; default = 0, 0, 1)
    /// DXF: X value; APP: 3D vector
    /// DXF : Y and Z values of extrusion direction(optional)
    /// </summary>
    QList<QVector3D> extrusionDirection;
};


DxfLWPolylineNode::DxfLWPolylineNode(int groupCode)
    : DxfEntityNode(groupCode, "LWPOLYLINE", new DxfLWPolylineNodePrivate(this))
{
}

QString DxfLWPolylineNode::toString() const
{
    Q_D(const DxfLWPolylineNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "ENTITY " << name() << "\n";
    stream << DxfNode::toString();
    stream << DxfItemNode::toString();
    stream << "    entityName = " << d->layerName << "\n";
    stream << "    verticesCount = " << d->verticesCount << "\n";
    stream << "    polylineFlag = " << d->polylineFlag << "\n";
    stream << "    constantWidth = " << d->constantWidth << "\n";
    stream << "    elevation = " << d->elevation << "\n";
    stream << "    thickness = " << d->thickness << "\n";
    stream << "    points = \n";
    for (QPointF point : d->points)
    {
        stream << "      " << point.x() << ", " << point.y() << "\n";
    }
    stream << "    vertexIdentifier = " << d->vertexIdentifier << "\n";
    stream << "    verticesWidth = \n";
    for (QVector2D point : d->verticesWidth)
    {
        stream << "      " << point.x() << ", " << point.y() << "\n";
    }
    stream << "    bulges = \n";
    for (qreal bulge : d->bulges)
    {
        stream << "      " << bulge << "\n";
    }
    stream << "    extrusionDirection = \n";
    for (QVector3D point : d->extrusionDirection)
    {
        stream << "      " << point.x() << ", " << point.y() << ", " << point.z() << "\n";
    }
    return buf;
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
    else if (group.match(210))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.append(QVector3D(value, 0, 0));
    }
    else if (group.match(220))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.last().setY(value);
    }
    else if (group.match(230))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.last().setZ(value);
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

    /// <summary>
    ///	Extrusion direction(optional; default = 0, 0, 1)
    /// DXF: X value; APP: 3D vector
    /// DXF : Y and Z values of extrusion direction(optional)
    /// </summary>
    QList<QVector3D> extrusionDirection;
};

DxfCircleNode::DxfCircleNode(int groupCode)
    : DxfEntityNode(groupCode, "CIRCLE", new DxfCircleNodePrivate(this))
{

}

QString DxfCircleNode::toString() const
{
    Q_D(const DxfCircleNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "ENTITY " << name() << "\n";
    stream << DxfNode::toString();
    stream << DxfItemNode::toString();
    stream << "    entityName = " << d->layerName << "\n";
    stream << "    thickness = " << d->thickness << "\n";
    stream << "    center = " << d->center.x() << ", " << d->center.y() << ", " << d->center.z() << "\n";
    stream << "    radius = " << d->radius << "\n";
    stream << "    extrusionDirection = \n";
    for (QVector3D point : d->extrusionDirection)
    {
        stream << "      " << point.x() << ", " << point.y() << ", " << point.z() << "\n";
    }
    return buf;
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
    else if (group.match(210))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.append(QVector3D(value, 0, 0));
    }
    else if (group.match(220))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.last().setY(value);
    }
    else if (group.match(230))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.last().setZ(value);
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
    QVector3D endPoint;

    /// <summary>
    ///	Extrusion direction(optional; default = 0, 0, 1)
    /// DXF: X value; APP: 3D vector
    /// DXF : Y and Z values of extrusion direction(optional)
    /// </summary>
    QList<QVector3D> extrusionDirection;

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

DxfEllipseNode::DxfEllipseNode(int groupCode)
    : DxfEntityNode(groupCode, "ELLIPSE", new DxfEllipseNodePrivate(this))
{

}

QString DxfEllipseNode::toString() const
{
    Q_D(const DxfEllipseNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "ENTITY " << name() << "\n";
    stream << DxfNode::toString();
    stream << DxfItemNode::toString();
    stream << "    entityName = " << d->layerName << "\n";
    stream << "    thickness = " << d->thickness << "\n";
    stream << "    center = " << d->center.x() << ", " << d->center.y() << ", " << d->center.z() << "\n";
    stream << "    endPoint = " << d->endPoint.x() << ", " << d->endPoint.y() << ", " << d->endPoint.z() << "\n";
    stream << "    extrusionDirection = \n";
    for (QVector3D point : d->extrusionDirection)
    {
        stream << "      " << point.x() << ", " << point.y() << ", " << point.z() << "\n";
    }
    stream << "    ratio = " << d->ratio << "\n";
    stream << "    startParameter = " << d->startParameter << "\n";
    stream << "    endParameter = " << d->endParameter << "\n";
    return buf;
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
    else if (group.match(210))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.append(QVector3D(value, 0, 0));
    }
    else if (group.match(220))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.last().setY(value);
    }
    else if (group.match(230))
    {
        qreal value;
        if (!group.assign(value))
            return false;
        d->extrusionDirection.last().setZ(value);
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

DxfObjectNode::DxfObjectNode(int groupCode, const QString& variable, DxfObjectNodePrivate* ptr)
    : DxfItemNode(groupCode, variable, ptr)
{}

DxfObjectNode::DxfObjectNode(int groupCode, const QString & variable)
    : DxfItemNode(groupCode, variable, new DxfObjectNodePrivate(this, NT_Object))
{
}

QString DxfObjectNode::toString() const
{
    Q_D(const DxfObjectNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "OBJECT " << name() << "\n";
    stream << DxfNode::toString();
    stream << DxfItemNode::toString();
    return buf;
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

class DxfThumbnailImageNodePrivate : public DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfThumbnailImageNode)
public:
    DxfThumbnailImageNodePrivate(DxfThumbnailImageNode* ptr)
        : DxfNodePrivate(ptr, NT_ThumbnailImage)
    {}
};

DxfThumbnailImageNode::DxfThumbnailImageNode(int groupCode)
    : DxfNode(groupCode, "THUMBNAILIMAGE", new DxfThumbnailImageNodePrivate(this))
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

DxfCollectionNode::DxfCollectionNode(int groupCode, const QString& variable, DxfCollectionNodePrivate* ptr)
    : DxfNode(groupCode, variable, ptr)
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

DxfClassesNode::DxfClassesNode(int groupCode)
    : DxfCollectionNode(groupCode, "CLASSES", new DxfClassesNodePrivate(this))
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
            DxfClassNode* childNode = new DxfClassNode(group.groupCode);
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

QString DxfClassesNode::toString() const
{
    Q_D(const DxfClassesNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "CLASSES\n";
    stream << DxfNode::toString();
    for (DxfClassNode* node : *this)
    {
        stream << node->toString();
    }
    return buf;
}

class DxfTablesNodePrivate : public DxfCollectionNodePrivate
{
    Q_DECLARE_PUBLIC(DxfTablesNode)
public:
    DxfTablesNodePrivate(DxfTablesNode* ptr)
        : DxfCollectionNodePrivate(ptr, NT_Tables)
    {}
};

DxfTablesNode::DxfTablesNode(int groupCode)
    : DxfCollectionNode(groupCode, "TABLES", new DxfTablesNodePrivate(this))
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
            DxfTableNode* childNode = new DxfTableNode(group.groupCode);
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

QString DxfTablesNode::toString() const
{
    Q_D(const DxfTablesNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "TABLES\n";
    stream << DxfNode::toString();
    for (DxfTableNode* node : *this)
    {
        stream << node->toString();
    }
    return buf;
}

class DxfBlocksNodePrivate : public DxfCollectionNodePrivate
{
    Q_DECLARE_PUBLIC(DxfBlocksNode)
public:
    DxfBlocksNodePrivate(DxfBlocksNode* ptr)
        : DxfCollectionNodePrivate(ptr, NT_Blocks)
    {}
};

DxfBlocksNode::DxfBlocksNode(int groupCode)
    : DxfCollectionNode(groupCode, "BLOCKS", new DxfBlocksNodePrivate(this))
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
            DxfBlockNode* childNode = new DxfBlockNode(group.groupCode);
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
        else if (group.match(0, "ENDBLK"))
        {
            DxfEndBlockNode* childNode = new DxfEndBlockNode(group.groupCode);
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

QString DxfBlocksNode::toString() const
{
    Q_D(const DxfBlocksNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "BLOCKS\n";
    stream << DxfNode::toString();
    for (DxfBlockNode* node : *this)
    {
        stream << node->toString();
    }
    return buf;
}

class DxfEntitiesNodePrivate : public DxfCollectionNodePrivate
{
    Q_DECLARE_PUBLIC(DxfEntitiesNode)
public:
    DxfEntitiesNodePrivate(DxfEntitiesNode* ptr)
        : DxfCollectionNodePrivate(ptr, NT_Entities)
    {}
};

DxfEntitiesNode::DxfEntitiesNode(int groupCode)
    : DxfCollectionNode(groupCode, "ENTITIES", new DxfEntitiesNodePrivate(this))
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

        if (group.match(0, "ENDSEC"))
        {
            break;
        }
        else if (group.match(0, "LWPOLYLINE"))
        {
            DxfLWPolylineNode* childNode = new DxfLWPolylineNode(group.groupCode);
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
        else if (group.match(0, "CIRCLE"))
        {
            DxfCircleNode* childNode = new DxfCircleNode(group.groupCode);
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
        else if (group.match(0, "ELLIPSE"))
        {
            DxfEllipseNode* childNode = new DxfEllipseNode(group.groupCode);
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
        else if (group.match(0))
        {
            DxfEntityNode* childNode = new DxfEntityNode(group.groupCode, group.variable);
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
    }
    return true;
}

QString DxfEntitiesNode::toString() const
{
    Q_D(const DxfEntitiesNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "ENTITIES\n";
    stream << DxfNode::toString();
    for (DxfEntityNode* node : *this)
    {
        stream << node->toString();
    }
    return buf;
}

class DxfObjectsNodePrivate : public DxfCollectionNodePrivate
{
    Q_DECLARE_PUBLIC(DxfObjectsNode)
public:
    DxfObjectsNodePrivate(DxfObjectsNode* ptr)
        : DxfCollectionNodePrivate(ptr, NT_Objects)
    {}
};

DxfObjectsNode::DxfObjectsNode(int groupCode)
    : DxfCollectionNode(groupCode, "OBJECTS", new DxfObjectsNodePrivate(this))
{

}

bool DxfObjectsNode::parse(DxfStream* stream)
{
    Q_D(DxfObjectsNode);
    while (true)
    {
        DxfGroup group;
        if (!stream->readGroup(group))
        {
            return false;
        }

        if (group.match(0, "ENDSEC"))
        {
            break;
        }
        if (group.match(0))
        {
            DxfObjectNode* childNode = new DxfObjectNode(group.groupCode, group.variable);
            addChildNode(childNode);
            append(childNode);
            if (!childNode->parse(stream))
                return false;
        }
    }
    return true;
}

QString DxfObjectsNode::toString() const
{
    Q_D(const DxfObjectsNode);
    QString buf;
    QTextStream stream(&buf);
    stream << "OBJECTS\n";
    stream << DxfNode::toString();
    for (DxfObjectNode* node : *this)
    {
        stream << node->toString();
    }
    return buf;
}

