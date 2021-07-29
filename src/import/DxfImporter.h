#ifndef DXFIMPORTER_H
#define DXFIMPORTER_H

#include "Importer.h"
#include "QStack"

class QTextStream;

class DxfNodePrivate;
class DxfNode
{
public:
    enum NodeType
    {
        NT_Unknown,
        NT_Section,
        NT_KeyValue,
        NT_Collection
    };

    enum SectionType
    {
        ST_Section,
        ST_Header,
        ST_Classes,
        ST_Class,
        ST_Tables,
        ST_Blocks,
        ST_Entities,
        ST_Objects,
        ST_ThumbnailImage,
        ST_DrawingInterchangeFileFormats,
        ST_Custom
    };

    ~DxfNode();

    int groupCode() const;
    QString variable() const;

    DxfNode* parent() const;
    void setParent(DxfNode* node);

    QList<DxfNode*>& children();
    DxfNode* addChildNode(DxfNode* node);

    SectionType sectionType() const;
    NodeType nodeType() const;

    bool check(int groupCode, const QString& variable) const;
    bool check(int groupCode) const;
    bool check(const QString& variable) const;

    QString toString() const;

protected:
    explicit DxfNode(int groupCode, const QString& variable, DxfNodePrivate* privateData);
    QScopedPointer<DxfNodePrivate> m_ptr;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfNode)
        Q_DISABLE_COPY(DxfNode)

    friend class DxfSectionNode;
    friend class DxfCollectionNode;
    friend class DxfKeyValueNode;
};

class DxfSectionNodePrivate;
class DxfSectionNode : public DxfNode
{
public:
    DxfSectionNode(int groupCode, const QString& variable);
    ~DxfSectionNode();

    QMap<QString, DxfNode*>& propertyNodes();

protected:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfSectionNode)
    Q_DISABLE_COPY(DxfSectionNode)
};

class DxfCollectionNodePrivate;
class DxfCollectionNode : public DxfNode
{
public:
    DxfCollectionNode(int groupCode, const QString& variable);
    ~DxfCollectionNode();
};

class DxfKeyValueNodePrivate;
class DxfKeyValueNode : public DxfNode
{
public:
    DxfKeyValueNode(int groupCode, const QString& variable);
    ~DxfKeyValueNode();

protected:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfKeyValueNode)
    Q_DISABLE_COPY(DxfKeyValueNode)
};

QDebug operator<<(QDebug debug, const DxfNode& node);

class DxfImporterPrivate;
class DxfImporter : public Importer
{
    Q_OBJECT
public:
    explicit DxfImporter(QObject* parent = nullptr);
    virtual ~DxfImporter();

    virtual LaserDocument* import(const QString& filename, LaserScene* scene, const QVariantMap& params = QVariantMap());

protected:
    DxfNode* readLines(QTextStream* stream);

private:
    QScopedPointer<DxfImporterPrivate> m_ptr;
    Q_DECLARE_PRIVATE_D(m_ptr, DxfImporter)
    Q_DISABLE_COPY(DxfImporter)

    friend class Importer;
};

#endif // DXFIMPORTER_H