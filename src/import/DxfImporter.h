#ifndef DXFIMPORTER_H
#define DXFIMPORTER_H

#include "Importer.h"

class QTextStream;

class DxfNodePrivate;
class DxfNode
{
public:
    enum SectionType
    {
        ST_Section,
        ST_Classes,
        ST_Tables,
        ST_Blocks,
        ST_Entities,
        ST_Objects,
        ST_ThumbnailImage,
        ST_DrawingInterchangeFileFormats,
        ST_Custom
    };

    explicit DxfNode(int groupCode, const QString& variable);
    ~DxfNode();

    int groupCodes() const;
    QString variable() const;

    QList<DxfNode*>& children();
    DxfNode* addChildNode(DxfNode* node);

    SectionType sectionType() const;

    QVariantMap& values();
    void insertValue(const QString& key, const QVariant& value);
    bool contains(const QString& key) const;

    QString toString() const;

private:
    QScopedPointer<DxfNodePrivate> m_ptr;
    Q_DECLARE_PRIVATE_D(m_ptr, DxfNode)
    Q_DISABLE_COPY(DxfNode)
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